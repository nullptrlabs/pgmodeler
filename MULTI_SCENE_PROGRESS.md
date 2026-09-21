# Progresso — Suporte a Múltiplas Cenas

> Log de execução do `MULTI_SCENE_PLAN.md`, atualizado sessão a sessão. Leia primeiro o plano
> (arquitetura, decisões fechadas, subtarefas) — este arquivo é só "onde paramos e por quê",
> para retomar rápido sem reler todo o histórico de commits.

## Estado atual

- Branch: `multi-scene-support-2.1.x`.
- Fase 0 em andamento. Subtarefas **0.1 a 0.7 concluídas** (0.7 ainda não commitada nesta sessão —
  ver abaixo).
- **Próxima subtarefa: 0.8** (scaffolding de persistência — reservar modo de segmentação e
  `SceneRect` por cena no formato XML/enum de `DatabaseModel`, sem popular/usar ainda).

## Commits desta branch (mais recente primeiro)

```
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
