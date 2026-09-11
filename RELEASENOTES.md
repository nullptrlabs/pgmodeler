v2.0.0-beta1
------
*Release date: September 14, 2026*
*Changes since:* ***v2.0.0-beta***

**Summary:** pgModeler 2.0.0-beta1 is a focused stability and feature refinement release building on top of beta. The core application receives important fixes for crashes and data integrity issues, gains support for PostgreSQL 18's new temporal keys, along with a set of targeted UI improvements. The star of this release, however, is the versioning plugin for pgModeler Plus: it has been significantly expanded with a complete set of day-to-day Git workflow features, making it a practical tool for teams and individual developers who manage their database models under version control. The SSH tunnel plugin, also part of pgModeler Plus, received a smaller but welcome rework of its own.

**Stability and crash fixes:** Several issues that could cause crashes in specific situations have been resolved. A crash that occurred during model validation when a column was restored through an undo operation is now fixed. A use-after-free race condition in the model close and reload flow, caused by deferred object destruction timing, has been eliminated. Stale pointers in the export tool that could crash the application after model changes were also addressed. This release also closes a batch of memory leaks found across relationship handling, the undo/redo history, copy-and-paste, and model loading, which should help keep memory usage in check during long editing sessions with several models open at once. Exporting to PNG or SVG images now happens in a single, uninterruptible step instead of running in the background, working around a limitation in how the underlying graphics engine handles multi-threaded rendering. In total, this release brings well over a dozen targeted bug fixes across editing forms, export, SQL completion, and the debug output panel — including two annoying UI quirks: the model objects panel no longer clears the current selection when it loses focus, and editing a column from a relationship's tab attributes now correctly opens the column editor instead of the constraint editor.

**FK naming convention change (breaking):** The default column naming pattern for foreign key relationships has been updated to follow a table-first convention. The source column pattern changed from `{sc}_{st}` to `{st}_{sc}`, and the destination column pattern for many-to-many relationships changed from `{sc}_{dt}` to `{dt}_{sc}`. If you have models that rely on the previous defaults, review your `relationships.conf` after upgrading.

**Temporal keys support:** pgModeler now supports the temporal keys introduced in PostgreSQL 18. Primary and unique keys can be defined with the `WITHOUT OVERLAPS` clause, and foreign keys can reference them using `PERIOD`, letting you properly model time-bound data — such as validity periods or scheduling ranges — instead of relying on ad-hoc check constraints. A new "Temporal key" option is available wherever these constraints are configured.

**View editing improvements:** The View editing form now shows the SQL preview in a dedicated tab that is generated on demand when you navigate to it. The tag selector and references panel were repositioned into a cleaner layout, and the minimum form size was reduced to better fit smaller screens.

**Layer and command-line enhancements:** The Layers panel gained a new option to select all objects belonging to the currently selected layers directly on the canvas, speeding up bulk operations on layered models. On the command-line side, `pgmodeler-cli` now supports the `--no-escape-comments` option giving finer control over how object comments are escaped during SQL generation.

**Visual consistency polish:** Checked menu items that carry an icon now display a highlighted background consistent with checked toolbar buttons, providing clearer visual feedback across all built-in themes. Split-button controls (buttons with an attached dropdown arrow) are now rendered as a single unified shape with a clean shared border, fixing a double-line artifact at the junction. The disabled state for hint-colored frames and buttons now preserves the color identity of the hint instead of falling back to a generic gray. A new accent-color frame hint follows the system palette's accent color, adapting automatically to different themes.

**Plugin API additions:** The model reload flow has been updated to go through a signal (`s_modelLoadRequested`) rather than directly into `addModel()`, allowing plugins to intercept and handle reload requests. The `s_modelClosed` signal is now emitted when a model is closed, giving plugins a reliable hook. Several `CustomTableWidget` additions — scroll bar access, cell font control, double-click signal, and multi-row selection retrieval — expand the toolkit available to plugin developers. The former `FileSelectorWidget` was renamed to `PathSelectorWidget` across the codebase to better reflect its ability to select both files and directories; plugins using the old class name will need to update accordingly.

