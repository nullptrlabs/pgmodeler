# Plano de Implementação — Suporte a Múltiplas Cenas (Multi-Scene Support)

> Branch: `multi-scene-support-2.1.x`. Este documento é o plano de trabalho de referência entre sessões/máquinas.
> Estado: análise concluída, decisões de design fechadas, implementação ainda não iniciada.
> Prioridade: fechar segmentação por schema (Fases 0-2) primeiro, ponta a ponta, antes de detalhar
> o modo por threshold (Fase 3) — ver seção 4.
> Atualização 2026-09-18: seções marcadas "achado de revisão de código" / "verificado em revisão
> de código" foram adicionadas após uma leitura profunda do código real envolvido na Fase 0 (ver
> arquivos citados nas próprias notas). Uma delas corrige uma afirmação da versão anterior deste
> documento sobre `connected_rels` (ver subseção "Relacionamentos cross-scene: promoção/
> rebaixamento dinâmico" na Fase 1).

## 1. Objetivo

Hoje o pgModeler gerencia um modelo inteiro com **uma única `QGraphicsScene` (`ObjectsScene`)**
renderizada por **uma única `QGraphicsView`**, ambas possuídas por `ModelWidget`
([modelwidget.h:126,129](libs/libgui/src/widgets/modelwidget.h#L126)). Em modelos com dezenas de
milhares de objetos isso fica pesado (seleção, bounding-rect, render, itens do Qt Graphics View
Framework todos escalando com o tamanho total do modelo).

O objetivo é segmentar um modelo em **múltiplas `ObjectsScene`**, escolhidas por um de dois modos:

- **Por schema**: ao detectar um novo schema sendo inserido no modelo (carga de arquivo ou edição
  manual), cria-se uma `ObjectsScene` exclusiva para aquele schema. Toda tabela/view/ftable/
  relacionamento pertencente ao schema é renderizado nela.
- **Por threshold**: a cada N objetos inseridos (tabelas, views e ftables contam 1 cada,
  independente do tipo; ex. N=1000), cria-se uma nova cena e as inserções seguintes vão para ela.

Este documento cobre **apenas o suporte de core/model** a múltiplas cenas. A UI **definitiva** de
troca entre cenas (dropdown estilizado, sidebar, abas, etc.) fica **fora de escopo** — será feita
separadamente pelo usuário (ver seção 7). A Fase 1 inclui, no entanto, um `QComboBox` simples de
troca de cena só para viabilizar teste manual durante o desenvolvimento (ver decisão #4, seção 3,
e subtarefa 1.3) — não deve ser confundido com a UI final.

## 2. Arquitetura atual (baseline, para contexto)

- `1 DatabaseModel : 1 ObjectsScene : 1 QGraphicsView`, todos amarrados em `ModelWidget`.
  `DatabaseModel` é puro dado, não conhece cena.
- Sincronização model → gráfico já passa por um choke point único:
  `DatabaseModel::s_objectAdded`/`s_objectRemoved` ([databasemodel.h:956,959](libs/libcore/src/databasemodel.h#L956))
  → `ModelWidget::handleObjectAddition`/`handleObjectRemoval`
  ([modelwidget.cpp:1043-1090, 1184-1199](libs/libgui/src/widgets/modelwidget.cpp#L1043))
  → `scene->addItem`/`removeItem` ([objectsscene.cpp:1004-1049, 1051-1078](libs/libcanvas/src/objectsscene.cpp#L1004)).
  **Este é o ponto de inserção natural da lógica de roteamento "para qual cena este objeto vai".**
- Ligação objeto-modelo → item-gráfico: ponteiro único `BaseGraphicObject::receiver_object`
  ([basegraphicobject.cpp:125-136](libs/libcore/src/basegraphicobject.cpp#L125)), lido via
  `getOverlyingObject()`. Não existe índice "objeto → cena"; dado um item gráfico já inserido,
  `QGraphicsItem::scene()` já resolve a cena dona — não é necessário criar um índice novo, só a
  lógica de **decisão** de bucket no momento da inserção.
- `SchemaView` (retângulo colorido do schema) calcula seu bounding box varrendo a posição de
  **todos** os filhos via `DatabaseModel::getObjects(type, schema)`
  ([schemaview.cpp:79-99, 175-230](libs/libcanvas/src/schemaview.cpp#L79)).
- Grid, cor de fundo/grid, delimitadores de página, layout de página, snap-to-grid: **static** na
  classe `ObjectsScene` ([objectsscene.h:59-143](libs/libcanvas/src/objectsscene.h#L59)) —
  compartilhado automaticamente entre todas as instâncias, sem trabalho extra.
- Zoom já vive na `QGraphicsView` (`viewport->scale()` em `ModelWidget::applyZoom`,
  [modelwidget.cpp:1015-1023](libs/libgui/src/widgets/modelwidget.cpp#L1015)), não na cena.
- Undo/redo (`Operation`/`OperationList`) não tem nenhuma referência a `QGraphicsScene`/
  `QGraphicsItem` — opera 100% sobre o modelo e só toca a cena indiretamente via o choke point
  `s_objectAdded`/`s_objectRemoved`. Nenhuma mudança necessária além de rotear esse choke point.
- Geometria de relacionamento (`RelationshipView::configureLine()`,
  [relationshipview.cpp:602-1287](libs/libcanvas/src/relationshipview.cpp#L602)) usa
  `mapFromItem`/`mapToItem` entre o item do relacionamento e os itens das duas tabelas — só bem
  definido quando ambos os itens estão na **mesma** `QGraphicsScene`.
- **Verificado em revisão de código (2026-09-18)**: `BaseTableView::connected_rels`
  ([basetableview.h:52](libs/libcanvas/src/basetableview.h#L52)) só é populado/limpo pelo próprio
  `RelationshipView`, via `addConnectedRelationship()`/`removeConnectedRelationship()` chamados em
  `configureLine()` e `disconnectTables()`
  ([relationshipview.cpp:581-584, 538-541](libs/libcanvas/src/relationshipview.cpp#L581)). Ou seja,
  hoje um relacionamento só entra em `connected_rels` quando já existe um `RelationshipView` para
  ele — não há nenhuma mistura de casos a desembaraçar. Isso é relevante para a Fase 1: ver nota na
  subseção "Relacionamentos cross-scene: promoção/rebaixamento dinâmico".
- **Verificado em revisão de código**: `ObjectsScene::removeItem()`
  ([objectsscene.cpp:1044-1072](libs/libcanvas/src/objectsscene.cpp#L1044)) não deleta o item
  imediatamente — apenas o desconecta, oculta e empilha em `removed_objs`. A deleção real só
  acontece no destrutor da própria `ObjectsScene`
  ([objectsscene.cpp:152-158](libs/libcanvas/src/objectsscene.cpp#L152)). Isso é favorável para a
  destruição de cena por remoção de schema (Fase 1): destruir a `ObjectsScene` do schema já libera
  corretamente qualquer item pendente em `removed_objs` daquela cena.

## 3. Decisões de design já fechadas

Estas decisões foram tomadas em conversa e **não devem ser reabertas** sem justificativa nova:

1. **Relacionamentos cross-scene**: usar conector "off-page" — nenhuma linha real cruzando cenas,
   e **nenhum `RelationshipView` (nem qualquer item gráfico de relacionamento) chega a ser criado**
   para um relacionamento cross-scene. Em vez disso, cada tabela ganha um marcador "cut-off"
   próprio (badge/ícone desenhado pelo próprio `BaseTableView`), indicando que existe um
   relacionamento com uma tabela em outra cena — clicável, no estilo dos conectores de página
   usados em ferramentas de ER/diagramação. Isso evita completamente a dependência de
   `mapFromItem`/`mapToItem` entre cenas distintas E evita ter que manter um item gráfico "meio
   órfão" representando um relacionamento sem as duas pontas presentes.
   **Refinamento importante**: essa decisão não é estática por relacionamento — é reavaliada toda
   vez que uma das duas tabelas muda de cena. Se as duas tabelas de um relacionamento acabam na
   mesma cena (ex. usuário move uma tabela para o schema/bucket da outra), o `RelationshipView`
   formal passa a ser criado normalmente naquela cena (comportamento idêntico ao de hoje), e os
   marcadores cut-off somem das duas tabelas. Se, ao contrário, uma tabela com relacionamento
   in-scene é movida para outra cena, o `RelationshipView` existente é destruído e os marcadores
   cut-off aparecem. Ou seja, "ter ou não um item gráfico formal" é uma propriedade **derivada e
   dinâmica** da posição atual das duas tabelas, não algo decidido de uma vez na criação do
   relacionamento. Ver detalhamento na Fase 1 ("Relacionamentos cross-scene: promoção/rebaixamento").
2. **Layers**: `DatabaseModel` continua sendo a fonte de persistência (serialização XML), como já
   é hoje. `LayersConfigWidget` continua sendo o ponto central de gerenciamento em runtime — ele já
   funciona como o funil único por onde toda mutação de layer passa
   (`addLayer`/`removeLayer`/`setActiveLayers`/`setLayerColors`/`setLayerRectsVisible`/
   `setLayerNamesVisible`, todos hoje chamando `model->scene->X(...)` diretamente — ver
   [layersconfigwidget.cpp:132,156,174,201,265-274,407-459](libs/libgui/src/widgets/layersconfigwidget.cpp#L132)).
   A mudança necessária é apenas fazer esses métodos aplicarem a chamada a **todas** as cenas do
   model, em vez de à única `model->scene`. O caminho model → cena (carga) já é análogo e
   centralizado em `ModelWidget::updateSceneLayers()` ([modelwidget.cpp:1980-1991](libs/libgui/src/widgets/modelwidget.cpp#L1980)),
   e o caminho cena → model (persistência) em `ModelWidget::updateModelLayersInfo()`
   ([modelwidget.cpp:5420-5431](libs/libgui/src/widgets/modelwidget.cpp#L5420)) — ambos precisam da
   mesma generalização "iterar por todas as cenas".
3. **Zoom/posição e cena focada**: o arquivo do modelo passa a persistir qual foi a **última cena
   focada** (novo atributo, análogo a `LastPosition`/`LastZoomFactor`, ver
   [databasemodel.cpp:3991-3996, 8000, 9458, 9760](libs/libcore/src/databasemodel.cpp#L3991)).
   Ao restaurar (`ModelWidget::restoreLastCanvasPosition()`,
   [modelwidget.cpp:1000](libs/libgui/src/widgets/modelwidget.cpp#L1000)), primeiro ativa-se essa
   cena, depois aplica-se zoom/scroll salvos — a restauração é sempre relativa à cena focada, não
   a "a cena" genérica.
4. **Sem UI final por enquanto, mas um controle mínimo de teste entra na Fase 1**: este plano
   cobre somente o core (DatabaseModel, ObjectsScene, ModelWidget internamente). Não construir a
   UI definitiva de troca de cena (dropdown estilizado/sidebar/abas) — isso continua a cargo do
   usuário em trabalho separado, e o core deve expor uma API pública limpa o suficiente para essa
   UI futura consumir (ver seção 7).
   **Atualização 2026-09-18**: para viabilizar teste manual da Fase 1 (principalmente
   relacionamentos cross-scene e troca de cena pelo usuário) sem esperar a UI definitiva, entra no
   escopo da Fase 1 um controle **propositalmente simples** — um `QComboBox` listando as cenas
   (por nome de schema) e chamando `setActiveScene()` ao selecionar. Ver subtarefa 1.3. Ele é o
   primeiro consumidor real da API mínima da seção 7 e serve de prova de conceito para ela, mas não
   é a UI final — pode ser descartado ou reaproveitado quando a UI definitiva for construída.
5. **Remoção de schema (inclusive em cascata) deve destruir a `ObjectsScene` correspondente**,
   apenas no modo por schema. Ver detalhamento na Fase 1 ("Remoção de schema e destruição de
   cena") e riscos associados na seção 6.
6. **Containers Qt, não std** (adicionado em 2026-09-18): todo código novo desta feature usa
   containers Qt (`QList`, `QHash`, `QMap`, `QStringList`, etc.) em vez de `std::vector`/`std::map`/
   etc., para não misturar as duas famílias de container na implementação nova. Só recorrer a um
   container std quando houver razão técnica concreta que o Qt não cubra (raro) — e nesse caso,
   registrar o motivo inline no código. Não é retroativo: código pré-existente que já usa std
   (ex. `ModelWidget::selected_objects`) não precisa ser convertido como parte deste trabalho.

## 4. Fases de implementação

> **Convenção de subtarefas (adicionada em 2026-09-18)**: cada fase abaixo é quebrada em
> subtarefas numeradas (`Fase.N`, ex. "0.3", "1.7"). A regra é **uma subtarefa = um commit**.
> Cada subtarefa deve, isoladamente, compilar e não regredir a suíte/os `.dbm` de exemplo — o
> objetivo é que, se uma delas causar uma regressão, dê para reverter (`git revert`) só aquele
> commit específico, sem precisar reverter a fase inteira. Numerar o commit na mensagem (ex.
> "Fase 1.6: rotear Relationship/BaseRelationship por cena das tabelas") facilita achar o commit
> certo para reverter depois. Onde uma subtarefa depende de outra, isso está anotado explicitamente
> — respeitar a ordem evita quebrar o build no meio do caminho. Subtarefas marcadas
> **[Requer fechamento de decisão com o usuário]** não devem ser codadas antes de revisitar a
> subseção correspondente (ver seção 6, último bullet).

### Fase 0 — Fundação (sem mudança de comportamento visível)

Objetivo: introduzir a pluralidade de cenas na estrutura interna enquanto o comportamento
observável permanece idêntico ao atual (efetivamente 1 cena). Isola o refactor mecânico
(alta superfície, baixo risco) do problema algorítmico difícil (particionamento real).

- Substituir o membro único `ObjectsScene *scene` em `ModelWidget` por uma coleção (ex.:
  `std::vector<ObjectsScene *> scenes` + índice/ponteiro da cena ativa). Manter
  `getObjectsScene()` retornando a cena ativa para não quebrar callers existentes; adicionar
  `getObjectsScene(int idx)` / `getObjectsScenes()` / `getSceneCount()`.
- Refatorar os call sites que hoje assumem uma cena só para iterar sobre a coleção e agregar
  resultado. Principais focos (ver relatório de pesquisa desta branch para lista exaustiva):
  - `ModelWidget::selectAllObjects()`, `scene->clearSelection()` (vários pontos em modelwidget.cpp),
    `scene->selectedItems()`, `scene->items().count()` (gate de "Select all").
  - `ObjectsScene::itemsBoundingRect()`/`alignObjectsToGrid()` — usados para fit-to-view, export,
    tamanho de página.
  - `ObjectSearchWidget` (busca de objeto) — hoje assume que a cena do objeto encontrado é a cena
    atual do `viewport` ([objectsearchwidget.cpp:344,352](libs/libgui/src/widgets/objectsearchwidget.cpp#L344)).
    O caso de multi-seleção nos resultados da busca tem risco próprio e mais profundo — ver
    subseção dedicada na Fase 1 ("Multi-seleção no Object Search Widget").
  - `ModelOverviewWidget` (minimap): hoje conecta diretamente em `model->scene`
    (`selectionChanged`, `sceneRectChanged`, [modeloverviewwidget.cpp:62-64](libs/libgui/src/widgets/modeloverviewwidget.cpp#L62))
    e renderiza com `model->scene->render(...)` ([modeloverviewwidget.cpp:144](libs/libgui/src/widgets/modeloverviewwidget.cpp#L144)).
    **Decisão**: o minimap segue mostrando só a cena ativa (não uma composição de todas) — ao
    trocar de cena, desconectar dos sinais da cena antiga, reconectar na nova
    (`model->getObjectsScene()` já retorna a ativa) e chamar `updateOverview()` imediatamente.
    Isso depende do sinal `s_activeSceneChanged` previsto na API mínima (seção 7); sem UI de troca
    ainda, este ponto só pode ser validado de fato a partir da Fase 1/exposição da API, mas a
    reconexão deve ser implementada como parte do suporte de core (não é UI nova, é adaptação de
    um widget existente).
  - Exportação/impressão (`ModelExportHelper` PNG/SVG, `ModelWidget::printModel`) — hoje fazem
    `scene->render(...)` assumindo uma cena cobre o modelo inteiro; precisam de render por cena +
    composição.
- Layers: generalizar `LayersConfigWidget` e `ModelWidget::updateSceneLayers()`/
  `updateModelLayersInfo()` conforme decisão #2 da seção 3.
  - **Achado de revisão de código (2026-09-18)**: `layers`, `active_layers` e `layers_paths` são
    membros de **instância** de `ObjectsScene`, não `static`
    ([objectsscene.h:71-77](libs/libcanvas/src/objectsscene.h#L71)) — ao contrário de grid/cor/
    página, que são de fato compartilhados de graça (ver riscos, seção 6). Isso significa que uma
    `ObjectsScene` nova nasce **sem nenhuma layer**. Não basta generalizar as chamadas de
    `LayersConfigWidget` para "iterar todas as cenas" daqui em diante — é preciso também
    **bootstrapar** a cena recém-criada (na Fase 1, ao criar a cena de um schema novo) com o
    conjunto atual de layers (nomes, ativas, cores, visibilidade) das cenas já existentes, senão
    ela diverge silenciosamente das demais.
- **Achado de revisão de código**: `DatabaseModel::getSceneRect()`/`setSceneRect()`
  ([databasemodel.h:898-899](libs/libcore/src/databasemodel.h#L898)) persistem um **único**
  `QRectF` para o modelo inteiro, consumido por
  `ModelWidget::adjustSceneRect(use_model_rect=true)` na carga do arquivo
  ([modelwidget.cpp:1946-1951](libs/libgui/src/widgets/modelwidget.cpp#L1946)). Com N cenas por
  schema esse único retângulo persistido fica ambíguo — precisa virar por-cena (provavelmente
  indexado pela mesma chave que `LastScene`, Fase 2). Não estava coberto pela Fase 2 original;
  adicionar ao escopo de persistência daquela fase.
- **Achado de revisão de código**: `ModelWidget::eventFilter()` identifica eventos de mouse da
  cena comparando `object == scene` ([modelwidget.cpp:825](libs/libgui/src/widgets/modelwidget.cpp#L825)),
  usado para panning, magnifier e emissão de `s_sceneInteracted`. Com múltiplas cenas registradas
  via `installEventFilter(this)`, essa comparação precisa virar "object é alguma cena da coleção"
  (na prática, deveria ser sempre a cena ativa, já que só a cena anexada ao `viewport` recebe
  eventos de mouse reais do usuário — mas o código não deve depender dessa suposição implícita sem
  verificar).
- Preparar (mas não necessariamente popular ainda) os novos atributos de persistência que as
  Fases 1-2 vão usar (modo de segmentação, cena focada) para estabilizar o formato de arquivo
  desde já. Reservar espaço no formato/enum para o modo threshold (Fase 3, ainda não detalhada),
  sem implementar nada em cima disso agora.
- **Critério de aceite da Fase 0**: suíte de regressão / abertura dos `.dbm` de exemplo do repo
  não deve mudar nada visualmente ou funcionalmente — é puramente estrutural.

#### Subtarefas da Fase 0 (uma por commit)

0.1. **Infra da coleção de cenas** — trocar o membro único `ObjectsScene *scene` em `ModelWidget`
     por `QList<ObjectsScene *> scenes` + índice da cena ativa (containers Qt, não std — ver
     seção 3). `scene` continua existindo como ponteiro de conveniência para a cena ativa (mantido
     em sincronia internamente), então nenhum dos 82 call sites que hoje usam `scene->...` em
     `modelwidget.cpp` precisa mudar neste commit. Adicionar a API nova: `getObjectsScene(int idx)`,
     `getObjectsScenes()`, `getSceneCount()`, `getSceneForObject(BaseObject *)`. A criação da cena
     em si fica isolada num método privado `ModelWidget::addScene()` (cria a `ObjectsScene`,
     instala o event filter, adiciona a `scenes`, e só ativa/atribui a `scene` se for a primeira —
     para que chamadas futuras da Fase 1, ao criar uma cena por schema, não roubem o foco da cena
     em que o usuário já está trabalhando), reutilizado pelo construtor. Sem mudança de
     comportamento observável — só 1 cena existe em runtime ainda. Depende de: nenhuma.

0.2. **`setActiveScene()` + `s_activeSceneChanged`** — implementar
     `ModelWidget::setActiveScene(int idx)` (troca `viewport->setScene(...)`, reaplica
     `current_zoom` atual) e o sinal `s_activeSceneChanged(int)` (API mínima, seção 7). Com 1 única
     cena em runtime, `setActiveScene(0)` é a única chamada possível — serve para validar a
     mecânica antes de ter efeito visível de verdade. Depende de: 0.1.

0.3. **Fix do `eventFilter()`** — trocar a comparação `object == scene`
     ([modelwidget.cpp:825](libs/libgui/src/widgets/modelwidget.cpp#L825)) por uma checagem de
     pertencimento à coleção de cenas. Isolado, baixo risco. Depende de: 0.1.

0.4. **`ModelOverviewWidget` reage a `s_activeSceneChanged`** — novo método privado
     `connectToActiveScene()` ([modeloverviewwidget.cpp](libs/libgui/src/widgets/modeloverviewwidget.cpp)),
     único ponto responsável por desconectar da cena anterior (rastreada no novo membro
     `connected_scene`, já que por `s_activeSceneChanged` disparar depois que
     `ModelWidget::scene` já foi reatribuído, não dá para descobrir a cena antiga a partir do
     model) e reconectar `selectionChanged`/`sceneRectChanged` na cena ativa atual, chamando
     `updateOverview(true)` ao final. Chamado tanto em `show()` (conexão inicial) quanto via
     `connect(model, &ModelWidget::s_activeSceneChanged, this, &ModelOverviewWidget::connectToActiveScene)`.
     **Pegadinha encontrada em teste manual (F10 logo após abrir um modelo)**: `connectToActiveScene()`
     precisa ser chamado **depois** de `resizeOverview()` em `show()`, nunca antes — é
     `resizeOverview()` quem calcula `pixmap_size`, e chamar `updateOverview(true)` (dentro de
     `connectToActiveScene()`) antes disso pinta num `QPixmap` de tamanho inválido (overview vazio
     + warning `QPainter::begin: Paint device returned engine == 0`). Com 1 cena só o sinal nunca
     dispara de verdade ainda, mas fecha o contrato antes da Fase 1 popular múltiplas cenas.
     Depende de: 0.2.

0.5. **Agregação de seleção e bounding rect entre cenas** — `ModelWidget::selectAllObjects()`
     ("select all" via `scene->setSelectionArea(scene->sceneRect())`), `itemsBoundingRect()` (usado
     em `emitSceneInteracted()`/`s_sceneInteracted`) e o gate `scene->items().count() > 1`
     ([modelwidget.cpp:4713](libs/libgui/src/widgets/modelwidget.cpp#L4713)) passam a iterar
     `getObjectsScenes()` e agregar o resultado. Com 1 cena, resultado idêntico ao atual. Depende
     de: 0.1.

0.6. **Generalização de Layers** — `LayersConfigWidget` (12 call sites) e
     `ModelWidget::updateSceneLayers()`/`updateModelLayersInfo()` passam a aplicar mutações a
     **todas** as cenas da coleção (decisão #2, seção 3); leituras continuam vindo só da cena
     ativa. Incluir também o helper de bootstrap (copiar layers — nomes, ativas, cores,
     visibilidade — de uma cena existente para uma cena recém-criada) que a Fase 1 vai chamar ao
     criar a `ObjectsScene` de um schema novo, já que `layers`/`active_layers`/`layers_paths` são
     membros de **instância** de `ObjectsScene` (achado de revisão de código, seção 2). Depende
     de: 0.1.

0.7. **Exportação/impressão por cena + composição** — `ModelWidget::printModel()` e
     `ModelExportHelper` (PNG/SVG) passam a iterar `getObjectsScenes()`, renderizando cada cena e
     compondo o resultado, em vez de assumir uma única `scene->render(...)`. Com 1 cena, a saída
     deve ser byte-idêntica à atual — este é o subtask de maior risco mecânico da fase; o caso de
     N>1 cenas só é testável de verdade depois da Fase 1 popular a coleção, então revisitar este
     commit com `/home/raphael/dbm_models/big_model.dbm` quando a Fase 1 estiver pronta. Depende
     de: 0.1.

0.8. **Scaffolding de persistência** — reservar (sem popular/usar ainda) no formato XML e no enum
     de `DatabaseModel`: modo de segmentação (nenhum/schema/threshold) e o campo `SceneRect` por
     cena (troca do único `QRectF` hoje retornado por `getSceneRect()`/`setSceneRect()`,
     [databasemodel.h:898-899](libs/libcore/src/databasemodel.h#L898)). `LastScene` fica para a
     Fase 2 (subtarefa 2.1). Depende de: nenhuma (independente do resto da fase).

> **Gate de aceite da fase (não é commit de código)**: depois de 0.1-0.8, rodar a suíte de
> regressão e abrir os `.dbm` de exemplo do repo — nada deve mudar visual ou funcionalmente, já
> que ainda existe efetivamente 1 cena por modelo em runtime.

### Fase 1 — Segmentação por Schema + conector cross-scene

- Chave de bucket: 1 `Schema` = 1 `ObjectsScene`.
- Em `ModelWidget::handleObjectAddition()` ([modelwidget.cpp:1043-1090](libs/libgui/src/widgets/modelwidget.cpp#L1043)):
  - Ao inserir um `Schema`, criar a `ObjectsScene` correspondente se ainda não existir (cobre tanto
    carga de arquivo quanto criação manual pelo operador — ambos os fluxos já passam por este
    mesmo choke point via `DatabaseModel::s_objectAdded`).
  - Ao inserir `Table`/`View`/`ForeignTable`, resolver `getSchema()` do objeto e rotear
    `scene->addItem()` para a `ObjectsScene` daquele schema.
  - Ao inserir `Relationship`/`BaseRelationship`: resolver a cena de cada uma das duas tabelas
    envolvidas. Se coincidem, criar o `RelationshipView` normalmente (comportamento atual,
    inalterado) e adicioná-lo àquela cena compartilhada. **Se são cenas diferentes, não criar
    nenhum `RelationshipView`** — `BaseGraphicObject::getOverlyingObject()` desse relacionamento
    fica `nullptr` — e em vez disso registrar o relacionamento na lista de "conexões externas" de
    cada uma das duas tabelas, para que cada `BaseTableView` desenhe seu próprio marcador cut-off.

#### Relacionamentos cross-scene: promoção/rebaixamento dinâmico

"Ter ou não um `RelationshipView`" não é decidido só na criação do relacionamento — é reavaliado
toda vez que uma das duas tabelas muda de cena (refinamento da decisão #1, seção 3):

- `ModelWidget::moveToSchema()` (mudança de schema de uma tabela via menu) e qualquer outro ponto
  que reatribua a cena de uma tabela precisam, para cada relacionamento em
  `BaseTableView::connected_rels` ([basetableview.cpp:304-320](libs/libcanvas/src/basetableview.cpp#L304))
  da tabela movida, reavaliar se o parceiro está na mesma cena após a mudança:
  - **Promoção** (estava cross-scene, virou in-scene): criar o `RelationshipView` formal na cena
    compartilhada, configurar `receiver_object`, remover o marcador cut-off das duas tabelas.
  - **Rebaixamento** (estava in-scene, virou cross-scene): destruir o `RelationshipView`
    (`scene->removeItem()` + limpar `receiver_object`), criar/atualizar o marcador cut-off nas
    duas tabelas.
- `BaseTableView` precisa de um novo estado: lista de relacionamentos "externos" (sem
  `RelationshipView`), **adicional** a `connected_rels`. **Correção pós-revisão de código
  (2026-09-18)**: a redação original desta subseção dizia que `connected_rels` hoje "mistura os
  dois casos sem distinção" — isso não corresponde ao código. `addConnectedRelationship()`/
  `removeConnectedRelationship()` só são chamados de dentro do próprio `RelationshipView`
  (`configureLine()`/`disconnectTables()`,
  [relationshipview.cpp:581-584, 538-541](libs/libcanvas/src/relationshipview.cpp#L581)), então
  hoje `connected_rels` só contém relacionamentos que **já têm** view gráfica — não há mistura
  porque só existe um caso. Consequência prática: depois da Fase 1, um relacionamento cross-scene
  simplesmente **nunca entra** em `connected_rels` (nenhum código a mais precisa filtrá-lo daí). A
  lista nova de conexões externas é puramente **aditiva**, não uma separação de dados existentes —
  o que simplifica esta parte da fase. Isso também confirma que
  `BaseTableView::selectRelationships()` ([basetableview.cpp:634-638](libs/libcanvas/src/basetableview.cpp#L634))
  não vai precisar de nenhum null-check novo: ele itera só `connected_rels`, que nunca vai conter
  um relacionamento sem `getOverlyingObject()`.
  Falta ainda: lógica de desenho para o(s) marcador(es) cut-off, e a política de agrupamento visual
  quando uma tabela tem várias conexões externas (um marcador por relacionamento vs. um marcador
  único "N relacionamentos externos" com detalhamento ao clicar/passar o mouse) — decisão de UX que
  pode ficar para quando a UI for desenhada, mas a estrutura de dados subjacente (lista, não um
  único ponteiro) precisa já prever múltiplas conexões externas por tabela.
- O clique no marcador cut-off é o que dispara a navegação (core expõe o sinal/dado necessário —
  ex. "usuário clicou no marcador da tabela X, o alvo é este `BaseObject*` nesta `ObjectsScene*`";
  a ação de efetivamente trocar a view fica para a UI futura, decisão #4).
- **Atenção**: qualquer código que hoje assume `graph_obj->getOverlyingObject() != nullptr` para
  um `Relationship`/`BaseRelationship` já inserido no modelo passa a ter uma exceção legítima
  (relacionamento cross-scene sem view formal) — auditar os usos existentes de
  `getOverlyingObject()` sobre relacionamentos ao implementar esta fase, não assumir que só
  precisa de null-check nos pontos novos.
- Revisar `SchemaView::fetchChildren()`/cálculo de bounding box
  ([schemaview.cpp:79-99, 175-230](libs/libcanvas/src/schemaview.cpp#L79)) — neste modo, schema e
  cena são 1:1 por construção, então a lógica atual deve continuar válida sem alteração de fundo,
  só confirmar que os itens retornados por `getObjects(type, schema)` estão sempre na mesma cena
  do próprio `SchemaView` (deve ser garantido pela própria invariante do modo).

#### Remoção de schema e destruição de cena (incluindo cascade delete)

Um schema pode ser removido sozinho (só se estiver vazio — remoção de schema com filhos sem
cascade já é rejeitada hoje pela validação de referências) ou em modo cascade
(`ModelWidget::removeObjects(true)`, [modelwidget.cpp:3542](libs/libgui/src/widgets/modelwidget.cpp#L3542)),
que remove o schema e tudo que referencia/pertence a ele. No modo por schema, a `ObjectsScene`
daquele schema precisa ser destruída quando o schema for removido.

- **Ordem de remoção em cascade já é favorável**: `removeObjects(true)` monta `objs_map`
  (`std::map<unsigned /*object_id*/, ...>`) com o schema e todos os objetos que o referenciam
  (direta e indiretamente), e processa a remoção **em ordem decrescente de object_id**
  (`ritr=objs_map.rbegin()`, [modelwidget.cpp:3684-3685](libs/libgui/src/widgets/modelwidget.cpp#L3684)).
  Como um schema só pode ser criado antes dos objetos que ele contém (ids menores), essa ordem
  decrescente remove **filhos primeiro, schema por último** — ou seja, quando a remoção do próprio
  `Schema` chega em `ModelWidget::handleObjectRemoval()`, todas as tabelas/views/relacionamentos
  que pertenciam a ele já foram individualmente removidos de sua cena (cada um já passou pelo
  choke point de remoção). Isso significa que, na prática, a cena do schema **já deveria estar
  vazia** no momento em que a destruímos — não é necessário nenhum "remove em lote" especial, só
  destruir a `ObjectsScene` vazia quando o objeto `Schema` correspondente for removido.
- Implementação: em `ModelWidget::handleObjectRemoval()`
  ([modelwidget.cpp:1184-1199](libs/libgui/src/widgets/modelwidget.cpp#L1184)), ao identificar que
  o objeto removido é um `Schema`, localizar a `ObjectsScene` associada e destruí-la (após o
  `scene->removeItem()` do próprio `SchemaView`). Adicionar uma asserção/log de aviso em builds de
  debug se a cena não estiver vazia nesse ponto — sinal de que algum objeto ficou órfão (bug a
  investigar, não um caso esperado).
- **Cena ativa sendo destruída**: se a `ObjectsScene` destruída for a que está atualmente presa ao
  `viewport` (cena "focada"), é preciso trocar o `viewport` para outra cena existente (ex. a
  primeira disponível) **antes** de destruir o objeto `ObjectsScene`, para nunca deixar
  `QGraphicsView` apontando para um ponteiro liberado. Mesmo cuidado vale para o atributo
  persistido `LastScene` (Fase 2): se ele referenciar um schema que não existe mais na próxima
  carga do arquivo, cair para um fallback (primeira cena / cena "Default").
- **Marcadores cut-off apontando para dentro do schema removido**: como (após o refinamento da
  decisão #1) um relacionamento cross-scene não tem `RelationshipView` nenhum — só marcadores nas
  duas tabelas — a remoção padrão via
  `scene->removeItem(dynamic_cast<QGraphicsItem*>(graph_obj->getOverlyingObject()))`
  ([modelwidget.cpp:1190](libs/libgui/src/widgets/modelwidget.cpp#L1190)) não se aplica a esses
  relacionamentos (`getOverlyingObject()` já é `nullptr` para eles). O que precisa acontecer:
  quando o relacionamento em si é removido (cascade ou não), remover sua entrada da lista de
  conexões externas de **ambas** as tabelas para que os marcadores cut-off desapareçam; quando é
  uma das duas *tabelas* que é removida (ex. schema inteiro em cascata), a tabela sobrevivente
  também precisa ter sua entrada removida — na ordem de remoção em cascata (filhos antes do
  schema, ver item acima), isso deveria acontecer naturalmente quando o próprio relacionamento for
  removido do `objs_map`, mas vale confirmar com teste que a tabela sobrevivente nunca fica com um
  marcador "fantasma" apontando para um objeto já removido.
- **Undo de cascade delete**: `OperationList` desfaz operações em ordem LIFO
  (`OperationList::undoOperation()`, [operationlist.cpp:570](libs/libcore/src/operationlist.cpp#L570),
  consome `operations[current_index-1]` e decrementa). Como as remoções foram registradas na mesma
  ordem em que foram executadas (filhos primeiro, schema por último —
  [modelwidget.cpp:3766](libs/libgui/src/widgets/modelwidget.cpp#L3766) chama
  `op_list->registerObject(...)` dentro do mesmo loop de remoção), desfazer a pilha remonta o
  **schema primeiro** (recriando a `ObjectsScene`) e só depois os filhos (que caem de volta na
  cena recém-recriada pelo choke point normal de inserção). Essa ordem é favorável e não deveria
  exigir tratamento especial — mas **precisa de teste de regressão dedicado** (cascade delete de
  schema + undo completo) antes de fechar a Fase 1, já que a conclusão acima é baseada na leitura
  do código de consumo da pilha, não em execução observada.

#### Ferramenta de criação de relacionamento (caso cross-scene) — PRECISA DE ESTUDO DEDICADO

> Sinalizado explicitamente pelo usuário como um ponto que exige cuidado extra antes de
> implementar. O que segue é o mapeamento do mecanismo atual e uma direção possível — **não é
> uma decisão fechada** como as da seção 3. Antes de implementar a Fase 1, revisitar esta
> subseção com o usuário e fechar o desenho de interação.

**Mecanismo atual (single-scene)**: a criação interativa de relacionamento é dirigida quase
inteiramente por `ModelWidget::configureObjectSelection()`
([modelwidget.cpp:1393-1469](libs/libgui/src/widgets/modelwidget.cpp#L1393)), que a cada mudança de
seleção lê `scene->selectedItems()` (a **cena única**, hard-coded) e decide o que fazer:

1. Usuário aciona uma ação de "novo relacionamento" → `ModelWidget::addNewObject()`
   ([modelwidget.cpp:1092-1182](libs/libgui/src/widgets/modelwidget.cpp#L1092)) arma
   `new_obj_type` com o tipo de relacionamento e muda o cursor.
2. Usuário clica na tabela A (origem) → `scene->selectedItems().size()==1` →
   `configureObjectSelection()` chama `scene->showRelationshipLine(true, centro_da_tabela_A)`
   ([modelwidget.cpp:1445-1447](libs/libgui/src/widgets/modelwidget.cpp#L1445)), que ativa
   `rel_line` — um `QGraphicsLineItem` **membro da própria `ObjectsScene`**, criado no construtor
   dela ([objectsscene.cpp:75-81](libs/libcanvas/src/objectsscene.cpp#L75)) — desenhando uma linha
   de borracha da tabela A até o cursor.
3. Enquanto `rel_line->isVisible()`, `ObjectsScene::mousePressEvent()` força
   `Qt::ControlModifier` no evento ([objectsscene.cpp:1126-1127](libs/libcanvas/src/objectsscene.cpp#L1126))
   para que o próximo clique **adicione** a tabela B à seleção em vez de substituí-la.
4. Com as duas tabelas selecionadas na mesma cena, `configureObjectSelection()` roda de novo,
   detecta `count==2` ([modelwidget.cpp:1457-1458](libs/libgui/src/widgets/modelwidget.cpp#L1457))
   e chama `showObjectForm(new_obj_type)` diretamente, finalizando a criação.

**Por que isso não funciona entre cenas diferentes**: `rel_line` é um item gráfico local a UMA
`ObjectsScene` — não pode ser "arrastado visualmente" de uma cena para outra, porque cada cena
só recebe eventos de mouse do viewport enquanto está anexada a ele. E mais grave:
`configureObjectSelection()` conta objetos selecionados lendo **apenas a cena atualmente ativa**
— uma tabela selecionada numa cena que deixou de estar ativa simplesmente não aparece em
`scene->selectedItems()` da cena nova, então hoje não há nenhum jeito do estado "tabela A já
escolhida como origem" sobreviver a uma troca de cena.

**Direção possível (a validar)**: desacoplar a contagem de "quantas tabelas foram escolhidas
para o relacionamento" da leitura direta de `scene->selectedItems()`, substituindo por um estado
explícito em `ModelWidget` (não em `ObjectsScene`), algo como um `BaseTable *pending_rel_src`
que **não** é derivado da seleção do Qt e portanto sobrevive a uma troca de cena:

- Clique na tabela A → grava `pending_rel_src`, mostra `rel_line` normalmente **se** a cena da
  tabela A for a cena ativa (comportamento atual preservado 1:1 para o caso same-scene).
- Se o usuário trocar de cena com `pending_rel_src` setado, a linha de borracha não pode mais
  aparecer (pertence à cena antiga) — o core precisa expor esse estado
  (`ModelWidget::hasPendingRelationshipSource()` / `getPendingRelationshipSource()`) para que
  alguma UI futura sinalize "relacionamento pendente, origem: `public.pedidos`" de algum jeito
  (mensagem, cursor diferente, badge na tabela de origem se ela ainda estiver visível em outro
  local) — a forma exata de comunicar isso ao usuário é decisão de UX, fora do escopo deste plano
  de core, mas o **hook** de estado precisa existir.
- Clique na tabela B numa cena diferente da tabela A finaliza a criação usando
  `pending_rel_src` + tabela B — cai direto no caminho de conector off-page (decisão #1) em vez
  de um `RelationshipView` único.
- Cancelamento (`ModelWidget::cancelObjectAddition()`, Esc, clique fora, etc.) precisa limpar
  `pending_rel_src` do mesmo jeito que hoje limpa a seleção/linha da cena única.

**Pergunta em aberto para a próxima sessão**: vale a pena permitir a criação cross-scene via essa
interação em duas etapas (clica A, troca de cena, clica B), ou é aceitável exigir que o usuário
crie relacionamentos cross-scene por outro caminho (ex. formulário de relacionamento aberto
manualmente, escolhendo origem/destino de uma lista, sem depender de clique-e-arrasto no canvas)?
A segunda opção é bem mais simples de implementar e evita todo esse desenho de estado
pendente — vale decidir isso explicitamente antes de codar a Fase 1.

#### Seleção é sempre local à cena ativa

Levantado junto com o ponto acima: hoje `ModelWidget::selected_objects` é um cache atualizado por
`configureObjectSelection()` toda vez que `scene->selectionChanged()` dispara, sempre refletindo
`scene->selectedItems()` da única cena existente. Cada `ObjectsScene` já tem, de graça, seu
próprio estado de seleção isolado (a flag `QGraphicsItem::isSelected()` só existe nos itens
daquela cena) — o problema é que o Qt **não limpa automaticamente** a seleção de uma cena quando
ela deixa de estar anexada ao `viewport`. Sem tratamento, um usuário poderia selecionar objetos na
cena A, trocar para a cena B, selecionar mais objetos lá, e uma operação em lote generalizada pela
Fase 0 ("iterar por todas as cenas e agregar", usada por remoção múltipla, copy/paste, set layer
em lote, etc.) passaria a incluir também os objetos "esquecidos" selecionados na cena A —
invisíveis no momento, mas afetados pela operação. Isso é perigoso (ex.: usuário acha que está
removendo só o que vê na cena B, mas também remove algo que tinha selecionado na cena A minutos
antes e esqueceu).

- **Decisão recomendada** (conforme apontado pelo usuário): `ModelWidget::setActiveScene(int idx)`
  (API mínima, seção 7) deve chamar `clearSelection()` na cena que está sendo desativada (ou em
  todas as cenas, por simplicidade/segurança) como parte da troca. Isso já dispara o
  `selectionChanged()` existente → `configureObjectSelection()` → `selected_objects` esvazia
  naturalmente, sem precisar de nenhum código novo de sincronização.
- **Tensão a resolver**: algumas ações de seleção semântica existentes seguem relacionamentos e
  podem, no caso cross-scene, precisar tocar objetos fora da cena ativa —
  `ModelWidget::selectTableRelationships()` (seleciona relacionamentos conectados a uma tabela;
  no caso cross-scene, um desses relacionamentos "pertence" à outra cena) é o exemplo mais claro.
  `selectSchemaChildren()` fica automaticamente seguro no modo por schema (schema == cena, por
  construção), mas `selectTaggedTables()` pode span múltiplas cenas livremente (tags não são
  amarradas a schema/bucket). **Ainda não decidido**: essas ações (a) só selecionam a parte
  alcançável na cena ativa e sinalizam visualmente o que ficou de fora (via o mesmo conector
  off-page), ou (b) têm uma exceção explícita à regra "seleção só na cena ativa" para casos de
  seleção programática/semântica. Precisa da mesma revisão dedicada do item anterior antes da
  Fase 1.

#### Multi-seleção no Object Search Widget — PRECISA DE ESTUDO DEDICADO

> Sinalizado explicitamente pelo usuário. A busca de objetos é, por natureza, uma operação
> **de modelo inteiro** (procura em todas as tabelas/views/relacionamentos/schemas do
> `DatabaseModel`, sem noção de cena) — isso coloca essa feature em tensão direta com a regra
> "seleção só na cena ativa" da subseção anterior, de um jeito diferente dos outros dois pontos
> desta fase. Aqui o cross-scene não é uma exceção rara, é o caso normal de uso.

**Mecanismo atual**: `ObjectSearchWidget` mantém **duas** listas de seleção paralelas,
independentes da seleção nativa do Qt:

- `selected_obj` / `ObjectSearchWidget::selectObject()`
  ([objectsearchwidget.cpp:291-358](libs/libgui/src/widgets/objectsearchwidget.cpp#L291)): reage à
  seleção na lista de resultados (`result_view`). Primeiro faz
  `model_wgt->scene->clearSelection()` (cena única, hard-coded,
  [objectsearchwidget.cpp:295](libs/libgui/src/widgets/objectsearchwidget.cpp#L295)), monta seu
  próprio `selected_objs` (vetor local, não é `ModelWidget::selected_objects`), e para **cada**
  objeto selecionado na lista chama `model_wgt->viewport->centerOn(obj_view)` seguido de
  `obj_view->setSelected(true)` ([objectsearchwidget.cpp:344-346, 352](libs/libgui/src/widgets/objectsearchwidget.cpp#L344)).
  Se o usuário multi-seleciona 5 linhas da lista de resultados espalhadas em 3 cenas diferentes,
  hoje isso chamaria `centerOn()` 5 vezes seguidas — sem nenhum `viewport->setScene()` entre elas
  — e marcaria `isSelected(true)` em itens de 3 `QGraphicsScene` diferentes simultaneamente.
- `ObjectSearchWidget::selectObjects()` ([objectsearchwidget.cpp:168-218](libs/libgui/src/widgets/objectsearchwidget.cpp#L168))
  — a ação de menu "selecionar todos os encontrados" (ou o complemento, "selecionar os não
  encontrados") — itera por **todos** os objetos do tipo Table/View/Textbox/Relationship/
  BaseRelationship/Schema do `DatabaseModel` inteiro, chama `obj_view->setSelected(true)` em cada
  um, e ao final chama `model_wgt->configureObjectSelection()`
  ([objectsearchwidget.cpp:217](libs/libgui/src/widgets/objectsearchwidget.cpp#L217)) — ou seja,
  **esta** via alimenta o `ModelWidget::selected_objects` normal, reabrindo exatamente a mesma
  questão da subseção anterior (bulk-ops agregando por todas as cenas via Fase 0).

**Por que isso é diferente dos outros dois pontos desta fase**: no caso da ferramenta de
relacionamento e do clique manual no canvas, cross-scene é a exceção que precisa de tratamento
especial. Aqui, "selecionar todos os objetos que batem com o filtro de busca no modelo inteiro" é
**a proposta de valor da feature** — restringir a busca à cena ativa por padrão de segurança
anularia boa parte da utilidade dela (ex.: fade-out em massa de todas as tabelas com um
determinado prefixo, espalhadas pelo modelo inteiro).

**Opções levantadas pelo usuário, sem decisão fechada ainda**:

1. **Permitir livremente** — seleção lógica (`selected_objs`, ações do próprio widget: editar,
   fade, popup menu, que operam sobre `BaseObject*` do modelo e não dependem de estar todos na
   mesma cena) continua cross-scene sem restrição. Only a materialização visual no canvas
   (`setSelected`/`centerOn`) precisa de uma regra: por exemplo, focar automaticamente a cena do
   primeiro objeto da seleção, selecionar visualmente só os objetos que pertencem a ela, e para os
   demais manter apenas a marca lógica em `selected_objs` (sem tocar `isSelected()` de itens fora
   da cena focada) — evita a "seleção fantasma" acumulando em cenas fora de vista.
2. **Restringir** — multi-seleção nos resultados só permitida entre objetos da mesma cena;
   selecionar um objeto de outra cena junto com uma seleção existente é bloqueado ou substitui a
   seleção anterior. Mais simples e mais seguro, mas quebra o caso de uso "operar em lote no
   modelo inteiro" que hoje já existe (`selectObjects()`/"select all found").
3. **Híbrida** — a busca e a seleção lógica continuam sempre cross-scene (nenhuma restrição na
   ação de encontrar/marcar), mas ações que dependem de estado visual do canvas (abrir formulário
   que centraliza a view, popup-menu no canvas) escolhem uma cena-alvo (ex. a do primeiro objeto)
   e ignoram/avisam sobre os demais só para fins de destaque visual; ações "de dados" (fade,
   set layer, set tag, remoção) continuam operando sobre a lista lógica completa, sem exigir mesma
   cena.

Recomendação preliminar (a confirmar com o usuário antes da Fase 1): opção 3, por preservar o
propósito original da feature (operar em lote no modelo inteiro) sem violar a regra de segurança
de seleção-visual-por-cena fechada na subseção anterior. Mas, assim como os outros dois pontos
desta fase, **não implementar antes de revisitar esta escolha com o usuário**.

#### Subtarefas da Fase 1 (uma por commit)

1.1. **Criar `ObjectsScene` ao inserir `Schema`** — em `handleObjectAddition()`, ao detectar um
     `Schema` sendo inserido, criar a `ObjectsScene` correspondente (indexada por schema) se ainda
     não existir, e bootstrapar layers a partir de uma cena existente (helper da subtarefa 0.6).
     Depende de: 0.1, 0.6 (nenhuma dependência dentro da própria Fase 1).

1.2. **Rotear Table/View/ForeignTable para a cena do schema** — resolver `getSchema()` do objeto
     inserido e chamar `addItem()` na `ObjectsScene` daquele schema em vez da cena única. Depende
     de: 1.1.

1.3. **Controle simples de troca de cena (`QComboBox`)** — adicionado a pedido do usuário
     (2026-09-18) para viabilizar teste manual do resto da fase sem esperar a UI definitiva
     (ver decisão #4, seção 3). Um `QComboBox` na UI de `ModelWidget` listando as cenas existentes
     por nome de schema (populado a partir de `getObjectsScenes()`/`getSceneForObject()`,
     atualizado quando cenas são criadas/destruídas — 1.1, 1.10), chamando `setActiveScene(idx)`
     (0.2) ao selecionar. Propositalmente simples — sem estilização, sem sincronizar com o
     `s_sceneDestroyed` da seção 7 além do mínimo para não referenciar uma cena morta. É o
     primeiro consumidor real da API mínima da seção 7, e passa a ser a ferramenta usada para
     testar manualmente 1.5/1.6 (roteamento e auditoria de relacionamento cross-scene), 1.7
     (marcador cut-off), 1.8 (promoção/rebaixamento ao mover tabela de schema) e as subtarefas de
     decisão 1.12/1.13/1.14 — sem ele, testar troca de cena exigiria código de teste descartável
     em cada uma dessas subtarefas. Depende de: 0.1, 0.2, 1.2.

1.4. **Confirmação de `SchemaView::fetchChildren()`** — validar com teste (não necessariamente
     mudança de código) que `getObjects(type, schema)`
     ([schemaview.cpp:79-99, 175-230](libs/libcanvas/src/schemaview.cpp#L79)) sempre retorna itens
     da mesma cena do próprio `SchemaView` no modo por schema. Commit pequeno/de teste, separado
     para não esconder essa verificação dentro do commit de feature 1.2. Depende de: 1.2.

1.5. **Estrutura de dados de "conexões externas" em `BaseTableView`** — nova lista (adicional a
     `connected_rels`, que — conforme achado de revisão de código na seção 2 — já fica
     automaticamente restrita a relacionamentos com `RelationshipView` de verdade, sem precisar de
     filtro) para relacionamentos cross-scene. Só a estrutura + API de leitura nesta subtarefa,
     ainda sem desenho visual (isso é 1.8). Depende de: 1.2.

1.6. **Roteamento de Relationship/BaseRelationship** — em `handleObjectAddition()`, resolver a
     cena das duas tabelas envolvidas; mesma cena → `RelationshipView` normal (comportamento atual,
     inalterado); cenas diferentes → não criar `RelationshipView`, registrar nas listas de conexão
     externa das duas tabelas (1.5). Com o combo de 1.3 já dá para testar manualmente criando
     tabelas em schemas/cenas diferentes e ligando-as. Depende de: 1.5.

1.7. **Auditoria de `getOverlyingObject()` para relacionamentos** — varrer os usos existentes que
     assumem `graph_obj->getOverlyingObject() != nullptr` para `Relationship`/`BaseRelationship` e
     corrigir os que quebram com a exceção legítima introduzida em 1.6 (ver alerta na seção 6).
     Depende de: 1.6.

1.8. **Marcador visual "cut-off"** — desenho do badge/ícone em `BaseTableView` para as conexões
     externas (1.5), incluindo a política de agrupamento quando há múltiplas conexões, e o sinal de
     clique carregando o objeto/cena alvo (API da seção 7) — com o combo de 1.3, esse clique já
     pode chamar `setActiveScene()` diretamente para navegação mínima de teste, mesmo antes da UI
     definitiva. Depende de: 1.6.

1.9. **Promoção/rebaixamento dinâmico** — `moveToSchema()`
     ([modelwidget.cpp:2538](libs/libgui/src/widgets/modelwidget.cpp#L2538)) (e qualquer outro
     ponto que reatribua schema de uma tabela) passa a reavaliar, para cada relacionamento da
     tabela movida, se o parceiro está na mesma cena após a mudança; cria/destrói `RelationshipView`
     e atualiza marcadores cut-off conforme a subseção "Relacionamentos cross-scene: promoção/
     rebaixamento dinâmico". Depende de: 1.8.

1.10. **Destruição de cena ao remover schema** — em `handleObjectRemoval()`, destruir a
      `ObjectsScene` do schema removido (com asserção/log de aviso em debug se não estiver vazia);
      tratar o caso da cena ativa sendo destruída (trocar `viewport` para outra cena **antes** de
      destruir o objeto, e atualizar/reposicionar a seleção do combo de 1.3); limpar entradas de
      conexão externa apontando para o schema removido. Depende de: 1.3, 1.6, 1.8.

1.11. **Teste de regressão dedicado: cascade delete de schema + undo** — cobrir explicitamente
      schema com filhos cross-scene, remoção da cena atualmente focada, e undo completo de uma
      remoção em cascata (ver riscos, seção 6). Commit de teste próprio, separado de 1.10, para
      poder reverter só a destruição de cena caso o teste revele um problema sem perder o resto da
      fase. Depende de: 1.10.

1.12. **[Requer fechamento de decisão com o usuário] Ferramenta de criação de relacionamento
      cross-scene** — implementar `pending_rel_src` (ou a alternativa de formulário manual,
      conforme a pergunta em aberto da subseção "Ferramenta de criação de relacionamento") só
      depois de decidir com o usuário qual caminho seguir. O combo de 1.3 é útil aqui mesmo se a
      direção escolhida não depender dele, como forma rápida de montar cenários de teste
      cross-scene. Depende de: 1.8.

1.13. **[Requer fechamento de decisão com o usuário] Seleção sempre local à cena ativa** —
      `setActiveScene()` limpa a seleção ao trocar de cena (testável diretamente pelo combo de
      1.3); resolver a tensão com `selectTableRelationships()`/`selectTaggedTables()` (opção (a)
      ou (b) da subseção "Seleção é sempre local à cena ativa"). Depende de: 0.2, 1.3, 1.8.

1.14. **[Requer fechamento de decisão com o usuário] Multi-seleção no Object Search Widget** —
      implementar a política escolhida entre as opções 1/2/3 da subseção dedicada (recomendação
      preliminar: opção 3). Depende de: 1.13.

> **Gate de aceite da fase (não é commit de código)**: modelo grande
> (`/home/raphael/dbm_models/big_model.dbm`) segmentado por schema deve navegar (usando o combo de
> 1.3), editar, remover (com e sem cascade) e desfazer/refazer corretamente, com conectores
> cut-off aparecendo/sumindo conforme tabelas mudam de schema. As três subtarefas 1.12-1.14 só
> entram depois de fechadas com o usuário (seção 6).

### Fase 2 — Persistência (modo de segmentação, cena focada)

Objetivo: fechar a segmentação por schema de ponta a ponta (carga, edição, salvamento, reabertura)
antes de sequer começar a olhar para threshold. Foco exclusivo em persistência do modo por schema
nesta fase — nada de atributo de bucket/threshold ainda (isso fica para a Fase 3, quando o modo
threshold for de fato detalhado).

- Novos atributos em `DatabaseModel` (mesmo padrão de `LastPosition`/`LastZoomFactor`):
  - Modo de segmentação do modelo (por ora só nenhum / por schema — deixar o valor reservado para
    "por threshold" no enum/atributo, mas sem implementar nada em cima disso ainda).
  - `LastScene`: identificador da cena focada (nome do schema, já que no modo por schema a chave
    natural é o próprio schema).
  - **Adicionado após revisão de código**: `getSceneRect()`/`setSceneRect()`
    ([databasemodel.h:898-899](libs/libcore/src/databasemodel.h#L898)) hoje persistem um único
    `QRectF` para o modelo inteiro. Precisa virar um valor por cena (indexado pela mesma chave de
    `LastScene`, isto é, por schema) para que `ModelWidget::adjustSceneRect(use_model_rect=true)`
    ([modelwidget.cpp:1946-1951](libs/libgui/src/widgets/modelwidget.cpp#L1946)) continue
    restaurando o tamanho de canvas correto de cada cena na carga do arquivo.
- Estender `ModelWidget::saveLastCanvasPosition()`/`restoreLastCanvasPosition()`
  ([modelwidget.cpp:972-1023](libs/libgui/src/widgets/modelwidget.cpp#L972)) para gravar/ler
  `LastScene` e, na restauração, ativar aquela cena **antes** de aplicar scroll/zoom salvos.
- **Critério de aceite da Fase 2**: fechar, salvar, reabrir um modelo segmentado por schema deve
  preservar cena focada + zoom/posição daquela cena, e a segmentação por schema deve se
  reconstruir corretamente na carga sem depender de nenhum estado extra persistido por tabela
  (a chave é o próprio schema de cada objeto, já persistido hoje).

#### Subtarefas da Fase 2 (uma por commit)

2.1. **Persistência de modo de segmentação + `LastScene`** — popular de fato (leitura/escrita XML)
     o scaffolding preparado em 0.8: modo de segmentação do modelo e `LastScene` (identificador da
     cena focada, nome do schema). Ainda sem uso em runtime neste commit — só grava/lê. Depende
     de: 0.8.

2.2. **Persistência de `SceneRect` por cena** — estende o scaffolding de 0.8 para popular de fato
     um `QRectF` por cena (indexado pela mesma chave de `LastScene`), usado depois por
     `ModelWidget::adjustSceneRect(use_model_rect=true)`
     ([modelwidget.cpp:1946-1951](libs/libgui/src/widgets/modelwidget.cpp#L1946)). Depende de: 0.8,
     1.1 (precisa que cenas por schema já existam de verdade para ter o que persistir por cena).

2.3. **Wiring runtime de `LastScene`** — estender
     `ModelWidget::saveLastCanvasPosition()`/`restoreLastCanvasPosition()`
     ([modelwidget.cpp:972-1023](libs/libgui/src/widgets/modelwidget.cpp#L972)) para gravar a cena
     ativa ao salvar e, ao restaurar, ativar aquela cena (via `setActiveScene()`, 0.2) **antes** de
     aplicar zoom/scroll salvos. Depende de: 2.1, 0.2.

2.4. **Fallback de `LastScene` inválido** — se o schema referenciado por `LastScene` não existir
     mais na carga do arquivo (ex. schema removido em edição posterior ao save), cair para a
     primeira cena disponível em vez de falhar. Depende de: 2.3, 1.10 (cenário de schema removido já
     precisa existir para este fallback fazer sentido).

2.5. **Teste de regressão fim-a-fim da Fase 2** — fechar, salvar, reabrir um modelo segmentado por
     schema preserva cena focada + zoom/posição daquela cena, e a segmentação por schema se
     reconstrói corretamente na carga. Depende de: 2.2, 2.3.

### Fase 3 — Segmentação por Threshold (a detalhar — não iniciar ainda)

> Deliberadamente deixada por último e em aberto. O modo por schema (Fases 1-2) precisa estar
> "redondo" — implementado, testado, com os pontos de estudo dedicado (ferramenta de
> relacionamento, seleção, Object Search Widget) resolvidos — antes de investir tempo em detalhar
> o modo threshold. As notas abaixo são só o entendimento inicial, não um plano fechado como o das
> fases anteriores; espere revisar/reescrever esta seção quando chegar a hora.

- Contador de objetos inseridos (Table + View + ForeignTable, sem distinção de tipo, cf.
  especificação original). Ao cruzar um múltiplo do threshold, nova `ObjectsScene` passa a
  receber as inserções seguintes.
- Risco identificado: bucketing ingênuo por ordem de inserção pode gerar um número muito alto de
  relacionamentos cross-scene (o modo por schema tem fronteiras naturais; threshold não tem).
  Mitigação especulativa: ao carregar um arquivo existente (contexto em lote, grafo de FKs
  disponível de antemão), usar um agrupamento ciente de conectividade (ex. BFS/union-find sobre o
  grafo de relacionamentos) para manter clusters fortemente conectados no mesmo bucket, respeitando
  o threshold como teto por bucket. Em criação manual/interativa (um objeto de cada vez, sem
  visão do grafo completo ainda), cair para ordem de inserção simples. **Não validado, precisa de
  desenho próprio quando esta fase for retomada.**
- Reutilizar o mesmo mecanismo de marcador cut-off da Fase 1 para os cross-scene que restarem —
  isso depende de a Fase 1 já ter esse mecanismo bem resolvido (é o motivo de deixar threshold por
  último).
- Política de rebalanceamento: **não** mesclar/destruir cenas automaticamente quando uma fica
  esvaziada por remoções — diferente do modo por schema (onde a cena é destruída porque está
  atrelada ao ciclo de vida de um `Schema`), no modo threshold a cena não tem nenhum objeto-dono
  cujo removal sirva de gatilho, então ela simplesmente permanece vazia até (se algum dia)
  receber novas inserções. Simplificação deliberada, mas ainda sujeita a revisão.
- Persistência (retomando o que ficou de fora da Fase 2 de propósito): diferente do modo por
  schema (cuja chave é auto-reconstrutível na carga), o modo threshold com clustering precisaria
  persistir a atribuição de bucket por objeto (atributo por tabela/view/ftable, análogo ao
  atributo de layer já persistido por objeto), para que reabrir o arquivo não dependa de re-rodar
  o clustering e obter um resultado potencialmente diferente. Formato exato a definir junto com o
  resto desta fase.

#### Subtarefas da Fase 3 (provisórias — a refinar quando a fase for retomada)

> Ao contrário das Fases 0-2, esta fase ainda não teve o desenho fechado (ver nota no topo desta
> seção), então a quebra abaixo é grossa e especulativa por natureza — serve de esqueleto para
> reescrever quando a Fase 1 estiver "redonda", não como uma lista pronta para codar.

3.1. Contador de objetos inseridos (Table + View + ForeignTable) + criação de nova cena ao cruzar
     o threshold, usando ordem de inserção simples (caminho interativo, sem visão do grafo
     completo). Depende de: 1.1, 1.2, 1.5, 1.6 (reaproveita o choke point de criação de cena e de
     roteamento).

3.2. Agrupamento ciente de conectividade (BFS/union-find sobre o grafo de relacionamentos) para
     carga em lote de arquivo existente, respeitando o threshold como teto por bucket. Depende de:
     3.1.

3.3. Reaproveitamento do marcador cut-off da Fase 1 (1.8) para os cross-scene que restarem no modo
     threshold — deve ser praticamente gratuito se 1.8 foi bem desenhado; se não for, é sinal de
     que o desenho de 1.8 vazou detalhes específicos do modo por schema. Depende de: 3.1, 1.8.

3.4. Persistência da atribuição de bucket por objeto (novo atributo por tabela/view/ftable,
     análogo ao atributo de layer já persistido por objeto). Depende de: 3.2.

3.5. Documentar e validar com teste a política de não-rebalanceamento (cenas esvaziadas por
     remoção não são mescladas/destruídas automaticamente no modo threshold) — verificação, não
     feature nova. Depende de: 3.1.

## 5. Fora de escopo (por enquanto)

- UI **definitiva** de troca de cena (dropdown estilizado, sidebar, abas) — o usuário vai construir
  separadamente. O core deve deixar pronta uma API pública limpa para consumo futuro (seção 7).
  **Exceção**: a Fase 1 inclui um `QComboBox` simples (subtarefa 1.3) só como aid de teste manual
  durante o desenvolvimento — não é a UI definitiva e não deve receber investimento de design.
- Merge automático de cenas subutilizadas.
- Minimap unificado mostrando todas as cenas simultaneamente. **Decisão fechada**: o
  `ModelOverviewWidget` (minimap) passa a exibir sempre a cena atualmente ativa (foco), e se
  atualiza automaticamente quando a cena ativa muda — nenhuma visão combinada de múltiplas cenas
  é necessária. Ver detalhamento na Fase 0.

## 6. Riscos conhecidos / pontos de atenção para quem retomar

- `RelationshipView::configureLine()` é grande (~700 linhas) e **não deve ser tocado** para tentar
  fazer `mapFromItem`/`mapToItem` funcionar entre cenas — pelo refinamento da decisão #1, um
  relacionamento cross-scene simplesmente não tem `RelationshipView` nenhum (o marcador cut-off é
  responsabilidade de `BaseTableView`, uma classe diferente). Se alguém se pegar adicionando um
  "modo detached" dentro de `RelationshipView`, é sinal de que a decisão foi reaberta sem querer —
  volte para a seção 3, decisão #1.
- Undo/redo não precisa de nenhuma mudança own própria — só se beneficia automaticamente do
  roteamento correto no choke point de `handleObjectAddition`/`handleObjectRemoval`. Se alguma
  mudança em undo/redo parecer necessária, é sinal de que a lógica de roteamento vazou para o
  lugar errado.
- Grid/página/cor de fundo são `static` em `ObjectsScene` — não crie estado por-cena para essas
  configurações, já são compartilhadas de graça.
- Ao testar a Fase 1 (segmentação por schema) com modelos grandes, usar
  `/home/raphael/dbm_models/big_model.dbm`
  (~4.500 tabelas, ~10.000 relacionamentos) como referência de escala real — já usado para medir o
  ganho da indexação de `DatabaseModel::getObject()` (commit `681fa6597` nesta mesma branch de
  trabalho de performance).
- Remoção de schema (cascade ou não) precisa de teste explícito cobrindo: schema com filhos
  cross-scene (relacionamentos com conector off-page apontando para/da cena removida), remoção do
  schema cuja cena é a atualmente focada pelo `viewport`, e undo completo de uma remoção em
  cascata. Ver detalhamento em "Remoção de schema e destruição de cena" na Fase 1 — é a área mais
  nova e menos testada mentalmente deste plano, adicionada depois da análise inicial.
- **Não começar a codar a ferramenta de criação de relacionamento cross-scene, a política de
  seleção multi-cena, nem a multi-seleção do Object Search Widget sem antes fechar as três
  subseções dedicadas na Fase 1** ("Ferramenta de criação de relacionamento", "Seleção é sempre
  local à cena ativa" e "Multi-seleção no Object Search Widget"). São, junto com o desenho do
  conector off-page, os pontos de maior risco de interação/UX de todo o plano — `rel_line` é um
  item por-cena ([objectsscene.cpp:75-81](libs/libcanvas/src/objectsscene.cpp#L75)),
  `configureObjectSelection()` conta seleção lendo só a cena ativa
  ([modelwidget.cpp:1393-1469](libs/libgui/src/widgets/modelwidget.cpp#L1393)), e
  `ObjectSearchWidget` faz `setSelected()`/`centerOn()` direto em objetos de qualquer cena do
  modelo sem nenhuma noção de cena ativa hoje
  ([objectsearchwidget.cpp:168-218, 291-358](libs/libgui/src/widgets/objectsearchwidget.cpp#L168))
  — os três precisam de redesenho de estado, não só refactor mecânico.

## 7. API pública mínima que o core deve expor (para a UI futura do usuário)

Ainda que a UI não seja construída agora, o core deve deixar pronto o suficiente para que ela seja
construída sem precisar tocar em `ModelWidget` internamente de novo:

- `int ModelWidget::getSceneCount()`
- `ObjectsScene *ModelWidget::getObjectsScene(int idx)` (além do `getObjectsScene()` atual, que
  passa a retornar a cena ativa)
- `ObjectsScene *ModelWidget::getSceneForObject(BaseObject *object)`
- `void ModelWidget::setActiveScene(int idx)` — encapsula `viewport->setScene(...)` +
  reaplicação do `current_zoom` atual, para que zoom não "perca o valor" ao trocar de cena.
- `void s_activeSceneChanged(int new_idx)` — emitido por `setActiveScene()`. **Não é só para a UI
  futura**: `ModelOverviewWidget` já precisa consumir este sinal internamente (Fase 0) para
  desconectar/reconectar seus sinais de `selectionChanged`/`sceneRectChanged` na cena nova e
  redesenhar o minimap (decisão da seção 5 — minimap sempre mostra a cena ativa).
- Sinal emitido quando um conector off-page é clicado, carregando o objeto/cena alvo, para a UI
  decidir a navegação.
- Sinal emitido quando uma cena é destruída (ex. `s_sceneDestroyed(int old_idx, int fallback_idx)`)
  para que a UI futura (dropdown/sidebar) possa atualizar sua própria lista/seleção sem precisar
  fazer polling em `getSceneCount()`.
