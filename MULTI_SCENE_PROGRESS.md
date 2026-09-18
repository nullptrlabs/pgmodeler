# Progresso — Suporte a Múltiplas Cenas

> Log de execução do `MULTI_SCENE_PLAN.md`, atualizado sessão a sessão. Leia primeiro o plano
> (arquitetura, decisões fechadas, subtarefas) — este arquivo é só "onde paramos e por quê",
> para retomar rápido sem reler todo o histórico de commits.

## Estado atual

- Branch: `multi-scene-support-2.1.x`.
- Fase 0 em andamento. Subtarefas **0.1, 0.2, 0.3, 0.4 concluídas e commitadas**.
- **Próxima subtarefa: 0.5** (agregação de seleção e bounding rect entre cenas —
  `selectAllObjects()`, `itemsBoundingRect()`, gate `scene->items().count() > 1`).

## Commits desta branch (mais recente primeiro)

```
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