**pgModeler Plus — Versioning plugin:** This release brings a major leap forward for the versioning plugin. The scope of work covered in this cycle effectively transforms it from a basic history viewer into a full Git client embedded in pgModeler. Here is what is new:

- **Repository cloning:** Clone remote repositories directly from the plugin interface. The clone dialog shows live progress as data is transferred, supports HTTPS credentials and SSH keys, and can automatically open the models found in the cloned repository once cloning completes.

- **Remote management:** Add, edit, and remove Git remotes from the Repository Settings panel. Multiple push URLs per remote are supported. SSH key paths are stored and reused automatically for subsequent operations, and the configuration panel now shows all active Git configuration parameters in a dedicated tab.

- **Commit staging and diff:** The new commit widget gives you fine-grained control over what goes into each commit. Files can be staged or unstaged individually; filters let you focus on specific file types; and a side-by-side diff view lets you review changes before committing. Split-model files are fully supported.

- **Push, pull, and sync:** Push and pull operations are available directly from the toolbar, with a combined sync action for convenience. The pull reconcile strategy (merge, rebase, or fast-forward only) is configurable in Repository Settings. Upstream tracking is displayed in the branch indicator and can be set up from the interface.

- **Conflict resolution:** When a merge or pull produces conflicts, the plugin enters a dedicated conflict mode that disables potentially destructive actions. You can inspect each conflicting file in three versions (base, local, and remote), resolve it by choosing one of the versions or using the built-in editor, and then finalize or abort the merge.

- **Branch merge:** Merge any branch into the current one from the branch manager. The merge widget shows the target and source branches clearly and confirms the operation before executing.

- **Cherry-pick:** Select any commit from the history and cherry-pick it onto the current branch. Merge commits are handled correctly by automatically applying the needed flags for each case.

- **Commit amend:** The last commit can be amended directly from the commit panel — useful for quick message corrections or adding a forgotten file without creating a new commit entry in history.

- **Reset to commit:** Reset the repository state to any historical commit, with support for the different Git reset modes (soft, mixed, hard).

- **Revert commit:** Create a new revert commit that undoes the changes introduced by any selected commit, without rewriting history.

- **Tag management:** Create, remove, and sync tags from a dedicated tag manager. Tags can be annotated and pushed to or fetched from the remote repository.

- **Stash management:** Save uncommitted work to the stash at any time, browse the stash list with timestamps and diff previews, and apply, pop, or drop individual stash entries. Full stash clearing is also available.

- **SSH agent support:** The plugin now sets `GIT_SSH_COMMAND` when running Git operations so that keys loaded in an SSH agent are picked up automatically, eliminating repeated passphrase prompts.

- **Git credential manager:** Support for configuring the Git credential manager directly from Repository Settings, streamlining authentication setup for HTTPS remotes.

**pgModeler Plus — SSH tunnel plugin:** The SSH tunnel plugin has been reworked to give you more granular control over your tunnels. Automatic startup on launch is now configured per tunnel instead of being a single all-or-nothing switch, so you can choose exactly which connections should come up automatically when pgModeler starts. The toolbar button now shows a dedicated icon reflecting whether any tunnels are currently running, with a tooltip summarizing how many are active and how many are stopped at a glance. The tunnels menu was also redesigned to list every configured tunnel individually, so each one can be started or stopped on its own, in addition to the existing options to start or stop all of them at once. A bug that could cause the shutdown of a running tunnel to be silently skipped was also fixed.

**Looking ahead:** With 2.0.0-beta1, pgModeler Plus's versioning plugin is now feature-complete for the initial 2.0.0 release. The focus until the final release will be on stability testing across platforms, addressing community feedback on the core application, and fine-tuning any remaining rough edges identified during this beta phase.
