# Progresso — Suporte a Múltiplas Cenas

> Log de execução do `MULTI_SCENE_PLAN.md`, atualizado sessão a sessão. Leia primeiro o plano
> (arquitetura, decisões fechadas, subtarefas) — este arquivo é só "onde paramos e por quê",
> para retomar rápido sem reler todo o histórico de commits.

## Estado atual

- Branch: `multi-scene-support-2.1.x`.
- **Fase 0 concluída (0.1 a 0.8).** Fase 1: **subtarefas 1.1 a 1.8 concluídas, testadas
  manualmente pelo usuário e commitadas** (1.2 recebeu um refinamento pequeno de deduplicação logo
  depois). Próxima: **1.9** (promoção/rebaixamento dinâmico de relacionamento cross-scene, ver
  plano).
- **Nova subtarefa registrada no plano (2026-09-22): 1.15 — cor exclusiva para o marcador
  cut-off.** Hoje `CutoffItem` reaproveita `Attributes::PositionInfo`; usuário pediu identidade
  visual própria, o que implica chave nova em `Attributes::`, entrada nova em
  `AppearanceConfigWidget::element_cmb`/`conf_ids`/`obj_conf_ids` e atualização do arquivo de
  configuração de aparência. Só registrada, sem código ainda. Depende de: 1.8 (já feita).
- **Achado importante confirmado por teste manual (2026-09-22)**: retângulo de cada cena está
  errado hoje pra toda cena que não é a ativa (`adjustSceneRect()` só toca a cena ativa). Virou
  **subtarefa 2.0**, primeira da Fase 2 (a pedido do usuário) — plano reorganizado, ver seção 2,
  subtarefa 2.0/2.2 e seção 6 (riscos). Design ampliado no mesmo dia (ver abaixo) — usuário
  reabriu a discussão da decisão "sem retângulo global" e fechamos um híbrido. Ainda não
  implementada (nem 2.0 nem esse híbrido).
- **1.5 concluída e commitada** (`f6a1a623c`): `BaseTableView` ganhou `external_rels`
  (`QList<BaseRelationship *>`, container Qt por ser código novo) e o par de mutators
  `addExternalRelationship()`/`removeExternalRelationship()` (protected), espelhando a validação
  de `addConnectedRelationship()`/`removeConnectedRelationship()`. Leitura pública via
  `getExternalRelsCount()`/`getExternalRelationships()`. `ModelWidget` virou `friend class` porque,
  diferente de `connected_rels` (só `RelationshipView` mexe), quem popula essa lista em 1.6 é
  `ModelWidget::handleObjectAddition()` — não existe `RelationshipView` pra relacionamento
  cross-scene. Só estrutura + API nesta subtarefa, sem desenho visual (isso é 1.8).
