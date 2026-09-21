# Progresso — Suporte a Múltiplas Cenas

> Log de execução do `MULTI_SCENE_PLAN.md`, atualizado sessão a sessão. Leia primeiro o plano
> (arquitetura, decisões fechadas, subtarefas) — este arquivo é só "onde paramos e por quê",
> para retomar rápido sem reler todo o histórico de commits.

## Estado atual

- Branch: `multi-scene-support-2.1.x`.
- **Fase 0 concluída (0.1 a 0.8).** Fase 1: **subtarefa 1.1 concluída, commitada e testada** —
  correção retroativa na 0.1 (wiring de sinais) e refinamento `pg_catalog`/`public` (achados do
  usuário), ambos testados manualmente e aprovados. Pronta pra seguir pra 1.2.
- **Próxima subtarefa: 1.2** — rotear Table/View/ForeignTable pra cena do schema; `Textbox` pra
  cena ativa (decisão do usuário, 2026-09-22, ver abaixo). Depende de 1.1 (pronta).
- **Convenção a partir de 2026-09-21**: `MULTI_SCENE_PLAN.md` fica só com o plano em si
  (arquitetura, decisões, descrição/dependência das subtarefas) — nunca marca subtarefa como
  feita/commitada. Todo status de execução (o quê foi feito, hash do commit, achados específicos
  de sessão) vive só aqui neste arquivo, atualizado junto de cada commit de subtarefa.
- **Convenção a partir de 2026-09-22**: ao terminar de implementar e validar (build) cada
  subtarefa, parar e deixar o usuário testar manualmente antes de prosseguir pra próxima — não só
  perguntar "commitar ou seguir", mas dar tempo/espaço pro teste manual entre elas.

## Commits desta branch (mais recente primeiro)

```
68bc47c11 Plan: document pg_catalog/public scene-activation handling
0300197fe Fase 1.1 (refinamento): pg_catalog never auto-activates, public forced default
71f8edaac Plan: document the addScene() signal-wiring architectural fix
f5c6eab4b Fase 1.1: create ObjectsScene per Schema on insertion
9e0c02972 Fase 0.1 (correção): addScene() wires every scene's signals, not just the first
7e4f314bc Plan: Textbox has no schema, routes to the active scene
c27badb0b Close Fase 0 in progress log, start Fase 1
e2521a6ab Fase 0.8: reserve DatabaseModel::SegmentationMode scaffolding
7422b4ab6 Scene DTD becomes a dedicated file, mirroring changelog.dtd exactly
50a6a59f3 Rename scene persistence key from idx to id, drop name attribute
8130b5eba Revise scene identity key from schema name to numeric idx
fbae3cb2c Update progress log: plan/progress separation, full commit list
3616afa2e Remove stale [FEITO, commitar] tags from the plan
9ee0eee19 Fase 3: SchemaView needs the same cross-scene suppression as relationships
548e773ad Close per-scene SceneRect persistence design; revert Workspace naming
c8236e1ae Update plan: Fase 0.7 done, pgmodeler-cli parity promoted to Fase 4
24e233aef Fase 0.7: print and image export iterate every scene, no spatial composition
191c51888 Fase 0.6: generalize Layers mutations across every scene
7f055ea44 Update MULTI_SCENE_PROGRESS.md: Fase 0.5 done, next is 0.6
9ad7c9073 Fase 0.5: use auto& for scene iteration, matching codebase style
f670ccbcd Update plan: correct 0.5's scope after user review
41874cdce Fase 0.5: ModelWidget::alignObjectsToGrid() applies to every scene
2136562a9 Update plan: threshold segmentation stays naive, clustering deferred as a third mode
9f22275a1 Add MULTI_SCENE_PROGRESS.md session log
eb94f5b0e Update plan: document connectToActiveScene() and the resize-order gotcha
394e405f5 Fase 0.4: ModelOverviewWidget reacts to s_activeSceneChanged
278612c26 Update plan: document addScene() and Qt-containers decision
5edaf29e0 Fase 0.3: fix eventFilter() scene identity check
66e7602b6 Fase 0.1 (refinamento): extract addScene() private helper
a66134e73 Fase 0.2: add setActiveScene() and s_activeSceneChanged
21c79dfd9 Fase 0.1: introduce scenes collection in ModelWidget
c29b147ce Add multi-scene support implementation plan
```

> A branch já foi recriada uma vez nesta iniciativa (histórico anterior perdido, plano
> recommitado a partir da cópia em disco) — se o `git log` um dia parecer curto demais pro que
> se espera, é por isso. O `MULTI_SCENE_PLAN.md` é a fonte de verdade, não o histórico de commits.

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
