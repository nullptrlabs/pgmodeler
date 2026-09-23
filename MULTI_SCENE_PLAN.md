# Plano de Implementação — Suporte a Múltiplas Cenas (Multi-Scene Support)

> Branch: `multi-scene-support-2.1.x`. Este documento é o plano de trabalho de referência entre sessões/máquinas.
> Estado: implementação em andamento — ver `MULTI_SCENE_PROGRESS.md` para onde paramos exatamente
> (log sessão a sessão, não duplicado aqui).
> Prioridade: fechar segmentação por schema (Fases 0-2) primeiro, ponta a ponta, antes de detalhar
> o modo por threshold (Fase 3) — ver seção 4. **Fase 4** (paridade do `pgmodeler-cli`) é dona do
> usuário e só começa depois das Fases 0-3 prontas — não iniciar por conta própria.
> Atualização 2026-09-18: seções marcadas "achado de revisão de código" / "verificado em revisão
> de código" foram adicionadas após uma leitura profunda do código real envolvido na Fase 0 (ver
> arquivos citados nas próprias notas). Uma delas corrige uma afirmação da versão anterior deste
> documento sobre `connected_rels` (ver subseção "Relacionamentos cross-scene: promoção/
> rebaixamento dinâmico" na Fase 1).
> Atualização 2026-09-21: decisão #7 (seção 3) fecha o algoritmo da Fase 3 (contador ingênuo, sem
> clustering ciente de conectividade dentro do modo threshold). Clustering ciente de conectividade
> **não foi descartado** — vira candidato a um terceiro modo de segmentação futuro, separado do
> threshold ingênuo (ver seção 5). Fase 3 reescrita para refletir isso. A decisão #8 (terminologia
> "Workspace") foi cogitada e depois **revertida no mesmo dia** — o nome final é "cena"/"Scene" em
> tudo, ver decisão #8 atualizada na seção 3. Fechado também o design completo de retângulo por
> cena persistido (achado de revisão de código expandido na seção 2): sem retângulo global
> compartilhado, formato XML mirror de `changelog`/`entry` (tag `<scenes>`/`<scene>`), com
> retrocompatibilidade explícita para arquivos de versões anteriores — ver Fase 2, subtarefa 2.2.
> Também registrado (achado do usuário): `SchemaView` tem o mesmo problema que `RelationshipView`
> tinha antes da decisão #1, mas só no modo threshold — schema deixa de ser 1:1 com cena, então um
> schema com filhos em cenas diferentes precisa suprimir seu retângulo em vez de tentar renderizar
> algo cross-scene. Ver subseção dedicada e subtarefa 3.5 na Fase 3.
> Atualização 2026-09-21 (2): chave de identidade de `<scene>`/`LastScene` revisada de nome do
> schema para **índice numérico, atributo XML `id`** — uniformiza os três modos de segmentação
> (threshold e o futuro modo com clustering não têm nome natural) e é seguro por verificação de
> código (ordem de carga de `Schema` é determinística, ver seção 2). **Atualização 2026-09-21 (3)**:
> o atributo `name` foi descartado de `<scene>` (não só rebaixado a auxiliar) — sem necessidade real
> comprovada ainda; pode ser reintroduzido no futuro se surgir um motivo concreto. Limitação aceita:
> `swapObjectsIds()`/"Edit creation order" pode desalinhar índices no save/reload seguinte —
> documentado como conhecido, sem código de mitigação (decisão do
> usuário). Ver seção 2, decisões #3 e #7 (seção 3), e subtarefas 2.1/2.2.
> **Atualização 2026-09-21 (4)**: DTD de `<scenes>`/`<scene>` vira arquivo dedicado
> `assets/schemas/xml/dtd/scene.dtd` (mirror fiel de `changelog.dtd`, incluído em `dbmodel.dtd` via
> entidade, não declarações soltas nele) — a pedido do usuário.
> **A partir desta atualização, este documento não registra mais status de execução** ("[FEITO]",
> commits, etc.) — isso passou a viver só em `MULTI_SCENE_PROGRESS.md`.

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
  Estritamente por ordem de inserção, sem nenhum agrupamento por conectividade (decisão #7,
  seção 3) — um terceiro modo com agrupamento ciente de conectividade fica registrado como
  candidato futuro fora do escopo atual (seção 5), não uma variação deste.

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
   focada** (novo atributo `LastScene`, análogo a `LastPosition`/`LastZoomFactor`, ver
   [databasemodel.cpp:3991-3996, 8000, 9458, 9760](libs/libcore/src/databasemodel.cpp#L3991)).
   Ao restaurar (`ModelWidget::restoreLastCanvasPosition()`,
   [modelwidget.cpp:1000](libs/libgui/src/widgets/modelwidget.cpp#L1000)), primeiro ativa-se essa
   cena, depois aplica-se zoom/scroll salvos — a restauração é sempre relativa à cena focada, não
   a "a cena" genérica. **Atualização 2026-09-21**: `LastScene` é numérico (índice, mesma chave
   `id` usada em `<scenes>`/`<scene>`), não o nome do schema — ver detalhamento e motivação na
   subseção de persistência de `SceneRect`, seção 2.
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
7. **Segmentação por threshold (Fase 3): estritamente ingênua, sem clustering — clustering vira
   candidato a um TERCEIRO modo futuro, separado** (adicionado em 2026-09-21, refinado depois da
   revisão do usuário no mesmo dia). O algoritmo da Fase 3 é só um contador incremental de objetos
   inseridos — ao cruzar o threshold, abre-se uma nova cena e as inserções seguintes vão para ela,
   sem nenhum agrupamento ciente de conectividade. Motivo: sofisticar o critério de particionamento
   **dentro do modo threshold** arriscaria reordenar objetos entre cenas de um jeito que o usuário
   não pediu e não consegue prever — o valor de uma regra estritamente ingênua é que "o que cai em
   qual cena" continua sendo consequência direta e previsível da ordem de inserção, sem heurística
   escondida por trás. Consequência aceita nesta fase: relacionamentos
   cross-scene podem ser numerosos (ao contrário do modo por schema, que tem fronteiras naturais) —
   a única mitigação é o mecanismo de marcador cut-off já construído na Fase 1, reaproveitado sem
   modificação.
   **Importante — não é a mesma coisa que descartar a ideia**: agrupamento ciente de conectividade
   (BFS/union-find sobre o grafo de relacionamentos) continua uma opção válida — só não como uma
   variação/flag do modo threshold ingênuo, e sim como um **modo de segmentação novo e independente**
   a considerar no futuro (candidato a "Fase 4", fora do escopo atual). Ver seção 5 ("Fora de
   escopo") e a nota correspondente na Fase 3.
8. ~~**Terminologia: "Workspace" para o usuário, "cena" internamente**~~ **REVERTIDA em
   2026-09-21** — decisão final do usuário: usar **"cena"/"Scene"** em tudo (UI futura, atributos
   no arquivo `.dbm`, código), sem distinção usuário/interno. Motivo dado: praticidade, e o formato
   já tem atributos nomeados com "Scene" (`SceneRect`, `LastScene`) — introduzir "Workspace" só pro
   lado do usuário criaria uma inconsistência de nomenclatura sem necessidade real. Onde quer que
   este documento mencione "Workspace"/"workspace" (decisão #7, Fase 3, subtarefa 1.3, etc.), leia
   como "cena"/"Scene" — foi feita uma varredura corrigindo essas menções nesta mesma atualização,
   mas se algo escapou, esta é a decisão que vale.
9. **Cena-local vs. modelo-inteiro-replicado-por-cena: dois tipos diferentes de "corrigir uma
   suposição de cena única"** (adicionado em 2026-09-21, na revisão da subtarefa 0.5). Nem toda
   suposição de "só existe uma cena" no código atual é um bug a corrigir agregando entre cenas —
   existem dois tipos bem diferentes:
   - **Cena-local** (correto usar só a cena ativa, `scene`, sem tocar as demais): qualquer coisa
     ligada a **interação/visualização** — seleção (`selectAllObjects()`,
     `scene->selectedItems()`, `scene->clearSelection()`), bounding rect de itens selecionados,
     gates de menu que dependem da cena atualmente visível (ex. `scene->items().count() > 1` para
     habilitar "Select all"). Não faz sentido selecionar ou contar objetos que não estão visíveis
     em outra cena — isso é interação com o que está na tela, não uma operação sobre o modelo.
   - **Modelo-inteiro, mas replicado por cena** (correto iterar `getObjectsScenes()`, chamando a
     mesma operação em cada uma, **sem combinar o resultado em um valor só**): normalização/
     configuração que vale para o modelo inteiro independente do que está visível — o exemplo
     concreto já corrigido é `alignObjectsToGrid()` (subtarefa 0.5): quando a opção "alinhar à
     grade" está ativa, alinhar só a cena ativa deixaria objetos de outras cenas desalinhados sem
     motivo. Layers (subtarefa 0.6) é outro exemplo desta categoria: uma mutação de layer (criar,
     renomear, mudar cor) precisa valer para todas as cenas, não só a ativa.
   Ao encontrar uma nova suposição de cena única durante a implementação, classificar antes de
   "corrigir" — errar para o lado de agregar/replicar por reflexo (como a redação original da
   subtarefa 0.5 fez) introduz comportamento que ninguém pediu.

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
- **Achado de revisão de código, expandido em 2026-09-21**: `DatabaseModel::getSceneRect()`/
  `setSceneRect()` ([databasemodel.h:898-899](libs/libcore/src/databasemodel.h#L898)) persistem um
  **único** `QRectF` para o modelo inteiro, consumido por
  `ModelWidget::adjustSceneRect(use_model_rect=true)` na carga do arquivo
  ([modelwidget.cpp:1946-1951](libs/libgui/src/widgets/modelwidget.cpp#L1946)). Com N cenas por
  schema esse único retângulo persistido fica ambíguo — vira por-cena, indexado pela mesma chave
  que `LastScene` (Fase 2).
  - **`ObjectsScene::setSceneRect(rect)` não move nem reposiciona item nenhum**
    ([objectsscene.cpp:676-693](libs/libcanvas/src/objectsscene.cpp#L676)) — só ajusta o retângulo
    de rolagem/indexação espacial da `QGraphicsScene`. Um objeto persistido com posição absoluta
    `(1001, 0)` continua exatamente ali não importa qual seja o `sceneRect()` da cena. Ou seja,
    aplicar um retângulo por-cena calculado corretamente (a partir só dos itens daquela cena) **não
    desloca objeto nenhum pra origem** — carregar cada cena com seu próprio retângulo, começando
    onde quer que seus objetos realmente estejam, é seguro.
  - **Cuidado real encontrado**: `ObjectsScene::adjustSceneRect(bool expand_only)`
    ([objectsscene.cpp:1680-1709](libs/libcanvas/src/objectsscene.cpp#L1680)) — o método que
    *recalcula* o retângulo a partir da posição atual dos itens (usado em edição interativa, não no
    carregamento de arquivo) — quando `expand_only == false`, **puxa a borda esquerda/superior do
    retângulo pra perto de zero incondicionalmente** (`lp = -rect.left()` quando `rect.left() > 0`,
    depois `rect.adjust(lp, tp, grid_size, grid_size)`). Isso não move os objetos, mas faz o
    retângulo da cena "esticar" de volta até a origem mesmo quando os objetos daquela cena estão
    todos longe dali — se aplicado por-cena sem cuidado na Fase 1, cada cena acabaria com um
    retângulo desnecessariamente grande cobrindo espaço vazio. **Ao computar o retângulo inicial de
    uma cena nova/recém-dividida, usar `itemsBoundingRect()` direto sobre os itens daquela cena,
    nunca `adjustSceneRect(false)`.**
  - **Decisão (2026-09-21): sem retângulo global compartilhado entre cenas.** Foi cogitado (e
    descartado) persistir um único retângulo "soma de todas as cenas" **aplicado a todas elas em
    runtime** (cada `ObjectsScene` individual compartilhando as mesmas dimensões). Cada cena
    mantém **só o seu próprio** `sceneRect()` em runtime, calculado a partir dos seus próprios
    itens — nada sincronizado entre cenas ao vivo.
    **Revisitada e reafirmada em 2026-09-22** (usuário levantou de novo, motivado por flexibilidade
    de troca de modo de segmentação): o custo real de um retângulo compartilhado em runtime não é
    tanto o índice espacial (a profundidade da BSP tree do Qt escala com número de itens, não com
    área do retângulo — a justificativa original nesse ponto era imprecisa) — é
    **`ObjectsScene::getPagesForPrinting()`** ([objectsscene.cpp](libs/libcanvas/src/objectsscene.cpp)),
    usado pela exportação/impressão por-cena da subtarefa 0.7: uma cena pequena forçada a
    compartilhar as dimensões de uma cena grande geraria uma quantidade enorme de páginas em
    branco ao exportar/imprimir só ela. **Decisão final**: cada cena continua com seu retângulo
    independente em runtime (decisão original mantida) — mas o atributo legado `scene-rect` (único,
    em `<dbmodel>`) passa a guardar um valor **"geral"**, calculado como a **união**
    (`QRectF::united()`) do retângulo atual de todas as cenas, **recalculado do zero só no momento
    de salvar** (nunca sincronizado ao vivo a cada mudança de uma cena) — serve como valor de
    fallback pra quando o usuário desliga a segmentação e volta pro modo de cena única, preservando
    a área de trabalho "como se fosse uma cena só" sem pagar o custo de impressão/exportação
    durante a operação segmentada normal. Ver detalhamento e o método novo proposto
    (`updateModelSceneRectInfo()`) nas subtarefas 2.0/2.2.
  - **Formato de persistência escolhido**: mirror do mecanismo já existente e testado de
    `changelog`/`<entry>` ([databasemodel.cpp:10520-10555](libs/libcore/src/databasemodel.cpp#L10520)
    escrita, [databasemodel.cpp:3647-3664](libs/libcore/src/databasemodel.cpp#L3647) leitura;
    schemas em [assets/schemas/xml/changelog.sch](assets/schemas/xml/changelog.sch) e
    [assets/schemas/xml/entry.sch](assets/schemas/xml/entry.sch)) — uma tag contêiner envolvendo N
    tags filhas simples, uma por cena:
    ```xml
    <scenes>
    	<scene id="0" left="0" top="0" right="1000" bottom="1000" />
    	<scene id="1" left="1001" top="0" right="2000" bottom="1200" />
    </scenes>
    ```
  - **Chave de identidade: `id` (índice numérico), não nome** (revisado em 2026-09-21, a pedido
    do usuário). Motivação original era evitar o "custo de renomear a cena junto com o schema" —
    na prática esse custo não existiria mesmo com chave por nome, já que todo o bloco `<scenes>` é
    recalculado do zero a cada save (igual ao `changelog`), nunca sincronizado incrementalmente;
    tanto faz nome quanto índice nesse quesito. O motivo real e válido pra usar índice é outro:
    **uniformidade entre os três modos de segmentação**. Por schema tem nome natural (o schema);
    threshold e o futuro modo com clustering não têm nenhum nome natural — teriam que inventar um
    rótulo artificial, o que é pior que só usar a posição na lista. `id` é um atributo **explícito**
    (não a posição implícita do elemento no XML) — depender só da ordem dos elementos seria frágil
    contra edição manual/reformatação do arquivo.
    **Atributo `name` descartado** (revisado em 2026-09-21, a pedido do usuário) — cogitado como
    auxiliar de depuração, mas sem necessidade real comprovada ainda; se surgir motivo concreto no
    futuro, é fácil reintroduzir (não há nada na Fase 1/2 que dependa da ausência dele).
    `LastScene` (decisão #3, seção 3) passa a ser numérico pelo mesmo motivo — sem problema de
    retrocompatibilidade, é um atributo novo, não existe em arquivo salvo por versão anterior a
    esta feature.
  - **Determinismo verificado por revisão de código (2026-09-21)**: para o índice sobreviver a um
    ciclo salvar→recarregar, a ordem de criação dos `Schema` na carga precisa ser estável.
    Confirmado: `DatabaseModel::loadModel()` cria os `Schema` na ordem exata em que os `<schema>`
    aparecem no XML (sem reordenar), e a gravação (`getCreationOrder()`,
    [databasemodel.cpp:8022](libs/libcore/src/databasemodel.cpp#L8022)) ordena por `object_id`
    ascendente — atribuído em ordem crescente de criação
    ([baseobject.cpp:109-112](libs/libcore/src/baseobject.cpp#L109)). Como a carga cria objetos na
    ordem do arquivo, isso é um ponto fixo estável: carregar → salvar → recarregar preserva a ordem
    relativa dos schemas indefinidamente no caso comum.
  - **Limitação conhecida e aceita (decisão do usuário, 2026-09-21): "Edit creation order"
    (`BaseObject::swapObjectsIds()`, [baseobject.cpp:1081-1121](libs/libcore/src/baseobject.cpp#L1081))
    pode desalinhar os índices no ciclo de save/reload seguinte.** Trocar manualmente a ordem de
    criação de dois schemas (ação explícita do usuário, ou reparo automático de referência quebrada
    da validação de modelo) não afeta a sessão atual — a `QList<ObjectsScene *> scenes` em memória
    não é tocada por essa ação, só o `object_id` dos objetos. O efeito só aparece no **próximo**
    save→reload: `<schema>` passa a ser serializado na nova ordem (por `object_id`), mas `<scenes>`
    é escrito na ordem atual da lista em memória (que não mudou) — na recarga seguinte, os `Schema`
    são recriados na ordem nova, e um retângulo persistido pode acabar aplicado à cena errada.
    **Não é cumulativo**: nessa mesma recarga o `<scenes>` já é reescrito refletindo a ordem nova,
    então é um hiccup visual único (uma cena "herda" o retângulo salvo de outra numa única reabertura),
    autocorrigido no save seguinte — e como já existe fallback para "sem retângulo persistido"
    (`itemsBoundingRect()` recalculado), o pior caso é uma cena com canvas em posição/tamanho
    estranho, não perda de dado. **Decisão: aceitar como limitação documentada, sem código de
    mitigação** — opção mais simples; o custo de qualquer checagem cruzada de validação na carga
    não se justifica pra uma ação rara e deliberada do usuário.
    Escrita e leitura seguem o padrão do changelog: um `schparser.getSourceCode(Attributes::Scene,
    attribs, XmlCode)` por cena concatenado num buffer, envolvido pelo template do container
    (`Attributes::Scenes`); leitura via `xmlparser.accessElement(ChildElement)`/`NextElement`
    reconstruindo um `QMap<int, QRectF>` (chave `id`) em `DatabaseModel`. `Attributes::Scenes`/
    `Scene` são constantes novas em [attributes.h](libs/libparsers/src/attributes.h); `Id`/`Left`/
    `Top`/`Right`/`Bottom` já existem, reaproveitados (`Name` não é mais usado aqui).
  - **Retrocompatibilidade com modelos salvos por versões antigas (requisito explícito do
    usuário)**: o atributo legado `scene-rect` (único, plano, em `<dbmodel>`) **continua existindo e
    sendo escrito exatamente como hoje quando o modo de segmentação está desligado** (situação de
    todo modelo salvo por uma versão anterior a esta feature, e de todo modelo que não ativou
    segmentação) — formato de arquivo **inalterado** no caso comum. O novo bloco `<scenes>` só passa
    a ser escrito quando a segmentação está de fato ativa (N>1 cenas). Ao carregar um arquivo antigo
    (sem `<scenes>`, só o `scene-rect` legado) num modelo que a Fase 1 vai segmentar em N cenas: **não
    tentar redistribuir o valor legado entre as cenas novas** — cada cena simplesmente cai no
    fallback já existente (`adjustSceneRect(false)`/`itemsBoundingRect()` recalculado a partir dos
    seus próprios itens), do jeito que já acontece hoje quando `getSceneRect()` retorna inválido.
  - **DTD e template (.sch) precisam ser atualizados junto com o código C++**, não é só
    `DatabaseModel`. **Atualização 2026-09-21 (4), a pedido do usuário**: mirror mais fiel de
    `changelog.dtd` é um **arquivo DTD dedicado**, não declarações soltas dentro de `dbmodel.dtd` —
    `changelog.dtd` já funciona assim (`<!ELEMENT changelog (entry*)>` + `<!ELEMENT entry EMPTY>`
    num arquivo próprio, incluído via `<!ENTITY % changelog SYSTEM "changelog.dtd"> %changelog;`
    em [dbmodel.dtd:89-90](assets/schemas/xml/dtd/dbmodel.dtd#L89)). Mesmo padrão pra cenas: novo
    [assets/schemas/xml/dtd/scene.dtd](assets/schemas/xml/dtd/scene.dtd) contendo
    `<!ELEMENT scenes (scene*)>` + `<!ELEMENT scene EMPTY>` com `id`/`left`/`top`/`right`/`bottom`
    (`id` é a chave); `dbmodel.dtd` só ganha `<!ENTITY % scene SYSTEM "scene.dtd"> %scene;` (mirror
    exato da linha do changelog) — mantém `<!ATTLIST dbmodel scene-rect CDATA #IMPLIED>` como está.
    [assets/schemas/xml/dbmodel.sch](assets/schemas/xml/dbmodel.sch) ganha um
    `%if {scenes} %then {scenes} %end` (mirror do `%if {changelog} %then {changelog} %end` já
    existente); novos `assets/schemas/xml/scenes.sch` (container) e `assets/schemas/xml/scene.sch`
    (entrada), mirror exato de `changelog.sch`/`entry.sch`.
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
     **Correção arquitetural (achada ao implementar a 1.1, 2026-09-22)**: o construtor de
     `ModelWidget` conectava ~19 sinais de `ObjectsScene` (movimento/modificação/duplo-clique/
     seleção de objeto, popup menu, modo de colapso, paginação, mudança de layer, `sceneRectChanged`
     etc. — [modelwidget.cpp:682-724](libs/libgui/src/widgets/modelwidget.cpp#L682) na revisão
     original) **só uma vez, só para a cena única criada ali**. `addScene()` precisa fazer todo esse
     wiring pra **cada** cena que cria, não só a primeira — senão cenas criadas depois (por schema,
     a partir da 1.1) ficam sem reagir a movimento/edição/seleção/etc. Consequência: o construtor
     **não cria mais nenhuma cena eagerly** (`viewport` nasce sem cena, `new QGraphicsView(this)`) —
     do contrário essa cena "fantasma" criada antes de qualquer schema existir venceria a regra
     "ativa se for a primeira" de `addScene()` e ficaria permanentemente presa ao `viewport` em vez
     da cena do primeiro schema real. `handleObjectAddition()` ganhou uma rede de segurança: se
     nenhuma cena existir ainda quando um objeto precisar ser inserido, cria uma sob demanda (caso
     patológico: um objeto sem schema, como `Textbox`, chegando antes de qualquer schema existir).

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

0.5. **`ModelWidget::alignObjectsToGrid()` iterando todas as cenas** — revisão
     de escopo em relação à redação original desta subtarefa (ver nota abaixo). Novo método público
     `ModelWidget::alignObjectsToGrid()` ([modelwidget.h](libs/libgui/src/widgets/modelwidget.h),
     [modelwidget.cpp](libs/libgui/src/widgets/modelwidget.cpp)) itera `scenes` chamando
     `ObjectsScene::alignObjectsToGrid()` em cada uma. Substituídos os 4 call sites que antes
     acessavam `scene->alignObjectsToGrid()` (a cena única/ativa) diretamente:
     `MainWindow` (3 pontos — carga de modelo com a opção "alinhar à grade" ativa,
     [mainwindow.cpp:1279,1312](libs/libgui/src/mainwindow.cpp#L1279), e o toggle da opção nas
     configurações com um modelo já aberto, [mainwindow.cpp:1548](libs/libgui/src/mainwindow.cpp#L1548))
     e `ModelWidget::adjustSceneRect()` ([modelwidget.cpp:2024-2027](libs/libgui/src/widgets/modelwidget.cpp#L2024)).
     Motivo: alinhar à grade é uma normalização de posição do objeto, não uma operação ligada ao
     que está visível no momento — não faz sentido alinhar só a cena ativa e deixar objetos de
     outras cenas desalinhados. `MainWindow` passou a chamar `current_model->alignObjectsToGrid()`
     em vez de alcançar `current_model->scene` diretamente (também simplifica: não precisa mais de
     acesso de `friend` a um membro privado para isso). Depende de: 0.1.

     **Nota sobre a redação original desta subtarefa**: a versão anterior descrevia agregar
     `ModelWidget::selectAllObjects()`, os `itemsBoundingRect()` usados em
     `emitSceneInteracted()`/`eventFilter()` e o gate `scene->items().count() > 1`
     ([modelwidget.cpp:4780](libs/libgui/src/widgets/modelwidget.cpp#L4780), controla se o item de
     menu "Select all" aparece) entre todas as cenas. **Revisado em 2026-09-21, a pedido do
     usuário**: esses três já operam corretamente sobre `scene` (o ponteiro de conveniência da
     cena ativa) e devem **continuar assim** — selecionar/contar objetos que não estão visíveis em
     outra cena não faz sentido; seleção é uma operação de interação com o que está na tela, ao
     contrário do alinhamento à grade. **Nenhuma mudança de código foi feita para esses três** —
     ficam verificados como corretos como estão, não como pendência.

0.6. **Generalização de Layers** — `LayersConfigWidget` e
     `ModelWidget::updateSceneLayers()` passam a aplicar mutações a **todas** as cenas da coleção
     (decisão #2, seção 3); leituras continuam vindo só da cena ativa (`updateLayersList()`,
     `updateModelLayersInfo()` — sem mudança, confiam no invariante de que todas as cenas têm
     estado de layer idêntico). Classificação por método (decisão #9), todos em
     [layersconfigwidget.cpp](libs/libgui/src/widgets/layersconfigwidget.cpp):
     - **Mutadores simples** (passam a iterar `model->getObjectsScenes()`): `updateActiveLayers()`
       (`setActiveLayers`), `removeLayer()` (`removeLayers()`/`removeLayer()`), `updateLayerColors()`
       (`setLayerColors`), `toggleLayersRects()` (`setLayerRectsVisible`/`setLayerNamesVisible` —
       a leitura que decide `updated` continua contra a cena ativa, só a escrita passa a
       propagar).
     - **Mutadores com nome desambiguado** (`addLayer()`, `finishLayerRenaming()`): `addLayer()`/
       `renameLayer()` retornam o nome já desambiguado; resolvido primeiro na cena ativa, depois
       o **mesmo nome resultante** (não o texto bruto do usuário) é aplicado às demais cenas, para
       não arriscar desambiguação divergente entre cenas.
     - **`updateRelsVisibility()`**: passou a iterar `scn->items()` de todas as cenas (era só
       `model->scene->items()`) — reage a uma mudança global de layers ativas, precisa recalcular
       visibilidade de relacionamento em toda cena, não só na visível.
     - **`updateLayersRects()`**: `clearSelection()` continua só na cena ativa (interação/seleção,
       categoria cena-local da decisão #9); `updateLayerRects()` (geometria) passa a rodar em
       todas as cenas — é chamado via `s_operationExecuted` após qualquer undo/redo, que pode
       ter alterado objetos de qualquer cena, não só a ativa.
     - `ModelWidget::updateSceneLayers()` ([modelwidget.cpp](libs/libgui/src/widgets/modelwidget.cpp))
       teve seu corpo extraído para o novo método privado `ModelWidget::applySceneLayers(ObjectsScene *scn)`
       (lê de `db_model`, escreve numa cena só); `updateSceneLayers()` virou um loop chamando
       `applySceneLayers()` para cada cena. Esse método novo é o helper de bootstrap que a Fase 1
       vai chamar para popular a `ObjectsScene` de um schema recém-criado (não foi ligado a nenhum
       call site novo ainda — só a infraestrutura, já que a criação de cena por schema é escopo da
       Fase 1). Depende de: 0.1.

0.7. **Exportação/impressão por cena** — decisão do usuário (2026-09-21): **sem
     composição espacial**. PNG/SVG geram **um arquivo por cena**; impressão continua **um único
     job/diálogo**, concatenando as páginas de todas as cenas em sequência (sem sobrepor
     espacialmente).
     - `ModelWidget::printModel()` ([modelwidget.cpp](libs/libgui/src/widgets/modelwidget.cpp)):
       loop externo por `scenes`, loop interno pelas páginas de cada cena
       (`scn->getPagesForPrinting(...)`). Numeração de página (`global_page`) é contínua através de
       todas as cenas (como um documento só); `h_pg_id`/`v_pg_id` (posição X/Y da página dentro da
       grade de páginas) reinicia a cada cena, já que é relativo ao layout de páginas daquela cena.
       `printer->newPage()` decidido por um flag `first_page` (chama antes de toda página exceto a
       primeira do job inteiro) em vez de "página < total-1", evitando pré-computar o total. Backup/
       restauração de grid, `setShowSceneLimits()` e `clearSelection()` passam a rodar em todas as
       cenas (não só a ativa) — aqui `clearSelection()` é modelo-inteiro-replicado (decisão #9), ao
       contrário do uso interativo: está preparando o output pra impressão, uma seleção residual em
       qualquer cena vazaria pro papel/PDF.
     - `ModelExportWidget::exportModel()` ([modelexportwidget.cpp](libs/libgui/src/tools/modelexportwidget.cpp)):
       branch de exportação de imagem (PNG/SVG) agora itera `model_wgt->getObjectsScenes()`, uma
       `QGraphicsView` nova por cena (mesma necessidade de hoje, ligada ao workaround de thread-
       affinity do Qt Graphics Scene). Nome do arquivo: com 1 cena só, usa o path exatamente como
       digitado (saída idêntica a hoje); com N>1, insere um sufixo `_<índice 1-based>` antes da
       extensão. **Limitação conhecida**: o sufixo é por índice posicional na coleção, não pelo
       nome do schema — não há ainda (Fase 0) nenhuma API que amarre cena a schema; melhorar para
       usar o nome do schema é candidato a ajuste quando a Fase 1 existir de fato.
     - **Gap de escopo encontrado, fora desta subtarefa**: `pgmodeler-cli` (exportação via linha de
       comando) tem seu próprio `ObjectsScene *scene`, construído direto do `DatabaseModel`
       ([pgmodelercliapp.h:99](libs/libcli/src/pgmodelercliapp.h#L99)), sem nenhuma relação com a
       coleção de cenas do `ModelWidget`. A segmentação por schema/threshold (Fase 1+) não vai
       valer automaticamente pra exportação via CLI — replicar o roteamento de cena lá é trabalho
       à parte, não coberto por este plano até agora. Registrado como risco (seção 6).
     - `AppearanceConfigWidget` ([appearanceconfigwidget.cpp:585](libs/libgui/src/settings/appearanceconfigwidget.cpp#L585))
       usa sua própria `ObjectsScene` de preview de estilo, sem relação com `ModelWidget` — conferido,
       não precisa de nenhuma mudança.
     - Caso N>1 cenas só é testável de verdade depois da Fase 1 popular a coleção — revisitar com
       `/home/raphael/dbm_models/big_model.dbm` quando a Fase 1 estiver pronta. Depende de: 0.1.

0.8. **Scaffolding de persistência** — reservar (sem popular/usar ainda) o modo de segmentação
     (nenhum/schema/threshold) no enum/atributo de `DatabaseModel`. O `SceneRect` por cena **não**
     entra aqui: virou seu próprio design fechado (ver "Achado de revisão de código, expandido em
     2026-09-21" na seção 2) — tag `<scenes><scene id="" left="" top="" right="" bottom="" />
     </scenes>` (`id` numérico é a chave), mirror de `changelog`/`<entry>`, DTD e `.sch` inclusos —
     e fica inteiramente na Fase 2 (subtarefas 2.2), já que só faz sentido calcular/persistir de
     verdade quando as cenas da Fase 1 existirem. `LastScene` (também numérico, mesma chave `id`)
     também fica para a Fase 2 (subtarefa 2.1). Depende de: nenhuma (independente do resto da
     fase).

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
  - **`Textbox` não pertence a nenhum schema** (achado durante a implementação da subtarefa 1.1,
    2026-09-22) — nem a chave "1 Schema = 1 cena" nem o contador da Fase 3 (que só conta Table/
    View/ForeignTable) cobrem esse tipo. **Decisão do usuário**: roteia sempre pra **cena ativa no
    momento da inserção** (`ModelWidget::scene`), nos dois modos de segmentação — casa com a
    expectativa de quem desenha um textbox olhando pra uma cena específica, e durante carga de
    arquivo cai naturalmente na cena inicial (nada troca a cena ativa durante o load).
    **Consideração futura registrada pelo usuário (2026-09-22)**: essa regra funciona sem
    persistência nova por ser, hoje, determinística na carga (a cena ativa durante o load é sempre
    a mesma, graças ao tratamento `pg_catalog`/`public` da 1.1) — mas isso só cobre o caso
    "recarregar sem editar". Dois casos que essa regra **não** cobre ainda, e que apontam pra um
    mecanismo comum: (a) **modo threshold** (Fase 3) já vai precisar de um atributo `Scene`
    persistido por objeto (subtarefa 3.2) — hoje redigido só pra Table/View/ForeignTable, mas
    `Textbox` provavelmente precisa do mesmo tratamento lá, já que não participa do contador; (b)
    **movimentação manual de um objeto entre cenas** (ainda sem UI/mecanismo desenhado em lugar
    nenhum do plano — diferente de `moveToSchema()`, que já existe pra tabelas com schema) também
    precisaria gravar explicitamente "esse objeto pertence à cena X" pra sobreviver a um
    save/reload, em vez de recair na regra implícita "cena ativa no momento". Ou seja, o mesmo
    atributo `Scene` por objeto que a Fase 3 vai introduzir é candidato natural a também resolver
    (a) e (b) — não é escopo da Fase 1 fechar isso agora, mas vale desenhar o atributo (3.2) já
    pensando em cobrir `Textbox` e reatribuição manual, não só o contador de threshold.

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
     **Tratamento especial pra `pg_catalog`/`public` (achado do usuário, 2026-09-22)**:
     `DatabaseModel::createSystemObjects()` sempre cria `pg_catalog` **antes** dos schemas do
     próprio arquivo serem parseados na carga (`public`, quando presente no arquivo, chega depois)
     — então `pg_catalog` seria o primeiro schema processado por `handleObjectAddition()` na
     imensa maioria dos casos (abrir um arquivo existente). Sem tratamento especial, a regra
     "ativa se for a primeira cena" faria a cena do `pg_catalog` — um schema raramente tocado
     diretamente pelo usuário, geralmente só populado via engenharia reversa — ficar presa ao
     `viewport` por padrão, escondendo o conteúdo real do usuário. `addScene()` ganhou um parâmetro
     `auto_activate` (default `true`); a condição de ativação virou "nenhuma cena ativa ainda"
     (`!scene`) em vez de "essa é a primeira cena" (`scenes.size() == 1`), pra uma cena poder abrir
     mão da ativação sem bloquear permanentemente a próxima de reivindicá-la. `pg_catalog` cria sua
     cena com `auto_activate = false`; `public` (schema padrão convencional) força sua própria cena
     a ficar ativa explicitamente enquanto o modelo está carregando
     (`db_model->isModelLoading()`), mesmo que outro schema tenha reivindicado o padrão primeiro —
     só dispara durante a carga, nunca rouba foco do usuário numa edição interativa depois.

1.2. **Rotear Table/View/ForeignTable para a cena do schema; `Textbox` pra cena ativa** — resolver
     `getSchema()` do objeto inserido e chamar `addItem()` na `ObjectsScene` daquele schema em vez
     da cena única. `Textbox` (sem schema) roteia pra `scene` (cena ativa no momento da inserção —
     ver decisão na intro da Fase 1). Depende de: 1.1.

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
     **Nota**: essa garantia vale só para o modo por schema (schema e cena são 1:1 por construção);
     no modo threshold (Fase 3) ela não vale mais — ver subseção "`SchemaView` cross-scene:
     supressão dinâmica" na Fase 3, achado à parte, não é bug desta subtarefa.
     **Implementação (2026-09-22)**: nenhuma mudança de código em `SchemaView` foi necessária —
     confirmado que o cálculo do retângulo usa posição absoluta (`obj_view->pos()`, sem
     `mapFromItem`/`mapToItem` entre cenas) e que 1.1/1.2 resolvem a cena de um schema e de seus
     filhos pelo mesmo lookup (`getSchemaScene()`), então não há caminho de código pra divergirem
     no modo por schema. A verificação virou um teste automatizado de verdade: novo target Qt Test
     `tests/src/modelwidgettest` (primeiro teste deste projeto a instanciar um widget GUI —
     `ModelWidget`/`QGraphicsView` — via a lib `gui` já linkada por todos os testes) usando a API
     pública `getSceneForObject()` (já existente desde a Fase 0). Exigiu reconfigurar a árvore de
     build com `-DBUILD_TESTS=ON` (estava desligado). Rodar a suíte inteira revelou falhas
     pré-existentes e não relacionadas em vários outros targets de teste (algumas simplesmente não
     compilam, uma até crasha) — confirmado via `git log` que não são desta sessão; fora de escopo,
     não corrigidas.

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

1.8.1. **Movimentação livre do cutoff + persistência de posição em XML** — complementa a 1.8:
       revisita a decisão de escopo reduzido tomada durante a implementação da 1.8 (marcador com
       posição fixa, ancorado ao título da tabela, sem persistência — "item efêmero", ver
       `MULTI_SCENE_PROGRESS.md`). Decisão do usuário (2026-09-22): o cutoff passa a poder ser
       movido livremente pelo usuário, e essa posição customizada deve sobreviver a salvar/reabrir
       o modelo, sem exigir um `BaseObject` novo dedicado ao cutoff.
       - **Onde persistir**: sem objeto de modelo novo. A posição fica num atributo na própria
         tabela/view/foreign table (`BaseTable`-derivado), pré-preenchido antes de gerar o XML
         daquele objeto — mesmo espírito de como `Position` (x/y da própria tabela) já é gravado
         hoje (`BaseTable::getCodeDefinition()`/`setPosition()`,
         [basetable.cpp:157-158](libs/libcore/src/basetable.cpp#L157)). Como existe **um só**
         `CutoffItem` por tabela (agrega todos os relacionamentos cross-scene dela — decisão #1,
         subtarefa 1.5), basta um atributo de posição por tabela, sem granularidade por
         relacionamento.
       - **Como sincronizar**: `CutoffItem::itemChange()` (`ItemPositionHasChanged`) escreve a
         posição atual de volta no atributo da tabela pai sempre que o usuário arrasta o marcador.
       - **Posição relativa vs. absoluta (fechado, 2026-09-22)**: relativa ao ponto de ancoragem
         (`getCutoffAnchorPoint()`), não absoluta de cena — preserva o comportamento de seguir a
         tabela automaticamente (`updatePosition()`) por padrão, só o desvio manual do usuário fica
         persistido. Quando o cutoff é destruído por promoção (1.9) e um novo nascer depois na
         mesma tabela, o offset salvo é **descartado** (não restaurado) — o novo cutoff nasce na
         âncora padrão.
       - **Formato XML (fechado, 2026-09-22)**: não são atributos simples nem um elemento novo com
         tag própria — é um **segundo `<position>`** dentro da tag de table/view/foreign table
         (que já emite um `<position>` para a posição da própria tabela), diferenciado por um
         atributo `id="cutoff"` (a posição da tabela não leva `id` — sua ausência já indica "é a
         posição do próprio objeto"). O DTD compartilhado de `<position>` (`baseelements.dtd`)
         ganhou `id CDATA #IMPLIED`; os DTDs de table/view/foreigntable passaram a aceitar um
         segundo `position?` opcional na sequência.
       - **Atualização 2026-09-23 (consolidação, achado de bug real)**: a primeira implementação
         evitou tocar `position.sch` (motor do `SchemaParser` exige todo atributo referenciado num
         `.sch` pré-declarado no mapa de QUALQUER chamador) criando um `.sch` dedicado
         (`cutoff-pos.sch`) com `id="cutoff"` fixo no literal. Essa duplicação escondeu um bug real:
         `BaseGraphicObject::setPositionAttribute()` (o método comum usado tanto pra posição própria
         quanto, por engano, reaproveitado pro cutoff) sempre serializava `this->position` — o x/y
         da própria tabela — não importa qual atributo/id fosse passado, então o `cutoff-pos`
         gravado no XML era sempre uma cópia do `<position>` normal da tabela, nunca refletindo o
         offset real do marcador arrastado. Corrigido consolidando pra um único caminho: `.sch`
         dedicado removido; `position.sch` ganhou `%if {id}...%end` de verdade, e
         `setPositionAttribute()` virou duas sobrecargas —
         `setPositionAttribute()` (mantém o comportamento antigo, serializa `position`) e
         `setPositionAttribute(const QPointF &pos, const QString &pos_attr, const QString &id = "")`
         (serializa um ponto explícito). A sobrecarga nova sempre atribui `attributes[Attributes::Id]
         = id` (vazio por padrão) antes de chamar o template — isso garante que `id` esteja
         pré-declarado no mapa em **todo** caminho que passa por `position.sch` agora (não só o do
         cutoff), eliminando o risco de `UnkownAttribute` que motivou o `.sch` separado na primeira
         versão. `PhysicalTable`/`View` chamam
         `setPositionAttribute(cutoff_pos, Attributes::CutoffPos, Attributes::Cutoff)` no lugar do
         antigo `BaseTable::setCutoffPositionAttribute()` (removido, junto com `cutoff-pos.sch`).
       Depende de: 1.8.

1.9. **Promoção/rebaixamento dinâmico** — `moveToSchema()`
     ([modelwidget.cpp:2538](libs/libgui/src/widgets/modelwidget.cpp#L2538)) (e qualquer outro
     ponto que reatribua schema de uma tabela) passa a reavaliar, para cada relacionamento da
     tabela movida, se o parceiro está na mesma cena após a mudança; cria/destrói `RelationshipView`
     e atualiza marcadores cut-off conforme a subseção "Relacionamentos cross-scene: promoção/
     rebaixamento dinâmico". Depende de: 1.8.
     - **Achado ao implementar, dividido em duas subtarefas (2026-09-23)**: o texto acima pressupõe
       que a tabela **já está** na cena nova quando `moveToSchema()` reavalia seus relacionamentos —
       mas `moveToSchema()`, antes desta subtarefa, só trocava o `Schema` do objeto e reposicionava
       o item graficamente perto da caixa do schema novo; nunca chamava `scene()->removeItem()`/
       `addItem()` pra migrar o item entre `ObjectsScene`s. `getSchemaScene()`/`schema_scenes` só
       eram consultados em `handleObjectAddition()` (inserção), nunca numa mudança de schema de um
       objeto já existente — ou seja, mover uma tabela de schema nunca movia seu item entre cenas.
       Sem isso, a premissa de "reavaliar se o parceiro está na mesma cena" nunca teria efeito. Duas
       subtarefas, um commit cada:
       - **1.9.1 — relocação de cena em `moveToSchema()`**: nova
         `ObjectsScene::relocateItem(item, dst_scene)` move o item pra outra cena preservando seu
         estado (sinais reconectados do zero por `dst_scene->addItem()`). Não reaproveita
         `removeItem()` puro porque este empilha o item em `removed_objs` pra deleção quando a cena
         **atual** for destruída — errado pra relocação, onde o item continua vivo, só numa cena
         diferente (deletaria um item ainda em uso quando a cena de origem fosse destruída depois).
         `removeItem()` foi dividido num `detachItem()` privado compartilhado (desconecta sinais,
         some da lista interna do Qt, sem tocar `removed_objs`) + o empilhamento em `removed_objs`,
         que `relocateItem()` pula. `moveToSchema()` passou a chamar `getSchemaScene(schema)` (cena
         destino) e `cur_scene->relocateItem(item, dst_scene)` quando a cena atual do item difere da
         destino.
       - **1.9.2 — promoção/rebaixamento de fato**: `ModelWidget::reevaluateTableRelationships()`
         (chamado logo após a relocação de 1.9.1) reavalia `connected_rels` (candidatos a
         rebaixamento, se o parceiro saiu da cena) e `external_rels` (candidatos a promoção, se o
         parceiro entrou na cena) da tabela movida, delegando pra `demoteRelationship()`/
         `promoteRelationship()`. **Bug real achado e corrigido nesta subtarefa**:
         `ObjectsScene::removeItem()` nunca limpava a referência reversa do objeto de modelo pra sua
         view (`BaseGraphicObject::receiver_object`) — ela só é limpa implicitamente quando a view é
         de fato deletada em C++, o que pra um item removido só acontece quando a **cena** dele é
         destruída (`removed_objs` é drenado em `~ObjectsScene()`). Isso nunca importou em nenhum
         caminho existente porque o objeto de modelo sempre morria junto com sua view (remoção real
         do modelo). Rebaixamento quebra essa suposição: o relacionamento sobrevive, só a
         `RelationshipView` é descartada — sem o fix, `BaseRelationship::getOverlyingObject()`
         continuaria apontando pra uma view prestes a ficar pendurada em vez de `nullptr`, violando
         silenciosamente o invariante que todo o trabalho de 1.6/1.7 (relacionamento cross-scene sem
         view formal) depende. Corrigido com `BaseObjectView::detachFromObject()` novo (limpa só a
         referência reversa, sem tocar a view em si), chamado de `ObjectsScene::removeItem()` logo
         depois de destacar o item — aplicado **incondicionalmente** ali (não só no caminho de
         rebaixamento), já que é uma correção geral, consistente em qualquer remoção de item, não um
         caso especial.

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

1.15. **Cor exclusiva para o marcador cut-off** — hoje `CutoffItem` reaproveita
      `Attributes::PositionInfo` (fill/border/font) pra desenhar a linha e o rótulo, sem ter
      identidade visual própria. Precisa de: (a) nova chave em `Attributes::` (libparsers) pra
      identificar o elemento; (b) entrada nova em `AppearanceConfigWidget::element_cmb`
      (`appearanceconfigwidget.cpp`, junto de `conf_ids`/`obj_conf_ids`) pra expor fill/border/font
      configuráveis pelo usuário, do mesmo jeito que os demais elementos gráficos; (c) atualizar o
      arquivo de configuração de aparência (tema padrão + persistência) com a chave nova; (d)
      trocar `CutoffItem` (linha, fundo do rótulo, texto, `obj_selection`) de `PositionInfo` pra
      essa chave nova. Depende de: 1.8.

> **Gate de aceite da fase (não é commit de código)**: modelo grande
> (`/home/raphael/dbm_models/big_model.dbm`) segmentado por schema deve navegar (usando o combo de
> 1.3), editar, remover (com e sem cascade) e desfazer/refazer corretamente, com conectores
> cut-off aparecendo/sumindo conforme tabelas mudam de schema. As três subtarefas 1.12-1.14 só
> entram depois de fechadas com o usuário (seção 6).

### Fase 2 — Persistência (modo de segmentação, cena focada)

Objetivo: fechar a segmentação por schema de ponta a ponta (carga, edição, salvamento, reabertura)
antes de sequer começar a olhar para threshold. Foco exclusivo em persistência do modo por schema
nesta fase — nada de atributo de bucket/threshold ainda (isso fica para a Fase 3, quando o modo
threshold for de fato detalhado). **A fase abre com a subtarefa 2.0** (fix mecânico do retângulo
de cena, sem persistência nova) antes de qualquer trabalho de persistência — a pedido do usuário,
por mexer numa base de métodos que 2.2 reaproveita depois.

- Novos atributos em `DatabaseModel` (mesmo padrão de `LastPosition`/`LastZoomFactor`):
  - Modo de segmentação do modelo (por ora só nenhum / por schema — deixar o valor reservado para
    "por threshold" no enum/atributo, mas sem implementar nada em cima disso ainda).
  - `LastScene`: identificador da cena focada (nome do schema, já que no modo por schema a chave
    natural é o próprio schema).
  - `getSceneRect()`/`setSceneRect()` ([databasemodel.h:898-899](libs/libcore/src/databasemodel.h#L898))
    hoje persistem um único `QRectF` no atributo legado `scene-rect`. Passa a existir também uma
    versão por-chave (mesma chave de `LastScene`), persistida numa tag `<scenes>` nova — design
    completo, incluindo o motivo de não usar um retângulo global e o formato XML exato (mirror de
    `changelog`/`entry`), na seção 2 ("Achado de revisão de código, expandido em 2026-09-21"). O
    atributo `scene-rect` legado continua sendo escrito como hoje sempre que há só 1 cena
    (segmentação desligada) — formato de arquivo inalterado nesse caso.
- Estender `ModelWidget::saveLastCanvasPosition()`/`restoreLastCanvasPosition()`
  ([modelwidget.cpp:972-1023](libs/libgui/src/widgets/modelwidget.cpp#L972)) para gravar/ler
  `LastScene` e, na restauração, ativar aquela cena **antes** de aplicar scroll/zoom salvos.
- **Critério de aceite da Fase 2**: fechar, salvar, reabrir um modelo segmentado por schema deve
  preservar cena focada + zoom/posição daquela cena, e a segmentação por schema deve se
  reconstruir corretamente na carga sem depender de nenhum estado extra persistido por tabela
  (a chave é o próprio schema de cada objeto, já persistido hoje).

#### Subtarefas da Fase 2 (uma por commit)

2.0. **Fix mecânico: retângulo de cada cena, não só da cena ativa** — primeira subtarefa da fase
     (a pedido do usuário, 2026-09-22), antes de qualquer persistência nova, porque mexe numa série
     de métodos ligados a retângulo de cena que 2.2 vai reaproveitar/estender depois. **Motivação**:
     confirmado por teste manual (ver seção 6 e nota na 2.2) que, com a Fase 1 já criando múltiplas
     cenas, toda cena que não é a ativa fica com o retângulo minúsculo padrão do construtor de
     `ObjectsScene` — `ModelWidget::adjustSceneRect(use_model_rect=true)`, chamado uma única vez ao
     final de `loadModel()` ([modelwidget.cpp:2051](libs/libgui/src/widgets/modelwidget.cpp#L2051)),
     só toca a cena ativa. **Escopo**: novo método `ModelWidget::adjustAllScenesRects()` (nome
     sugerido, chamado no lugar do `adjustSceneRect(true)` atual dentro de `loadModel()`) — aplica
     o único `scene-rect` legado à cena ativa como hoje (ainda sem persistência por-cena, isso é a
     2.2), e para **cada outra cena**, calcula o retângulo a partir dos seus próprios itens via
     `itemsBoundingRect(true, false)` — **nunca** `ObjectsScene::adjustSceneRect(false)` pra esse
     cálculo inicial, porque puxa a borda esquerda/superior pra origem incondicionalmente (achado
     já registrado na seção 2, "Cuidado real encontrado"). **Fora de escopo aqui**: os demais call
     sites de `adjustSceneRect(false)` (recalculo reativo a interação do usuário — mover objeto,
     etc., [modelwidget.cpp:3505,5467,5744,6044](libs/libgui/src/widgets/modelwidget.cpp#L3505) e
     dois em `mainwindow.cpp`) continuam cena-local, tocando só a cena ativa — não é bug, é
     categoria "cena-local" da decisão #9 (seção 3): mover um objeto numa cena não deveria mexer no
     retângulo de outra. O mesmo vale pra `ModelWidget::expandSceneRect()`/as ações de menu
     "expandir topo/esquerda/baixo/direita" — continuam tocando só a cena ativa, sem sincronizar as
     demais (reafirmado explicitamente com o usuário em 2026-09-22, ver seção 2).
     **Escopo adicional — retângulo "geral" pro atributo legado `scene-rect`** (adicionado
     2026-09-22, junto da reafirmação da decisão "sem retângulo global"): novo método
     `ModelWidget::updateModelSceneRectInfo()` (nome sugerido, mesmo padrão de
     `updateModelLayersInfo()`), chamado só no momento de salvar, que calcula a **união**
     (`QRectF::united()`) do `sceneRect()` atual de todas as cenas e grava em
     `db_model->setSceneRect(...)` — populando o atributo legado `scene-rect` com "como se todas as
     cenas fossem uma só", usado como fallback se o usuário desligar a segmentação depois. **Corrige
     um bug real introduzido pela própria subtarefa 0.1 (correção)**: o handler de
     `ObjectsScene::s_sceneRectChanged` já commitado dentro de `addScene()` hoje chama
     `db_model->setSceneRect(rect)` **reativamente, com o retângulo de uma única cena** toda vez que
     qualquer cena muda de tamanho — isso precisa ser removido dali (só sobra
     `viewport->resetCachedContent()`/`setModified(true)` nesse handler); o `scene-rect` do modelo
     só deve ser escrito pela nova lógica de união, e só no save, nunca reativamente por-cena.
     Depende de: 1.1, 1.2 (precisa de múltiplas cenas de verdade pra ter o que corrigir).

2.1. **Persistência de modo de segmentação + `LastScene`** — popular de fato (leitura/escrita XML)
     o scaffolding preparado em 0.8: modo de segmentação do modelo e `LastScene` (índice numérico
     da cena focada — não o nome do schema, ver decisão revisada na seção 2). Ainda sem uso em
     runtime neste commit — só grava/lê. Depende de: 0.8.

2.2. **Persistência de retângulo por cena** — implementação completa do design fechado na seção 2:
     - DTD dedicado novo [assets/schemas/xml/dtd/scene.dtd](assets/schemas/xml/dtd/scene.dtd) com
       `<!ELEMENT scenes (scene*)>`/`<!ELEMENT scene EMPTY>` (`id`/`left`/`top`/`right`/`bottom`),
       mirror exato de [changelog.dtd](assets/schemas/xml/dtd/changelog.dtd) (arquivo próprio, não
       declarações soltas em `dbmodel.dtd`);
       [assets/schemas/xml/dtd/dbmodel.dtd](assets/schemas/xml/dtd/dbmodel.dtd) ganha só
       `<!ENTITY % scene SYSTEM "scene.dtd"> %scene;` (mirror da linha do changelog,
       [dbmodel.dtd:89-90](assets/schemas/xml/dtd/dbmodel.dtd#L89)); mantém `scene-rect` como está.
     - Templates novos [assets/schemas/xml/scenes.sch](assets/schemas/xml/scenes.sch) (container) e
       [assets/schemas/xml/scene.sch](assets/schemas/xml/scene.sch) (entrada), mirror de
       `changelog.sch`/`entry.sch`; adicionar `%if {scenes} %then {scenes} %end` em
       [dbmodel.sch](assets/schemas/xml/dbmodel.sch), mirror do `%if {changelog}...` já existente.
     - `Attributes::Scenes`/`Attributes::Scene` novos em
       [attributes.h](libs/libparsers/src/attributes.h) (`Id`/`Left`/`Top`/`Right`/`Bottom` já
       existem).
     - `DatabaseModel`: `QMap<int, QRectF>` indexado por `id` (mesma chave de `LastScene`); escrita
       mirror de `getChangelogDefinition()` ([databasemodel.cpp:10520-10555](libs/libcore/src/databasemodel.cpp#L10520)),
       leitura mirror do parse de `<changelog>` ([databasemodel.cpp:3647-3664](libs/libcore/src/databasemodel.cpp#L3647)).
       `id` escrito é a posição da cena na `QList<ObjectsScene *> scenes` em memória no momento do
       save. Quando há só 1 cena (segmentação desligada), continua escrevendo só o `scene-rect`
       legado, sem `<scenes>` — formato de arquivo inalterado nesse caso.
     - Carga de arquivo antigo (sem `<scenes>`) num modelo que a Fase 1 segmenta em N cenas: cada
       cena cai no fallback já existente (`itemsBoundingRect()` a partir dos seus próprios itens) —
       não tentar redistribuir o `scene-rect` legado entre as cenas novas.
     - **Não implementar** nenhuma checagem de validação cruzada na carga (ex. detectar
       desalinhamento de `id` pós "Edit creation order" e cair pro fallback) — decisão do usuário:
       aceitar esse desalinhamento como limitação documentada (ver seção 2), não como algo a
       mitigar em código.
     - **Confirmado por teste manual (2026-09-22, usuário)**: com múltiplas cenas já criadas pela
       Fase 1, o retângulo de cada cena que não é a ativa fica errado hoje — não é mais só um risco
       teórico do design fechado na seção 2. O fix mecânico (iterar todas as cenas, sem depender
       ainda da persistência) virou a subtarefa **2.0**, a primeira da fase — ver lá para a causa
       exata e o escopo. Esta subtarefa (2.2) reaproveita/estende o método que a 2.0 introduz,
       trocando o recomputo via `itemsBoundingRect()` (fallback de arquivo antigo) pela leitura do
       valor persistido por-cena quando disponível.
     Depende de: 2.0, 0.8, 1.1 (precisa que cenas por schema já existam de verdade para ter o que
     persistir por cena).

2.3. **Wiring runtime de `LastScene`** — estender
     `ModelWidget::saveLastCanvasPosition()`/`restoreLastCanvasPosition()`
     ([modelwidget.cpp:972-1023](libs/libgui/src/widgets/modelwidget.cpp#L972)) para gravar o
     índice da cena ativa ao salvar e, ao restaurar, ativar aquela cena por índice (via
     `setActiveScene()`, 0.2) **antes** de aplicar zoom/scroll salvos. Depende de: 2.1, 0.2.

2.4. **Fallback de `LastScene` inválido** — se o índice referenciado por `LastScene` estiver fora
     dos limites da coleção de cenas na carga do arquivo (ex. cena removida em edição posterior ao
     save, deixando `LastScene` apontando além do fim da lista), cair para a primeira cena
     disponível em vez de falhar. Depende de: 2.3, 1.10 (cenário de schema removido já precisa
     existir para este fallback fazer sentido).

2.5. **Teste de regressão fim-a-fim da Fase 2** — fechar, salvar, reabrir um modelo segmentado por
     schema preserva cena focada + zoom/posição daquela cena, e a segmentação por schema se
     reconstrói corretamente na carga. **Inclui retrocompatibilidade** (requisito explícito do
     usuário): abrir um `.dbm` salvo por uma versão anterior a esta feature (só `scene-rect` legado,
     sem `<scenes>`) precisa continuar funcionando — cada cena calcula seu retângulo pelo fallback
     existente, sem erro nem tentativa de redistribuir o valor legado. Depende de: 2.2, 2.3.

### Fase 3 — Segmentação por Threshold (algoritmo fechado, detalhes de UX/borda a refinar)

> Deliberadamente deixada por último. O modo por schema (Fases 1-2) precisa estar "redondo" —
> implementado, testado, com os pontos de estudo dedicado (ferramenta de relacionamento, seleção,
> Object Search Widget) resolvidos — antes de investir tempo em detalhar o modo threshold.
> **Atualização 2026-09-21**: o algoritmo central (decisão #7, seção 3) já está fechado — contador
> ingênuo, sem clustering, com persistência por objeto obrigatória desde o início. O que ainda
> falta detalhar são os pontos de borda (fallback de contador entre sessões de edição, formato
> exato do atributo de cena por objeto) — bem menos em aberto do que a versão anterior desta seção,
> mas ainda assim revisitar antes de codar 3.1 em diante.

- Contador de objetos inseridos (Table + View + ForeignTable, sem distinção de tipo, cf.
  especificação original). Ao cruzar um múltiplo do threshold, abre-se uma nova `ObjectsScene` e as
  inserções seguintes vão para ela. Mesmo algoritmo serve tanto para criação interativa quanto para
  carga de arquivo — não há mais um caminho especial de "carga em lote com visão do grafo
  completo", porque não há clustering (decisão #7, seção 3). `Textbox` não entra no contador
  (mesma decisão da Fase 1) — roteia pra cena ativa, igual ao modo por schema.
- Relacionamentos cross-scene podem ser numerosos neste modo (trade-off aceito pela decisão #7) —
  a única mitigação **dentro desta fase** é reutilizar o mecanismo de marcador cut-off da Fase 1
  sem modificação. Reduzir a quantidade de cross-scene algoritmicamente (clustering) não é escopo
  desta fase — fica reservado como um terceiro modo de segmentação separado e futuro (seção 5), não
  uma evolução deste modo ingênuo.
- Política de rebalanceamento: **não** mesclar/destruir cenas automaticamente quando uma fica
  esvaziada por remoções — diferente do modo por schema (onde a cena é destruída porque está
  atrelada ao ciclo de vida de um `Schema`), no modo threshold a cena não tem nenhum objeto-dono
  cujo removal sirva de gatilho, então ela simplesmente permanece vazia até (se algum dia)
  receber novas inserções. Simplificação deliberada.
- **Persistência por objeto é obrigatória desde o primeiro commit desta fase** — não é mais uma
  consequência condicional de "se houver clustering", já que não há clustering nenhum; a razão
  agora é outra e mais direta: o contador é um processo com estado que precisa ficar "travado" ao
  salvar. Cada `Table`/`View`/`ForeignTable` ganha um atributo persistido `Scene` (nome de
  arquivo) indicando a qual cena pertence, análogo ao atributo de layer já persistido por objeto
  hoje. Dois motivos práticos para persistir em vez de recalcular relendo o arquivo:
  1. Se objetos forem removidos entre uma sessão de edição e outra, re-derivar a atribuição do zero
     exigiria reconstituir a ordem histórica de inserção original — frágil comparado a só ler o
     resultado já gravado.
  2. Deixa o caminho livre, sem nenhum desenho extra, para o caso futuro de mover manualmente um
     objeto de uma cena para outra — o atributo persistido já é a fonte de verdade a ler/escrever,
     exatamente como o atributo de layer por objeto já funciona hoje.
- **Ponto ainda em aberto**: como o contador em si (não só a atribuição já feita) sobrevive entre
  sessões de edição — ex. usuário abre o arquivo, a cena mais recente ainda tem espaço sobrando no
  threshold, cria mais 3 tabelas manualmente: elas devem continuar enchendo aquela cena ou sempre
  abrir uma nova? Precisa decidir se o contador persiste seu valor corrente (provavelmente como
  parte do mesmo mecanismo de atributo de segmentação da Fase 2) ou se é recalculado a partir da
  contagem de objetos já atribuídos a cada cena na carga do arquivo.

#### `SchemaView` cross-scene: supressão dinâmica (achado do usuário, 2026-09-21)

Problema exclusivo do modo threshold — **não afeta a Fase 1** (lá, schema e cena são 1:1 por
construção, já coberto pela subtarefa 1.4). No modo threshold, os buckets são por ordem de
inserção, sem noção de schema: duas tabelas do mesmo schema podem perfeitamente cair em cenas
diferentes.

- **O problema**: `SchemaView::fetchChildren()`/`configureObject()`
  ([schemaview.cpp:79-99, 175-230](libs/libcanvas/src/schemaview.cpp#L79)) calcula o retângulo do
  schema varrendo **todas** as tabelas daquele schema via `DatabaseModel::getObjects(type, schema)`,
  sem olhar em qual cena cada uma está. Um `SchemaView` é um `QGraphicsItem` — só existe em **uma**
  cena. Se os filhos do schema estão espalhados em cenas diferentes, um retângulo desenhado numa
  cena só ficaria geometricamente errado (delimitando uma área onde parte do conteúdo real nem está
  presente) — o mesmo problema de fundo que tornou o `RelationshipView` inviável entre cenas
  (decisão #1, seção 3).
- **Solução (mesma filosofia da decisão #1)**: em vez de tentar renderizar um retângulo
  parcial/composto entre cenas, **suprimir completamente** o `SchemaView` de um schema sempre que
  seus filhos vivem em mais de uma cena — nenhum item "meio existente" escondido em alguma cena
  arbitrária. Custo baixo: o nome qualificado por schema continua aparecendo no cabeçalho de cada
  tabela, só a moldura visual de agrupamento some.
  - **Promoção** (schema tinha filhos em >1 cena, todos convergiram pra uma só — ex. depois de uma
    futura movimentação manual entre cenas): cria/reativa o `SchemaView` normalmente naquela cena,
    igual ao comportamento de hoje.
  - **Rebaixamento** (schema tinha todos os filhos numa cena, um deles foi pra outra): destrói/
    remove o `SchemaView` daquele schema.
  - Reavaliado no mesmo choke point de mudança de cena de um objeto que já dispara a lógica
    equivalente pra relacionamentos (promoção/rebaixamento, subseção da Fase 1) — não precisa de
    mecanismo de detecção novo, só estender o que já existe pra também considerar o schema da
    tabela movida, não só seus relacionamentos.
  - **Não otimizado ainda**: a verificação "todos os filhos deste schema estão na mesma cena?" pode
    começar como um re-scan completo dos filhos do schema afetado (não do modelo inteiro) a cada
    mudança — aceitável para começar; otimizar (ex. contador incremental por schema) só se motivado
    por medição real de performance.

#### Subtarefas da Fase 3 (a refinar quando a fase for retomada, mas algoritmo já fechado)

3.1. Contador ingênuo (Table + View + ForeignTable) + criação de nova cena ao cruzar o threshold —
     mesmo algoritmo para criação interativa e carga de arquivo. Depende de: 1.1, 1.2, 1.5, 1.6
     (reaproveita o choke point de criação de cena e de roteamento).

3.2. Persistência do atributo `Scene` por objeto (novo atributo por tabela/view/ftable, análogo ao
     atributo de layer já persistido por objeto) + resolução do "ponto ainda em aberto" acima (como
     o contador sobrevive entre sessões). **Escopo a revisitar** (achado do usuário na Fase 1,
     2026-09-22): avaliar se esse mesmo atributo deve cobrir também `Textbox` (que não entra no
     contador de threshold, hoje roteado só pela cena ativa no momento da inserção — ver nota na
     intro da Fase 1) e reatribuição manual de qualquer objeto entre cenas (mecanismo ainda não
     desenhado em nenhum modo) — os três casos (threshold, `Textbox`, reatribuição manual)
     precisam da mesma pergunta de fundo respondida: "essa cena persistida sobrevive a um
     save/reload independente da ordem de carga?" Depende de: 3.1.

3.3. Reaproveitamento do marcador cut-off da Fase 1 (1.8) para os cross-scene resultantes no modo
     threshold — deve ser praticamente gratuito se 1.8 foi bem desenhado; se não for, é sinal de
     que o desenho de 1.8 vazou detalhes específicos do modo por schema. Depende de: 3.1, 1.8.

3.4. Documentar e validar com teste a política de não-rebalanceamento (cenas esvaziadas por
     remoção não são mescladas/destruídas automaticamente no modo threshold) — verificação, não
     feature nova. Depende de: 3.1.

3.5. Supressão dinâmica de `SchemaView` cross-scene — estender o choke point de promoção/
     rebaixamento (já usado pra relacionamentos) para também reavaliar, ao mover uma tabela de
     cena, se o schema dela ainda tem todos os filhos numa única cena; criar/destruir o
     `SchemaView` conforme a subseção acima. Depende de: 3.1.

### Fase 4 — Paridade do `pgmodeler-cli` com a segmentação multi-cena

> **Tarefa pendente, ainda não iniciada.** Encontrada na subtarefa 0.7, registrada aqui a pedido
> do usuário. Ordem de trabalho definida por ele: só começar depois que as Fases 0-3 (o lado GUI,
> via `ModelWidget`) estiverem inteiramente acertadas. Dono: usuário ("Eu farei a revisão da CLI
> assim que tivermos tudo acertado na GUI", 2026-09-21) — não iniciar por conta própria antes
> disso.

**O problema**: `PgModelerCliApp` monta e usa seu próprio `ObjectsScene *scene`
([pgmodelercliapp.h:99](libs/libcli/src/pgmodelercliapp.h#L99)), construído direto a partir do
`DatabaseModel`, sem nenhuma relação com `ModelWidget` ou com a coleção `scenes` que as Fases 0-3
constroem. Toda a lógica de segmentação (criar cena por schema, rotear objeto pra cena certa,
promoção/rebaixamento de relacionamento cross-scene, marcador cut-off) vive em
`ModelWidget::handleObjectAddition()`/métodos correlatos — o CLI nunca passa por esse choke point,
então nunca vai ganhar segmentação nenhuma só por causa do trabalho feito nas Fases 0-3. Impacto
concreto hoje: `pgmodeler-cli --export-to-file` para PNG/SVG ([pgmodelercliapp.cpp:2274,2285](libs/libcli/src/pgmodelercliapp.cpp#L2274))
sempre produz uma única imagem do modelo inteiro, mesmo depois da GUI já segmentar em múltiplas
cenas.

**O que precisa ser decidido/feito quando esta fase for retomada** (não detalhado agora, só
mapeado):
- Extrair a lógica de roteamento objeto→cena de `ModelWidget` para um lugar reutilizável por quem
  não é `ModelWidget` (hoje ela está misturada com bastante código específico de GUI — criação de
  `QGraphicsView`, sinais Qt de interação, etc. — precisa avaliar o que é genuinamente core vs. o
  que é GUI-only antes de extrair).
  ou
- Reimplementar a segmentação de forma independente dentro de `PgModelerCliApp`, aceitando alguma
  duplicação de lógica em troca de não mexer no caminho da GUI.
- Decidir se `--export-to-file` para imagem passa a gerar um arquivo por cena (mesmo
  padrão da subtarefa 0.7 na GUI) ou se o CLI ganha uma flag explícita pra optar por isso.
- Cobre os dois modos (por schema e por threshold) ou só o que estiver pronto no momento em que
  isso for retomado.

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
- **Sugestão automática de modo de segmentação** (achado do usuário, 2026-09-21, confirmado
  testando `big_model.dbm`): esse modelo tem 4 schemas (`pg_catalog`, `public`, `dv4`, `dvaudit`),
  mas só `dv4` tem objetos — a lentidão original persiste mesmo com a Fase 1 (por schema) já
  implementada, porque toda a densidade cai numa única cena. Confirma que os dois modos base são
  complementares, não substitutos: por schema ajuda quando os objetos já estão distribuídos entre
  vários schemas; `big_model.dbm` é justamente o caso oposto, candidato natural a threshold (Fase
  3). Ideia registrada para depois de threshold estar pronto: o pgModeler poderia detectar esse
  padrão (poucos schemas populados concentrando a maior parte dos objetos) e sugerir ativamente ao
  usuário o modo de segmentação mais adequado, em vez de deixar a escolha inteiramente manual. Não
  é escopo das Fases 1-3 (que só fecham os dois modos base) — é uma camada de UX por cima deles.
- **Terceiro modo de segmentação: threshold com agrupamento ciente de conectividade** (BFS/
  union-find sobre o grafo de relacionamentos, para minimizar cross-scene mantendo clusters
  fortemente conectados no mesmo bucket). Não descartado — é um modo candidato genuíno, só fica
  para depois: primeiro por schema (Fases 1-2) e threshold ingênuo (Fase 3) precisam estar redondos
  e em uso. Quando for retomado, tratar como um modo de segmentação novo e independente (não uma
  flag/variação do threshold ingênuo da Fase 3, que precisa continuar existindo e previsível como
  está — ver decisão #7, seção 3), já que muda a proposta de valor: o ingênuo prioriza ordem
  previsível dos objetos, o com clustering prioriza minimizar cross-scene à custa de reordenar
  objetos entre cenas de um jeito não literal à ordem de inserção.

## 6. Riscos conhecidos / pontos de atenção para quem retomar

- **Retângulo de cena errado pra toda cena que não está ativa** (confirmado por teste manual do
  usuário, 2026-09-22, com a Fase 1 já criando cenas por schema) — `adjustSceneRect()` e seus call
  sites só tocam a cena ativa hoje, nunca as demais; cenas recém-criadas ficam com o retângulo
  minúsculo padrão do construtor. Vira a subtarefa **2.0**, primeira da Fase 2 (a pedido do
  usuário, por mexer numa base de métodos que a 2.2 também usa) — não corrigir fora dessa
  subtarefa.
- **Vários targets de `tests/` (fora do multi-scene) estão quebrados hoje** (achado ao rodar a
  suíte inteira pela primeira vez pra validar a subtarefa 1.4, já que `BUILD_TESTS` estava
  desligado até então) — `fileselectortest`/`linenumberstest` nem compilam (header ausente/
  assinatura de função mudou), e vários outros (`basefunctiontest`, `foreigndatawrappertest`,
  `proceduretest`, `schemaparsertest`, `servertest`, `transformtest`, `xmlparsertest`) têm
  asserções falhando; `syntaxhighlightertest` chega a crashar. Confirmado via `git log` que nenhum
  desses arquivos foi tocado nesta feature — bit-rot pré-existente, não relacionado ao multi-scene.
  Fora de escopo deste plano; só registrado aqui pra não se perder.
- **`pgmodeler-cli` não participa da segmentação multi-cena** (achado na subtarefa 0.7) — virou
  tarefa própria, **Fase 4**, dona: usuário, só depois das Fases 0-3 estarem prontas. Não
  implementar por conta própria antes disso ser pedido.
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