- **1.6 concluída e commitada** (`ad284cc4c`): case `Relationship`/`BaseRelationship` em
  `handleObjectAddition()` ([modelwidget.cpp:1130](libs/libgui/src/widgets/modelwidget.cpp#L1130))
  agora resolve a `ObjectsScene` de cada uma das duas tabelas via `getOverlyingObject()` +
  `scene()`. Mesma cena → cria `RelationshipView` normalmente (comportamento inalterado,
  cobre também auto-relacionamento, já que src/dst são a mesma tabela/cena). Cenas diferentes →
  não cria `RelationshipView` nenhum, só chama `addExternalRelationship()` (1.5) nas duas
  `BaseTableView`; `graph_obj->getOverlyingObject()` fica `nullptr` pra esse relacionamento (a
  exceção legítima que 1.7 vai auditar). Efeito colateral necessário: o bloco final da função (que
  fazia `addItem`/`setModified`/`emit s_objectAdded`/`centerOn` tudo dentro de `if(item)`) foi
  reestruturado — `setModified()` e `emit s_objectAdded()` agora rodam sempre que um objeto é
  processado, mesmo sem item gráfico (senão um relacionamento cross-scene não marcaria o modelo
  como modificado); só `addItem`/`centerOn` continuam condicionados a `item != nullptr`. Testado:
  build de `canvas`/`gui`/`pgmodeler` limpo, e a suíte `modelwidgettest` (1.4) continua passando —
  ainda não cobre relacionamento cross-scene (isso fica pra 1.11, teste de regressão dedicado, ou
  pode virar um teste próprio antes disso). Teste manual: com o combo de 1.3, criar tabelas em
  schemas/cenas diferentes e ligá-las por relacionamento — sem `RelationshipView` visível, sem
  crash; ligar duas tabelas do mesmo schema continua desenhando a linha normalmente.
- **1.7 implementada e buildada, aguardando teste manual do usuário antes de commitar** — 4 fixes
  de null-check em call sites de `getOverlyingObject()` que crashavam com relacionamento
  cross-scene. Ver detalhamento abaixo, em "O que foi feito". Depois de aprovada, próxima é a
  **1.8** (marcador visual "cut-off").
- **Pausa para review (2026-09-21)**: usuário testou `big_model.dbm` (o modelo denso usado como
  referência do problema original de lentidão) com a Fase 1 (por schema) até 1.6 aplicada. Resultado
  esperado, não um bug: a lentidão persiste porque esse modelo tem 4 schemas
  (`pg_catalog`/`public`/`dv4`/`dvaudit`) mas só `dv4` tem objetos — segmentação por schema não
  distribui nada nesse caso, toda a densidade cai numa cena só. Confirma que este modelo é candidato
  a threshold (Fase 3), não por schema. Ideia nova do usuário registrada na seção 5 do plano:
  pgModeler poderia detectar esse padrão e sugerir o modo de segmentação mais adequado — fica para
  depois de threshold existir, fora do escopo das Fases 1-3.
- **Bug encontrado e corrigido na mesma pausa (2026-09-21)**: usuário reportou que "Select all" (menu
  de contexto) selecionava objetos em todas as cenas, enquanto limpar seleção afeta só a cena ativa —
  comportamento assimétrico, quebrando a invariante "seleção é local à cena ativa" (decisão #9, seção
  3, que já classifica `selectAllObjects()` como cena-local). Causa raiz:
  `ModelWidget::selectAllObjects()` ([modelwidget.cpp:1600](libs/libgui/src/widgets/modelwidget.cpp#L1600))
  tem dois caminhos — o genérico "All objects" (`ObjectType::BaseObject`) já usava
  `scene->setSelectionArea()` (cena ativa, correto); mas os itens específicos do submenu (Schemas,
  Tables, Views, Foreign Tables, Relationships, Textboxes) iteravam
  `db_model->getObjectList(obj_type)` — **todo o modelo**, sem filtrar por cena — e chamavam
  `obj_view->setSelected(true)` direto em cada um. Achado na subtarefa 0.5 (ver nota acima): o plano
  original desta função **nunca foi corrigido**, só deixado como estava (assumia 1 cena só) enquanto
  a decisão #9 dizia que deveria ser cena-local. `scene->clearSelection()` (usado em ~20 pontos do
  arquivo) já estava correto — só afeta a cena ativa nativamente via Qt, por isso a assimetria.
  **Correção**: adicionado filtro `obj_view->scene() == scene` antes de selecionar
  ([modelwidget.cpp:1634](libs/libgui/src/widgets/modelwidget.cpp#L1634)) — objetos de outras cenas
  ficam intocados, igual ao caminho "All objects". Não é a mesma coisa que a subtarefa 1.13 (tensão
  em `selectTableRelationships()`/`selectTaggedTables()`, que **propositalmente** podem cruzar cenas
  seguindo relacionamento/tag) — este é só o caso mecânico "selecionar todos os objetos de um tipo",
  que já devia ser cena-local desde a Fase 0. Build de `gui`/`pgmodeler` limpo, `modelwidgettest`
  (1.4) sem regressão.
- **Segundo bug relacionado, mesmo teste manual (2026-09-21)**: o filtro por cena acima resolveu o
  caso de uma cena isolada, mas o usuário então reportou o segundo sintoma: trocar de cena e repetir
  "Select all > Tables" seleciona corretamente a nova cena, mas **não limpa** a seleção da cena
  anterior — objetos ficam selecionados em duas cenas simultaneamente. Causa raiz:
  `ModelWidget::setActiveScene(int idx)` ([modelwidget.cpp:1095](libs/libgui/src/widgets/modelwidget.cpp#L1095))
  nunca limpava a seleção da cena de saída antes de trocar `viewport->setScene()` — exatamente o
  cenário descrito na subseção "Seleção é sempre local à cena ativa" (plano, ver `setActiveScene()`),
  que já recomendava isso como a correção-base (a parte "decisão recomendada", não a parte em aberto
  da subtarefa 1.13 — a tensão com `selectTableRelationships()`/`selectTaggedTables()`/Object Search
  Widget segue sem decisão, essas sim propositalmente cross-scene). **Correção**: adicionado
  `if(scene) scene->clearSelection();` logo no início de `setActiveScene()`, antes de reatribuir
  `active_scene_idx`/`scene` — dispara `selectionChanged`/`s_objectSelected` pros itens que estavam
  selecionados, mesmo mecanismo já usado nos ~20 outros call sites de `clearSelection()`. Build de
  `gui`/`pgmodeler` limpo, `modelwidgettest` sem regressão. Os dois fixes de seleção acima foram
  commitados juntos (`2afc4023d`, "Fix: selection must stay local to the active scene").
- **Terceiro bug reportado na mesma pausa (2026-09-21)**: usuário viu no console, ao só *carregar*
  um modelo multi-cena sem nenhuma interação (`demo.dbm`), o aviso
  `QGraphicsScene::removeItem: item 0x...'s scene (0x...) is different from this scene (0x...)`.
  Causa raiz: `ModelWidget::handleObjectRemoval()`
  ([modelwidget.cpp:1310](libs/libgui/src/widgets/modelwidget.cpp#L1310)) fazia
  `scene->removeItem(...)` usando o ponteiro de conveniência da cena **ativa**, e não a cena real do
  item sendo removido — mesma classe de bug do `selectAllObjects()`/`setActiveScene()` acima
  (código pré-multi-cena que sobrou sem ser generalizado). Sempre que o objeto removido não estiver
  na cena ativa no momento (ex. remoções que acontecem durante o parsing/carga do XML, antes do
  usuário interagir), o Qt recusa o `removeItem` porque o item pertence a outra `QGraphicsScene` —
  daí o aviso, mesmo sem nenhuma ação do usuário. Isso também é sério porque
  `ObjectsScene::removeItem()` é overridden ([objectsscene.cpp:1054](libs/libcanvas/src/objectsscene.cpp#L1054))
  e faz limpeza importante (`disconnectTables()` em relacionamentos, `updateLayerRects()`) que
  **não roda** quando o `removeItem` é rejeitado silenciosamente pela cena errada — ou seja, não é só
  um aviso cosmético, é limpeza de estado que estava sendo pulada. **Correção**: resolve a cena real
  do item via `item->scene()` (dynamic_cast pra `ObjectsScene`) e chama `removeItem()` nela, não na
  cena ativa. Build de `gui`/`pgmodeler` limpo, `modelwidgettest` sem regressão; smoke test
  headless (`QT_QPA_PLATFORM=offscreen ./pgmodeler assets/samples/demo.dbm`) não reproduziu o aviso
  depois do fix (não é confirmação definitiva — `PGMODELER_DEBUG` não estava ligado nesse build pra
  confirmar que o carregamento completou antes do timeout, mas o aviso do Qt em si não é gated por
  nenhuma flag de debug, então a ausência dele já é um sinal razoável). Não commitado ainda —
  aguardando teste manual do usuário.
- **Convenção a partir de 2026-09-21**: `MULTI_SCENE_PLAN.md` fica só com o plano em si
  (arquitetura, decisões, descrição/dependência das subtarefas) — nunca marca subtarefa como
  feita/commitada. Todo status de execução (o quê foi feito, hash do commit, achados específicos
  de sessão) vive só aqui neste arquivo, atualizado junto de cada commit de subtarefa.
- **Convenção a partir de 2026-09-22**: ao terminar de implementar e validar (build) cada
  subtarefa, parar e deixar o usuário testar manualmente antes de prosseguir pra próxima — não só
  perguntar "commitar ou seguir", mas dar tempo/espaço pro teste manual entre elas.

> A branch já foi recriada uma vez nesta iniciativa (histórico anterior perdido, plano
> recommitado a partir da cópia em disco) — se o `git log` um dia parecer curto demais pro que
> se espera, é por isso. Este arquivo (e o `MULTI_SCENE_PLAN.md`) são a fonte de verdade sobre
> decisões e progresso; a lista de commits em si é o `git log` — não duplicada aqui.

## O que foi feito

- **0.1** — `ModelWidget::scene` (único) virou `QList<ObjectsScene *> scenes` + `active_scene_idx`;
  `scene` continua existindo como ponteiro de conveniência pra cena ativa. API nova:
  `getObjectsScene(int idx)`, `getObjectsScenes()`, `getSceneCount()`, `getSceneForObject()`.
  Refinamento: a criação de cena foi extraída pro método privado `ModelWidget::addScene()` (cria,
  instala event filter, adiciona a `scenes`, só ativa se for a primeira — pra Fase 1 poder criar
  cena por schema sem roubar o foco da cena atual do usuário).
- **0.2** — `ModelWidget::setActiveScene(int idx)` (slot público) troca `viewport->setScene()`,
  reaplica o zoom atual, emite `s_activeSceneChanged(int)`. Guarda `idx == active_scene_idx` faz
  disso um no-op com 1 cena só.
- **0.3** — `ModelWidget::eventFilter()` trocou a comparação `object == scene` por
  `scenes.contains(qobject_cast<ObjectsScene *>(object))`, pra não depender implicitamente de
  "a única cena é sempre a ativa" quando N>1.
- **0.4** — `ModelOverviewWidget::connectToActiveScene()` (método privado) é o único ponto que
  desconecta da cena anterior (rastreada no membro `connected_scene`) e reconecta
  `selectionChanged`/`sceneRectChanged` na cena ativa, chamando `updateOverview(true)`. Chamado em
  `show()` e via `s_activeSceneChanged`.
- **0.5** — escopo revisado em relação à redação original do plano (ver decisão #9 abaixo). Único
  código novo: `ModelWidget::alignObjectsToGrid()` (público), itera `scenes` chamando
  `ObjectsScene::alignObjectsToGrid()` em cada uma; substituiu os 4 acessos diretos a
  `scene->alignObjectsToGrid()` (3 em `MainWindow`, 1 dentro de `ModelWidget::adjustSceneRect()`).
  `selectAllObjects()`, os `itemsBoundingRect()` de `emitSceneInteracted()`/`eventFilter()` e o
  gate `items().count() > 1` **não foram alterados** — usuário identificou que a redação original
  (agregar esses três entre cenas) estava errada.
- **0.6** — `LayersConfigWidget` (7 métodos) e `ModelWidget::updateSceneLayers()` generalizados
  para todas as cenas, classificando cada call site pela decisão #9 (ver plano, subtarefa 0.6, para
  a lista método a método). Dois padrões novos que vale lembrar: (a) `addLayer()`/`renameLayer()`
  retornam nome desambiguado — resolve na cena ativa primeiro, propaga o **nome já resolvido**
  (não o texto bruto) pras demais, pra não arriscar desambiguação divergente entre cenas; (b)
  dentro de um mesmo método (`updateLayersRects()`) uma chamada pode ficar cena-local
  (`clearSelection()`) e outra precisar propagar (`updateLayerRects()`) — a classificação é por
  call site, não por método inteiro. `ModelWidget::updateSceneLayers()` teve o corpo extraído pro
  novo `ModelWidget::applySceneLayers(ObjectsScene *scn)` (aplica numa cena só), que vira o loop de
  `updateSceneLayers()` e também o helper de bootstrap que a Fase 1 vai usar pra popular uma cena
  nova — sem call site novo ligado a isso ainda, só a infra. Build limpo (`gui` + `pgmodeler`), sem
  teste manual de UI nesta sessão (ambiente sem display).
- **0.7** — decisão do usuário: sem composição espacial. `ModelWidget::printModel()` virou loop
  externo por cena + loop interno pelas páginas de cada cena, num único job/diálogo de impressão
  (numeração de página contínua entre cenas). `ModelExportWidget::exportModel()` (branch de
  imagem) virou loop por cena gerando um arquivo por cena (sufixo `_<índice>` só quando há mais de
  1 cena — com 1 cena, path idêntico a hoje). Achado fora do escopo desta subtarefa: `pgmodeler-cli`
  tem seu próprio `ObjectsScene` standalone, não participa da coleção de cenas do `ModelWidget` —
  virou **Fase 4** no plano (tarefa própria, dona: usuário, só depois das Fases 0-3 prontas — ver
  plano, seção 4). `AppearanceConfigWidget` (preview de estilo) conferido, não precisa de mudança.
  Build limpo (`gui` + `pgmodeler`), sem teste manual de UI nesta sessão.
- **0.8** — novo enum público `DatabaseModel::SegmentationMode` (`NoSegmentation`/`BySchema`/
  `ByThreshold`, renomeado de `SceneSegmentationMode` a pedido do usuário), membro privado
  `scene_seg_mode` (default `NoSegmentation`), `setSegmentationMode()`/`getSegmentationMode()`
  mirror do padrão já existente de `last_zoom`/`getLastZoomFactor()`. Só reserva o atributo —
  nenhum DTD/`.sch`/leitura-escrita XML tocado (isso é a 2.1). Fecha a Fase 0. Build completo
  (`core` + `pgmodeler`) sem erros.

**Fase 0 encerrada.** Fase 1 (segmentação por schema + conector cross-scene) começa pela
subtarefa 1.1.

- **Achado antes de codar 1.1**: `Textbox` não pertence a nenhum schema — nem a chave "1 Schema =
  1 cena" (Fase 1) nem o contador da Fase 3 cobrem esse tipo. **Decisão do usuário**: roteia sempre
  pra cena ativa no momento da inserção, nos dois modos. Registrado no plano (intro da Fase 1,
  subtarefa 1.2, contador da Fase 3).
- **Correção retroativa na 0.1, achada ao testar a 1.1**: o construtor de `ModelWidget` conectava
  ~19 sinais de `ObjectsScene` só uma vez, só pra cena única criada ali — toda cena nova (a partir
  da 1.1, uma por schema) ficava sem reagir a movimento/edição/seleção/popup menu/etc. Usuário
  também notou que `this->scene` (criado eagerly no construtor) não tinha vínculo nenhum com as
  cenas de schema, e que o `QGraphicsView` deveria nascer sem cena em vez de receber uma cena
  "fantasma" no construtor. Corrigido: todo o wiring de sinais moveu pra dentro de `addScene()`
  (roda pra cada cena, não só a primeira); construtor não cria mais cena eagerly
  (`viewport = new QGraphicsView(this)`); `handleObjectAddition()` ganhou uma rede de segurança
  que cria uma cena sob demanda se nenhuma existir ainda. Commitado separado da 1.1 (é uma
  correção da 0.1). Detalhado no plano, subtarefa 0.1.
- **1.1** — `handleObjectAddition()` cria uma `ObjectsScene` por schema na primeira vez que o
  encontra (mapa novo `QHash<Schema *, ObjectsScene *> schema_scenes`), bootstrapa layers via
  `applySceneLayers()`, roteia o próprio `SchemaView` pra essa cena nova. Nada mais muda de rota
  ainda (Table/View/ForeignTable/Relationship/Textbox continuam na cena ativa até 1.2/1.6). Build
  completo (`gui` + `pgmodeler`) sem erros. **Testado manualmente pelo usuário**: confirmado que o
  retângulo do schema some do canvas visível (esperado — ele foi pra uma cena que não está anexada
  ao `viewport`; as tabelas continuam aparecendo normalmente, já que ainda não foram roteadas).
  Resolve na 1.2.
- **1.1 (refinamento) — achado do usuário**: `createSystemObjects()` sempre cria `pg_catalog`
  antes dos schemas do arquivo serem parseados na carga, então `pg_catalog` seria o primeiro
  schema processado por `handleObjectAddition()` ao abrir um arquivo — e a regra antiga "ativa se
  for a primeira cena" deixaria a cena do `pg_catalog` (schema raramente tocado direto pelo
  usuário, geralmente só via engenharia reversa) presa ao `viewport`, escondendo o conteúdo real.
  `addScene()` ganhou parâmetro `auto_activate` (default `true`); condição de ativação virou
  "nenhuma cena ativa ainda" (`!scene`) em vez de "é a primeira cena" (`scenes.size() == 1`), pra
  uma cena poder recusar ativação sem bloquear a próxima. `pg_catalog` nunca ativa; `public` força
  sua cena a ficar ativa enquanto `db_model->isModelLoading()` é verdadeiro, mesmo que outro
  schema já tenha reivindicado o padrão antes. Build completo sem erros. **Testado manualmente
  pelo usuário**: modelo com 4 schemas (`public`, `schema_a`, `schema_b`, `pg_catalog`) — ao fim
  da carga, a cena ativa é a do `public`, como esperado (tabelas ainda não roteadas corretamente
  entre cenas, isso é a 1.2, pendente).
- **Achado do usuário antes de codar 1.2**: `Textbox` roteado só pela cena ativa (sem persistência)
  funciona hoje porque a ativação durante a carga já é determinística (graças ao tratamento
  `pg_catalog`/`public` da 1.1) — mas isso não cobre (a) o modo threshold, que vai precisar de um
  atributo `Scene` persistido por objeto (3.2, hoje redigido só pra Table/View/ForeignTable — talvez
  precise cobrir `Textbox` também) nem (b) reatribuição manual de um objeto entre cenas (mecanismo
  ainda não desenhado em lugar nenhum). Registrado como escopo a revisitar quando a 3.2 for
  implementada de fato — não bloqueia a 1.2, que segue como já estava escopada.
- **1.2** — `handleObjectAddition()` resolve `getSchema()` do objeto e busca a cena via
  `getSchemaScene()`, roteando `TableView`/`GraphicalView` pra lá em vez da cena ativa. `Textbox`
  já caía no default de `target_scene` (a cena ativa) — nenhuma mudança de código necessária pra
  ele, só confirma que o comportamento já combinado está correto. Roteamento de
  `Relationship`/`BaseRelationship` continua intocado (isso é a 1.6). Build completo (`gui` +
  `pgmodeler`) sem erros. Testado manualmente pelo usuário e aprovado.
- **1.2 (refinamento) — achado do usuário**: a chamada a `getSchemaScene()` estava duplicada
  (uma cópia idêntica nos cases `Table`/`ForeignTable` e `View`). Usuário propôs mover pra dentro
  da rede de segurança (`if(!target_scene)`) — eu identifiquei que isso quebraria o roteamento da
  1.2 de novo, já que `target_scene` parte de `scene` (quase nunca nulo) e a rede de segurança só
  dispara quando nenhuma cena existe. Usuário ajustou sozinho pro gate certo: uma única linha após
  o `switch`, condicionada a `graph_obj->getSchema()` não ser nulo — resolve certo pra Table/View/
  ForeignTable (têm schema) sem interferir em Textbox/Relationship/Schema (`getSchema()` retorna
  nulo pros três, confirmado por inspeção de código). Build completo sem erros.
- **1.3** — `QComboBox` (`scene_combo`) criado como filho direto de `viewport`, posicionado fixo
  no topo-esquerdo (8,8), listando cenas pelo nome do schema associado (`schema_scenes.key()`,
  fallback "Scene N"). `activated(int)` chama `setActiveScene()`; `s_activeSceneChanged` mantém o
  combo sincronizado quando a cena ativa muda por outro caminho. Repopulado a cada `addScene()`
  via novo método `updateSceneCombo()`. Marcado como temporário no header, aponta pro plano. Build
  completo (`gui` + `pgmodeler`) sem erros. Commitado.
- **Achado do usuário, confirmado por teste manual**: com múltiplas cenas já criadas pela Fase 1,
  o retângulo de cada cena fica errado — só a cena ativa recebe o `scene-rect` legado (em
  `adjustSceneRect(use_model_rect=true)`, chamado ao final de `loadModel()`); as demais ficam com
  o retângulo minúsculo padrão do construtor de `ObjectsScene`. Nenhum call site de
  `adjustSceneRect()` (nem os que recalculam a partir dos itens) itera todas as cenas hoje.
  Elevado de "risco teórico" (já registrado no design da 2.2) pra "confirmado, precisa de fix" —
  usuário pediu pra registrar como etapa extra pendente (Fase 1 ou mais adiante), não corrigir
  agora. Sugestão registrada no plano: separar um fix mecânico barato (iterar todas as cenas,
  mesmo padrão da 0.5) da persistência de fato (já é a 2.2).
- **1.4** — nenhuma mudança de código em `SchemaView` foi necessária; confirmado por leitura de
  código que o cálculo do retângulo usa posição absoluta (sem `mapFromItem`/`mapToItem`) e que
  1.1/1.2 resolvem schema e filhos pelo mesmo `getSchemaScene()`, então não há caminho pra
  divergirem no modo por schema. A verificação virou teste automatizado de verdade: novo target
  `tests/src/modelwidgettest` (**primeiro teste deste projeto a instanciar um widget GUI**
  — `ModelWidget`/`QGraphicsView`), usando a API pública `getSceneForObject()` já existente desde
  a Fase 0. Duas asserções: schema + 2 tabelas na mesma cena; schemas diferentes em cenas
  diferentes. `View`/`ForeignTable` foram tentados como filhos também, mas exigem setup extra não
  relacionado (query válida / referência a `ForeignServer`) pra passar da validação do
  `DatabaseModel::addView()`/`addForeignTable()` — descartados, `Table` já prova o mecanismo
  (mesmo `getSchemaScene()` pros três tipos). Precisou reconfigurar a árvore de build com
  `-DBUILD_TESTS=ON` (estava desligado). **Achado colateral**: rodando a suíte inteira pela
  primeira vez, vários outros targets de teste (não relacionados ao multi-scene) estão quebrados —
  dois nem compilam, vários têm asserções falhando, um crasha. Confirmado via `git log` que não são
  desta sessão nem desta feature — registrado no plano (seção 6) como bit-rot pré-existente, fora
  de escopo, não corrigido. Build completo (`gui` + `pgmodeler`) e o teste novo (4/4) passando.
- **1.7** — auditados os 61 call sites de `getOverlyingObject()` do projeto; 4 crasham de verdade
  com relacionamento cross-scene (nenhum tinha checagem de nulo), corrigidos com um `if(!x) return;`
  logo após o cast:
  - `ObjectsScene::finishObjectsMove()` ([objectsscene.cpp:1568-1577](libs/libcanvas/src/objectsscene.cpp#L1568)) —
    arrastar um `SchemaView` varre `db_model->getObjects(Relationship/BaseRelationship, schema)` (não
    filtrado por cena) e fazia `dynamic_cast<RelationshipView*>(...)->isSelected()` sem checar nulo.
  - `ModelObjectsWidget` ([modelobjectswidget.cpp:264](libs/libgui/src/widgets/modelobjectswidget.cpp#L264)) —
    Alt+clique num item da árvore chamava `scn_item->setSelected(true)` sem checar nulo.
  - `ModelWidget::highlightObject()` ([modelwidget.cpp:5177](libs/libgui/src/widgets/modelwidget.cpp#L5177)) —
    ação "Select" do menu de contexto (`action_select_object`, disponível pra qualquer
    `BaseGraphicObject` quando acionada fora do canvas — confirmado em
    [modelwidget.cpp:4837-4841](libs/libgui/src/widgets/modelwidget.cpp#L4837) que relacionamento
    entra nessa condição) chamava `obj_view->setSelected(true)` sem checar nulo.
  - `ModelWidget::breakRelationshipLine()` ([modelwidget.cpp:5344](libs/libgui/src/widgets/modelwidget.cpp#L5344)) —
    chamava `rel_view->getConnectionPoint(...)` sem checar nulo.
  **Confirmados seguros** (sem mudança): `selectAllObjects()`, `configureFadeMenu()`/`fadeObjects()`,
  `updateObjectsOpacity()`, `getSceneForObject()` (já guardados); `BaseTableView::
  selectRelationships()`/`selectTaggedTables()` (iteram listas que, por construção, nunca têm
  relacionamento cross-scene — `connected_rels` só é populado por `RelationshipView`, e `Tag` não se
  aplica a relacionamento); código interno de `RelationshipView` (só roda com as duas tabelas já
  confirmadas na mesma cena); `handleObjectRemoval()` (já tratado desde o fix de remoção);
  `pgmodeler-cli` (Fase 4, cena própria, fora de escopo); `copyObjects()` (não toca item gráfico).
  Build limpo (`canvas`+`gui`+`pgmodeler`), `modelwidgettest` 4/4 sem regressão. **Sem teste manual
  ainda** — os 4 fixes cobrem casos que só ficam alcançáveis de verdade com relacionamento
  cross-scene já criado (possível desde a 1.6, via o combo de 1.3), mas nenhum tinha reprodução
  automatizada. **Testado manualmente pelo usuário e aprovado.**
- **1.8** — marcador visual ("cut-off marker") pra relacionamento cross-scene, nova classe
  `CutoffItem` em libcanvas (`QObject` + `QGraphicsItemGroup`, não deriva de `BaseObjectView`).
  Composição: `QGraphicsLineItem` (linha reta curta) + `RoundedRectItem` (fundo do rótulo) +
  `TextPolygonItem` (texto "Cross rels: N", polígono transparente — só o `RoundedRectItem` desenha
  o fundo visível) + um segundo `RoundedRectItem` (`obj_selection`) mostrado/escondido em
  `itemChange(ItemSelectedHasChanged)`, espelhando o padrão de seleção já usado pelos outros
  objetos gráficos de libcanvas. Guarda referência à tabela (`table`) e à lista de
  `BaseRelationship *` (não só as tabelas do outro lado — usuário quer poder editar os
  relacionamentos individualmente numa subtarefa futura). Decisões de escopo do usuário: âncora
  fixa (canto direito/centro vertical do título da tabela, via novo
  `BaseTableView::getCutoffAnchorPoint()`), não arrastável independente da tabela, sem elemento
  XML novo nem persistência — item efêmero, existe só em memória. Interação via clique/menu
  dedicado ficou explicitamente fora de escopo, é subtarefa futura ainda não numerada (usuário
  ainda decidindo entre form dedicado ou menu de contexto). `BaseTableView::
  addExternalRelationship()`/`removeExternalRelationship()` criam/destroem o `CutoffItem` sob
  demanda (nasce na primeira relação cross-scene da tabela, morre quando a última é removida).
  Zindex mais baixo possível (`-50`) pra ficar sempre atrás dos demais elementos da tabela.
  `ModelWidget::handleObjectRemoval()` ganhou um branch novo: quando o objeto removido é o próprio
  relacionamento cross-scene (sem `RelationshipView`, então não passa pelo caminho normal de
  remoção de item de cena), chama `removeExternalRelationship()` nas duas `BaseTableView` — gap
  real que existia antes (ponteiro de relacionamento ficaria pendurado em `external_rels`/
  `CutoffItem`).
  - Vários bugs encontrados e corrigidos ao longo da implementação, todos via teste manual do
    usuário entre rodadas:
    - `cutoffitem.cpp`/`.h` esquecidos da lista explícita de fontes do `CMakeLists.txt` do
      libcanvas — build "passava" (biblioteca compartilhada no Linux tolera símbolo indefinido até
      rodar), só o `.o` faltando denunciava.
    - Marcador nascendo na origem da cena em vez do ponto certo: `updatePosition()` só escutava
      `s_relUpdateRequest` (que só dispara ao **soltar** um arrasto), então tabela posicionada só
      via carga de arquivo (posição real só aplicada quando `BaseGraphicObject::
      isUpdatesEnabled()` volta a `true`, no fim da carga) e nunca arrastada manualmente depois
      ficava presa na posição capturada na construção. Fix: conectar também a `s_objectMoved` e
      `s_objectDimensionChanged` incondicionalmente — reposicionar é um `setPos()` barato, ao
      contrário do recálculo de linha do `RelationshipView` (que por isso escolhe um sinal só,
      condicionado a `isPlaceholderEnabled()`).
    - Crash ao selecionar o item: `ObjectsScene::mouseMoveEvent()`
      ([objectsscene.cpp:1458-1461](libs/libcanvas/src/objectsscene.cpp#L1458)) fazia
      `dynamic_cast<BaseObjectView *>(item)` sem checar nulo antes de chamar
      `togglePlaceholder(true)` — bug pré-existente, só exposto agora porque `CutoffItem` é o
      primeiro item selecionável da cena que não deriva de `BaseObjectView`. Corrigido com guarda
      de nulo, mesmo padrão já usado 15 linhas antes na mesma função.
    - `ObjectsScene::alignObjectsToGrid()` (o comando de menu) reposicionava o `CutoffItem`
      independente da tabela, quebrando a âncora — item excluído explicitamente do laço.
    - Retângulo de seleção (`obj_selection`) não aparecia, só a linha parecia "destacada": causa
      raiz é uma particularidade do Qt — `QGraphicsItemGroup::boundingRect()` fica congelado no
      tamanho que os filhos tinham no momento do `addToGroup()`, sem atualizar depois; como
      `label_bg`/`label` só recebem o tamanho real (`setRect()`/`setPolygon()`) **depois** de
      entrarem no grupo, o bounding rect do grupo ficava essencialmente só o retângulo (vazio) da
      linha. Fix inicial calculava a união manual de `line` + `label_bg`; usuário simplificou
      depois pra usar só o retângulo do `RoundedRectItem` do rótulo (a linha já recebe destaque
      próprio, união grande deixava a seleção "inorgânica").
    - Substituída a decoração de seleção nativa do Qt na linha pela mesma técnica já usada em
      `RelationshipView`: mistura por canal (R/G/B, alpha forçado a 255) entre a cor de borda de
      `Attributes::ObjSelection` e a cor normal da linha (`Attributes::PositionInfo`), aplicada ao
      `pen` da linha ao selecionar e revertida ao desselecionar (`CutoffItem::
      configureLineSelection()`) — corrigido, sem reproduzir, um typo aparente na fórmula
      original do `RelationshipView` (`cor2.greenF()` no lugar de `cor2.redF()` no canal
      vermelho).
    - Mesmo depois da troca de cor, ainda aparecia um retângulo nativo do Qt em volta da linha ao
      selecionar. Causa raiz achada lendo o fonte do Qt diretamente (`qgraphicsitem.cpp`):
      `QGraphicsItemGroup::paint()` desenha `painter->drawRect(itemsBoundingRect)` sempre que o
      grupo está selecionado, usando o mesmo bounding rect congelado do bug anterior (que
      coincide quase exatamente com a área da linha). É o mesmo problema que `RelationshipView::
      paint()` já contorna há tempos (comentário existente no código fala em "artifact rectangle
      ... drawn around the relationship"). Aplicado o mesmo contorno em `CutoffItem::paint()`:
      força `Qt::NoPen` e recorte pro `exposedRect` antes de delegar pro `QGraphicsItemGroup::
      paint()` base.
    - `ObjectsScene::finishObjectsMove()` (disparado a cada soltar de mouse depois de uma
      seleção, não só pelo comando de menu) tinha um laço incondicional que faz
      `item->setPos(alignPointToGrid(item->pos()))` em qualquer item selecionado que não seja
      `TableObjectView` nem `RelationshipView` — `CutoffItem` não estava excluído, então
      selecionar o marcador e soltar o mouse (o arrasto em si não move nada, já que o grupo não
      tem `ItemIsMovable`) já bastava pra ele ser encaixado na grade, longe da âncora. Excluído
      explicitamente, mesmo padrão do fix já aplicado em `alignObjectsToGrid()`.
  - Build limpo (`canvas`+`gui`+`pgmodeler`) e `modelwidgettest` 4/4 sem regressão em todas as
    rodadas. Sem cobertura automatizada nova (feature visual/interativa, validada por teste manual
    do usuário em várias rodadas sucessivas).

## Design de persistência por-cena (sem código ainda — vira 2.1/2.2 quando a Fase 2 chegar)

Discussão longa sobre como persistir o retângulo de cada cena (`SceneRect` hoje é um único
`QRectF` pro modelo inteiro) e como identificar cada `<scene>`. Nada disso foi codado ainda — é
puro fechamento de design, registrado no plano (seção 2, "Achado de revisão de código, expandido em
2026-09-21", decisão #3 na seção 3, e subtarefas 0.8/2.1/2.2):

- **Verificado**: `ObjectsScene::setSceneRect()` não move item nenhum — só ajusta o retângulo de
  rolagem/indexação. Aplicar um retângulo por-cena calculado a partir só dos itens daquela cena é
  seguro, não desloca nada pra origem.
- **Bug real encontrado** (diferente do que se temia): `ObjectsScene::adjustSceneRect(false)` (o
  método que *recalcula* a partir da posição dos itens, não o que carrega um valor persistido) puxa
  a borda esquerda/superior pra perto de zero incondicionalmente. Não usar esse método pra calcular
  o retângulo inicial de uma cena nova/recém-dividida na Fase 1 — usar `itemsBoundingRect()` direto.
- **Decisão**: sem retângulo global compartilhado entre cenas (foi cogitado, descartado — reintroduz
  o problema de performance que a segmentação resolve). Cada cena persiste só o seu próprio
  retângulo.
- **Formato escolhido**: tag `<scenes><scene id="" left="" top="" right="" bottom="" /></scenes>`,
  mirror exato do mecanismo já existente de `changelog`/`<entry>` (mesmo padrão de escrita/leitura,
  mesmo estilo de DTD/`.sch`) — ideia do usuário, confirmada como o padrão mais idiomático do
  formato depois de eu achar o precedente no código.
- **Chave de identidade revisada pra `id` numérico, sem `name`** (2026-09-21, sessão seguinte,
  refinado numa terceira rodada): a versão original chaveava por `name` (nome do schema). Motivação
  do usuário pra trocar: evitar custo de manter a cena "sincronizada" quando um schema é renomeado.
  Na investigação, esse custo não existiria de qualquer forma (tudo é recalculado do zero a cada
  save, nunca sincronizado incrementalmente) — mas o índice ganha por outro motivo válido:
  uniformiza os três modos de segmentação, já que threshold/clustering não têm nome natural.
  Verifiquei por leitura de código que a ordem de carga de `Schema` é determinística
  (`DatabaseModel::loadModel()` cria na ordem do XML; `getCreationOrder()` ordena por `object_id`,
  atribuído em ordem de criação) — então o índice sobrevive a um ciclo salvar→recarregar no caso
  comum. Único jeito de quebrar isso é `swapObjectsIds()`/"Edit creation order", que só afeta o
  **próximo** reload (não a sessão atual, não cumulativo) — usuário optou por aceitar como
  limitação documentada em vez de gastar código numa checagem cruzada de validação. Atributo
  escolhido: `id` (não `idx` — nome revisado numa rodada seguinte pra ficar mais curto/idiomático).
  `name` foi **descartado por completo** do `<scene>` (não só rebaixado a auxiliar como cogitado
  antes) — sem necessidade real comprovada; reintroduzir é barato se um motivo concreto aparecer no
  futuro. `LastScene` também é numérico pelo mesmo motivo. Detalhamento completo no plano (seção 2,
  subseção da chave de identidade) — não duplicado aqui.
- **DTD vira arquivo dedicado** (2026-09-21, a pedido do usuário): mirror mais fiel de
  `changelog.dtd` — que é um arquivo próprio incluído em `dbmodel.dtd` via entidade
  (`<!ENTITY % changelog SYSTEM "changelog.dtd">`), não declarações soltas dentro dele. Mesmo
  padrão pra cenas: novo `assets/schemas/xml/dtd/scene.dtd`; `dbmodel.dtd` só ganha a linha de
  inclusão da entidade. Templates `.sch` (`scenes.sch`/`scene.sch`) já estavam corretos desde o
  design original, sem mudança.
- **Retrocompatibilidade** (requisito explícito do usuário): o atributo legado `scene-rect` continua
  sendo escrito como hoje quando há só 1 cena — formato de arquivo inalterado no caso comum. Arquivo
  antigo carregado num modelo que a Fase 1 segmenta cai no fallback já existente por cena, sem
  redistribuir o valor legado.
- **Terminologia revertida**: a decisão #8 do plano (usar "Workspace" pro usuário/persistência)
  foi **revertida nesta sessão** — decisão final do usuário é usar "cena"/"Scene" em tudo, sem
  distinção usuário/interno, já que o formato já tinha atributos "Scene" (`SceneRect`, `LastScene`)
  e criar um segundo nome só pra UI não trazia benefício real. Fiz uma varredura no plano corrigindo
  as menções a "Workspace" adicionadas nas últimas sessões.
- **Achado do usuário, mesma sessão**: `SchemaView` tem o mesmo problema que motivou a decisão #1
  pra relacionamentos, mas só no **modo threshold** — lá schema deixa de ser 1:1 com cena (só é
  garantido no modo por schema, subtarefa 1.4), então os filhos de um schema podem cair em cenas
  diferentes. Solução, mesma filosofia da decisão #1: suprimir completamente o `SchemaView` daquele
  schema (não tentar renderizar um retângulo parcial/composto) sempre que seus filhos vivem em mais
  de uma cena, com promoção/rebaixamento dinâmico reaproveitando o mesmo choke point já usado pra
  relacionamentos. Registrado na Fase 3 (subseção dedicada + subtarefa 3.5).
- **Decisão "sem retângulo global" revisitada e reafirmada (2026-09-22)**: usuário voltou nessa
  decisão fechada motivado por flexibilidade de troca de modo de segmentação, propondo (leitura B)
  todas as cenas compartilharem as mesmas dimensões em runtime. Levantei o custo real — não é bem
  a BSP tree (escala por número de itens, não área; a justificativa original nesse ponto tava
  imprecisa), é `getPagesForPrinting()` gerando páginas em branco em excesso pra cena pequena
  forçada a compartilhar dimensões de cena grande (desfaz o ganho da 0.7). Propus um híbrido:
  cada cena mantém retângulo próprio em runtime (decisão original mantida), mas o atributo legado
  `scene-rect` vira um valor "geral" (união de todas as cenas), recalculado só no save, servindo
  de fallback se a segmentação for desligada depois. Usuário aprovou. Ao perguntar sobre
  expandir/encolher cena, revisitei o handler de `s_sceneRectChanged` já commitado (subtarefa "0.1
  correção") e achei um bug real: ele sobrescreve `db_model`'s scene-rect com o retângulo de uma
  única cena reativamente, a cada mudança — incompatível com o híbrido, precisa ser removido em
  favor de um `updateModelSceneRectInfo()` novo (só no save). Usuário confirmou e pediu pra manter
  esse hábito de revisitar código já commitado ao longo do resto do plano. Tudo registrado na
  subtarefa 2.0 e na seção 2 do plano — nada implementado ainda.

## Decisões/ajustes tomados durante a implementação (além do que já estava no plano)

Regras de trabalho combinadas nesta sessão — valem pro resto do plano, não só pra Fase 0:

1. **Containers Qt, não std** no código novo desta feature (`QList`, `QHash`, `QMap`,
   `QStringList`...) — não misturar com `std::vector`/`std::map`/etc. Já registrado como decisão
   #6 na seção 3 do `MULTI_SCENE_PLAN.md`. Código pré-existente (ex. `selected_objects`) não
   precisa ser convertido.
2. **Comentários de método/atributo**: descrever só o que o elemento *faz*, nunca contrastar
   "como é hoje" vs. "como vai ficar" — isso deixa os comentários longos demais e envelhece mal.
3. **Cadência de commit**: ao terminar de implementar uma subtarefa, perguntar se commita agora ou
   segue pra próxima — nunca commitar nem prosseguir automaticamente.
4. Preferência de design (surgiu na revisão da 0.4, vale como padrão pro resto do plano): quando
   uma cena/objeto precisa ser "lembrado" pra desconectar depois (ex. sinais Qt entre widgets),
   preferir guardar o **ponteiro do objeto conectado** num membro dedicado (aqui,
   `connected_scene`) e usar `disconnect(ponteiro, nullptr, this, nullptr)` na hora de trocar, em
   vez de (a) rastrear uma lista de `QMetaObject::Connection` pra desconectar uma por uma, ou (b)
   varrer todas as cenas do modelo desconectando de cada uma "pra garantir". Também: preferir um
   método privado nomeado a uma lambda quando a lógica é usada em mais de um lugar (conexão
   inicial + reação a um sinal) — lambda extensa dificultou a leitura.
5. **Cena-local vs. modelo-inteiro-replicado-por-cena** (decisão #9, seção 3 do plano; surgiu na
   revisão da 0.5): nem toda suposição de "só existe uma cena" no código atual é um bug de
   agregação. Interação/visualização (seleção, bounding rect do selecionado, gates de menu ligados
   ao que está visível) fica cena-local, sem tocar as demais. Normalização/config que vale pro
   modelo inteiro (alinhar à grade, layers) precisa ser replicada em cada cena, sem combinar
   resultado. Classificar antes de "corrigir" — o padrão errado (agregar por reflexo) foi cometido
   uma vez na redação original da 0.5 e corrigido só depois do usuário revisar.
6. **Separação entre os dois documentos** (2026-09-21): `MULTI_SCENE_PLAN.md` é só o plano —
   arquitetura, decisões, descrição/dependência de subtarefas. Nunca marcar ali se uma subtarefa
   foi feita/commitada. Todo log de execução (o quê foi feito, hash de commit, achados de sessão,
   bugs encontrados) vive só neste arquivo, atualizado junto de cada commit. Motivo: três
   subtarefas ficaram marcadas "[FEITO, commitar]" no plano numa sessão anterior e a tag nunca foi
   limpa depois do commit — status e plano acabam divergindo se dividirem o mesmo texto.
7. **Revisitar código já commitado quando uma nova discussão de design o afeta**, em vez de só
   raciocinar sobre a mudança nova isolada (2026-09-22). Antes de fechar um novo trecho do plano ou
   escrever código novo, checar se algo já implementado antes fica incoerente com a decisão nova —
   não deixar código velho ficar contradizendo design novo silenciosamente. Confirmado pelo usuário
   depois que isso pegou um bug real no handler de `s_sceneRectChanged` (ver seção de design do
   retângulo de cena acima) que ele mesmo tinha esquecido que existia.

## Bug encontrado e corrigido durante teste manual

Ao abrir um modelo e pressionar F10 (abrir o overview) logo em seguida, o overview aparecia vazio
e o log mostrava `QPainter::begin: Paint device returned engine == 0, type: 2`; mexer num objeto
da cena corrigia o overview sozinho. Causa: `connectToActiveScene()` (que chama
`updateOverview(true)`) estava sendo chamado **antes** de `resizeOverview()` dentro de `show()` —
`pixmap_size` só é calculado dentro de `resizeOverview()`, então o primeiro paint acontecia num
`QPixmap` de tamanho inválido. Fix: `resizeOverview()`/`updateZoomFactor()` voltaram a rodar antes
de `connectToActiveScene()` em `show()`, restaurando a ordem original. Detalhado na subtarefa 0.4
do plano.

## Ambiente de build usado nesta sessão

- Árvore de build real (Makefiles, aponta pra este repo): `/home/raphael/pgmodeler/cmake-build`.
  (O diretório `build/` na raiz do repo é só uma pasta de instalação/output, não uma árvore CMake —
  não confundir os dois.)
- Comandos usados pra validar cada subtarefa: `make -j$(nproc) gui` (rápido, só a lib que muda na
  Fase 0) e, quando queria confirmar o link final, `make -j$(nproc) pgmodeler`.
- **Testes automatizados (a partir da 1.4)**: a árvore precisou ser reconfigurada uma vez com
  `cmake -DBUILD_TESTS=ON .` (rodado de dentro de `cmake-build`) — sem isso `tests/` nem entra na
  build (guardado atrás de `CMAKE_BUILD_TYPE STREQUAL Debug AND BUILD_TESTS` no CMakeLists raiz).
  Depois disso, `make -j$(nproc) modelwidgettest` builda só o teste novo; os executáveis ficam em
  `cmake-build/tests/src/<nome>/<nome>`, cada um roda direto (não precisa de `ctest`).
