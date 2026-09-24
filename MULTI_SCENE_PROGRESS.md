# Progresso — Suporte a Múltiplas Cenas

> Log de execução do `MULTI_SCENE_PLAN.md`, atualizado sessão a sessão. Leia primeiro o plano
> (arquitetura, decisões fechadas, subtarefas) — este arquivo é só "onde paramos e por quê",
> para retomar rápido sem reler todo o histórico de commits.

## Estado atual

- Branch: `multi-scene-support-2.1.x`, com 1 commit à frente de `origin` (refactor próprio do
  usuário, `0f7ac6f61`, não relacionado a esta sessão — adicionou `BaseTableView::getCutoffItem()`
  como acesso público em vez de exigir `friend class ModelWidget`).
- **Fase 0 (0.1-0.8), Fase 1 subtarefas 1.1-1.8, 1.8.1 e 1.9 (1.9.1-1.9.6) — todas concluídas,
  testadas manualmente pelo usuário (1.9.4 e a rodada final também confirmadas por ele via ASan) e
  commitadas.** (1.2 recebeu um refinamento pequeno de deduplicação logo depois de feita.)
- **1.10 implementada, testada e commitada.** Ver detalhamento abaixo. Subtarefas 1.11-1.15 ficam em
  standby (ver bullet de reorganização de prioridade abaixo) — não são a próxima frente de trabalho.
- **Leak de `RelationshipView` do undo/redo (decisão #11) reproduzido e corrigido (2026-09-24)**
  — bug pré-existente em `~RelationshipView()` (`round_cf_descriptors` não limpo em lados de
  relação obrigatórios), não específico da feature multi-cena. Ver seção própria mais abaixo.
  Commitado.
- **Reorganização de prioridade: threshold antecipado (2026-09-24)** — decisão do usuário de
  implementar a segmentação por threshold logo após a Fase 1, em vez de fechar toda a persistência
  do modo por schema primeiro: o código ainda está pouco acoplado a "1 Schema = 1 cena" nesta
  altura, então reconciliar os dois modos agora é mais barato do que esperar a persistência
  aprofundar esse acoplamento. `MULTI_SCENE_PLAN.md` foi reorganizado: a antiga Fase 3 (Threshold)
  virou a nova **Fase 2**, a antiga Fase 2 (Persistência) virou a nova **Fase 3** — mesmo conteúdo,
  só trocaram de posição/número (subtarefas renumeradas para bater com a fase nova; Fase 4, CLI,
  não foi afetada). Subtarefas 1.11-1.15 da Fase 1 ficam em **standby** (não descartadas) até a
  nova Fase 2 estar pronta.
- **Achado de revisão de código na Fase 2 (2026-09-24)**: a subseção "`SchemaView` cross-scene:
  supressão dinâmica" e a subtarefa 2.5 assumiam reaproveitar "o mesmo choke point de promoção/
  rebaixamento já usado pelos relacionamentos" — essa suposição não corresponde mais ao código
  depois da simplificação das decisões #10/#11 (descartar+recriar, sinal do `OperationList`):
  `relocateObjectView()` só dispara via `getSchemaScene()`, que nunca resolve nada no modo
  threshold (não existe hoje nenhum evento de "tabela mudou de cena" nesse modo). Plano corrigido:
  2.5 precisa de três pontos de disparo próprios (inserção, remoção, reatribuição de schema sem
  troca de cena). Ver a subseção atualizada no plano para o detalhamento completo.
- **2.1 implementada (2026-09-24): contador ingênuo por threshold + criação de nova cena.**
  `ModelWidget::handleObjectAddition()` deixou de rotear por schema incondicionalmente — passou a
  ler `DatabaseModel::getSegmentationMode()` (scaffolding da 0.8, existia mas nunca era lido por
  ninguém) e ramificar: `BySchema` mantém o comportamento da Fase 1 inalterado; `ByThreshold` roteia
  por um contador (`ModelWidget::threshold_obj_count`, em memória, não persistido ainda — isso é a
  2.2) que conta só Table/View/ForeignTable e abre uma nova cena a cada `SceneObjThreshold` (1000)
  inserções, sempre que o contador cruza um múltiplo. **Decisão do usuário sobre o modo default**:
  `NoSegmentation` continua sendo o default real de `DatabaseModel` (não foi alterado) — os testes
  de regressão da Fase 1 (7 testes) precisaram ser atualizados para chamar
  `setSegmentationMode(BySchema)` explicitamente, já que antes dependiam do roteamento por schema
  incondicional que não existe mais. Para viabilizar teste manual da Fase 2 na GUI antes de existir
  qualquer seleção de modo de verdade, `ModelWidget` força `ByThreshold` no construtor, atrás de um
  `#warning` explícito marcando que é scaffolding temporário desta fase, a ser removido depois (ver
  MULTI_SCENE_PLAN.md subtarefa 2.1). Teste novo (`thresholdModeOpensNewSceneAfterCrossingCount`)
  cobre a travessia do threshold; suíte inteira + o teste novo isolado rodaram limpos sob ASan.
- **2.2 implementada (2026-09-24): persistência do atributo `scene` por objeto + contador
  sobrevivendo entre sessões.** `BaseTable` (mesma classe-mãe de Table/View/ForeignTable, já usada
  pra `cutoff_pos` na 1.8.1) ganhou `scene_idx` (int, -1 = não atribuído) +
  `setSceneIndex()`/`getSceneIndex()`/`resetSceneIndex()` + `setSceneIndexAttribute()` protegido.
  Novo atributo XML `scene` (plain attribute na própria tag, mesmo padrão de `layers`, não um
  sub-elemento como `cutoff-pos`) em table.sch/view.sch/foreigntable.sch + DTDs correspondentes;
  novo `Attributes::Scene` ("scene") em libparsers. Leitura em `createPhysicalTable()`/
  `createView()`. **Decisão sobre o "ponto em aberto"** (como o contador sobrevive entre sessões):
  optei por recalcular em vez de persistir o valor do contador — `handleObjectAddition()` agora
  checa `getSceneIndex()` primeiro (se >= 0, já tem cena atribuída — veio de um arquivo carregado ou
  de um undo restaurando um objeto removido — roteia direto pra `scenes[idx]`, alargando a coleção
  se preciso, sem tocar no contador); só cai no contador ingênuo quando `getSceneIndex()==-1` (objeto
  genuinamente novo). `ModelWidget::loadModel()` faz um passo final recontando quantos objetos
  ficaram na última cena e reseeda `threshold_obj_count` com isso — sem essa reseed, a primeira
  inserção interativa depois de reabrir um arquivo reiniciaria a contagem do zero e abriria uma cena
  extra prematuramente, mesmo com a última cena do arquivo quase cheia. **Escopo deliberadamente
  restrito a Table/View/ForeignTable** — a pergunta "esse mesmo atributo deve cobrir `Textbox` e
  reatribuição manual entre cenas?" (achado da Fase 1, 2026-09-22) continua em aberto, não decidida
  nesta subtarefa. Teste novo `thresholdModeSceneAssignmentSurvivesSaveAndReload` cobre o round-trip
  save→reload (atribuição de cena preservada + contador retomado sem abrir cena extra); suíte inteira
  (11 testes) + o teste novo isolado rodaram limpos sob ASan.
- **Ajuste de convenção de comentários (2026-09-24)**: usuário pediu para parar de referenciar
  `MULTI_SCENE_PLAN.md`/`MULTI_SCENE_PROGRESS.md`, números de Fase/subtarefa ou decisão/seção dentro
  de comentários de código — um comentário deve se explicar sozinho, sem depender de uma numeração
  que pode ser reorganizada independente do código (como já aconteceu na troca Fase 2/3). Todos os
  comentários já escritos nesta sessão (`modelwidget.h/.cpp`, `basetable.h`, `modelwidgettest.cpp`)
  foram reescritos removendo essas referências. Também ficou combinado que, ao fim de cada subtarefa,
  a lista de métodos alterados/criados deve ser apresentada como links clicáveis `arquivo (Lxx-yy)`
  pra facilitar a revisão — sem substituir o resumo em prosa. `MULTI_SCENE_PLAN.md` em si continua
  livre pra usar essa numeração normalmente (a regra é só sobre comentários no código-fonte).
  Também fechada no plano a decisão do contador de threshold sobreviver entre sessões via
  recálculo (ver subseção correspondente na Fase 2), que estava registrada como pergunta em aberto
  antes da 2.2 resolvê-la na prática. **Próxima frente de trabalho: subtarefa 2.3** (reaproveitamento
  do marcador cut-off da Fase 1 para os cross-scene do modo threshold).
- **Pendência da pausa anterior, resolvida por decisão de design em vez de investigação (2026-09-23)**:
  a dúvida "será que o bug de corrupção de memória do `relocateItem()` também afeta `TableView`?"
  (`columns`/`ext_attribs` têm a mesma estrutura de grupo aninhado com filhos dinâmicos que quebrou
  em `RelationshipView`) não foi investigada — usuário decidiu, por precaução, generalizar o padrão
  descartar+recriar pra **toda** relocação de item entre cenas, eliminando a pergunta pela raiz em
  vez de testar se o bug se manifesta. `ObjectsScene::relocateItem()` foi **removido por completo**
  (não sobrou nenhum call site). Ver detalhamento completo abaixo.
- **Crash real achado testando a 1.10, causa raiz não era a 1.10 (2026-09-23)**: usuário reportou
  SIGSEGV ao fechar o modelo depois de remover um schema, capturado pelo crash handler e depois
  isolado só com ASan (sem crash handler) — mesmo stack trace nos dois: `RelationshipView::
  ~RelationshipView()` (relationshipview.cpp:127, limpando `lines`/`fk_lines`/`pk_lines`/
  `src_cf_lines`/`dst_cf_lines`) → `QGraphicsItemGroup::removeFromGroup()` → `QGraphicsItem::
  sceneTransform()` → `QGraphicsItemPrivate::ensureSceneTransformRecursive()`, sempre durante
  `ObjectsScene::~ObjectsScene()` dentro do teardown completo do `ModelWidget`. Isolado com um
  teste dedicado (`ModelWidgetTest`) que reproduzia 100% das vezes **sem nunca chamar
  `removeScene()`/1.10** — só chamando `ObjectsScene::relocateItem()` numa `RelationshipView` (o
  mesmo método que `ModelWidget::relocateRelationshipView()`, da 1.9.4, já usa em produção) e
  depois deixando o `ModelWidget` fechar normalmente. **Causa raiz real, não é a 1.10**: mover uma
  `RelationshipView` entre cenas via `relocateItem()` deixa alguns dos seus itens internos de linha
  com memória corrompida — confirmado com `gdb`, inspecionando o item que crasha: o ponteiro de
  vtable dele não aponta mais pra dentro de nenhuma biblioteca do Qt (virou um valor pequeno tipo
  `0x430e`), ou seja, não é só "esqueceu de avisar a cena nova", é corrupção de memória de verdade.
  Três tentativas de corrigir dentro do próprio `relocateItem()`/`ObjectsScene` falharam de forma
  **idêntica** (mesmo stack trace, mesmo padrão de endereço) — registradas aqui porque já foram
  eliminadas, não valem a pena tentar de novo:
  1. Forçar cada filho (recursivo) a se re-registrar na cena nova via `child->setParentItem(nullptr)`
     seguido de `child->setParentItem(parent)` (truque comum pra invalidar cache de transform do Qt).
  2. Versão mais agressiva do mesmo: `group->removeFromGroup(child)` seguido de
     `group->addToGroup(child)` pra cada filho, recursivo em grupos aninhados (`cf_descriptors`) —
     o mesmo caminho de código que o Qt usa pra adicionar um item a um grupo a primeira vez.
  3. Pular o `detachItem()`/`QGraphicsScene::removeItem()` explícito e deixar só
     `dst_scene->addItem(item)` (que segundo a documentação do Qt já detecta e remove da cena
     antiga sozinho) fazer a transferência inteira.
  **Correção adotada**: não tentar mover a `RelationshipView` existente entre cenas. Reescrito
  `ModelWidget::relocateRelationshipView()` pra descartar a view antiga (`cur_scene->
  removeItem(item)`, caminho já testado/seguro) e construir uma nova diretamente na cena de destino
  (`dst_scene->addItem(new RelationshipView(rel))`) — exatamente o que `promoteRelationship()` já
  fazia com sucesso. `ObjectsScene::relocateItem()` em si **não foi alterado** — continua sendo
  usado sem problema pra tabelas/schemas (nunca mostrou esse bug nos testes extensivos da 1.9);
  só deixou de ser chamado para relacionamentos.
  - **Achado colateral durante a validação**: a primeira versão do teste de regressão (só
    descartava a view do relacionamento, sem também relocar as tabelas antes) causou um
    heap-use-after-free **diferente e genuíno**, pego limpo pelo ASan:
    `RelationshipView::disconnectTables()` → `BaseTableView::removeConnectedRelationship()` lendo
    uma `TableView` já destruída. Isso confirmou que a ordem importa de verdade: só é seguro
    descartar/recriar a view do relacionamento **depois** que as tabelas já foram relocadas pra
    cena de destino (exatamente a ordem que `reevaluateTableRelationships()` já usa em produção —
    tabela primeiro, `relocateRelationshipView()` depois). Corrigido o teste pra seguir essa ordem
    real, e aí sim passou limpo. Não é um bug de produção (a ordem real já está certa), mas reforça
    por que a subtarefa 1.11 (teste de regressão dedicado) é importante.
  - Novo teste `ModelWidgetTest::discardingAndRecreatingRelationshipViewSurvivesModelClose`
    substitui o teste exploratório anterior — cobre exatamente esse cenário (tabelas relocadas,
    relacionamento descartado+recriado na cena nova, modelo fechado) como regressão permanente.
    Build limpo (`canvas`+`gui`+`pgmodeler`), suíte `modelwidgettest` 5/5 sem regressão. Binário
    reinstalado. Não commitado ainda — aguardando teste manual do usuário (mover tabelas entre
    schemas repetidamente, inclusive causando promoção/rebaixamento de relacionamento, depois
    fechar o modelo).
  - **Usuário confirmou a escolha de design** (descartar+recriar em vez de mover) — mais memória
    (aloca uma `RelationshipView` nova em vez de reaproveitar), mas código bem mais simples e sem
    depender de bookkeeping interno do Qt pra grupos aninhados. Trade-off aceito conscientemente.
  - **Pendência levantada pelo usuário, registrada aqui pra não esquecer: o mesmo bug pode existir
    pra tabelas** — `ObjectsScene::relocateItem()` também é usado pra `TableView`/`SchemaView` (via
    `moveToSchema()`/`relocateEditedObject()`), e a análise de código confirma que `BaseTableView`
    tem a mesma forma estrutural que causou o bug em `RelationshipView`: `columns`/`ext_attribs`
    são `QGraphicsItemGroup`s aninhados dentro do `TableView`, e cada coluna/constraint vira um
    `TableObjectView` criado dinamicamente e adicionado via `groups[obj_idx]->addToGroup(col_item)`
    ([tableview.cpp:194](libs/libcanvas/src/tableview.cpp#L194)) — o mesmo padrão de "grupo dentro
    de grupo com filhos dinâmicos" que quebrou pros `cf_descriptors` do relacionamento.
    `BaseObjectView` (base de **todo** objeto gráfico, não só relacionamento) também adiciona/remove
    `obj_shadow`/`protected_icon`/`pos_info_item`/`sql_disabled_item` do mesmo jeito. A favor de
    tabelas serem seguras: `relocateItem()` em tabela é o mecanismo central de todo o teste extenso
    da Fase 1.9 (mover tabelas entre schemas repetidamente, inclusive com colunas/constraints) e
    nunca crashou — mas isso é ausência de falha observada, não prova de que não pode acontecer (a
    causa raiz exata da corrupção no Qt nunca foi identificada, só contornada). **Ação pendente pra
    próxima sessão**: escrever um teste de stress dedicado — tabela com várias colunas/constraints,
    relocada entre cenas via `relocateItem()` (o mesmo caminho, sem o fix de descartar+recriar, já
    que tabelas ainda usam relocação de verdade), modelo fechado sob ASan — pra confirmar se
    tabelas são de fato seguras ou se precisam do mesmo tratamento de descartar+recriar. Usuário
    pediu explicitamente pra registrar isso como pendência, não investigar agora — retomamos com
    mais testes numa sessão futura.
  - **Atualização (2026-09-23, retomada da sessão): usuário decidiu generalizar o fix em vez de
    investigar a pendência acima.** Por precaução, dado o quanto os internals do Qt envolvidos são
    obscuros (achados só via `gdb` na rodada anterior, sem entender o mecanismo exato da corrupção),
    o padrão de relocação de qualquer item entre cenas virou: (1) remover o item por completo da
    cena de origem; (2) recriar do zero na cena de destino. Trade-off consciente: mais memória (uma
    alocação nova em vez de reaproveitar a existente) em troca de nunca depender de bookkeeping
    interno do Qt pra grupos aninhados com filhos dinâmicos — eliminando essa classe inteira de bug
    pela raiz, sem precisar provar caso a caso quais tipos de item são "seguros" o suficiente pra
    relocar de verdade.
    - `ObjectsScene::relocateItem()` **removido por completo** (declaração + implementação) — não
      sobrou nenhum call site depois da migração abaixo.
    - **Tabelas/views** (`moveToSchema()`, `relocateEditedObject()`): novo par de métodos privados
      em `ModelWidget` — `createGraphicItem(BaseGraphicObject *graph_obj)` (fábrica pequena, só
      Table/ForeignTable/View, espelhando os mesmos dois casos já existentes em
      `handleObjectAddition()`, sem duplicar a função inteira) e `relocateObjectView(graph_obj,
      dst_scene)` (`cur_scene->removeItem(cur_view)` + `dst_scene->addItem(createGraphicItem(...))`).
      Os dois call sites antigos (que resolviam `item_view`/`cur_scene`/`dst_scene` na mão e
      chamavam `relocateItem()`) ficaram bem mais enxutos; `reevaluateTableRelationships()` passou a
      buscar `getOverlyingObject()` de novo depois da relocação, já que o ponteiro da view mudou.
    - **Marcador cut-off**: novo `BaseTableView::relocateCutoffItem()` — captura a lista de
      relacionamentos do `cutoff_item` atual, `delete`, cria um `CutoffItem` novo, readiciona cada
      relacionamento, e só então `this->scene()->addItem(cutoff_item)` (usa a cena atual da própria
      tabela, que já foi relocada antes deste ponto — não precisa receber a cena de destino como
      parâmetro). Chamado de `reevaluateTableRelationships()` no lugar do `relocateItem()` antigo.
    - **Relacionamento**: já estava feito (`relocateRelationshipView()`, ver bloco acima) — não
      mudou nesta rodada, só deixou de ser a exceção isolada e virou o padrão geral.
    - Build limpo (`canvas`+`gui`+`pgmodeler`+`modelwidgettest`), suíte `modelwidgettest` 5/5 sem
      regressão (o teste de regressão da subseção acima foi ajustado pra também descartar+recriar as
      duas `TableView`s manualmente, já que `relocateItem()` não existe mais pra chamar). Binário
      reinstalado.
  - **Bug real introduzido por essa generalização, achado pelo usuário no teste manual (2026-09-23):
    "mover tabelas para um novo esquema não está movendo as tabelas juntas se estas já existiam no
    esquema anterior"** — ou seja, o relacionamento entre duas tabelas movidas juntas simplesmente
    desaparecia. Causa raiz: `connected_rels`/`external_rels` (quais relacionamentos uma tabela
    rastreia) vivem na **instância da view**, não no modelo — `relocateObjectView()` descartava a
    `TableView` antiga e criava uma nova (zerada), e só **depois** `reevaluateTableRelationships()`
    lia essas listas — já vazias — da view nova, então nunca descobria que o relacionamento existia.
    A `RelationshipView` antiga ficava presa, referenciando a `TableView` antiga (órfã).
  - **Redesenho completo, proposto pelo usuário**: em vez de métodos de relocação dedicados em
    `ObjectsScene`/`libcanvas`, reaproveitar a mesma lógica de roteamento que `handleObjectAddition()`
    já usa (que resolve a cena de um relacionamento lendo a cena *atual* de cada tabela via
    `getOverlyingObject()->scene()` — já é independente de "cena ativa"). Sequência nova, pensada com
    cuidado extra a pedido do usuário pra não deixar ponteiros soltos em `RelationshipView` (que
    guarda `tables[0]`/`tables[1]`, ponteiros crus pras duas `BaseTableView`):
    1. Antes de tocar a `TableView` antiga, capturar `connected_rels`/`external_rels` dela (só aqui
       essas listas ainda têm os dados certos).
    2. Pra cada relacionamento capturado: se tem uma `RelationshipView` (mesma cena), remover **agora**
       via `scene->removeItem()` — isso chama `disconnectTables()`, que lê `tables[0]`/`tables[1]`;
       precisa rodar **enquanto as duas `BaseTableView` (incluindo a que está prestes a ser
       descartada) ainda existem**, senão vira ponteiro pendurado no instante em que a view for
       tocada de novo. Se é externo (sem view), só limpar do lado do parceiro
       (`partner_view->removeExternalRelationship()`) — o lado da tabela sendo relocada nem precisa
       de limpeza, morre junto com a `TableView`/`cutoff_item` antigos logo abaixo.
    3. Só então descartar a `TableView` antiga e criar a nova na cena de destino.
    4. Re-rotear cada relacionamento capturado (`ModelWidget::routeRelationship()`, nova função —
       mesma regra do case `Relationship` de `handleObjectAddition()`: mesma cena agora → cria
       `RelationshipView` nova; cenas diferentes → registra externo nas duas tabelas, o que recria o
       `cutoff_item` sob demanda). Isso também autocorrige o caso de duas tabelas relacionadas sendo
       movidas juntas na mesma operação: a primeira table processada rerroteia o relacionamento como
       externo (parceira ainda não moveu), e quando a segunda tabela é processada e o relacionamento
       é rerroteado de novo, as duas já estão na cena nova → vira `RelationshipView` de verdade.
    - `demoteRelationship()`/`promoteRelationship()`/`relocateRelationshipView()`/
      `reevaluateTableRelationships()`/`BaseTableView::relocateCutoffItem()` — todos **removidos**,
      substituídos por essa sequência única dentro de `relocateObjectView()` + `routeRelationship()`.
      `getRelationshipPartner()` sobrevive (ainda usado pra achar a view parceira na etapa 2).
    - **Decisão deliberada de não generalizar `handleObjectAddition()` em si** — cheguei a considerar
      extrair o switch inteiro pra um método compartilhado parametrizado por cena, mas
      `handleObjectAddition()` carrega efeitos colaterais que não fazem sentido pra um re-roteamento
      (`blink_new_objs`, `setModified()`, `emit s_objectAdded()`) — `routeRelationship()` duplica só
      as ~15 linhas do case `Relationship`/`BaseRelationship`, isolado, sem tocar nesse choke point
      já bem testado.
    - **Teste de regressão novo**: `ModelWidgetTest::relocatingTablesTogetherPreservesTheirRelationship`
      — primeira versão tornou `ModelWidgetTest` `friend class` de `ModelWidget` pra poder chamar
      `relocateObjectView()` (privado); usuário achou desnecessário e pediu pra deixar o método
      público em vez disso — mais simples, sem sujar `ModelWidget` com uma amizade só pra teste, e
      `relocateObjectView()` já é útil como API pública de qualquer forma. `routeRelationship()`
      continua privado (só é chamado internamente por `relocateObjectView()`, o teste não precisa
      dele diretamente). Duas tabelas com relacionamento (`RelationshipView` real) relocadas juntas,
      na mesma ordem que `moveToSchema()` usaria; verifica que o relacionamento sobrevive como
      `RelationshipView` de verdade na cena nova, e que `connected_rels`/`external_rels` das duas
      tabelas ficam corretos (1 conectado, 0 externo cada). Build limpo (`canvas`+`gui`+`pgmodeler`),
      suíte `modelwidgettest` **6/6** sem regressão. Binário reinstalado.
    - **Não testado ainda nesta sessão**: auto-relacionamento (`SrcTable == DstTable`) durante
      relocação — a lógica de `disconnectTables()` já tem uma guarda pra não remover duas vezes do
      mesmo `connected_rels` nesse caso (pré-existente, não tocada), e o redesenho não deveria mudar
      esse comportamento, mas vale confirmar manualmente.
  - **`relocateObjectView()` virou público, sem `friend class` (2026-09-23)** — pedido do usuário: a
    primeira versão do teste tornava `ModelWidgetTest` `friend class` de `ModelWidget` só pra poder
    chamar `relocateObjectView()` (privado); achou desnecessário, preferiu deixar o método público
    de vez (é útil como API de qualquer forma). `routeRelationship()` continua privado (só é chamado
    internamente). `friend class ModelWidgetTest;` removido.
  - **Bug real achado pelo usuário no teste manual (2026-09-23): "cutoff itens ficando nas cenas
    antigas quando suas tabelas são movidas e passam a fazer parte de um RelationshipView de
    verdade"** — causa raiz: o passo de "detach" em `relocateObjectView()` só limpava
    `external_rels` do lado da tabela **parceira** (`partner_view->removeExternalRelationship()`);
    do lado da própria tabela sendo relocada (`old_view`), o comentário original assumia que "o
    marcador cut-off é descartado por completo junto com `old_view`" — **suposição errada**.
    `CutoffItem` é um `QGraphicsItem` de topo, adicionado direto na cena
    (`this->scene()->addItem(cutoff_item)`, [basetableview.cpp:341](libs/libcanvas/src/basetableview.cpp#L341)),
    **não é filho Qt de `old_view`** — então `old_scene->removeItem(old_view)` nunca o toca. Ele só
    seria destruído quando `old_view` fosse de fato `delete`ado (dentro de `~BaseTableView()`), o
    que só acontece quando `old_scene` inteira é destruída — ou seja, o marcador ficava órfão,
    **visualmente renderizado na cena antiga**, pelo resto da vida daquela cena (até o modelo
    fechar). **Fix**: chamar `old_view->removeExternalRelationship(rel)` também, simétrico ao que já
    era feito pro parceiro — reaproveita a lógica já existente e testada (`delete cutoff_item;
    cutoff_item = nullptr;` quando a última relação externa sai), sem precisar de nenhum código novo
    de limpeza manual.
  - **Novo teste de regressão**: `ModelWidgetTest::relocatingTableIntoPartnersSceneClearsBothCutoffMarkers`
    — duas tabelas em schemas/cenas diferentes com relacionamento cross-scene (marcador cut-off dos
    dois lados), relocar uma pra cena da outra, confirmar que o relacionamento vira `RelationshipView`
    de verdade **e** que os dois marcadores cut-off desapareceram (`getCutoffItem() == nullptr` nos
    dois lados). Precisou tornar `BaseTableView::getCutoffItem()` público também (estava em
    `private slots:` desde o commit `0f7ac6f61` do próprio usuário — não é slot de verdade, só ficou
    naquela seção; movido pro `public:` junto dos outros getters de relacionamento). Primeira versão
    do teste tinha uma asserção forte demais (`scene_a->items().isEmpty()`) que falhou por motivo
    errado — a cena antiga ainda tem legitimamente o `SchemaView` do próprio schema (teste não
    reatribui o schema do modelo, só realoca a view) mais a infraestrutura padrão de toda cena
    (`rel_line`/`selection_rect`); corrigido pra checar especificamente a ausência de um `CutoffItem`
    entre os itens da cena, não a cena inteira vazia. Build limpo (`canvas`+`gui`+`pgmodeler`), suíte
    `modelwidgettest` **7/7** sem regressão. Binário reinstalado.
- **Bug reportado pelo usuário (2026-09-23) testando a rodada anterior de código: Ctrl+Z não
  devolvia uma tabela relocada pra sua cena original, e fechar o modelo em seguida disparava um
  relatório ASan** — investigação encontrou **dois bugs distintos, independentes e pré-existentes**
  em `libcore/operationlist.cpp` (arquivo não tocado por nenhuma mudança desta sessão):
  - **#1 — undo não sabe desfazer relocação de cena.** `OperationList::executeOperation()`
    ([operationlist.cpp:920-926](libs/libcore/src/operationlist.cpp#L920-L926)) já detecta a
    restauração de schema no undo (`graph_obj->getSchema()!=bkp_obj->getSchema()`), mas só marca os
    dois schemas com `setModified(true)` (repintura). Cadeia até a GUI conferida por completo
    (`OperationListWidget::undoOperation()` → `s_operationExecuted()` → handlers em
    `mainwindow.cpp:549-573`) — nenhum ponto resincroniza a view da tabela com a cena do schema
    restaurado. Gap estrutural: `libcore` não conhece `ObjectsScene` (é `libcanvas`), então o gancho
    teria que existir na GUI, e hoje não existe. **Fix aplicado, escopo mínimo**: em
    `ModelWidget::moveToSchema()`, a chamada `op_list->registerObject(obj, Operation::ObjModified, -1)`
    passou a rodar só para objetos que aceitam schema mas **não** são `BaseGraphicObject` — para
    Table/View/ForeignTable a mudança de schema continua acontecendo (`setSchema()` +
    `relocateObjectView()`), só que fica de fora do histórico de undo por ora, deixando a ação
    simplesmente não-desfazível em vez de meio-desfeita. **Não mexido**: o caminho de edição via
    diálogo (`relocateEditedObject()`, disparado por `openEditingForm()` em
    `modelwidget.cpp:2467-2471`) registra undo via `BaseObjectWidget::startConfiguration()`
    ([baseobjectwidget.h:288-317](libs/libgui/src/dbobjects/baseobjectwidget.h#L288-L317)), uma
    gravação genérica que cobre a edição inteira do objeto (não é específica de schema) — desativar
    ali quebraria undo de qualquer edição feita por esse diálogo. Se o usuário trocar o schema de uma
    tabela por esse caminho o mesmo sintoma pode aparecer; registrado como pendência, ainda por
    resolver de verdade (precisa de um mecanismo de re-roteamento pós-undo, não só suprimir o
    registro).
  - **#2 — leak confirmado, corrigido.** `bkp_obj` (alocado sob demanda por
    `CoreUtilsNs::copyObject()` em [operationlist.cpp:774](libs/libcore/src/operationlist.cpp#L774),
    ver "allocates if not allocated" em [coreutilsns.cpp:68-86](libs/libcore/src/coreutilsns.cpp#L68-L86))
    é uma variável local de `executeOperation()`, nunca `delete`ada — cada undo/redo de
    `ObjModified`/`ObjMoved` vazava uma instância nova do tipo do objeto, batendo com o relatório
    ASan (`CoreUtilsNs::copyObject<Table>`). Durante a investigação achei um segundo vazamento
    idêntico não reportado: `aux_obj` ([operationlist.cpp:719-732](libs/libcore/src/operationlist.cpp#L719-L732),
    alocado via `model->createTrigger()`/`createColumn()`/etc. pra objetos especiais referenciados
    por relacionamento) tem o mesmo padrão — só não apareceu no relatório porque o teste do usuário
    não mexeu com esse tipo de objeto. **Fix**: `delete bkp_obj; delete aux_obj;` logo após o último
    uso de ambos, antes do bloco final da função ([operationlist.cpp:946-948](libs/libcore/src/operationlist.cpp#L946-L948));
    `delete` em ponteiro nulo é no-op, cobre os casos em que um ou outro não foi alocado.
  - Build limpo (`core`+`gui`, `cmake --build cmake-build --target core gui`). Suíte
    `modelwidgettest` ainda não rerrodada pra essas duas mudanças especificamente. Nenhum binário
    completo/`install` gerado nesta rodada (build só das libs tocadas, a pedido do usuário).
- **Achado do usuário durante teste da 1.9.6 (2026-09-23), registrado no plano (intro da Fase 1),
  não implementado agora**: `Textbox` não tem schema, então não existe nenhum mecanismo (nem
  indireto) pra movê-lo entre cenas depois de criado — diferente de Table/View/ForeignTable, que
  ganharam isso na 1.9. Fica pendente pra quando threshold/reatribuição manual precisarem disso.
- **Nova subtarefa registrada no plano (2026-09-22), ainda não iniciada: 1.15 — cor exclusiva
  para o marcador cut-off.** Hoje `CutoffItem` reaproveita `Attributes::PositionInfo`; usuário
  pediu identidade visual própria, o que implica chave nova em `Attributes::`, entrada nova em
  `AppearanceConfigWidget::element_cmb`/`conf_ids`/`obj_conf_ids` e atualização do arquivo de
  configuração de aparência. Depende de: 1.8 (já feita) — pode ser encaixada a qualquer momento,
  não bloqueia nem depende da 1.10 em diante.
- **Achado colateral, sem relação com multi-cena (2026-09-23, `dedf21c1e`)**: usuário testando com
  ASan pegou um crash (`new-delete-type-mismatch`) ao abrir/fechar o formulário de edição de tabela
  — `Element::~Element()` não era `virtual`, então `ElementsTableWidget::~ElementsTableWidget()`
  (`delete handled_elem`, um `Element*` que na prática aponta pra `PartitionKey`/`IndexElement`/
  `ExcludeElement`) era UB clássico de destrutor não-virtual. Bug pré-existente, reproduzível em
  qualquer branch — `git log` confirma que nenhum commit desta feature tocou `element.h`/
  `elementstablewidget.*`. Corrigido com uma linha (`virtual ~Element() = default;`), a pedido do
  usuário, mesmo fora do escopo da 1.9. `make core` limpo, sem teste de regressão adicional além
  disso (fix isolado e mecânico).
- **1.8.1 fechada (2026-09-23, confirmada pelo usuário depois de uma auditoria de código
  ponta a ponta contra o texto da subtarefa no plano).**
  Commits: `3a4f4f4a9`/`02508f6ab` (movimentação livre + persistência XML),
  `0281a1d85` (linha dinâmica tabela↔marcador + clamp geométrico contra o retângulo),
  `95d09e331` (caixa do rótulo reposicionada por direção contínua em vez de offset fixo; guard de
  órbita impedindo o marcador de invadir o retângulo da tabela), `699b3562b` (2026-09-23 — fix do
  bug real de persistência: a posição do cutoff nunca chegava no XML da tabela, porque
  `setPositionAttribute()` serializava a posição da própria tabela em vez da do marcador; guard
  `repositioning` removido de propósito, não por engano — decisão do usuário, ver detalhamento
  abaixo). Subtarefa cobre movimentação livre do cutoff + persistência de posição em XML. Decisões
  fechadas pelo usuário antes de codar: posição
  relativa ao anchor (não absoluta), e offset descartado (não restaurado) quando o cutoff é
  destruído e um novo nasce depois na mesma tabela.
  - `CutoffItem` ganhou a flag `ItemIsMovable`. Posição é sincronizada em duas direções:
    `updatePosition()` (table→cutoff, dispara em qualquer movimento/resize da tabela) lê
    `BaseTable::getCutoffPosition()` e aplica `anchor + offset`; `itemChange(ItemPositionHasChanged)`
    (cutoff→table, dispara quando o usuário arrasta o marcador) grava
    `this->pos() - anchor` de volta na tabela via `BaseTable::setCutoffPosition()`. Um member
    booleano `repositioning` guarda o primeiro caminho pra não realimentar o segundo (sem isso,
    todo `updatePosition()` reescreveria a mesma posição de volta no modelo, marcando-o modificado
    à toa a cada vez que a tabela é movida).
  - Persistência: nova `QPointF cutoff_pos` em `BaseTable` (core), sentinela `QPointF(DNaN, DNaN)`
    pra "sem posição customizada" (mesma técnica já usada em `BaseRelationship::lables_dist`).
    `setCutoffPosition()`/`getCutoffPosition()`/`resetCutoffPosition()` públicos;
    `setCutoffPositionAttribute()` protegido, chamado de `PhysicalTable::setTableAttributes()`
    (cobre Table+ForeignTable) e `View::getSourceCode()`, logo depois de `setPositionAttribute()`.
  - `BaseTableView::removeExternalRelationship()` chama `resetCutoffPosition()` no momento em que
    o `CutoffItem` é destruído (última relação cross-scene da tabela virou in-scene) — implementa
    a decisão "descartar, não restaurar".
  - **Formato XML — revisado após review do usuário**: primeira versão usou dois atributos simples
    (`cutoff-x`/`cutoff-y`) direto na tag de table/view/foreigntable; usuário pediu troca por um
    **segundo `<position>`** (a tabela já emite um pra si mesma), diferenciado por `id="cutoff"`
    (a posição da própria tabela não leva `id`). Implementação: `position.sch` (compartilhado por
    tabela, pontos de relacionamento, distância de rótulo etc.) ficou **intocado** — o motor do
    `SchemaParser` exige que todo `{atributo}` referenciado num `.sch` esteja pré-declarado no mapa
    de QUALQUER chamador, então um `%if {id}...%end` ali quebraria (`UnkownAttribute`) todo
    chamador existente que nunca declarou `id` (confirmado na prática: carregar um `.dbm` com
    textbox quebrou já na primeira tentativa). Solução: novo `.sch` dedicado
    (`assets/schemas/xml/cutoff-pos.sch`) com `id="cutoff"` fixo no literal do template (não é
    variável, não exige declaração no chamador). `baseelements.dtd` ganhou `<!ATTLIST position id
    CDATA #IMPLIED>` (validação de DTD é por nome de elemento, não por qual `.sch` gerou); os DTDs
    de table/view/foreigntable passaram a aceitar um segundo `position?` opcional. Leitura
    centralizada no handler já existente de `<position>` em `DatabaseModel::setBasicAttributes()`
    (mesmo lugar que já lia a posição da própria tabela) — agora checa `attribs[Attributes::Id]==
    Attributes::Cutoff` pra decidir se é a posição do objeto ou a do cutoff.
  - **Movimento livre não funcionava — dois bugs reais, corrigidos em duas rodadas**:
    1. `QGraphicsItemGroup::boundingRect()` fica congelado no tamanho que os filhos tinham no
       momento do `addToGroup()` (mesmo bug já achado antes pro `obj_selection` invisível e pro
       retângulo artefato do `paint()` — nunca tinha sido corrigido na raiz, só contornado caso a
       caso). Como só a `line` tinha geometria final antes de entrar no grupo, o bounding rect (e
       portanto a área de hit-test do grupo, já que `shape()` deriva dele por padrão) ficava
       restrito a uma fatia minúscula perto da linha — clicar no rótulo visível (a caixa
       arredondada, alvo óbvio) não atingia o grupo. `CutoffItem` ganhou override de
       `boundingRect()` computando a união de `line`+`label_bg` a cada chamada, com
       `prepareGeometryChange()` em `updateLabel()` (o texto "Cross rels: N" muda de tamanho
       conforme relações são adicionadas/removidas).
    2. Mesmo com o hit-test corrigido, o arrasto continuava sem efeito visível persistente. Causa
       raiz: `CutoffItem` nunca setava a flag `QGraphicsItem::ItemSendsGeometryChanges`. Sem ela,
       `QGraphicsItem::setPos()` usa um atalho que aplica a posição direto e **nunca chama
       `itemChange()`** — então o handler de `ItemPositionHasChanged` (que persiste o offset
       arrastado na tabela) nunca rodava. O item provavelmente até se movia visualmente durante o
       arrasto (o atalho ainda aplica a posição), mas como o offset nunca era salvo no modelo, a
       próxima vez que qualquer sinal de movimento/redimensionamento da tabela disparasse
       `updatePosition()` ele voltava pra posição antiga — lido pelo usuário como "não faz nada".
       Confirmado por comparação: `BaseObjectView` (base de todo objeto arrastável do pgModeler)
       seta essa flag explicitamente no construtor; `CutoffItem` era o único item que não herdava
       isso (não deriva de `BaseObjectView`).
    3. Usuário apontou (2026-09-22) que havia exclusões explícitas do `CutoffItem` em
       `ObjectsScene`, sobra da 1.8 (quando o cutoff era propositalmente fixo/não-movível):
       `finishObjectsMove()` pulava o item inteiro (sem alinhar à grade, sem entrar no `sel_rect`
       de auto-scroll) e `alignObjectsToGrid()` (comando de menu) tinha um branch vazio só pra
       excluí-lo do alinhamento em massa. As duas fazem sentido quando "mover independente da
       tabela" era sempre um bug a evitar — agora que é o comportamento pretendido, são
       inconsistências, não proteções. Ambas removidas; `CutoffItem` cai nos mesmos branches
       genéricos que qualquer outro item (com `ItemSendsGeometryChanges` já setado, o `setPos()`
       de alinhamento à grade dispara `itemChange()` e persiste a posição normalmente).
       `#include "cutoffitem.h"` em `objectsscene.cpp` ficou temporariamente sem uso (readicionado
       no fix nº4 abaixo). Essa limpeza sozinha não resolveu o problema relatado.
    4. **Causa raiz real, achada só depois de um teste automatizado (headless, via `QTest::mousePress`/
       `mouseMove`/`mouseRelease` num `QGraphicsView` de verdade) confirmar que o mecanismo de
       seleção+arrasto funcionava perfeitamente quando o relacionamento cross-scene era criado
       programaticamente (`db_model->addRelationship()` direto) — só falhava com um modelo real,
       onde o usuário quase certamente já usou a ferramenta interativa de criar relacionamento em
       algum momento.** `ObjectsScene::showRelationshipLine()` (chamada ao ligar/desligar essa
       ferramenta) varre **todos os itens da cena** (`this->items()`, não filtrado) e chama
       `setFlags()` (substitui o conjunto inteiro de flags, não só uma) incondicionalmente em cada
       um. Para qualquer item que não seja `BaseObjectView`, cai no default definido no topo do
       laço: `ItemIsSelectable | ItemSendsGeometryChanges` — **sem `ItemIsMovable`**. Como
       `CutoffItem` não deriva de `BaseObjectView`, a primeira vez que qualquer relacionamento é
       desenhado pela ferramenta interativa (em qualquer lugar da cena, não só perto do cutoff),
       todo `CutoffItem` existente perde `ItemIsMovable` de forma permanente — sobrevivem só
       `ItemIsSelectable` e `ItemSendsGeometryChanges`, batendo exatamente com o sintoma relatado
       (seleciona, mas arrastar não faz nada). Fix: `CutoffItem` ganhou tratamento explícito nesse
       laço, recebendo `ItemIsMovable` de volta quando `value=false` (ferramenta desligada), mesmo
       padrão de restauração já usado pros objetos `BaseObjectView` elegíveis.
  - **Achado de infra**: o binário roda com `SCHEMASDIR` apontando pra `build/schemas` (instalado
    via `PGM_PREFIX=/home/raphael/pgmodeler/build`), não direto pra `assets/schemas` — editar os
    `.sch`/`.dtd` em `assets/` não basta, é preciso rodar `cmake --build . --target install` (de
    dentro de `cmake-build`) pra sincronizar. `install(DIRECTORY ...)` do `InstallAssets.cmake`
    copia o conteúdo atual da pasta a cada `install` (não precisa reconfigurar o CMake pra um
    arquivo `.sch` novo ser pego).
  - **Verificado via programa standalone** (não é teste automatizado permanente, só validação
    pontual — não commitado): linkado direto contra `libcore`/`libparsers`/`libutils` já buildados,
    confirmando round-trip completo — `BaseTable::getSourceCode()` emite
    `<position id="cutoff" x=".." y=".."/>` quando setado; `DatabaseModel::loadModel()` de um
    `.dbm` com esse elemento restaura `getCutoffPosition()` corretamente; tabela sem o elemento
    carrega com posição NaN (unset, retrocompatível); reserializar depois de carregar preserva o
    valor.
  - **Gap conhecido, não implementado nesta subtarefa**: arrastar o cutoff não entra no sistema de
    undo/redo (`op_list`/operation chain) — `setCutoffPosition()` marca o modelo como modificado
    (`setCodeInvalidated`), mas Ctrl+Z não desfaz o arrasto como desfaz mover uma tabela. Não foi
    pedido explicitamente pelo usuário; registrado aqui pra decidir depois se vira subtarefa própria.
  - Build limpo (`core`+`canvas`+`gui`+`pgmodeler`) e `modelwidgettest` 4/4 sem regressão.
    **Testado manualmente pelo usuário (2026-09-22): "funcionou em partes"** — o arrasto em si
    já funciona, mas o usuário tem melhorias e problemas adicionais a reportar antes de fechar a
    subtarefa. Commit feito como WIP a pedido do usuário; próxima sessão/mensagem traz os
    apontamentos específicos.
  - **Duas correções pedidas pelo usuário na sessão seguinte (2026-09-22)**:
    1. **Linha deve ligar centro da tabela ao centro do retângulo do marcador** — antes a linha era
       um segmento fixo `QLineF(0, 0, MaxOffsetDistance, 0)` (renomeado de `LineLength`) só dentro
       do sistema de coordenadas local do grupo, sempre do mesmo tamanho, sem relação real com a
       geometria da tabela. Novo método `CutoffItem::updateLine()` recalcula os dois pontos a cada
       chamada: ponta da tabela = `table->sceneBoundingRect().center()` convertido pra coordenada
       local do grupo (`- this->pos()`, válido por não haver rotação/escala); ponta do marcador =
       centro de `label_bg` (já conhecido em coordenada local). Chamado em três pontos pra cobrir
       todo caminho de mutação: fim de `updateLabel()` (tamanho do rótulo muda, ex. contagem de
       relacionamentos), fim de `updatePosition()` (tabela pode redimensionar sem mover o `pos()`
       do grupo — título intocado, mas o centro da tabela desce), e dentro de
       `itemChange(ItemPositionHasChanged)` (arrasto do usuário). A posição por-relação do
       `box_pos` dentro do grupo (`QPointF(MaxOffsetDistance, -altura/2)`) não mudou — só a linha
       passou a ser desenhada dinamicamente em vez de fixa.
    2. **Movimento livre, mas com margem** — usuário só queria o suficiente pra tirar o marcador de
       cima dos conectores da tabela, não movimento irrestrito. Adicionado o branch
       `ItemPositionChange` (hook do Qt chamado **antes** de aplicar a posição, valor de retorno vira
       a posição de fato aplicada — mecanismo padrão pra restringir arrasto) em
       `CutoffItem::itemChange()`. Sem guarda de `repositioning` — aplica também durante o
       `setPos()` programático de `updatePosition()` (não deveria ter efeito prático, já que o
       offset persistido já nasce validado por este mesmo clamp quando o usuário arrastou, mas
       também funciona como rede de segurança pra um offset fora do limite salvo antes deste fix
       existir).
       - **Primeira versão (errada) — corrigida na mesma sessão**: clampava a distância euclidiana
         entre a posição proposta e `getCutoffAnchorPoint()` (um ponto fixo) pra um círculo de raio
         `MaxOffsetDistance` ao redor do anchor. Usuário apontou que a regra real é por eixo e
         relativa ao **retângulo da tabela inteira**, não a um ponto fixo: se o marcador está à
         esquerda/direita da tabela, só a distância **horizontal** até a tabela é limitada
         (verticalmente pode deslizar livre ao longo da lateral); se está acima/abaixo, só a
         distância **vertical** é limitada; nos cantos, os dois eixos entram em jogo ao mesmo tempo
         (caso de borda que views precisa de tratamento próprio). Reescrito pra clamp geométrico
         contra o retângulo, não contra um ponto: acha o ponto mais próximo em
         `table->sceneBoundingRect()` da posição proposta (`qBound()` por eixo — equivale a "ponto
         mais próximo num retângulo"), mede a distância euclidiana **desse ponto** até a posição
         proposta, e só recorta se ultrapassar `MaxOffsetDistance`. Essa única fórmula cobre os três
         casos pedidos automaticamente: quando o marcador está lateral à tabela (dentro da faixa
         vertical do retângulo), o ponto mais próximo fica na mesma altura → a distância medida é
         puramente horizontal; quando está acima/abaixo (dentro da faixa horizontal), o ponto mais
         próximo fica na mesma coluna → distância puramente vertical; nos cantos, o ponto mais
         próximo é o vértice do retângulo → distância euclidiana até o vértice, resultando num
         quarto-de-círculo suave na transição entre lateral e topo/base (o "edge case" pedido). Na
         prática, deixa o marcador "orbitar" a tabela inteira dentro de uma faixa de largura
         constante `MaxOffsetDistance`, incluindo os cantos.
    Build limpo (`canvas`+`gui`+`pgmodeler`), `modelwidgettest` 4/4 sem regressão, binário
    reinstalado (`cmake --build . --target install`) pra refletir no `build/pgmodeler` que o usuário
    roda. Nenhum arquivo `.sch`/`.dtd` tocado nesta rodada (só `cutoffitem.h`/`.cpp`). Commitado
    (`0281a1d85`).
  - **Terceira rodada: caixa do rótulo desalinhada da linha, reportada com capturas de tela
    (2026-09-22)** — usuário testou o resultado da rodada anterior e reportou, com screenshots, que
    a distância entre a caixa "Cross rels: N" e a tabela não era uniforme conforme o marcador era
    arrastado ao redor da tabela, e que no pior caso (cutoff arrastado pra esquerda da tabela) a
    caixa sumia atrás dela. Causa raiz: `updateLabel()` posicionava `label`/`label_bg` num offset
    local **fixo** em relação a `this->pos()` (`QPointF(MaxOffsetDistance, -altura/2)`) — sempre à
    direita, herdado da 1.8 quando `pos()` era fixo no canto do título e a caixa só precisava
    "flutuar" pra direita. Na 1.8.1, `pos()` virou o próprio ponto arrastável (já clampado a no
    máximo `MaxOffsetDistance` de qualquer lado da tabela), mas a caixa continuava desenhada sempre
    à direita dele — arrastando o marcador pra esquerda da tabela, o offset `+MaxOffsetDistance` em
    x empurrava a caixa de volta em direção à tabela (podendo cair dentro dela); acima/abaixo, a
    caixa sempre deslocava pra direita em vez de acompanhar o eixo certo, daí a distância não
    uniforme. A linha (`updateLine()`) em si estava correta — ela só documentava fielmente a
    posição errada da caixa, não causava o problema. Perguntado ao usuário se a orientação da caixa
    devia ser por "snap cardinal" (lado dominante, sem rotação) ou "direção contínua" (vetor exato
    tabela→`pos()`, girando suavemente) — escolhida a **direção contínua**. Implementação: novo
    `CutoffItem::getOutwardDirection()` (vetor unitário de `table->sceneBoundingRect()` até
    `this->pos()`, via `getClosestTablePoint()` novo, fatorado do clamp de arrasto já existente) e
    `updateLabelPosition()` (recalcula `box_pos` a partir dessa direção, `dir.x()/dir.y() ∈ [-1,1]`
    escalando `MaxOffsetDistance` por eixo — reduz exatamente à fórmula antiga quando `dir=(1,0)`).
    Chamado não só de `updateLabel()` (texto/tamanho muda) como também de `itemChange
    (ItemPositionHasChanged)` e `updatePosition()` (posição/geometria da tabela muda) — antes só a
    linha era recalculada nesses pontos, a caixa nunca.
  - **Guard de órbita: cutoff não pode mais invadir o retângulo da tabela (2026-09-22)** — pedido
    explícito do usuário depois da rodada acima: o marcador deve só "orbitar" a tabela, nunca ficar
    "em cima" dela. Causa raiz do gap: o clamp de `ItemPositionChange` (rodada anterior, ponto 2
    acima) só limitava a distância **máxima** pra fora do retângulo — para um ponto proposto **já
    dentro** do retângulo, `qBound()` por eixo é um no-op (devolve o próprio ponto), então a
    distância medida dava 0 e nada impedia o arrasto pra dentro da tabela. Novo
    `CutoffItem::clampToTableOrbit()` (substitui o corpo inline de `ItemPositionChange`): se o ponto
    proposto cai dentro do retângulo, empurra pra borda mais próxima em vez de aceitá-lo; caso
    contrário, mantém o comportamento já existente (recorta a no máximo `MaxOffsetDistance` pra
    fora). A decisão de qual borda é mais próxima ficou num helper compartilhado,
    `getNearestEdgeNormal()`, usado tanto pelo clamp quanto por `getOutwardDirection()` — esta
    última precisava do mesmo ajuste, já que caía num fallback fixo "direita" sempre que `pos()`
    estava exatamente sobre uma borda (distância 0 pro `getClosestTablePoint()`, vetor nulo); sem
    o fallback certo, um cutoff encostado na borda esquerda/superior teria a caixa desenhada de
    volta pra dentro da tabela. Commitado (`95d09e331`, sem coautoria a pedido do usuário).
  - **Regressão introduzida por um refactor do usuário (clang-tidy), achada e corrigida na mesma
    sessão** — depois do commit acima, o usuário fez refatorações próprias motivadas por warnings do
    clang-tidy (largura de linha/borda escalada por DPI via novo `getLinePenWidth()`, reorganização
    de `public`/`private` no header) e pediu pra eu só ajustar uma warning residual: falta de
    parênteses explícitos em `updateLabelPosition()` entre os operadores `*` e os termos de soma,
    em `dir.x() * (box_size.width() / 2 + MaxOffsetDistance) - box_size.width() / 2`. Corrigi
    adicionando parênteses — mas o usuário reportou, no teste seguinte, que o deslocamento original
    (rodada da caixa desalinhada, ver acima) tinha voltado. Investigação: a reescrita que eu apliquei
    preservava a matemática original corretamente, mas notei que o arquivo no disco já continha uma
    reescrita diferente, feita antes da minha (provavelmente por um auto-fix do clang-tidy que o
    usuário aplicou) — `(dir.x() * (box_size.width() / 2)) + MaxOffsetDistance - (box_size.width() / 2)`
    — que **redistribuiu errado**: tirou `MaxOffsetDistance` de dentro da multiplicação por
    `dir.x()`/`dir.y()`, somando ele **inteiro e sem escala** em ambos os eixos sempre, em vez de só
    proporcionalmente ao eixo em que o marcador de fato estava afastado (ex.: cutoff embaixo da
    tabela, `dir=(0,1)`, deveria ter offset horizontal zero, mas a fórmula errada somava `+20` no x
    de qualquer jeito). Fix: reescrita para `(dir.x() * ((box_size.width() / 2) + MaxOffsetDistance))
    - (box_size.width() / 2)` — parênteses explícitos em todo subtermo (silencia o linter) sem mudar
    o resultado numérico da fórmula original. Só a edição pontual feita, sem build completo desta
    vez (novo hábito de trabalho combinado nesta sessão — ver item 8 na lista de convenções mais
    abaixo). Commitado junto com a correção de persistência XML abaixo (`699b3562b`), já que
    nenhum dos dois tinha sido commitado individualmente até então.
  - **Bug real: posição do cutoff nunca era persistida no XML da tabela (2026-09-23)** — usuário
    começou um refactor próprio pra reaproveitar `BaseGraphicObject::setPositionAttribute()` (em
    vez do `BaseTable::setCutoffPositionAttribute()`/`cutoff-pos.sch` dedicados da 1.8.1 original)
    e, nesse caminho, notou que a posição do cutoff no XML não acompanhava o arrasto do marcador.
    Primeira suspeita do usuário: o guard `repositioning` em `CutoffItem` (comentou ele, sem
    resolver o problema, e me pediu pra investigar). Causa raiz real, achada por leitura de código:
    `BaseGraphicObject::setPositionAttribute(pos_attr, id)` sempre serializava `this->position` — a
    posição **da própria tabela** — não importa qual `pos_attr`/`id` fosse passado; os call sites do
    cutoff (`physicaltable.cpp`/`view.cpp`, `setPositionAttribute(Attributes::CutoffPos,
    Attributes::Cutoff)`) só trocavam o nome do atributo/`id` gravado, nunca a fonte dos números —
    então o `cutoff-pos` no XML sempre espelhava o `<position>` normal da tabela, e só "mudava"
    quando a própria tabela se movia (nunca quando só o marcador era arrastado). O `repositioning`
    nunca teve relação com isso — ele só controlava se `BaseTable::setCutoffPosition()` (setter em
    memória, não geração de XML) rodava durante o reposicionamento programático.
    - **Fix, já implementado pelo usuário antes da minha resposta** (eu só confirmei por leitura de
      código + `make core`): `setPositionAttribute()` virou duas sobrecargas — a antiga (sem
      parâmetros, mantém o comportamento de sempre serializar `position`) e uma nova,
      `setPositionAttribute(const QPointF &pos, const QString &pos_attr, const QString &id = "")`,
      que serializa um ponto explícito. `PhysicalTable`/`View` passaram a chamar
      `setPositionAttribute(cutoff_pos, Attributes::CutoffPos, Attributes::Cutoff)`.
      `BaseTable::setCutoffPositionAttribute()` e o `.sch` dedicado (`cutoff-pos.sch`) foram
      removidos — `position.sch` (compartilhado) ganhou `%if {id}...%end` de verdade. O risco que
      motivou o `.sch` separado na primeira versão da 1.8.1 (`UnkownAttribute` se `id` não estiver
      pré-declarado por TODO chamador do template) foi resolvido porque a sobrecarga nova sempre
      atribui `attributes[Attributes::Id] = id` (vazio por padrão) antes de chamar o template —
      todo caminho que passa por `position.sch` agora pré-declara `id`, não só o do cutoff. Detalhe
      completo registrado no plano, subtarefa 1.8.1 (nota "Atualização 2026-09-23 (consolidação)").
    - **Decisão do usuário sobre o `repositioning`: não restaurar, de propósito** — eu tinha
      sugerido restaurar o guard já que ele não era a causa do bug. Usuário discordou: quer que
      **todo** reposicionamento do cutoff (inclusive o eco programático de quando a própria tabela
      se move) chame `BaseTable::setCutoffPosition()`, que sempre marca `setCodeInvalidated(true)`
      — o objetivo é sinalizar "modelo modificado" ao usuário de forma consistente, mesmo quando o
      offset recalculado é numericamente igual ao anterior. `repositioning` foi removido por
      completo (membro, comentário de doc, os dois pontos de uso) — não é uma pendência, é
      definitivo.
    Build de conferência: `make core` (já estava com cache válido — usuário tinha buildado antes de
    reportar). Commitado (`699b3562b`, sem coautoria a pedido do usuário).
- **1.9 implementada em seis subtarefas (2026-09-23, `2ef548b59`/`ab6823a8f`/`04aea9ef2`/
  `f4b104d04`/`4f8a615e8`/`97146b4a7`), mais um commit de estilo (`7adc1c7a7`) — todas testadas e
  aprovadas pelo usuário (inclui confirmação por ASan dos dois crashes achados, 1.9.4 e a checagem
  geral em 2026-09-23).** Ver detalhamento subtarefa a subtarefa logo abaixo.
- **1.9 iniciada (2026-09-23)**: antes de codar, revisão do texto da subtarefa contra o código
  atual de `moveToSchema()` achou uma lacuna real — ver nota "Achado ao implementar" na subtarefa
  1.9 do plano. Dividida em duas subtarefas, um commit cada (pedido do usuário).
  - **1.9.1 — relocação de cena em `moveToSchema()`** (`2ef548b59`): `ObjectsScene::relocateItem
    (item, dst_scene)` novo — reaproveita um `detachItem()` privado (extraído do corpo antigo de
    `removeItem()`) que desconecta sinais e some da lista interna do Qt sem empilhar em
    `removed_objs`; `removeItem()` (real remoção) continua empilhando normalmente, só que agora via
    o helper compartilhado. `moveToSchema()` passou a resolver `cur_scene`/`dst_scene` via
    `item_view->scene()`/`getSchemaScene(schema)` e chamar `cur_scene->relocateItem(item_view,
    dst_scene)` quando diferem, antes do reposicionamento perto da caixa do schema novo (que já
    existia). Build de conferência: `make canvas` + `make gui`, ambos limpos.
  - **1.9.2 — promoção/rebaixamento de fato** (`ab6823a8f`): `ModelWidget::
    reevaluateTableRelationships()` (chamado logo após a relocação de 1.9.1) tira um snapshot de
    `connected_rels`/`getExternalRelationships()` da tabela movida (mutados durante a própria
    iteração, por isso o snapshot) e, pra cada relacionamento, compara a cena do parceiro
    (`getRelationshipPartner()` novo, resolve "a outra tabela" do relacionamento) com a cena atual
    da tabela — chama `demoteRelationship()` (parceiro saiu da cena: destrói o `RelationshipView`,
    registra como conexão externa nas duas tabelas) ou `promoteRelationship()` (parceiro entrou na
    cena: remove das conexões externas, cria `RelationshipView` novo na cena compartilhada).
    **Bug real achado e corrigido nesta subtarefa** (detalhe completo no plano, subtarefa 1.9):
    `ObjectsScene::removeItem()` nunca limpava `BaseGraphicObject::receiver_object` (a referência
    reversa objeto-de-modelo → view) — só era limpa implicitamente quando a view chegava a ser
    deletada em C++, o que pra um item removido só acontece quando a cena dele é destruída
    (`removed_objs` drenado em `~ObjectsScene()`). Nunca importou antes porque o objeto de modelo
    sempre morria junto com a view em toda remoção existente; rebaixamento quebra essa suposição
    (o relacionamento sobrevive, só a view é descartada). Sem o fix,
    `BaseRelationship::getOverlyingObject()` continuaria retornando um ponteiro prestes a ficar
    pendurado em vez de `nullptr` depois de um rebaixamento — violando silenciosamente o invariante
    que todo o código de 1.6/1.7 (relacionamento cross-scene sem view formal) depende. Fix:
    `BaseObjectView::detachFromObject()` novo (limpa só a referência reversa, sem tocar a view),
    chamado de `ObjectsScene::removeItem()` logo depois de `detachItem()` — aplicado
    incondicionalmente ali (correção geral, não um caso especial do caminho de rebaixamento). Build
    de conferência: `make canvas` + `make gui`, ambos limpos. **Sem teste manual do usuário ainda**
    — aguardando.
  - **Correção de estilo (`7adc1c7a7`)**: usuário pediu, como preferência geral de código (não
    específica desta subtarefa), pra usar `auto &` em vez do tipo de ponteiro explícito
    (`BaseRelationship *`) em laços `for` sobre containers de ponteiros — os dois laços novos de
    `reevaluateTableRelationships()` (sobre `in_scene_rels`/`ext_rels`) foram os únicos casos no
    código gerado pra 1.9 até então. Registrado em memória pra aplicar em código novo daqui pra
    frente sem precisar ser lembrado de novo.
  - **1.9.3 — segundo caminho de troca de schema, achado pelo usuário (2026-09-23, `04aea9ef2`)**:
    usuário reportou que `BaseObjectWidget` (base de `TableWidget`/`ViewWidget`) tem seu próprio
    seletor de schema (`schema_sel`), aplicado em `applyConfiguration()` — um caminho totalmente
    independente do menu de contexto (`moveToSchema()`) pelo qual uma tabela/view pode trocar de
    schema, com a mesma lacuna: nada movia o item entre cenas nem reavaliava relacionamentos.
    `applyConfiguration()` já capturava `prev_schema` antes da troca (comentário original do campo:
    "used to update the schemas when moving tables/views from one to another"), mas
    `finishConfiguration()` só usava isso pra marcar o schema antigo como modificado — a intenção
    documentada no campo nunca foi totalmente implementada pro caso multi-cena. Fix: novo
    `ModelWidget::relocateEditedObject(BaseObjectWidget *obj_wgt)`, chamado de dentro do template
    `ModelWidget::openEditingForm(WidgetClass *widget, ...)` (o choke point único por onde todo
    formulário de edição de objeto passa) quando `res == QDialog::Accepted` e `WidgetClass` deriva
    de `BaseObjectWidget` — reaproveita `ObjectsScene::relocateItem()` (1.9.1) e
    `reevaluateTableRelationships()` (1.9.2) sem duplicar nada, só resolvendo `cur_scene`/
    `dst_scene` a partir de `obj_wgt->object`/`obj_wgt->prev_schema` (acesso direto via
    `friend class ModelWidget` já existente em `BaseObjectWidget`). Seguro por construção pra objeto
    recém-criado no mesmo formulário (nesse ponto, ou ainda sem view — `getOverlyingObject()` nulo —
    ou já inserido na cena certa por `handleObjectAddition()`, que roda antes do diálogo fechar) —
    não precisou de tratamento especial "objeto novo vs. editado". Build de conferência: `make gui`
    (é um método template, instanciado em vários pontos de `showObjectForm()` — só um build da lib
    inteira confirma todas as instanciações), limpo. **Sem teste manual do usuário ainda**.
  - **1.9.4 — crash real (heap-use-after-free), achado pelo usuário via ASan (2026-09-23,
    `f4b104d04`)**: usuário reportou, com log do AddressSanitizer anexado, que mover todas as
    tabelas de um schema pra outro e depois fechar o modelo completamente crashava a aplicação.
    Reprodução: modelo com múltiplos schemas/cenas, mover todas as tabelas de um schema pra outro
    deixando um auto-relacionamento (`SrcTable == DstTable`) na tabela movida — ao voltar pra cena
    do schema de origem (agora sem tabelas), sobrava um `RelationshipView` órfão lá.
    - **Leitura do log ASan**: heap-use-after-free em `BaseTableView::removeConnectedRelationship()`
      (basetableview.cpp:323), chamado de `RelationshipView::disconnectTables()`, chamado de
      `ObjectsScene::detachItem()`/`removeItem()`, chamado de dentro de `~ObjectsScene()`
      (objectsscene.cpp:147 — a fase que varre `this->items()` chamando `removeItem()` por tipo,
      relacionamentos primeiro). O ponteiro lido já tinha sido `delete`ado — rastreando a pilha de
      "freed by": um `TableView::~TableView()` chamado de **outro** `~ObjectsScene()`
      (objectsscene.cpp:158, o loop de dreno de `removed_objs`). Ou seja: duas cenas diferentes
      sendo destruídas no fechamento do modelo, cada uma limpando só os próprios itens — a segunda
      cena tinha uma `RelationshipView` cujo `tables[]` apontava pra uma `TableView` que já tinha
      sido liberada pela primeira cena minutos antes (no mesmo teardown).
    - **Causa raiz**: `getRelationshipPartner()` (1.9.2), pra um auto-relacionamento, resolve o
      parceiro como a própria tabela (`SrcTable == tab` → retorna `DstTable`, que é a mesma tabela).
      Depois que `tab_view` já foi relocado pra cena nova, `partner_view->scene() == tab_scene`
      fica trivialmente verdadeiro — `reevaluateTableRelationships()` concluía "parceiro já está na
      cena certa, nada a fazer" pra esse relacionamento. Mas **nada nunca relocava a
      `RelationshipView` em si** — `moveToSchema()` (1.9.1) e `relocateEditedObject()` (1.9.3) só
      chamam `ObjectsScene::relocateItem()` no item da própria tabela, nunca nos relacionamentos
      conectados a ela. Resultado: a view do auto-relacionamento ficava fisicamente presa na cena
      antiga, enquanto sua (única) tabela já vivia na cena nova — um ponteiro cruzado entre cenas
      que só explode quando as duas cenas são destruídas, cada uma sem saber da outra.
    - **Fix**: `reevaluateTableRelationships()` — quando o parceiro acaba na mesma cena da tabela
      movida (sempre verdadeiro pra auto-relacionamento; também verdadeiro quando duas tabelas
      diferentes de um mesmo relacionamento são movidas juntas no mesmo `moveToSchema()`), chama
      `relocateRelationshipView()` novo em vez de não fazer nada — reaproveita
      `ObjectsScene::relocateItem()` (1.9.1) pra mover a `RelationshipView` existente pra cena
      certa. Efeito colateral positivo: o caso de duas tabelas movidas juntas, que antes se
      autocorrigia via rebaixamento seguido de promoção imediata (funcional, mas descartava e
      recriava a view à toa), agora também usa esse caminho mais barato.
    - Build de conferência: `make gui`, limpo. **Confirmado pelo usuário (2026-09-23): crash
      resolvido.** Reproduziu o mesmo cenário (mover todas as tabelas de um schema, voltar pro
      schema de origem, fechar o modelo) sem o heap-use-after-free reportado no log ASan original.
  - **1.9.5 — preservar posição original ao relocar entre cenas, achado pelo usuário (2026-09-23,
    `4f8a615e8`)**: usuário reportou que, com segmentação por schema, os objetos movidos entre
    schemas estavam sendo empilhados na origem do retângulo do schema de destino — perdendo a
    posição original, indesejável pro usuário. Causa: `moveToSchema()` (comportamento herdado de
    antes do multi-cena) sempre reposicionava o item logo abaixo da caixa do `SchemaView` de
    destino depois de relocar (1.9.1), incondicionalmente — fazia sentido só quando todo schema
    compartilhava a mesma tela. Fix: bloco de reposicionamento removido por completo — o item
    mantém seu `(x, y)` original através da relocação de cena, igual ao que `relocateEditedObject()`
    (1.9.3) já fazia (nunca reposicionava, só relocava). Build de conferência: `make gui`, limpo.
    **Sem teste manual do usuário ainda**.
  - **1.9.6 — marcador cut-off órfão, mesma classe de bug da 1.9.4, achado pelo usuário (2026-09-23,
    `97146b4a7`)**: no mesmo modelo em que achou o bug da 1.9.4 (auto-relacionamento órfão), usuário
    reportou — dessa vez sem conseguir rastrear o ponto de falha sozinho — que repetir "mover todas
    as tabelas de um schema pra outro" deixava marcadores cut-off (`CutoffItem`) órfãos na cena de
    origem. Causa raiz: `BaseTableView::addExternalRelationship()`
    ([basetableview.cpp:343](libs/libcanvas/src/basetableview.cpp#L343)) adiciona o `cutoff_item`
    direto na cena em que a tabela estava no **momento da criação do marcador** — não é filho Qt do
    item da tabela, e nada no caminho de relocação (`relocateItem()`, 1.9.1) o toca. Depois da
    tabela relocada, o marcador ficava preso na cena antiga, ainda reagindo aos sinais da tabela
    (conexões Qt independem de cena) mas desenhado num lugar sem relação mais com ela — mesma classe
    de bug da 1.9.4 (algo "pendurado" na tabela que `relocateItem()` não sabe que precisa seguir
    junto), só que pro cut-off em vez do auto-relacionamento. Fix: `reevaluateTableRelationships()`
    relocaliza `tab_view->cutoff_item` (acesso direto, `ModelWidget` já é `friend` de
    `BaseTableView`) pra cena atual da tabela, antes de reavaliar qualquer relacionamento. Build de
    conferência: `make gui`, limpo. **Sem teste manual do usuário ainda**.
- **1.10 — destruição de cena ao remover schema (2026-09-23)**: `ModelWidget::removeScene(scn)`
  novo (mirror de `addScene()`, ver [modelwidget.cpp](libs/libgui/src/widgets/modelwidget.cpp)) —
  tira `scn` de `scenes`; se era a cena ativa, escolhe outra (a primeira restante, via
  `setActiveScene(0)` depois de zerar `active_scene_idx`/`scene` pra passar pelo guard de no-op de
  `setActiveScene()`) **antes** de destruir, ou desanexa o `viewport` (`setScene(nullptr)`) se não
  sobrou nenhuma cena; senão só recalcula `active_scene_idx` (a posição de `scene` mudou na lista
  depois do `removeOne()`); por fim `delete scn` e `updateSceneCombo()`. Log de aviso em build de
  debug (`#ifdef PGMODELER_DEBUG`) se a cena ainda tiver itens no momento da destruição — sinal de
  objeto órfão, não um caso esperado. `ModelWidget::handleObjectRemoval()` ganhou o gatilho: quando
  o objeto removido é um `Schema`, resolve a cena via `getSchemaScene()`, tira a entrada de
  `schema_scenes` e chama `removeScene()`. Não precisou de código novo pra limpar conexões externas
  apontando pro schema removido — a ordem de remoção em cascata (filhos antes do schema, já
  documentada na subseção "Remoção de schema e destruição de cena") garante que todo relacionamento
  cross-scene já passou pelo branch existente de `removeExternalRelationship()`
  (`handleObjectRemoval()`, adicionado na 1.8) antes do `Schema` em si chegar aqui — não há
  marcador cut-off "fantasma" a limpar neste ponto. Build limpo (`gui`+`pgmodeler`) e
  `modelwidgettest` 4/4 sem regressão (rodado com o binário ASan do usuário — os únicos leaks
  reportados pelo LeakSanitizer vêm de `DatabaseModel`/`ModelWidget` nunca destruídos no fim do
  teste, que nem chama `handleObjectRemoval()`; nenhum stack trace passa por `removeScene()` ou
  qualquer código desta subtarefa). Binário reinstalado (`cmake --build . --target install`).
  - **Correção do usuário, testado (2026-09-23)**: comportamento de roteamento/destruição
    confirmado correto no teste manual, mas o log de aviso em si estava furado. `scn->items()`
    inclui itens auxiliares que **toda** `ObjectsScene` sempre tem (`rel_line` — linha de borracha
    da ferramenta de criação de relacionamento, `selection_rect`, `layers_paths`, o item do timer
    de indicação de zoom, entre outros) — não só objetos de banco de dados. Como esses itens
    convivem com a cena por toda a vida dela, `!scn->items().isEmpty()` seria **sempre** verdadeiro,
    disparando o aviso mesmo numa cena legitimamente vazia de objetos de modelo. Usuário confirmou
    (documentação do Qt) que `~QGraphicsScene()` já destrói sozinha qualquer item que sobrar nela —
    esses auxiliares (e também um eventual `CutoffItem` remanescente) são limpos automaticamente,
    não precisam de checagem manual. **Fix**: a checagem virou um laço filtrando por
    `dynamic_cast<BaseObjectView *>` sem `parentItem()` — mesma classificação que
    `~ObjectsScene()` já usa pra achar os 6 tipos de objeto de banco de dados (table/view/
    foreigntable/schema/relationship/textbox, todos derivados de `BaseObjectView`); só dispara o
    aviso se sobrar um destes, ignorando a infraestrutura da própria cena. Build de `gui`+
    `pgmodeler` limpo, `modelwidgettest` 4/4 sem regressão, binário reinstalado. Não commitado
    ainda — aguardando teste manual do usuário.
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
8. **Build sob demanda: parar de compilar/instalar a aplicação inteira (ou testes) a cada
   subtarefa/edição** (2026-09-22). Antes disso, cada subtarefa fechava com um build completo
   (`gui`+`pgmodeler`, às vezes `modelwidgettest`) e `cmake --build . --target install` pra
   refletir no `build/pgmodeler` que o usuário roda. Usuário pediu pra isso parar: ele builda por
   conta própria e avisa se houver erro de sintaxe/compilação. No máximo um build rápido e pontual
   da lib/arquivo alterado (ex. `make canvas` depois de mexer em `libs/libcanvas/`) como checagem
   de sintaxe é aceitável — nunca encadear pra `gui`/`pgmodeler`/alvos de teste/`install` por conta
   própria a partir de agora.

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

## Undo/redo de relocação de cena + leak não resolvido (2026-09-23) — **WIP/instável, não commitar como fechado**

Retomando a pendência da entrada anterior ("Bug reportado pelo usuário... Ctrl+Z não devolvia uma
tabela relocada"): os dois fixes registrados lá (#1 supressão condicional de `registerObject()` em
`moveToSchema()`, #2 leak de `bkp_obj`/`aux_obj`) foram revisados nesta sessão. O #2 (leak de
`bkp_obj`/`aux_obj` em `OperationList::executeOperation()`) permanece corrigido e válido, sem
mudança. O #1 foi **revertido e substituído** por um mecanismo diferente, a pedido do usuário:

- **Correção do #1 rejeitada pelo usuário**: suprimir o registro no histórico de undo (deixando a
  ação simplesmente não-desfazível) foi a solução errada — undo/redo de `moveToSchema()` **já
  funcionava** antes desta rodada de refactors (confirmado pelo usuário, inclusive com a primeira
  versão baseada nos métodos `relocate*` antigos), então o gap era resolvível, não algo pra
  contornar suprimindo. `ModelWidget` deveria ser "inteligente o suficiente" pra rotear a tabela de
  volta pra cena certa a partir do schema já restaurado pelo undo, sem `OperationList`/`libcore`
  precisar saber nada sobre cena.
- **Mecanismo novo**: `OperationList` ganhou o sinal `s_operationExecuted(BaseObject *object,
  Operation::OperType op_type)` ([operationlist.h](libs/libcore/src/operationlist.h)), emitido ao
  final de `executeOperation()` ([operationlist.cpp:965](libs/libcore/src/operationlist.cpp#L965))
  pra toda operação restaurada (undo ou redo). `ModelWidget` conecta esse sinal **uma única vez**,
  no construtor, a `handleObjectSchemaChanged(BaseObject*, Operation::OperType)` — que filtra por
  `BaseTable::isBaseTable()` + `op_type` em `{ObjModified, ObjMoved}` antes de sequer olhar pra
  cena, e só chama `relocateObjectView()` quando a cena atual da view diverge da cena do schema já
  restaurado. `moveToSchema()` voltou a registrar `Operation::ObjModified` incondicionalmente,
  exatamente como antes desta sessão.
  - **Tentativa anterior, descartada**: a primeira versão conectava `BaseGraphicObject::
    s_objectModified` (sinal por instância, já usado por cada `TableView`/`GraphicalView` pra
    refresh de geometria) diretamente em `handleObjectAddition()`, uma conexão por objeto. Usuário
    encontrou em teste real que isso reagia **excessivamente** — qualquer edição de atributo
    (nome, coluna, comentário...) dispara esse sinal, não só mudança de schema, então o handler
    rodava a cada edição comum, mesmo que na prática fizesse nada (checagem de cena já seria
    no-op). Rejeitado por ruído/design, mesmo funcionando; substituído pelo sinal dedicado do
    `OperationList`, que só dispara em undo/redo de verdade.
  - Ver decisão #11 na seção 3 do plano pra motivação arquitetural completa.
- **Dois novos testes de regressão** em `modelwidgettest.cpp`:
  - `undoRedoOfTablesMovedTogetherKeepsExactlyOneRelationshipView`: duas tabelas relacionadas
    (relacionamento real, mesma cena) movidas juntas via a sequência exata de `moveToSchema()`
    (`registerObject`+`setSchema`+`relocateObjectView`, encadeadas com `startOperationChain()`/
    `finishOperationChain()`), depois um ciclo de undo+redo via `op_list` direto. Confirma que o
    relacionamento sobrevive como `RelationshipView` real e que existe exatamente uma view (não
    duplicada/órfã) depois do ciclo.
  - `undoRedoOfTableWithTwoCutoffRelationshipsLeavesNoOrphanedView`: uma tabela só, com dois
    relacionamentos cross-scene (cut-off) pra duas parceiras já co-localizadas no schema destino —
    o cenário relatado pelo usuário (ver abaixo). Move, undo, `setActiveScene()` pro destino (pra
    bater com os passos confirmados do usuário), redo. Mesma asserção de "exatamente uma view por
    relacionamento".
  - **Suíte `modelwidgettest` 9/9 passando**, sem regressão. Build só de `core`+`canvas`+`gui`+
    `modelwidgettest` — nenhum `install`/binário completo gerado nesta sessão.
- **Leak real, confirmado por ASan em teste manual do usuário, NÃO reproduzido em teste
  automatizado apesar de várias tentativas — investigação incompleta, retomando amanhã com o
  `.dbm` real do usuário**:
  - Relatório do usuário: `ERROR: LeakSanitizer: detected memory leaks`, duas alocações de
    `RelationshipView` (928B/2 objetos cada, via `QGraphicsEllipseItem` em `configureLine()`, mais
    32B/2 objetos cada via `configureCrowsFootDescriptors()`) nunca liberadas. Stack de ambas:
    `ModelWidget::routeRelationship()` ([modelwidget.cpp:2846](libs/libgui/src/widgets/modelwidget.cpp#L2846))
    ← `relocateObjectView()` (uma via o loop de `rels`, linha ~2911; outra via o loop de
    `ext_rels`, linha ~2914) ← `handleObjectSchemaChanged()` ← `OperationList::executeOperation()`
    ← `OperationList::redoOperation()` ← `OperationListWidget::redoOperation()` — confirmadamente
    um clique em "Refazer", não um artefato de undo.
  - **Passos confirmados com o usuário** (repro original + duas rodadas de perguntas): (1) tabela
    no schema `public` com marcador cut-off representando 2 relacionamentos cruzados, ambas as
    parceiras já no mesmo schema destino; (2) `moveToSchema()` pra esse schema destino (os dois
    relacionamentos viram `RelationshipView` reais); (3) undo (tabela volta pro `public`, os dois
    relacionamentos voltam a cut-off); (4) navega pra cena destino (`setActiveScene()`); (5) **redo**
    (confirmado explicitamente pelo usuário, não estava na narração original); (6) fecha a
    aplicação, ASan reporta o leak no `atexit`.
  - **Reprodução tentada, sem sucesso**: escrevi
    `undoRedoOfTableWithTwoCutoffRelationshipsLeavesNoOrphanedView` replicando exatamente essa
    sequência (1 tabela, 2 relacionamentos cut-off pra parceiras já co-localizadas, move, undo,
    `setActiveScene()`, redo) — passa limpo, sem leak, mesmo sob ASan/LSan direto (não só a suíte
    completa). Instrumentei temporariamente `relocateObjectView()`/`routeRelationship()` com
    `qDebug()` (removido depois) pra confirmar que o fluxo realmente segue o esperado: forward cria
    2 `RelationshipView`s (via o loop de `ext_rels`), undo detacha as 2 corretamente
    (`ObjectsScene::removeItem()`, que enfileira em `removed_objs` até a cena ser destruída — não é
    leak) e rebaixa pra cut-off, redo cria 2 `RelationshipView`s novas — tudo tracked, nada
    unreachable. Também tentei uma variação com 3 tabelas (uma parceira real + uma externa, testando
    os dois loops de `relocateObjectView()` disparando na mesma chamada) e um ciclo duplo de
    undo+redo — nenhuma reproduziu.
  - **Não é regressão desta sessão isoladamente identificável**: `operationlist.cpp`/`.h` só
    ganharam o sinal novo (aditivo); a lógica de `relocateObjectView()`/`routeRelationship()` é a
    mesma do commit anterior (`71fb13b09`). Não descartada a hipótese de que o bug é mais sutil e
    depende de alguma característica do modelo real do usuário (tipo de relacionamento,
    generalização, self-relationship, volume de objetos, ou alguma interação não capturada na
    narração dos passos) que a reconstrução sintética minimalista não está capturando.
  - **Decisão explícita do usuário**: parar por hoje, commitar tudo **inclusive o código com o leak
    não resolvido**, marcado como WIP/instável (não como subtarefa fechada). Usuário vai fornecer o
    `.dbm` real amanhã pra reprodução direta contra os dados reais, em vez de continuar
    reconstrução sintética por tentativa e erro.
- **Pendência para a próxima sessão**: carregar o `.dbm` do usuário (provavelmente via
  `DatabaseModel::loadModel()`/`SchemaParser` direto num teste, ou reproduzindo os passos na UI
  real) e repetir exatamente os 6 passos confirmados acima até o leak aparecer sob ASan — só então
  diagnosticar a causa raiz e propor fix. Não reabrir a hipótese de suprimir o registro de undo
  (#1 antigo) — já rejeitada explicitamente pelo usuário nesta sessão.

## Leak de RelationshipView — reproduzido e corrigido (2026-09-24)

Retomando a pendência acima. Usuário testou em **outra máquina** com o código recompilado do zero
e **não conseguiu mais reproduzir o leak** mesmo repetindo a sequência várias vezes — ficou em
dúvida se ainda existia. Anexou o `.dbm` real usado no relato original (renomeado aqui pra
`demo.txt`) pra eu tentar reproduzir.

- **Reprodução bem-sucedida com o modelo real**: escrevi um teste temporário (`QTest`, não
  commitado — instrumentação e teste removidos ao final da investigação) carregando o `.dbm` do
  usuário via `DatabaseModel::loadModel()`. Precisou de `db_model->createSystemObjects(false)`
  **antes** do `loadModel()` — replica exatamente `MainWindow::openModel()`
  ([mainwindow.cpp:1220](libs/libgui/src/mainwindow.cpp#L1220)), que cria os objetos de sistema
  (role `postgres` incluído) antes de carregar um arquivo existente; sem isso o load falha porque o
  arquivo referencia `postgres` sem nunca declará-lo (é um papel de sistema, não serializado).
  - **Cenário identificado no modelo real**: `table_b` (schema `public`) tem exatamente 2
    relacionamentos cross-scene — `table_b_has_many_table_d` (`rel1n`, `src-required="true"`, →
    `schema_a.table_d`) e `rel_view_b_table_b` (`reltv`, → `schema_a.view_a`) — batendo
    exatamente com "tabela com 2 relacionamentos cut-off, ambas as parceiras já no schema destino"
    do relato original. Sequência: mover `table_b` pra `schema_a` (mesma chamada que
    `moveToSchema()` usa), undo, `setActiveScene(schema_a)`, redo, destruir o `ModelWidget`.
  - **Leak confirmado sob ASan** logo na primeira tentativa com o modelo real: 3 alocações/624
    bytes (`QGraphicsEllipseItem` 464B + `QTransform` 144B + um ponteiro pequeno 16B), todas
    alocadas dentro de `RelationshipView::configureCrowsFootDescriptors()`/`configureLine()`
    durante a construção de uma `RelationshipView` no passo de redo — usuário depois confirmou o
    **mesmo relatório, com a mesma contagem exata (624 bytes, 3 alocações)**, reproduzindo a
    sequência na GUI de verdade (não só no teste).
  - **Diagnóstico inicial descartado com evidência concreta**: instrumentei temporariamente
    (`qDebug()`, removido depois) `ObjectsScene::removeItem()`/`~ObjectsScene()` e os pontos de
    roteamento em `ModelWidget`, rastreando ponteiro por ponteiro. Confirmei que as 4 instâncias de
    `RelationshipView` criadas ao longo da sequência (2 no movimento inicial, 2 no redo) são todas
    corretamente detectadas e enfileiradas em `removed_objs` pra deleção — descartando "objeto
    órfão nunca chega em `removeItem()`" como mecanismo.
  - **Causa raiz real, achada lendo `RelationshipView::configureCrowsFootDescriptors()`
    ([relationshipview.cpp:1555-1690](libs/libcanvas/src/relationshipview.cpp#L1555)) e
    `~RelationshipView()` lado a lado**: `round_cf_descriptors[tab_id]` (o círculo que desenha
    cardinalidade opcional na notação pé-de-galinha) é alocado uma vez, lazy, e a cada chamada de
    `configureLine()` (construção e qualquer refresh de geometria posterior) é **desanexado
    incondicionalmente** de `cf_descriptors[tab_id]` e de `this` (linhas 1657-1658), e só
    **reanexado** quando aquele lado da relação é opcional (`!mandatory[tab_id]`, linha 1680).
    Quando o lado é **obrigatório** (`mandatory[tab_id]` true — `src-required="true"` no caso de
    `table_b_has_many_table_d`), o círculo fica permanentemente desanexado — não é filho de nada,
    só um ponteiro vivo no member array. O destrutor (`~RelationshipView()`,
    [relationshipview.cpp:136-150](libs/libcanvas/src/relationshipview.cpp#L136)) **nunca
    referencia `round_cf_descriptors` diretamente** — só varre `cf_descriptor->childItems()` pra
    achar e deletar itens do pé-de-galinha. Pra um lado obrigatório, o círculo não é mais filho de
    nada, então essa varredura nunca o encontra — vaza, junto com seu `QTransform` interno (a
    rotação/posição que recebeu antes de ser desanexado) e um pequeno ponteiro incidental,
    batendo exatamente com as 3 alocações/624 bytes do relatório.
  - **Bug pré-existente, não introduzido pela feature multi-cena**: o mecanismo é usado por
    qualquer `RelationshipView` com pelo menos um lado obrigatório e notação pé-de-galinha
    habilitada — sempre existiu. Só nunca tinha aparecido como leak notado porque, no uso normal,
    relacionamentos raramente são destruídos e recriados (criados uma vez, vivem o resto da sessão)
    — o padrão de descartar+recriar introduzido por esta sessão pra relocação entre cenas passou a
    construir/destruir `RelationshipView`s com muito mais frequência, expondo o bug.
  - **Fix**: `~RelationshipView()` ganhou um laço explícito deletando `round_cf_descriptors[0]`/
    `[1]` incondicionalmente (sem depender de estarem anexados a `cf_descriptors`), rodando **antes**
    da varredura de `cf_descriptor->childItems()` — `delete` num item que já é filho de um grupo
    também o desanexa automaticamente (comportamento padrão do Qt), então a ordem evita
    double-free independente de qual dos dois estados (anexado/desanexado) o item estava.
  - **Verificado**: reconstruí o teste temporário com o fix aplicado — **zero leaks** reportados
    (antes: 3 alocações/624 bytes de leak; depois: nenhum). Suíte `modelwidgettest` completa
    (9/9) sem regressão sob `ASAN_OPTIONS=detect_leaks=0` (convenção já estabelecida pra rodar a
    suíte inteira — ruído pré-existente de libxml2/alocador std não relacionado à mudança, mesmo
    padrão já documentado em sessões anteriores). Teste temporário e toda a instrumentação de
    debug foram removidos ao final — não commitados.
  - **Decisão #11 do plano (seção 3) atualizada**: o mecanismo de undo/redo via
    `OperationList::s_operationExecuted` pode ser considerado fechado agora — o leak não tinha
    relação com esse sinal/mecanismo em si, era um bug independente em `~RelationshipView()`.
  - Build limpo (`core`+`canvas`+`gui`+`pgmodeler`+`modelwidgettest`). **Aguardando decisão do
    usuário sobre commit** (fix + esta atualização de plano/progresso ainda não commitados).
