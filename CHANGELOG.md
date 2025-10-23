Changelog
---------

v2.0.0-alpha
------
*Release date: October 30, 2025*

* [New] Added initial support for PostgreSQL 18.
* [New] Added the message box type Success and the static method Messagebox::success.
* [New] Added the enum StyleHint and the method setStyleHint in CustomUiStyle that renders frames background and border in a contextual way.
* [New] Added support for hiding empty object groups in database object trees in DatabaseExplorerWidget and DatabaseImportWidget. The hidden status is persisted in general.conf file.
* [New] Added a button in ModelFixWidget to generate output filename based on the input filename.
* [New] Added support for grid duplication in DataGridWidget.
* [New] Added a button to browse the selected table in DataHandlingForm.
* [New] Moved the entire build system from qmake to CMake on all platforms.
* [New] Added support for diffing two models in CLI and GUI.
* [New] Added the method ModelDbSelectorWidget::clearSelection.
* [New] Added the method Catalog::getServerVersion.
* [New] Created the method updateConnections in ModelValidationWidget, DatabaseImportWidget, DiffToolWidget and SQLToolWidget.
* [New] Added the class ModelDbSelectorWidget that encapsulates features to select models or databases to be used where models or databases need to be selected.
* [New] Created the function GuiUtilsNs::createWidgetInParent.
* [New] Added the method ModelWidget::setInteractive that toggles the interaction over the model.
* [New] Created aliases PGM_FILE, PGM_LINE and PGM_FUNC that refer respectively to macros __FILE__, __LINE__ and __PRETTY_FUNCTION__.
* [New] Added a dedicated icon for the Welcome view.
* [New] Added the widget FixToolsWidget, integrated into main window, that runs both the model fix and metadata handling operations.
* [Change] Adjusted the build to use clang-tidy to identify where code can be modernized.
* [Change] Applied a full refactor to the code based on clang-tidy checks.
* [Change] Refactored the class ResultSet to avoid copying the PGresult; instead, it will reuse it during initialization.
* [Change] Renamed the files palette.conf to theme.conf.
* [Change] Removed the method AppearanceConfigWidget::isDarkUiTheme and replaced its usage with CustomUiStyle::isDarkPalette.
* [Change] Moved the usage of CustomUiStyle to Application class.
* [Change] Renamed the enum IconType in Messagebox to MessageType.
* [Change] Removed the method Messagebox::setMessageFrameColor.
* [Change] Allowed tool/config widgets to be resized via splitter.
* [Change] Changed the attributes foreground-color and background-color to fg-color and bg-color in *-highlight.conf files.
* [Change] Replaced the filter options checkboxes in SQLExecutionWidget with tool buttons.
* [Change] Adjusted the column filter routine when filtering using regular expressions.
* [Change] Improved English grammar in internationalization strings across multiple files.
* [Change] Adjusted the rendering of QTableWidget/QTableView grids via polish.
* [Change] Rearranged the UI of AboutWidget and UpdateNotifierWidget.
* [Change] Adjusted the position and size of tab bar scroll buttons.
* [Change] Adjusted the minimum size of MainWindow.
* [Change] Adjusted the rendering of arrows in scroll buttons of tab bars.
* [Change] Fixed the position of scroll buttons of the SQL execution tab widget.
* [Change] Added missing SQL keywords to sql-highlight.conf.
* [Change] Added the style hint SuccessFrmHint in CustomUiStyle.
* [Change] Adjusted the rendering of frames with hint DefaultFrmHint.
* [Change] Adjusted the themes stylesheets.
* [Change] Adjusted CustomUiStyle and AppearanceConfigWidget for better theme detection.
* [Change] Added custom rendering routines for multiple Qt widgets in CustomUiStyle including QToolTips, QScrollBars, QTabBar, QProgressBar, QHeaderView, and others.
* [Change] Refactored various rendering methods in CustomUiStyle to improve code reusability and maintainability.
* [Change] Added support for smart theme detection in AppearanceConfigWidget.
* [Change] Added support for loading color palette from theme folder, file theme.conf.
* [Change] Adjusted the rendering of disabled checkboxes and radio buttons.
* [Change] Fixed the rendering of editable comboboxes.
* [Change] Adjusted the dark palette colors and rendering of various UI components.
* [Change] Implemented enhanced CustomUiStyle with bitwise corner flags for flexible corner control.
* [Change] Added custom disabled icon styling with dynamic theme color blending in CustomUiStyle.
* [Change] Adjusted the UI of GeneralConfigWidget.
* [Change] Adjusted the initial sizes of splitters in tools/config widgets.
* [Change] Refactored the ui-style.conf to remove duplicate styles.
* [Change] Forcing the Qt version to >= 6.4.3 in find_packages().
* [Change] Refactored parts of the UI in ModelExportWidget to include a ModelDbSelectorWidget instance.
* [Change] Renamed MetadataHandlingForm to MetadataHandlingWidget.
* [Change] MetadataHandlingWidget is now part of FixToolsWidget.
* [Change] Renamed the method PgModelerCliApp::diffModelDatabase to diffModels.
* [Change] Minor adjustment in GeneralConfigWidget::resetAlertChoices to reset the Attributes::AlertApplyMetadata parameter.
* [Change] Minor UI adjustment in RelationshipConfigWidget.
* [Change] Minor UI adjustment in SchemaEditorForm.
* [Change] Minor UI adjustment in SchemaEditorForm.
* [Change] Minor UI adjustment in MetadataHandlingWidget.
* [Change] Minor UI adjustment in ConfigurationWidget.
* [Change] Minor UI adjustment in PluginsConfigWidget.
* [Change] Minor UI adjustment in SnippetsConfigWidget.
* [Change] Minor UI adjustment in AppearanceConfigWidget.
* [Change] Minor UI adjustment in GeneralConfigWidget.
* [Change] Minor UI adjustment in MainWindow.
* [Change] Renamed the class ModelFixForm to ModelFixWidget since it now runs integrated into FixToolsWidget.
* [Change] Adjusted the display of the Fix view in MainWindow.
* [Change] Minor size adjustment in ConfigurationWidget.
* [Change] Minor adjustment in DatabaseImportWidget::s_importFinished to send a bool flag indicating whether the signal was sent during import error.
* [Change] Minor adjustment in DiffToolWidget to disable the import options group when the user selects model -> model diff.
* [Change] Adjusted the method Application::copyFilesRecursively to ignore the diff-presets.conf from previous versions of pgModeler.
* [Change] Minor improvement in DiffToolWidget presets saving and loading.
* [Change] Adjusted the invalidated model alert display in ModelExportWidget.
* [Change] Removed the tab "Partial diff" and moved all controls related to it to the main tab in DiffToolWidget.
* [Change] Reorganized the diff widget UI.
* [Change] Removed unused components in DiffToolWidget.
* [Change] Minor adjustment in DatabaseImportWidget and ModelExportWidget to block changes in the model while the operations are running.
* [Change] Adjustments to MainWindow to start/stop timers when a diff, import or export operation starts/stops.
* [Change] The diff tool is now integrated into the main window via "Diff view".
* [Change] The import tool is now integrated into the main window via "Import view".
* [Change] The export tool is now integrated into the main window via "Export view".
* [Change] Adjusted some icon sizes in icons-*.conf files.
* [Change] Converted ModelExportWidget export mode radio buttons into tool buttons.
* [Change] Renamed the method exec() to setModel() in ModelExportWidget.
* [Change] Adjusted MainWindow::updateConnections to update the diff, import and export widgets connections.
* [Change] Adjusted the UI of ModelObjectsWidget and OperationListWidget.
* [Change] Changed some QToolButtons to QPushButton for better semantics.
* [Change] Adjustments in the light themes.
* [Change] Finished the UI adjustments in DataHandlingForm.
* [Change] Started the adjustments to DataHandlingForm.
* [Change] Adjusted the ui-style.conf.
* [Change] Adjusted the behavior of MainWindow::closeEvent to discard any unchanged models.
* [Change] Adjusted some class destructor declarations.
* [Change] Removed the class ConfigurationForm.
* [Change] Minor adjustment in ConnectionsConfigWidget::openConnectionsConfiguration.
* [Change] Removed the use of the deprecated method ConfigurationWidget::getConfigurationWidget(int).
* [Change] Minor improvement to BaseConfigWidget.
* [Change] In MainWindow, if the user changes the settings without applying them, a confirmation message will be displayed so the changes can be applied or reverted before switching the current view.
* [Change] Adjusted the tab close button icon.
* [Change] Adjusted the border radius of table objects.
* [Change] Modified the default color of canvas objects in dark theme.
* [Change] Adjusted the layout classes used to arrange the view widgets in MainWindow's stack widget.
* [Change] Adjusted the default colors of the dark palette.
* [Change] Updated the window icons of all app subprojects.
* [Change] Minor adjustment in logo images.
* [Change] Adjusted the splash screen display.
* [Change] Adjusted the logo design/color to match the icon set colors.
* [Change] Minor adjustment in resource files.
* [Change] Updated the icon colors for a flatter and more vivid style.
* [Change] Adjusted the Windows version build.
* [Change] Changed the deprecated define Q_OS_MAC to Q_OS_MACOS.
* [Fix] Fixed a bug in the connection class that was creating an invalid connection string when other params were manually specified.
* [Fix] Fixed a bug that was causing collations assigned to columns not to be reflected in SQL and XML codes.
* [Fix] Fixed a bug that was causing collations to be saved always as deterministic even when the user unchecked the option in the editing form.
* [Fix] Fixed the function PgSqlVersions::parseString to return the default version as a fallback when a major server version is not yet recognized by the tool.
* [Fix] Fixed a bug in XMLParser that was not configuring the DTD file path correctly.
* [Fix] Fixed a bug in ObjectsScene that was causing value overflow when aligning points to grid in some cases.
* [Fix] Fixed a bug in ModelWidget::protectObject.
* [Fix] Fixed the rendering of QTableView/QTableWidget item borders.
* [Fix] Minor fix in the crash handler to load the appearance settings and keep the UI uniform.
* [Fix] Fixed the method CustomUiStyle::setStyleHint to accept only QFrames.
* [Fix] Minor fix in DiffToolWidget.
* [Fix] Minor fix in relationshipconfigwidget.ui.
* [Fix] Minor fix in databaseimportwidget.ui.
* [Fix] Fixed the warning icon display in FileSelectorWidget.
* [Fix] Fixed a bug that was causing columns that were part of PKs not to be removed by the user.
* [Fix] Minor fix in difftoolwidget.ui.
* [Fix] Fixed a bug in DatabaseImportWidget that was allocating threads every time the showEvent was triggered.
* [Fix] Minor fix in the build process.
* [Fix] Fixed the catalog query for constraints for PostgreSQL 17.
* [Fix] Minor fix in libcli/CMakeLists.txt.

v1.2.0
------
*Release date: May 9, 2025*

* [New] Setup a wait cursor when loading a diff code in the SQL tool.
* [New] Added the static methods isAccepted, isRejected, and isCanceled to Messagebox.
* [New] Added the enum Messagebox::ResultId
* [New] Added support for multiple objects code preview in SourceCodeWidget.
* [New] Added the methods BaseObject::setPgOid and BaseObject::getPgOid.
* [New] Added a confirmation message in DataHandlingForm when the user tries to close the dialog even without uncommitted data.
* [New] Graphical objects added to the ModelWidget will now blink and the viewport will center on them to indicate to the user where they were put.
* [New] Added a copy action in NumberedTextEditor that copies all the text to the clipboard
* [Change] Improved the SyntaxHighlighter and the sql-highlight files so the regular expressions for each group don't take too much to process strings
* [Change] Messagebox::exec now calls qApp->restoreOverrideCursor
* [Change] Removed unneeded calls to qApp->restoreOverrideCursor
* [Change] Changed the return type of all create methods in DatabaseImportHelper to their respective object classes.
* [Change] In DatabaseImportHelper::createObject the OID is automatically assigned to the object created from XML code.
* [Change] Minor adjustment in ObjectsScene::addItem. Added an optional parameter to blink added objects.
* [Change] Adjusted pgmodeler-cli to raise errors when the ignore export error options are used in transactional export mode.
* [Change] Minor adjustment in ModelDatabaseDiffForm by making transactional mode mutually exclusive with ignore export errors options.
* [Change] Adjusted the use of Messagebox in several portions of the tool.
* [Change] Replaced the calls to Messagebox::result() == result_code by the respective isAccepted, isRejected or isCanceled methods.
* [Change] Adjusted the ModelWidget::configureFadeMenu to stop reallocating new actions on each call.
* [Change] Removed unused class attribute in ModelObjectsWidget.
* [Change] Removed the method SourceCodeWidget::setSourceCodeTab.
* [Change] Forcing the option "Ignore import errors" when the import to working model option is set on DatabaseImportHelper and DatabaseImportForm.
* [Change] Improved the reverse engineering so columns can also be imported using the option "Import to the working model" in DatabaseImportForm.
* [Fix] Minor fix in FileSelectorWidget that was not applying correctly the palette colors in the line edit field.
* [Fix] Minor fix in DatabaseModel::updateExtensionObjects to ignore duplicated schemas and use the ones already available in the model.
* [Fix] Minor fix in SchemaParser::resetParser to clear include info so the error location in schema code can be correctly informed.
* [Fix] Fixed a bug in the data dictionary generation for views.
* [Fix] Fixed a bug in the SQL generation of database model SQL related to an unknown database attribute.
* [Fix] Fixed the text stack trace in the message box class.
* [Fix] Fixed the code display for functions in DatabaseExplorerWidget.
* [Fix] Fixed a bug when importing composite types having attributes using arrays of other user-defined types.
* [Fix] Fixed a bug when displaying the source, in Database Explorer, of a composite type that has one or more attributes referencing used-defined types.
* [Fix] Fixed a bug in SyntaxHighlighter::highlightEnclosingChars.
* [Fix] Fixed a bug in ResultSet::accessTuple that was generating errors when running catalog queries of some objects in older versions of PostgreSQL.
* [Fix] Fixed a bug in PgModelerCliApp::diffModelDatabase that was informing invalid forced filtered types.
* [Fix] Minor fix in DatabaseModel::getChangelogDefinition to avoid returning an empty string when the changelog is not persisted to the dbm file.
* [Fix] Fixed a bug in ModelObjectsWidget::selectObject.
* [Fix] Minor fix in CodeCompletionWidget to list columns on "Order By".

v1.2.0-beta1
------
*Release date: March 31, 2025*

* [New] Added the method AppearanceConfigWidget::isDarkUiTheme.
* [New] Added per-instance control of line numbers and action buttons visibility in NumberedTextEditor (previously global-only).
* [New] Added an integrated search/replace widget in NumberedTextEditor.
* [New] Added a debug tab in DatabaseImportForm and ModelDatabaseDiffForm to display debug messages during reverse engineering.
* [New] Added support for force object re-creation in pgmodeler-cli's diff operation.
* [New] Added a popup menu in ModelDatabaseDiffForm to select object types for forced re-creation.
* [New] Added a flag in DatabaseModel to control inclusion of disabled SQL code in generated scripts.
* [New] Added support for toggling disabled objects' code generation in database model properties.
* [New] Added an attribute in .dbm files to persist system schemas' rectangle visibility.
* [New] Added the methods Extension::setObjectNames and Extension::getObjectNames.
* [New] Added struct Extension::ExtObject to manage child objects' metadata.
* [New] Added support for permission editing for system objects.
* [Change] Adjusted the UI in SQLExecutionWidget to reorganize action buttons.
* [Change] Adjusted the NumberedTextEditor internal widgets when the horizontal scrollbar is visible.
* [Change] Adjusted the class ModelFixForm to use DebugOutputWidget.
* [Change] Adjusted the object selection in ModelObjectsWidget (ALT + click highlights graphical objects).
* [Change] Adjusted the query filters in ModelDatabaseDiffForm to exclude only system objects during comparison.
* [Change] Improved ExtensionWidget to support custom schema names in data types.
* [Change] Refactored DatabaseImportHelper::createExtension to correctly import extension-owned types/schemas.
* [Change] Refactored DatabaseModel to use Extension::ExtObject when handling extensions.
* [Change] Simplified the function behavior types by removing redundant STRICT (merged with RETURNS NULL ON NULL INPUT).
* [Change] Moved Q_OBJECT macro to the top of class declarations.
* [Change] Updated catalog queries for indexes to retrieve comments.
* [Change] Removed redundant search/replace widget from SourceCodeWidget (now integrated in NumberedTextEditor).
* [Change] Removed unused checkbox from databaseimportform.ui.
* [Change] Minor adjustments in UtilsNs::saveFile and UtilsNs::loadFile.
* [Change] Minor adjustments in .pro/.pri files for private plugin assets.
* [Change] Minor schema adjustments in sql/dbmodel.sch.
* [Change] Fixed DatabaseImportForm to remain open in debug mode for output inspection.
* [Fix] Fixed crashes when diffing databases containing extension-created tables.
* [Fix] Fixed the importing of columns that use arrays of user-defined types.
* [Fix] Fixed the persistence of time zone state in timestamp types.
* [Fix] Fixed false-positive diffs for functions with comments or STRICT behavior.
* [Fix] Fixed reverse engineering of types with uppercase letters in names.
* [Fix] Fixed a crash in identifier relationships with FK indexes.
* [Fix] Fixed DatabaseImportHelper::createPermission to skip permissions whose references unavailable system roles.
* [Fix] Fixed ModelExportHelper::exportBufferToDBMS edge cases.
* [Fix] Fixed XML code generation for Extension class to include child objects.
* [Fix] Fixed SQL/XML code exclusion for protected schemas in DatabaseModel::getSourceCode.
* [Fix] Fixed SQL syntax highlighting files.
* [Fix] Minor fixes in LineNumbersWidget and NumberedTextEditor.
* [Fix] Minor fixes in PgModelerCliApp::fixObjectAttributes.

v1.2.0-beta
------
*Release date: February 10, 2025*

* [New] Added the enum PgModelerGuiPlugin::MenuSectionId that controls where the plugin actions are placed in the context menu in the model widget.
* [New] Added a call to qApp::alert() when the export, import, and diff processes finish so the taskbar blinks when the window is not visible.
* [New] Added the method PgModelerGuiPlugin::isSelectionValid.
* [New] Indexes created by relationships can have their parent relationships opened from a specific action in the context menu of the model widget.
* [New] Added support for the total number of rows in the table being handled in DataHandlingForm.
* [New] Added support for setting the global settings for FK columns index type in RelationshipConfigWidget.
* [New] Adding support for the creation of indexes on FK columns generated by relationships.
* [New] Set the "Stack trace" tab icon in CrashHandlerForm.
* [Change] Adjusted the method ModelWidget::configurePluginsActions to place actions in menu sections according to the plugins' custom menu sections.
* [Change] Minor adjustment in BaseForm::setMainWidget to set the default window icon.
* [Change] Adjusted the plugin API by removing the unnecessary const methods/objects usage.
* [Change] Added icons for tree widgets collapse and expand actions.
* [Change] Adjusted the order of types in IndexingType::types.
* [Change] Minor adjustment in Relationship::setNamePattern.
* [Change] Minor adjustment in ModelWidget::configurePluginsActionsMenu to enable/disable plugin actions based on the call to isSelectionValid of the plugin that owns the action.
* [Change] Updates on icons-*.conf files.
* [Change] Minor layout adjustment in DataGridWidget.
* [Change] Updated the class RelationshipConfigWidget to include FK column index name pattern.
* [Change] Adjusted the code generation of relationships to include FK column indexes.
* [Change] Minor refactor in Relationship::getSourceCode.
* [Change] Minor refactor in DatabaseModel::getCreationOrder.
* [Change] Minor adjustment in LayersWidget to accept Enter/Return to apply settings.
* [Fix] Minor fix in GlobalAttributes::setCustomPaths.
* [Fix] Fixed some shortcut conflicts in the main window.
* [Fix] Minor fix in ModelWidget::configurePluginsActionsMenu
* [Fix] Minor fix in ModelObjectsWidget::show to run in an event loop when configured in simplified view mode.
* [Fix] Minor fix in ObjectsFilterWidget.
* [Fix] Minor fix in the "Open relationship" action in ModelWidget.
* [Fix] Fixed the index catalog queries when using pgModeler in compatibility mode (PG 9.x).
* [Fix] Minor fix in RelationshipWidget to assign FK index pattern to the relationship being handled.
* [Fix] Minor fix in buttons' drop shadows in DataHandlingForm.

v1.1.6
------
*Release date: December 21, 2024*

* [Fix] Fixed a bug in the FK relationships update routine that was causing more relationships to be deleted than what really needed to be.
* [Fix] Fixed a crash when creating a many-to-many relationship where one or more columns of the involved primary keys use generated default values (GENERATED AS).
* [Fix] Fixed a bug in the routine that updates relationships' generated objects that was causing the application to hang up when importing a simple hierarchy of tables.

v1.1.5
------
*Release date: October 28, 2024*

* [New] Added support for PostgreSQL 17.
* [Fix] Fixed the catalog queries of collation and domain objects for PostgreSQL 17.
* [Change] Disabling objects' grids in TableWidget when the edited table is protected.

v1.2.0-alpha1
------
*Release date: October 25, 2024*

* [New] Added support for PostgreSQL 17.
* [New] Added support for tabbed table data handling.
* [New] Added an option in LayersConfigWidget that makes relationships follow tables visibility.
* [New] Added support for quickly creating a new layer and assigning it to the selected objects in ModelWidget.
* [New] Added support for the removal of recent models that aren't accessible anymore.
* [New] Added support for transactional export in pgmodeler-cli.
* [New] Added support for transactional export in ModelExportForm.
* [New] Added support for running diff code in a transaction.
* [New] Added support for displaying FK's update/delete actions in the data dictionary.
* [New] Added a confirmation message in ModelExportForm when exporting to DBMS with one of the drop options set.
* [New] Added the method CodeCompletionWidget::identifyObjectType.
* [New] Added the method CustomTableWidget::setSortingEnabled.
* [New] Added the signal CustomTableWidget::s_rowCountChanged.
* [New] Added support for ccache tool in the compilation process on Linux.
* [New] Added a new version of Messagebox::error.
* [New] Added the method CustomTableWidget::addCustomButton.
* [New] Added the method SQLExecutionHelper::getCommand.
* [New] CustomTableWidget now allows the removal of multiple rows at once.
* [New] Added the method CustomTableWidget::setAddRowOnTabPress.
* [New] Added the methods CustomTableWidget::setVerticalHeaderVisible and CustomTableWidget::updateVerticalHeader.
* [New] Created the slot SQLExecutionWidget::runSQLCommand(QString).
* [New] Created the struct PgModelerGuiPlugin::PluginWidgets.
* [New] Added support for plugin widgets in SQLExecutionWidget.
* [New] Added the methods registerPlugin, createWidgets, getPluginsActions, getPluginsToolButtons on PgModelerGuiPlugin.
* [Change] Removed the support for Qt 6.2 and 6.3.
* [Change] Improved the CodeCompletionWidget in such a way as to correctly suggest names in ALTER/DROP commands depending on the objects being handled.
* [Change] Minor refactor in BaseObject::getChildObjectTypes.
* [Change] Minor change in method BaseObject::getObjectType by adding the paramenter is_sql_name.
* [Change] Minor improvement in CodeCompletionWidget in order to trigger completion in the presence of more keywords.
* [Change] Minor adjustment in sql-highlight.conf.
* [Change] Minor change in methods copySelection and generateBuffer in SQLExecutionWidget.
* [Change] Minor change in CustomTableWidget::addRow that now returns the id of the row added.
* [Change] The file selection dialog now starts on the user's home by default and saves that last accessed directory, using it the next time it is opened.
* [Change] Minor adjustment in DataManipulationForm and TableDataWidget to use signals customContextMenuRequested when displaying items context menus.
* [Change] Changed the signature of some methods in CustomTableWidget.
* [Change] Renamed ObjectsTableWidget to CustomTableWidget
* [Change] Refactored MainWindow, PluginsConfigWidget, SQLToolWidget, and SQLExecutionWidget to use the new structure of PgModelerGuiPlugin for extra features inclusion.
* [Change] Moved the layers selection widget from the popup menu to a standalone dialog.
* [Change] Refactored the GuiUtilsNs::getHoveredWidgetCorner and GuiUtilsNs::resizeFloatingWidget.
* [Change] Materialized views and constraint trigger will ignore the option "Replace modified" option in diff.
* [Change] Avoid adding the "OR REPLACE" keywords to constraint triggers code.
* [Change] Improved the resizing of floating widgets via mouse move by creating the functions GuiUtilsNs::getHoveredWidgetCorner and GuiUtilsNs::resizeFloatingWidget.
* [Change] Minor adjustment in dark theme color palette.
* [Change] Disabling objects' grids in TableWidget when the edited table is protected.
* [Change] Minor adjustment in the diff_opts size in ModelsDiffHelper.
* [Change] Refactoring the SQL schema files including the use of the new special keyword @include to diminish code duplication.
* [Change] Minor update on attributes translations in DatabaseExplorerWidget.
* [Fix] Fixed the catalog queries of collation and domain objects for PostgreSQL 17.
* [Fix] Minor bug fix in code generation of tablespace, database, and user mapping objects.
* [Fix] Minor fix in SQLExecutionWidget::generateBuffer.
* [Fix] Minor fix in SQLExecutionWidget::generateCSVBuffer to consider the check state of items during CSV buffer generation.
* [Fix] Minor fix in SyntaxHighlighter to use default window text color if no foreground color is defined for a group in the syntax highlight config file.
* [Fix] Minor fix to avoid flickering when hiding widgets that lie in splitters.
* [Fix] Minor fix in the object search feature when searching by source/referenced constraint columns.
* [Fix] Minor fix in ObjectListModel to allow sorting the ID column as an integer value.
* [Fix] Fixed a bug in Function that was not resetting SETOF flag when changing to return table mode.
* [Fix] Fixed a malformed markdown code when the table/view has a comment.
* [Fix] Fixed a bug in ModelsDiffHelper that was ignoring changes in columns.
* [Fix] Fixed a crash in DatabaseImportHelper when destroying detached inherited columns.
* [Fix] Fixed a bug in Catalog::getCommentQuery that was crashing the application while trying to retrieve user mapping comments.
* [Fix] Minor fix in SchemaParser to avoid breaking the parsing when there are two or more @include statements on each row.
* [Fix] Fix a bug in the Index class that was preventing the removal of included columns.

v1.1.4
------
*Release date: August 21, 2024*

* [Fix] Fixed a bug in the function object that was not resetting the SETOF flag when changing the return type to TABLE.
* [Fix] Fixed a crash in the database import process when destroying detached inherited columns.
* [Fix] Fixed a bug in the system catalog query class that was crashing the application while trying to retrieve user mapping comments.
* [Fix] Fix a bug in the index object that was preventing the removal of included columns.

v1.2.0-alpha
------
*Release date: June 24, 2024*

* [New] Added support for @include statements in schema micro language.
* [New] Added the support for Markdown data dictionaries in ModelExportForm.
* [New] Added the option "--markdown" to PgModelerCliApp to allow generating markdown data dictionaries.
* [New] Added support to convert meta chars to escaped (and vice-versa) in pgmodeler-se/SourceEditorWidget.
* [New] Added support for contiguous escaped metachars in schema microlanguage files, e.g., \s\t\n. Contiguous metachar are also allowed, e.g., $tb$sp$br despite the poor readability.
* [New] Created the method GlobalAttributes::getDictSchemaFilePath.
* [New] Added support for using escaped \[ and \] in the middle of plain text expressions in schema microlanguage.
* [New] Added the method TextBlockInfo::removeFragmentInfo.
* [New] Created the struct SyntaxHighlighter::MatchInfo that holds the start and end positions of a match.
* [New] Created the class FragmentInfo that holds some info about formatted text fragments.
* [New] Added support for escaped characters in SchemaParser the valid ones are: \s (space), \t (tab), \n (line feed), \[, \], \{, \}, \$, \#, \%, \@, \&, \\, \*.
* [New] Added support for toggling comments on pgmodeler-se/SourceEditorWidget.
* [New] Added the method GlobalAttributes::getPgModelerBaseVersion.
* [New] Created the variadic template method GlobalAttributes::getFilePath.
* [New] Added the folder datadict/html and datadict/md to separate HTML data dictionaries from MD data dictionaries.
* [New] Added support for data dictionaries in Markdown format.
* [New] Added support for saving the status of the "Replace modified objects" option in diff presets.
* [New] Added the option ModelsDiffHelper::OptReplaceModified that toggles the replacement of objects that accept CREATE OR REPLACE.
* [New] Added the fields in ViewWidget to handle check_option, security_invoker, and security_barrier options
* [New] Objects that can be created with "CREATE OR REPLACE" are now having their DDL generated with "OR REPLACE".
* [Change] Removed the restriction of specifying OUT parameters in procedures.
* [Change] Refactored schema files of HTML data dicts to use the include statement.
* [Change] SourceCodeWidget now generates code on demand, when the current source code type tab changes.
* [Change] Moved the type strings from translation units in libcore/pgsqltypes/* to their respective class definition.
* [Change] Refactored static members initializations to C++17 style, e.g., inline static Type member {value}.
* [Change] Removed static members initialization from translation units.
* [Change] Refactored the syntax highlight config files.
* [Change] Minor adjustment in SyntaxHighlighter::matchExpression to avoid adding duplicated match infos to the list.
* [Change] Removed the "defaults" folder from assets/conf to avoid duplicate files. This directory will be created via make install according to the settings in pgmodeler.pro
* [Change] Adjusted SyntaxHighlighter::highlightBlock to allow nested multiline expressions, e.g. [" "], to replace each other.
* [Change] Several changes in SyntaxHighlighter methods involved highlighting to return key struct in the form of const * to avoid excessive copies.
* [Change] Minor change in source-code-highlight.dtd making the element <group> optional.
* [Change] The SyntaxHighlighter::highlightBlock routine is way more simple, and the configuration file structure has been simplified as well.
* [Change] Removed unused enum and methods in TextBlockInfo
* [Change] Adjusted the format group loading from file in SyntaxHighlighter::loadConfiguration.
* [Change] Converted char attributes in SchemaParser to QChar.
* [Change] Replaced switch statements by ifs in several parts of SchemaParser to handle QChars instead of char.
* [Change] Refactor in ModelExportForm to handle MD format in data dictionaries.
* [Change] Refactored the methods in PgModelerPlugin that returns file paths to use GlobalAttributes::getFilePath.
* [Change] Refactored GlobalAttributes::getTmplConfigurationFilePath and GlobalAttributes::getSchemaFilePath to use GlobalAttributes::getFilePath.
* [Change] Now pgModeler, in the first run, will try to copy the configuration files from a previous major version immediately before the current one. For example, running 1.2 the files to be copied will be from 1.1 not from 0.9.4 anymore.
* [Change] Dropped the support for "Forced object recreate" in ModelDatabaseDiffForm and ModelsDiffHelper. The option was confusing and was generating a diff code that was similar to generating the entire SQL of a model and re-export it to a server. So, it was considered not so useful anymore.
* [Change] Changed the Function::getAlterCode and Procedure::getAlterCode to stop generating CREATE OR REPLACE command, this will now be generated by ModelsDiffHelper.
* [Change] Minor layout adjustment in ViewWidget.
* [Change] Minor adjustment in TemplateType::setType to include extra information when raising exceptions.
* [Change] Minor adjustment in changelog entry validation in DatabaseModel::addChangelogEntry.
* [Fix] Minor fix in Application::createUserConfiguration to avoid trying to copy legacy configs when older version configurations don't exist.
* [Fix] Minor fix in SchemaParser::isSpecialCharacter to include CharStartEscaped as a special char.
* [Fix] Fixed all getDataDictionary methods to be able to generate data dictionaries for both HTML and MD formats
* [Fix] Fixed a bug in SyntaxHighlighter::loadConfiguration that was not properly setting regular expressions as case-sensitive.
* [Fix] Fixed a bug in PgModelerApp::createUserConfiguration that was not copying missing files correctly.
* [Fix] Fixed build process on windows.
* [Fix] Minor fix in SyntaxHighlighter::setFormat and SyntaxHighlighter::highlightBlock.
* [Fix] Fixed a crash in SchemaParser::evaluateComparisonExpr when a portion of a string value is commented out.
* [Fix] Minor adjustment in tooltips in modelexportform.ui.
* [Fix] Minor fix in GlobalAttributes::setSearchPath to set the path from the environment variable APPDIR when building an ApppImage.
* [Fix] Minor fix in CodeCompletionWidget::configureCompletion where the elements of keywords are treated.
* [Fix] Minor fix in HTML data dictionary schema files.
* [Fix] Fixed a bug in SchemaParser that was generating syntax errors when comment character # was put inside the plaintext operator [].
* [Fix] Fixed a bug in SchemaParser::getAttribute.
* [Fix] Minor fix in datadicttest.
* [Fix] Fixed the method SchemaParser::loadBuffer to parse commented lines as empty ones to keep the line numbers from the original buffer/file synchronized with the resulting parsed schema buffer.
* [Fix] Adjusted the syntax highlight conf to be used when loading a file in SourceEditorWidget.
* [Fix] Minor layout adjustment in ModelExportForm.
* [Fix] Minor fix in HtmlItemDelegate::paint when rendering items without icons and with multiple text lines.
* [Fix] Fixed a shortcut conflict in SearchReplaceWidget.
* [Fix] Minor fix in PgModelerCliApp::recreateObjects to warn about invalid changelog entries avoiding aborting the model fix process.
* [Fix] Fixed a bug in Catalog class that was not retrieving comments of sequences causing false-positive to be generated in diff process.

v1.1.3
------
*Release date: May 13, 2024*

* [New] Added an option to toggle the display of the mouse position, zoom, and object selection information.
* [Change] Minor adjustment in UI stylesheets of the scene info widget.
* [Change] Adjust the scene info widget layout to diminish the components wobbling when moving objects.
* [Change] Minor adjustment in changelog entry validation in the database model.
* [Fix] Fixed a shortcut conflict in the text search/replace widget.
* [Fix] Minor fix in the database model objects recreation process (during model fix operation) to warn about invalid changelog entries avoiding aborting the entire model fix process.
* [Fix] Fixed a bug in the catalog class that was not retrieving comments of sequences causing false-positive to be generated in diff process.

v1.1.2
------
*Release date: April 08, 2024*

* [New] Added a sample model of the famous northwind database ported to PostgreSQL.
* [Change] Updated the sample model pagila.dbm. Now it uses the table partition feature.
* [Change] Minor adjustment in the SQL execution widget to notify the OS and blink the taskbar after running a SQL command while the main window is minimized.
* [Change] The command-line interface tool had its menu texts reviewed.
* [Change] The view editing form now displays an alert regarding extra semicolons at the end of the definition command.
* [Change] Disabled the cached object code in DatabaseExplorerWidget::loadObjectSource.
* [Fix] Minor text correction in SQLExecutionWidget.
* [Fix] Fixed a bug in the generation of SQL for roles when no option is set.
* [Fix] Fixed a bug in the automatic name truncation/disambiguation for long names in CoreUtilsNs::generateUniqueName.
* [Fix] Minor fix in the code completion widget to select the first visible item in the list.
* [Fix] Fixed a bug in the generation of the command COMMENT ON CONSTRAINT.
* [Fix] Minor fix in libcli/CompatNs::convertToNewView to correctly extract the SQL definition of views with disabled code.
* [Fix] Fixed the generation of view's SQL to ignore extra semicolons at the end of the command.

v1.1.1
------
*Release date: March 14, 2024*

* [New] Added an XML code searching widget in SourceCodeWidget.
* [New] Added an option to place in random positions objects that are imported to a working model.
* [New] Created the methods Connection::isServerSupported and Catalog::isServerSupported.
* [New] Added alerts on DatabaseImportForm and ModelDatabaseDiffForm about unsupported server versions.
* [Change] Renamed ObjectFinderWidget to ObjectSearchWidget for better semantics.
* [Change] Renamed the button "Find objects" to "Search" in MainWindow.
* [Change] Renamed the widget FindReplaceWidget to SearchReplaceWidget for better semantics.
* [Change] pgModeler will not change the positions of the original objects in a model when importing new objects to that model.
* [Change] Adjusted the split layout that resizes ModelValidationWidget and ObjectFinderWidget in MainWindow.
* [Change] Adjusted the splitters handlers appearance via stylesheets.
* [Change] Minor adjustments on some widgets' layout margins.
* [Change] Minor layout adjustment in modeldatabasediffform.ui by moving the export and import options to dedicated group boxes.
* [Change] Minor improvement in Catalog::getObjectNames to include a signature attribute in the returned attributes map of each object.
* [Change] Minor adjustment in DatabaseExplorerWidget::updateItem.
* [Change] Moved the (set|get)ParentRelationship methods from Column to TableObject.
* [Change] Now in CodeCompletionWidget::adjustNameListSize when the list is empty the widget size will have a minimum size.
* [Change] Minor improvement in CodeCompletionWidget to avoid repeatedly querying system catalogs having the list already filled. The list is cleared and repopulated only if the cursor position gets back to the initial position when the method show() was called.
* [Change] Set the horizontal scroll bar off in the tree widget of ModelObjectsWidget.
* [Fix] Fixed the code generation for policies.
* [Fix] Minor adjustment in the enabled status toggling of several widgets in DatabaseImportForm.
* [Fix] Fixed the behavior of the "Auto browse" attribute of connections in DatabaseImportForm and ModelDatabaseDiffForm.
* [Fix] Fixed the method ModelWidget::eventFilter to force horizontal scene movement when the Shift key is held and the mouse wheel movement is made.
* [Fix] Minor layout fix in referenceswidget.ui.
* [Fix] Minor error message fix on MainWindow::showFixMessage.
* [Fix] Minor fix in MainWindow::loadModels.
* [Fix] Minor fix in Messagebox::alert to include an optional exception to be displayed.
* [Fix] Fixed a crash in TableWidget when handling foreign tables.
* [Fix] Fixed a QObject::connect warning regarding null parameter in ObjectSearchWidget::findObjects.
* [Fix] Fixed the partial diff/import filtering which was ignoring a wildcard pattern and listing all objects.
* [Fix] Minor fix in NumberedTextEditor::pasteCode.
* [Fix] Fixed a bug when importing collations with the ICU provider.
* [Fix] Minor workaround in ObjectRenameWidget to be displayed with the name input focused but the contents of that field deselected.
* [Fix] Minor fix in ModelWidget::pasteObjects.
* [Fix] Fixed the relationship conversion feature to preserve the original layer and line color information on the converted objects.
* [Fix] Minor fix in Relationship class to set the parent relationship attribute on all generated constraints.
* [Fix] Minor fix in CodeCompletionWidget to properly list column names when using non-schema-qualified table names in INSERT/DELETE commands.
* [Fix] Fixed a bug in code generation of generic SQL objects.

v1.1.0
------
*Release date: February 23, 2024*

* [New] Created the method Application::loadTranslations.
* [New] FindReplaceWidget now accepts ENTER/RETURN to search texts.
* [New] Added the method MainWindow::registerRecentModels.
* [New] Adding support for table name completion on ALTER/DROP commands in CodeCompletionWidget.
* [New] Added the methods PgModelerCliApp::(set|get)ParsedOptValue.
* [New] pgModeler CLI now supports plugins.
* [New] Created the method GuiUtils::updateDropShadow.
* [New] Create the method BaseFunction::isBaseFunction.
* [New] Now during object copy/paste or duplication, in case of name conflict, the user is asked to type a new name for each conflicting object.
* [New] Adding the ability to restore the scene rect from the database model file.
* [New] Adding support to expand canvas rect via the main window.
* [New] Added the method FileSelectorWidget::setAppendSuffix.
* [New] Added support for infinite canvas.
* [Change] Minor adjustment in BaseObjectWidget::configureFormFields.
* [Change] Moved the code that loads plugins' translations from PgModelerApp to Application::loadTranslations.
* [Change] Updated the pgmodeler-intl.pro
* [Change] The libutils/PgModelerPlugin class is now a base class of the new classes libgui/PgModelerGuiPlugin and libcli/PgModelerCliPlugin.
* [Change] The subproject pgmodeler-cli now uses the classes in libcli.
* [Change] Moved PgModelerCliApp to a dedicated library (libcli) to allow the proper creation of plugins for pgmodeler-cli.
* [Change] Removed extra frames in pluginsconfigwidget.ui, connectionsconfigwidget.ui and snippetsconfigwidget.ui.
* [Change] Minor adjustment in GeneralConfigWidget to change the enabled state of the "Reset alerts" button on the widget show.
* [Change] Removed the confirmation dialog in ModelWidget::copyObjects related to copying dependencies. Now the copy of dependencies during the copy of an object is triggered by the shortcut Ctrl+Shift+C (copy selection and dependencies) or Ctrl+Shift+X (cut selection and dependencies).
* [Change] Removed unneeded throw instruction in widget ctors.
* [Change] Minor improvement in ModelObjectsWidget::saveTreeState/restoreTreeState to correctly restore the vertical scrollbar position.
* [Change] Minor adjustment in MainWindow::applyConfigurations to correctly update grid settings on the current model.
* [Change] Minor adjustment in ModelWidget::finishSceneMove.
* [Change] Minor adjustment in ObjectsScene::setSceneRect to emit s_sceneRectChanged only when the new rect differs from the current one.
* [Change] Removed unneeded calls to QGraphicsView::resetCachedContent in ModelWidget.
* [Change] Forcing the drop shadows update in MainWindow::setCurrentModel every time the current model changes.
* [Change] Minor adjustment in debug message related to model loading time.
* [Change] Minor adjustment in ModelWidget::pasteObjects to avoid showing the rename widget when pasting table child objects.
* [Change] Moved the function GuiUtilsNs::formatMessage to UtilsNs.
* [Change] Minor adjustment in the example.dbm.
* [Change] Minor adjustment in ObjectsScene::getPagesForPrinting to return the correct page size so the page delimiters can be displayed on all edges.
* [Change] Refactored the method ObjectsScene::adjustSceneRect to get more accurate results.
* [Change] Adjusted the default size of StyledTextboxView.
* [Change] Changed the method FileSelectorWidget::getSelectedFile in such a way as to append the default suffix when the user types a filename that has no extension.
* [Change] Minor adjument in FileSelectorWidget usage in ModelDatabaseDiffForm, ModelExportForm and ModelFixForm.
* [Change] Changed the parent class of RoundedRectItem from QGraphicsRectItem to QAbstractGraphicsShapeItem due to the incorrect bounding rect returned when the rounded rect is null (all coordinates set to 0). This was causing undesired results when retrieving scene items' bounding rect.
* [Change] Adjusted the method ObjectsScene::getPagesForPrinting to cover infinite canvas.
* [Change] Minor adjustment in the stylesheet of NewObjectOverlayWidget.
* [Change] Removed the origin point locks at (0,0) in ObjectsScene.
* [Change] Changes in ModelOverviewWidget to handle the infinite canvas.
* [Fix] Fixed the plugins' loading process by ignoring the ones that don't implement the correct interface (PgModelerCliPlugin or PgModelerGuiPlugin).
* [Fix] Fixed a bug in MainWindow::loadModel that was not registering the loaded file as a recent model.
* [Fix] Fixed a malformed diff code when adding a column to a partitioned table.
* [Fix] Fixed a bug in MainWindow::updateRecentModelsMenu.
* [Fix] Fixed a bug in GeneralConfigWidget::loadConfiguration to reset alerts when the parameter "use default disambiguation" is absent.
* [Fix] Minor fix in ConnectionsConfigWidget that was preventing connections from being updated in MainWindow after editing them.
* [Fix] Minor fix in ModelWidget to build in Qt 6.2.x.
* [Fix] Minor fix in ModelWidget::pasetObjects to correctly paste table/view child objects in the destination parent table/view.
* [Fix] Fixed the disconnect call in ObjectRenameWidget::setAttributes.
* [Fix] Minor fix in ModelWidget::startSceneMove to reset viewport cached content.
* [Fix] Minor fix on ObjectsScene::drawBackground in order to draw the canvas background in the correct color defined in the settings.
* [Fix] Minor fix on all classes derived from BaseConfigWidget to correctly control the config changed status on loadConfiguration and saveConfiguration.
* [Fix] Fixed a bug that was preventing tables and schemas from being updated graphically when adding/removing relationships in some circumstances.
* [Fix] Fixed a bug in ModelWidget::copyObjects related to copying/pasting multiple objects.
* [Fix] Minor fix in ModelWidget::pasteObjects when pasting copied functions.
* [Fix] Minor fix in graphical objects tooltips in libcanvas.
* [Fix] Fixed the trigger catalog query to avoid referencing tgparentoid field when importing from PostgreSQL 12 or below.
* [Fix] Minor fix in MainWindow to adjust the scene size every time an operation (undo/redo) is performed in the operation list.
* [Fix] Minor fix in ObjectsScene::drawBackground to avoid the undesired shifting on the page delimiters.
* [Fix] Minor fix in SchemaView to avoid leaving the selection rectangle visible in the constructor.
* [Fix] Minor fix in ObjectsScene::expandSceneRect.
* [Fix] Fixed the PNG export process in GUI and CLI to handle infinite canvas.
* [Fix] Fixed a bug in ModelFixForm that was not locating pgmodeler-cli on Windows.
* [Fix] Fixed a bug in ObjectsFilterWidget that was setting wrong object types when doing a partial diff using filters generated from the changelog.
* [Fix] Minor fix in CodeCompletionWidget in order to list columns of tables non-schema qualified. In that case, pg_catalog and public will be used as default schemas.
* [Fix] Columns marked as PK are now restored when handling them in TableWidget.

v1.1.0-beta1
------
*Release date: December 30, 2023*

* [New] Added the method Operation::getOperationInfo that returns some info (in the form of a struct) of an operation in read-only format.
* [New] Created static methods error, info, and alert in Messagebox.
* [New] Added support for custom font size in NumberedTextEditor/SyntaxHighlighter. This way UI elements that use objects of these classes in database object forms can keep the font size uniform even if the user has configured a different font size for source code inputs in SQL tool, for example.
* [New] Added support for displaying a "Blog post" button in UpdateNotifierWidget.
* [New] Added the method SQLToolWidget::moveExecutionTab.
* [Change] Minor adjustment in ConnectionsConfigWidget openConnectionsConfiguration to return true when the user applies new connection settings or restores original connections by clicking "Cancel".
* [Change] Adjusted the usage of ConnectionsConfigWidget::openConnectionsConfiguration on DatabaseImportForm, ModelDatabaseDiffForm, ModelValidationWidget and SQLToolWidget.
* [Change] Minor improvement on SwapObjectsIdsWidget to preserve the sorting parameters after swapping object ids.
* [Change] Minor change in ConnectionsConfigWidget, ModelExportForm, ModelValidationWidget related to the use of ConnectionsConfigWidget::openConnectionsConfiguration.
* [Change] Minor improvement in Messagebox::error method in order to allow the user to specify the local in the code where the error occurred using __PRETTY_FUNCTION__, __FILE__ and __LINE__.
* [Change] Moved the definition of template method BaseForm::setMainWidget to the outside of the class definition.
* [Change] Refactored OperationListWidget::updateOperationList to use OperationInfo struct.
* [Change] Minor change in operationlistwidget.ui to use icons instead of labels for operation list size and current history position.
* [Change] Refactored the method ModelWidget::openEditingForm(QWidget*,Messagebox::ButtonId) turning it into a template method so the internal call to BaseForm::setMainWidget can be correctly used.
* [Change] Changed the method BaseForm::setMainWidget that takes a BaseObjectWidget instance to be enabled only if a class that is inherited from BaseObjectWidget is passed as a parameter.
* [Change] Refactored signal/slot connections that needed exception handling all over the tool.
* [Change] Minor adjustment in MainWindow::addModel to avoid marking the model as invalidated even if it's new/empty.
* [Change] Minor adjustment in CodeCompletionWidget to include sequences when listing living db objects.
* [Fix] Fixed the catalog query operator.sch so argument data types can have the schema name prepended.
* [Fix] Fixed a crash when trying to display function/procedure source code that references a domain in the parameters.
* [Fix] Fixed the catalog queries for function and procedure to correctly format parameters' data type signatures.
* [Fix] Fixed a problem in SwapObjectsIdsWidget::eventFilter that was not capturing arrow key press on macOS.
* [Fix] Fixed a bug in the importing of partitioned table indexes which was affecting the results produced by the diff process.
* [Fix] Fixed a bug in CodeCompletionWidget that was crashing when trying to list columns of an alias related to a table not schema-qualified.
* [Fix] Fixed a bug in reverse engineering related to the importing of partition tables hierarchy.
* [Fix] Fixed a bug in diff that was generating false-positive results for columns with the same user-defined type.
* [Fix] Fixed a bug in LineNumbersWidget that was not aligning the line numbers with their respective lines when the font size was different from the default.
* [Fix] Minor fix in XmlParser::readBuffer to use a const xmlError * instance instead of non-const.
* [Fix] Minor fix in CodeCompletionWidget to avoid the displaying of the widget in a position that extrapolates the screen limits.
* [Fix] Minor fix in the icon positioning in HtmlItemDelegate.

v1.1.0-beta
------
*Release date: November 17, 2023*

* [New] Added a compatibility code in pgmodeler-cli to enable the conversion of views in the legacy structure to the newer one.
* [New] Added support for handling object references in ViewWidget.
* [New] Added support for NULLS NOT DISTINCT attribute in unique constraints and indexes.
* [New] Added support for overriding the bg color when exporting the model to PNG.
* [New] Added a grid widget in ExtensionWidget to inform the child data type names.
* [New] Added a new method for the application path relocation by using an auxiliary file pgmpaths.conf that holds the env variables that customize assets/executables paths in pgModeler.
* [New] Added a SQL definition tab and a new References tab that will work like references in GenericSqlWidget in ViewWidget.
* [New] Added a step in PgModelerCliApp::fixObjectAttributes to add a <type name="ext-name"> tag in <extension> that has the deprecated handle-type attribute.
* [New] Added missing pseudo-types.
* [New] Added the class SimpleColumnsWidget to handle the view's columns.
* [New] Added the methods isReferenced and hasDependencies in BaseObject.
* [New] Added the method BaseObject::isDependingOn and BaseObject::isReferencedBy.
* [New] Added the method GenericSQL::addObjectReferences.
* [New] Added the method GenericSQL::setHideDescription that toggles the SQL code comment that indicates the object name/type.
* [New] Added the method ObjectsTableWidget::getCellTextss.
* [New] Added version descriptor for PostgreSQL 16.
* [New] Adding field ref_alias to GenericSQL::ObjectReference.
* [New] Created the method GlobalAttributes::init and added a call to it on all executable subprojects.
* [New] Created the widget ObjectReferencesWidget that handles GenericSQL references.
* [New] DatabaseImportHelper now imports extensions with the child types attached.
* [New] Extension object now stores the name of the child types.
* [New] In DatabaseModel, extension data types are automatically added/removed as the parent extension is added/removed from the model.
* [New] The corner button in SQLToolWidget which adds a new SQL execution tab is now positioned aside from the last tab open.
* [New] Validating the extension child types before adding the extension to the database model.
* [Change] Adjusted objectdepsrefswidget.ui to make the "Display unique results" option checked by default.
* [Change] Adjusted the class GenericSQL::Reference to use reference alias.
* [Change] Adjusted the class ObjectReferencesWidget to use reference aliases.
* [Change] Adjusted the CSV pasting in TableDataWidget.
* [Change] Adjusted the DatabaseExplorerWidget::formatExtensionAttribs to display types related to an extension.
* [Change] Adjusted the DLL export symbols to Reference and SimpleColumn (windows only).
* [Change] Adjusted the DTD and schema file for tag <object>
* [Change] Adjusted the DTD files view.dtd, genericsql.dtd, dbmodel.dtd, object.dtd
* [Change] Adjusted the methods formatOidAttribs, getObjectName, getObjectNames in DatabaseExplorerWidget to accept a list of types.
* [Change] Adjusted the SCH files view.sch, genericsql.sch, reference.sch, object.sch.
* [Change] Changed the struct SimpleColumn into a class due to the increase in the code complexity
* [Change] CodeCompletionWidget now resizes according to the displayed items' width.
* [Change] Code completion will not display a "no items found" popup if no element is found matching the word at the cursor's position.
* [Change] DatabaseModel::updateViewRelationships now use the new view references to create relationships.
* [Change] GenericSQL class now uses tags <reference> instead of <object> to store references in XML.
* [Change] GenericSQL::ObjectReferece renamed to GenericSQL::Reference and turned into a class.
* [Change] In relationshipconfigwidget.ui the images change with the selected theme.
* [Change] Minor adjustment in icons-*.conf
* [Change] Minor adjustment in ReferencesWidget to always display the signature of the object in the grid instead of the name.
* [Change] Minor adjustment in the behavior of source code display in DatabaseExplorerWidget.
* [Change] Minor adjustment in themes' appearance.conf
* [Change] Minor adjustment on ui style conf files.
* [Change] Minor change in reverse engineering to avoid importing extension child types into the model since the extension itself, when imported, already creates the types.
* [Change] Minor default size adjustment in TableWidget and ViewWidget.
* [Change] Minor refactor in DatabaseModel::createGenericSQL.
* [Change] Minor refactor in GenericSQL, GenericSQLWidget, ObjectReferencesWidget to use getters of GenericSQL::Reference.
* [Change] Minor refactor in ObjectReferencesWidget/GenericSQL::Referece to use a flag that indicates that view columns must be generated from the references.
* [Change] Minor refactor on View, Reference, and ReferenceWidget classes.
* [Change] Moved GenericSQL::Reference to dedicated source files.
* [Change] Moved the code that set the Qt UI style to the Application class.
* [Change] Refactored GenericSQLWidget in such a way to use ObjectReferencesWidget.
* [Change] Refactored View::generateColumns to generate columns from the new reference object.
* [Change] Removed CTE tab from ViewWidget.
* [Change] Removed deprecated method GenericSQL::getReferencedObjects.
* [Change] Removed the const UserTypeConfig::ExtensionType.
* [Change] Renamed ObjectReferencesWidget.(h|cpp) to ReferencesWidget.(h|cpp)
* [Change] Updated View XML definition to store simple column's code.
* [Fix] Fixed settings storing for the grid options in MainWindow.
* [Fix] Fix a crash that happens only on Windows.
* [Fix] Fixed a bug in the generation of diff commands for identity columns.
* [Fix] Fixed a bug in HtmlItemDelegate::paint that was causing the rendering of artifacts.
* [Fix] Fixed a bug in pgmodeler-cli that was aborting the fix process during the parsing of the model changelog.
* [Fix] Fixed a bug in SchemaParser that would cause an infinite loop when defining attributes via %set instruction combined with plain text operator []
* [Fix] Fixed a crash when trying to load an invalid model from the recent model's menu.
* [Fix] Fixed sample model structure to the new view's format.
* [Fix] Fixed several bugs in CodeCompletionWidget when completing using live database object names.
* [Fix] Fixed the method BaseTable::updateDependencies() to avoid duplicating dependencies.
* [Fix] Fixed the method DatabaseImportHelper::createView in such a way as to create references to tables/views and the columns that the view owns.
* [Fix] Minor fix in DatabaseImportHelper to correctly make a view reference another.
* [Fix] Minor fix in MainWindow::loadModels.
* [Fix] Minor fix on DatabaseModel::createRelationship to create a relationship between two views.

v1.0.6
------
*Release date: October 17, 2023*

* [New] Added version descriptor for PostgreSQL 16.
* [Fix] Fixed a crash when trying to load an invalid model from recents models menu.
* [Fix] Fixed a bug in generation of diff commands for identity columns.
* [Fix] Minor fix in PgModelerCliApp::extractObjectXML to restore correctly the layers names and count.
* [Fix] Fixed a crash when double-clicking the overview widget.

v1.1.0-alpha1
------
*Release date: September 29, 2023*

* [New] Added version descriptor for PostgreSQL 16.
* [New] Added support for highlighting table child objects in ObjectFinderWidget.
* [New] Added the method BaseTableView::setChildSelected.
* [New] Added support for resizing the text boxes in the model using Shift + mouse movement.
* [New] Created the method generateHashCode in TableObject and its derived classes.
* [New] Added the method BaseObject::getLinkedObjects.
* [New] Added the method AppearanceConfigWidget::getUiLightness.
* [New] Created the utility function CoreUtilsNs::filterObjectsByType.
* [New] Added the method BaseObject::setClearDepsInDtor to determine whether the dependencies/references should be undone when destroying an object.
* [New] Created a simplified mechanism for mapping dependencies and references between objects improving the performance of several parts of the tool.
* [New] Created the function UtilsNs::getStringHash and replaced all uses of QCryptographicHash by that function.
* [New] Added an option to ObjectDepsRefsWidget that toggles the display of unique results.
* [New] Created the class CustomSortProxyModel that preserves the vertical header line number when sorting an ObjectsListModel.
* [New] Created the method GuiUtilsNs::updateObjectsTable(QTableView *, std::vector<attribs_map>).
* [New] Created the method ObjectsListModel::fillModel.
* [New] Added the methods BaseObject::getSearchAttributeI18N and BaseObject::getSearchAttributesI18N.
* [New] Added the method BaseObject::acceptsComment.
* [New] Added support for caching the object names to avoid unnecessary formatting/validation.
* [New] Added the method BaseGraphicObject::setUpdatesEnabled to control whether the *::configureObject must be called to configure the graphical objects.
* [New] Added support for inksaver color theme.
* [New] Added support for using object comments as aliases in database import.
* [New] Added an option in GeneralConfigWidget to reset the exit alert display status.
* [New] Added the method GeneralConfigWidget::appendConfigurationSection.
* [New] Created the class ObjectsListModel to be used by any QTableView that is supposed to display objects listing.
* [New] Add support for remembering decisions on the alerts regarding unsaved models/open SQL tabs.
* [New] Added the method SQLToolWidget::ignoreAutoBrowseFlag.
* [New] Added the method SQLExecutionWidget::getSQLCommand.
* [New] Added the methods SQLToolWidget::getExecutionTabs.
* [New] pgModeler now asks the user about closing SQL execution tabs that are not empty (with typed commands).
* [New] Added a basic form to inspect changelog XML code.
* [New] Added a static method AppearanceConfigWidget::getUiThemeId that returns the current configured theme.
* [New] Added missing multirange types.
* [New] Added shortcuts Shift+Up and Shift+Down to control the Z-stack operations in ModelWidget.
* [Change] Adjusted the objects' movement using arrow keys in the object scene.
* [Change] Adjusted the PostgreSQL Version to 15 in pgmodeler.pri (only macos).
* [Change] Minor adjustment in ModelDatabaseDiffForm to exclude columns when using "any" filter.
* [Change] Replaced the use of DatabaseModel::getObjectReferences by BaseObject::getReferences
* [Change] All objects' dependencies/references retrieval methods were removed from DatabaseModel due to the new way to control/retrieve dependencies/references via the BaseObject class.
* [Change] Refactored DatabaseImportHelper::destroyDetachedColumns to use BaseObject::getReferences.
* [Change] Removed the method DatabaseModel::validateColumnRemoval.
* [Change] Changed DatabaseModel::__removeObject to use the new BaseObject::getReference.
* [Change] Changed ModelValidationHelper::validateModel to use BaseObject::getReference.
* [Change] Changed ModelWidget::copyObjects to use BaseObject::getDependencies.
* [Change] Minor adjustment in ModelWidget destructor.
* [Change] Minor adjustment in DatabaseModel destructor.
* [Change] Adjusted several virtual methods signatures.
* [Change] Refactored UserTypeConfig and PgSqlType to make use of BaseObject/DatabaseModel pointers instead of raw (void) pointers to configure user-defined types based on database model objects.
* [Change] Adjusted the font size for layer rects.
* [Change] Minor adjustment in DatabaseModel::validateSchemaRenaming.
* [Change] Improved the relationship point addition via mouse clicks.
* [Change] Minor adjustment in DatabaseModel::setObjectsModified.
* [Change] The "dot" grid mode is now the default in the appearance.conf file due to better drawing performance.
* [Change] Created a mechanism in Relationship to reuse allocated objects instead of deleting them and allocating them again every time the relationship is connected/disconnected.
* [Change] Improved the scene background (grid, delimiter, limits) drawing speed for big models.
* [Change] Refactored DatabaseImportForm in such a way as to use QTableView instead of QTableWidget to list filter results.
* [Change] Refactored ModelDatabaseDiffForm in such a way as to use QTableView instead of QTableWidget to list filter results.
* [Change] Minor adjustment in ObjectsListModel to accept a list of attributes map to fill the table model.
* [Change] Refactored SwapObjectsIdsWidget in such a way as to use QTableView instead of QTableView.
* [Change] Refactored ObjectDepsRefsWidget by replacing QTableWidget with QTableView to display model objects.
* [Change] Minor adjustment in ObjectFinderWidget filter items order.
* [Change] Minor adjustment in GuiUtilsNs::updateObjectsTable(QTableView *) to create a QSortFilterProxyModel in the QTableView allowing sorting.
* [Change] Refactored ObjectFinderWidget to use QTableView instead of QTableWidget to display search results.
* [Change] Removed the QTableWidget instance in ModelObjectsWidget for performance reasons and for being a widget almost unused.
* [Change] Minor adjustment in BaseRelationship::connectRelationship.
* [Change] Adjusted BaseObjectWidget::applyConfiguration to avoid crashing when the general attributes of the widget are in a non-visible tab like in TableWidget.
* [Change] Reduced the number of times BaseGraphicObject::setModified is called consequently reducing the excessive objects' rendering.
* [Change] Minor adjustment in ObjectsScene::drawBackground to avoid rendering the grid and page delimiters during corner/scene move.
* [Change] Moved the table's general attributes to a dedicated tab which opened more space for displaying columns and other children's objects.
* [Change] Refactored GuiUtilsNs::updateObjectTable to accept a QTableView instead of QTableWidget.
* [Change] Updated the pgmodeler-cli menu to include the "any" filter explanation.
* [Change] Removed the plugin's configuration menu from the main toolbar.
* [Change] Improving the objects' filtering in reverse engineering by adding an "any" filter type.
* [Change] Minor adjustment on SQLToolWidget
* [Change] Improved the import error message when it's not possible to create/import permission that references a predefined PostgreSQL role.
* [Change] Minor improvement on ChangelogWidget layout.
* [Change] Minor code refactor in ObjectsScene and DatabaseModel.
* [Change] DataManipulationForm now shows a confirmation message before closing when items are pending save.
* [Change] Moved the method AppearanceConfigWidget::updateDropShadows to GuiUtilsNs::updateDropShadows
* [Change] Refactored the usage of AppearanceConfigWidget::updateDropShadows to use GuiUtilsNs::updateDropShadows
* [Change] Minor adjustments in swapobjectsidswidget.ui layout.
* [Change] Several minor widget adjustments.
* [Change] Renamed the methods startPanningMove and finishPanningMove to, respectively, startSceneMove and finishSceneMove.
* [Fix] Minor fix in ObjectFinderWidget to avoid disconnecting a null selection model in findObjects.
* [Fix] Minor fix in ModelWidget to hide new objects overlay when moving a selection of objects.
* [Fix] Minor fix in GuiUtilsNs::disableObjectSQL.
* [Fix] Minor fix in RelationshipView::configureBoundingRect and RelationshipView::configureLine.
* [Fix] Minor fix in ModelWidget::removeObjects that was not erasing an object in case it shared the same name or other objects in the same schema.
* [Fix] Minor fix in DatabaseModel::__addObject that validates the layer of the object being added. If one or more layers are invalid the object will be moved to the default layer 0
* [Fix] Fixed the bounding rect calculation for RelationshipView.
* [Fix] Fixed a bug in DatabaseModel::findObject that was not parsing correctly the search filters with the "any" keyword.
* [Fix] Fixed the icons-*.conf to include QTableView class.
* [Fix] Minor fix in PgModelerCliApp::extractObjectXML to restore correctly the layers name/count.
* [Fix] Fixed a bug in partial reverse engineering that was not correctly importing functions in some specific conditions.
* [Fix] Fixed a bug in partial reverse engineering that was not importing some dependencies correctly.
* [Fix] Fixed a bug in AppearanceConfigWidget that was not updating the example model colors when changing the UI theme.
* [Fix] Fixed a crash when double-clicking the overview widget.
* [Fix] Fixed data dictionaries schema files for tables and views.
* [Fix] Fixed a bug in DatabaseModel that was causing FK relationships of a hidden layer to be displayed after loading the model.
* [Fix] Fixed a bug in MainWindow that was causing the plugin's config action icon to disappear when triggering validation.
* [Fix] Fixed the headers inclusion chain in all subproject sources.
* [Fix] Fixed a bug in scene move that was causing the grid to not be displayed after a panning/wheel move.

v1.0.5
------
*Release date: July 26, 2023*

* [New] Added shortcuts to Z-stack operations in ModelWidget.
* [New] Added missing PostgreSQL multirange types
* [Change] Improved the import error message when it's not possible to create/import a permission which references a predefined PostgreSQL role.
* [Change] Minor code refactor in ObjectsScene and DatabaseModel.
* [Fix] Fixed Qt 6.2 release in linuxbuild.yml and macosbuild.yml.
* [Fix] Fixed a bug in DatabaseModel that was causing FK relationships of a hidden layer to be displayed after loading the model.
* [Fix] Fixed a bug in MainWindow that was causing the plugins config action icon to disappear when triggering validation.
* [Fix] Fixed a bug in PgModelerCliApp::fixModel that was causing the generation of empty models when the input file had no tag <role>.

v1.1.0-alpha
------
*Release date: June 09, 2023*

* [New] Added support for drag & drop files in the main window to load models.
* [New] Added NOBYPASSRLS keyword to sql-highlight.conf.
* [New] Added OS type information in AboutWidget.
* [New] Added support for hiding objects' shadows in the settings.
* [New] Added support for installing tool buttons created by plugins in DatabaseExplorerWidget.
* [New] Added support for code completion based on living database object names in CodeCompletionWidget. It is now possible to list column/table names in the middle of the INSERT/DELETE/TRUNCATE/UPDATE commands.
* [New] Added support for exporting results in CSV and plain text format in DataManipulationForm and SQLExecutionWidget.
* [New] Added support for highlighting enclosing characters (),{},{} in SyntaxHighlighter.
* [New] Added the action NumberedTextEditor::pasteCode which tries to remove unneeded string concatenation chars ", ', + and . in the clipboard text before inserting it in the input field. This is useful for developers that copy SQL code in the middle of a programming language source code and want to test it in the database.
* [New] Added the button id Messagebox::CloseButton that creates a message box with only a close button.
* [New] Added the class CustomUiStyle to control global settings for pixel metrics of UI elements.
* [New] Added the method BaseForm::adjustMinimumSize.
* [New] Added the method CodeCompletionWidget::setConnectionParams.
* [New] Added the method PgModelerPlugin::getTmplPluginFilePath.
* [New] Added the methods setFileMustExist, setCheckExecutionFlag, setFileIsMandatory, and setNamePattern to FileSelectorWidget.
* [New] Added the options in GeneralConfigWidget that controls the column data truncation in the results grid.
* [New] Added support for setup name color of schema objects.
* [New] Added support to convert to XML entities the characters in attribute values in SchemaParser.
* [New] Added support for switching the current user when successfully connecting to a database via SET ROLE command.
* [New] Created the method DatabaseImportForm::listDatabases(Connection,QComboBox).
* [New] Enabling the displaying of byta column data in the results grid.
* [New] Now any SQL result grid header will display an icon according to the data type of each column.
* [New] Removed the method FileSelectorWidget::setFileMode and created to FileSelectorWidget::setDirectoryMode that toggles the selection of directories instead of files (the default behavior).
* [Change] Minor adjustment in FileSelectorWidget::validateSelectedFile.
* [Change] Added a wait cursor during the code preview in SourceCodeWidget.
* [Change] Added a wait cursor while searching objects in ObjectFinderWidget.
* [Change] Adjusted the access modifier in PgModelerPlugin methods.
* [Change] Adjusted the object catalog queries to include a general-purpose field named extra_info in the object list catalog query.
* [Change] Adjusted the warning icon size in FileSelectorWidget.
* [Change] Disabled the usage of keywords in the code completion instance of DataManipulationForm filter field.
* [Change] Dropped the use of syntax-hl-theme in appearance.conf.
* [Change] Improved the behavior of the "System default" UI theme.
* [Change] Improved the methods GuiUtilsNs::selectAndSaveFile, GuiUtilsNs::selectAndLoadFile.
* [Change] Improvements in LineNumbersWidget and NumberedTextEditor to correctly draw line numbers on text with the word wrap option activated.
* [Change] In data manipulation form when editing a single element with column data edit dialog, the form will display the current column's value.
* [Change] Change to medium the UI icons size of default settings.
* [Change] Minor adjustment in aboutsewidget.ui and aboutwidget.ui.
* [Change] Minor adjustment in the minimum size of Messagebox.
* [Change] Minor adjustment in Catalog::isConnectionValid.
* [Change] Minor adjustment in file coreutilsns.cpp.
* [Change] Minor adjustment in ModelFixForm to display pgmodeler-cli selector when the provided path to the executable is not valid.
* [Change] Minor adjustment in generalconfigwidget.ui.
* [Change] Minor adjustment in ObjectFinderWidget.
* [Change] Minor adjustment in schemaeditorform.ui.
* [Change] Minor refactor in TableObjectView, Connection, SchemaParser.
* [Change] Moved the copy actions in SQL results grid context menu to an action named the "Selection" in the same menu.
* [Change] Now the ColumnDataEditWidget instances have their geometry saved/restored.
* [Change] Replace the QPlainTextEdit by NumberedTextEditor in ColumnDataWidget.
* [Change] Refactored DatabaseImportHelper::createObject in such a way to use a map of bind methods to perform operations instead of using a long list of switch/case to determine which method must be called to create the object during reverse engineering.
* [Change] Refactored DatabaseModel::createObject, DatabaseModel::addObject, and DatabaseModel::removeObject in such a way to use a map of bind methods to perform operations instead of using a long list of if/else to determine which method must be called.
* [Change] Removed the get*Action and getDbExplorerButton methods in PgModelerPlugin and created the methods getAction and getWidget instead.
* [Change] Removed the method Catalog::getObjectCount(bool) and created a version that accepts a list of object types.
* [Change] Removed unneeded use of QString("").
* [Change] Removed unused frame in modelfixform.ui.
* [Change] Renamed BulkDataEditWidget to ColumnDataWidget
* [Change] Replaced QApplication::setOverrideCursor/restoreOverrideCursor by qApp- >setOverrideCursor/restoreOverrideCursor.
* [Change] The method DatabaseExplorerWidget::addPluginToolButton passes the current connection id and database name as the plugin button property to be read in the click/triggered slots implemented in the plugin.
* [Change] Update all schema files that generate XML code to use &{} attribute form where there's the need to convert chars to their XML entities counterparts.
* [Change] When retrieving objects from the database CodeCompletionWidget will not display keywords and snippets in the popup list.
* [Fix] Addition fix in CodeCompletionWidget in such a way to preserve cursor position when inserting the selected word after a special char.
* [Fix] Fix a bug in PgModelerCliApp::fixModel that was causing the generation of empty models when the input file had no tag <role>
* [Fix] Fixed a bug in Catalog::getObjectsCount.
* [Fix] Fixed a bug in DatabaseModel that could lead to an "unknown exception caught" error.
* [Fix] Fixed a bug in NumberedTextEditor::identSelection.
* [Fix] Fixed a bug in SqlExecutionWidget::fillResultsTable.
* [Fix] Fixed a bug in TableDataWidget::generateDataBuffer that was causing the generation of malformed CSV in some circumstances.
* [Fix] Fixed a crash in the SQL tool when closing a database explorer instance that owns a plugin tool button.
* [Fix] Fixed CodeCompletionWidget::retrieveColumnNames when referencing a table via an alias.
* [Fix] Fixed problems with comments on a database having the same OIDs of different types of objects
* [Fix] Fixed the diff process on legacy database versions.
* [Fix] Fixed the line selection operation in LineNumbersWidget.
* [Fix] Fixed the undefined reference error when building on Windows.
* [Fix] Minor fixes in the catalog queries for column, cast, collation, and role.
* [Fix] Minor fix in GeneralConfigWidget when using NO_UPDATE_CHECK.
* [Fix] Minor fix in process output capturing in ModelFixForm.
* [Fix] Minor tooltip fix in DataManipulationForm.

v1.0.4
------
*Release date: May 19, 2023*

* [New] Now pgModeler selects the UI element colors based upon the system's default colors set (light/dark).
* [New] pgModeler now restores the default settings in case of some configuration file is corrupted/incompatible and causes the initialization to fail.
* [New] Allowing sequences to be assigned to columns with numeric type.
* [New] Created the methods GuiUtilsNs::saveFile and GuiUtilsNs::loadFile.
* [Change] Removed the deprecated attribute partial-match from configuration files in conf/defaults.
* [Change] Minor adjustment in output messages in PgModelerCliApp::createConfigurations.
* [Change] SyntaxHighlighter when in single-line mode will strip any line break char in the input field.
* [Change] Minor adjustment in SyntaxHighlighter to force no line wrap in the parent input when single_line_mode is activated.
* [Change] Changed the behavior of TableDataWidget::populateDataGrid. Instead of failing and never opening the dialog again for the user to try to import another file, the method now ask for saving the current (corrupted data) to a file and opens an empty grid to a new CSV import.
* [Change] Removed unused constant PhysicalTable::DataLineBreak.
* [Fix] Fixed a false-positive diff result when comparing numeric columns.
* [Fix] Minor fix in the name pattern of the settings backup folder in PgModelerCliApp::createConfigurations.
* [Fix] Fixed bug in reverse engineering that was happening during the creation of object permissions.
* [Fix] Minor fixes in the catalog queries for cast, collation, and role.
* [Fix] Fixed the wrong usage of cached names and signatures in DatabaseImportHelper.
* [Fix] Additional fix in CsvParser::parseBuffer to append a line break character at the end of the buffer in case it is missing so the parsing can be done correctly.

v1.0.3
------
*Release date: April 24, 2023*

* [New] pgmodeler-cli now logs objects that fail to be recreated in fix process into a log file stored in pgModeler's temp directory.
* [New] Added a progress bar to model fix form and a cancel button which allows aborting the fix operation without close that form.
* [New] Added a specific icon for CSV load button in CsvLoadWidget.
* [New] Added the methods Trigger::getColumns and Trigger::addColumns.
* [Change] Adjusted the behavior of hide and close events of model fix form.
* [Change] Minor adjustment in pgmodeler-cli model fix messages.
* [Change] Refactored TriggerWidget to use an instance of ColumnPickerWidget.
* [Fix] Fixed the database model file header validation for huge models in pgmodeler-cli.
* [Fix] Fixed a bug in TableDataWidget::generateDataBuffer that was causing the generation of malformed CSV in some circumstances.
* [Fix] Fixed the PluginsConfigWidget::initPlugins in such a way to remove the plugins that failed to load from the plugins grid.
* [Fix] Fixed a bug in BaseRelationship::canSimulateRelationship11 that was wrongly returning true.

v1.0.2
------
*Release date: March 14, 2023*

* [Fix] Fixed a bug related to importing referenced tables of FKs in partial import mode.
* [Fix] Fixed the attributes toggler item border style in partition tables.
* [Fix] Fixed a regression that was preventing the canvas color to be changed.
* [Fix] Fixed a bug in BaseObjectWidget::finishConfiguration that was preventing the creation of a role and a table with the same names.
* [Fix] Fixed the generation of DROP command for policy, trigger, and rule.
* [Fix] Fixed a bug that was causing objects to be selected in the sample model at appearance settings.
* [Fix] Forcing the usage of Qt 6.x due to problems on Windows when compiling with Qt 5.
* [Fix] Minor adjustment in CodeCompletionWidget stylesheet.
* [Fix] Saving/restoring the painter settings after drawing the background in ObjectsScene.
* [Fix] Fixed a malformed diff code for policies.

v1.0.1
------
*Release date: February 17, 2023*

* [New] AppearanceConfigWidget now adjusts the drop shadows on tool buttons according to the current theme.
* [Change] pgModeler will now ask for PK columns uncheck when removing a primary key in the Constraints tab at TableWidget
* [Change] Disabling catalog query for transform objects in PG9.x
* [Change] Improving the visibility of some elements in QMenu and AboutWidget
* [Fix] Fixed the assignment of layer colors when loading the model in compatibility mode.
* [Fix] Minor fix in ModelWidget::toggleSchemasRectangles
* [Fix] Fixed the disable state change for delimiter scale lock action when changing the current main window view.
* [Fix] Fixed the method FileSelectorWidget::openFileExternally.

v1.0.0
------
*Release date: February 1, 2023*

* [New] Added an option in GeneralConfigWidget to allow pgModeler to connect to older PostgreSQL server versions, below 10 using a minimum (not reliable) compatibility mode.
* [New] Added an option to change ObjectsScene grid pattern. Available options are square and dot.
* [New] Added an option to lock the page delimiters resize during zoom out (<100%) to allow the user to place more objects on one page.
* [New] Added the ability to print models with a specified scale factor.
* [New] Added the method MainWindow::updateWindowTitle.
* [New] Created the method ObjectsTableWidget::setRowColors.
* [New] Added the public slot ObjectsTableWidget::resizeContents.
* [New] Added the signal MainWindow::s_modelLoadRequested.
* [New] Added the method PgModelerPlugin::postInitPlugin which is called when the main window initialization is ready.
* [New] Added the method MainWindow::registerRecentModel.
* [New] Added the method PgModelerPlugin::getPluginFilePath.
* [New] Created the method DatabaseModel::setDatabaseModelAttributes.
* [New] Added the class ObjectTypesListWidget that implements a QListWidget specialization for object types handling.
* [New] Added the method PgModelerPlugin::getPluginIconPath.
* [New] Added the method MainWindow::getCurrentModel.
* [Change] Minor adjustment in the factor applied to delimiter lines in ObjectsScene::setGridSize.
* [Change] Minor tooltip adjustment in generalconfigwidget.ui
* [Change] pgModeler now allows to import and diff in PostgreSQL versions below 10 using the minimum compatibility mode without guaranteeing reliable results.
* [Change] Moved the Q_DECLARE_METATYPE macro call to the header files of classes that use it.
* [Change] Dropped outdated UI translations.
* [Change] Minor adjustment in ModelWidget::printModel to hide scene boundary lines when printing model.
* [Change] Minor adjustment in ObjectsScene::finishObjectsMove and ModelWidget::adjustSceneSize to give extra size to the scene rectangle.
* [Change] The scene rectangle is now always resized to the current objects' bounding rectangle.
* [Change] Overloaded the method QGraphicsScene::drawBackground in ObjectsScene to draw the grid, page delimiters, and scene limits instead of using pixmap-based background.
* [Change] Minor adjustment in the plugins' actions in the toolbar.
* [Change] Minor adjustment in the data retrieving message in DataManipulationForm.
* [Change] Allowing plugins to insert actions in the top toolbar at the main window via PgModelerPlugin::getToolbarAction.
* [Change] Replaced the object type filter in ModelObjectsWidget with an instance of ObjectTypesListWidget.
* [Change] Moved the method ObjectFinderWidget::updateObjectTable to GuiUtilsNs.
* [Change] Minor refactor by replacing hardcoded references to .dbm extension by GlobalAttributes::DbModelExt.
* [Change] Changed the location where the MainWindow::s_modelSaved signal is emitted.
* [Change] Minor adjustment in the plugin's info dialog.
* [Change] Minor adjust in icons CSS.
* [Change] Changed the position of the recent model actions.
* [Change] Minor adjustment in mainwindow.ui.
* [Change] Minor refactor in MainWindow and WelcomeWidget.
* [Change] Removed the overloaded version of getSignature from Trigger, Policy, and Rule.
* [Change] Minor adjust in pgmodeler.pri.
* [Change] Replaced the clunky filter options in ObjectFinderWidget with a menu action associated with the "Filter" button.
* [Change] Changed the behavior of DatabaseModel::findObjects. Now it treats BaseRelationship and Relationship as the same object type to make the search results more accurate.
* [Change] Renamed some GlobalAttributes consts *Dir to *Path for a better semantics.
* [Change] Removed unused variable in DatabaseModel::findObjects.
* [Change] Adjusted Constraint::configureSearchAttributes to create a "signature" attribute in the form schema.table.name.
* [Change] Changed getPluginIconPath to retrieve icons from .qrc file instead of from the filesystem.
* [Change] Minor improvement in HtmlItemDelegate. Now one can construct this object by opting to ignore the HTML tags in sizeHint calculation.
* [Change] Refactored PgModelerPlugin by removing methods hasMenuAction, hasConfigurationForm, and getPluginShortcut.
* [Change] Refactored PluginsConfigWidget::installPluginsActions to receive only a QMenu instance.
* [Change] Removed MainWindow::executePlugin slot since the plugin execution must be triggered by the plugin itself via menu action or model action.
* [Change] Minor improvement in PluginsConfigWidget to force the unloading of plugins that do not meet a version requirement.
* [Fix] Fixed a bug in ModelOverviewWidget that was causing generated pixmap to be blurry on hi-dpi screens.
* [Fix] Fixed a crash when the user tried to browse a DB in PostgreSQL 15.
* [Fix] Fixed how printer settings are changed compared to the default one and triggers a message box so the user can decide which conf to use.
* [Fix] Fixed the use of zoom factor in the PNG export process.
* [Fix] Fixed the ConnectionsConfigWidget::handleConnection alias desambiguation routine.
* [Fix] Minor fix in the BaseForm::setButtonConfiguration signature.
* [Fix] Fixed a bug that was causing relationship name patterns to be loaded in the wrong order.
* [Fix] Fixed the generation of the COMMENT command for constraints.
* [Fix] Fixed the class ObjectTypesListWidget by adding __libgui directive (Windows only).
* [Fix] Minor fix in HtmlItemDelegate to display text in the correct position.
* [Fix] Minor fix in CodeCompletionWidget.
* [Fix] Fixed a bug in the generation of the ALTER OWNER command for materialized views.
* [Fix] Minor fix in ObjectRenameWidget and TaskProgressWidget window style.
* [Fix] Minor fix in PgModelerCliApp to properly restore special objects when fixing models.

v1.0.0-beta1
------
*Release date: November 25, 2022*

* [New] Added the file themes/light/extra-ui-style.conf that holds extra style for the light theme.
* [New] Added dllexport symbol __lib* to functions and static attributes declarations in all namespaces.
* [New] Created defines for each library name __lib[NAME] in file [NAME]global.h which wraps Q_DECL_EXPORT/Q_DECL_IMPORT to export or import symbols of the libraries (needed for Windows).
* [New] Added the ability to toggle the magnifier tool in the main window.
* [New] Added a text search widget in the diff code preview on ModelDatabaseDiffForm.
* [New] Added unary operators for enum operations: |= , &= , ^=
* [New] Added a new implementation of CSV file loading based on RFC4180. Now the classes CsvParse and CsvDocument are responsible for parsing CSV files.
* [Change] Disabled unneeded conditional code in MainWindow.
* [Change] Adjusted the font weight in toolbar actions.
* [Change] Minor text adjustment in DonateWidget.
* [Change] Minor adjustments in UI stylesheets.
* [Change] Refactored TemplateType and all its derived classes to avoid the error "explicit specialization of 'type_names'" when building with clang.
* [Change] Adjusted the zoom in/out actions enabled state according to the current model's zoom factor.
* [Change] Adjusted AppearanceConfigWidget to load the extra-ui-style.conf.
* [Change] Adjusted the behavior of the magnifier action in MainWindow.
* [Change] Minor adjustment in MainWindow::setBottomFloatingWidgetPos to take into account the main menu bar height when visible.
* [Change] Changed the MetadataHandlingForm's Apply and Cancel actions to Execute and Close.
* [Change] Minor refactor in ModelFixForm to remove clangd warning.
* [Change] Adjusted the icon of the current input model/DB in the partial diff tab in ModelDatabaseDiffForm.
* [Change] Adjusting the context object in thread signal/slot connections in ModelExportForm.
* [Change] Adjusted plugins and arranged object actions in the main menu.
* [Change] Adjusted the captured variables in lambda slots avoiding default capturing [&].
* [Change] Added a context object parameter in connect() which receives lambda slots.
* [Change] Refactored MainWindow signal/slot connections to the new syntax.
* [Change] Refactored ModelWidget signal/slot connections to the new syntax.
* [Change] Refactored ModelDatabaseDiffForm signal/slot connections to the new syntax.
* [Change] Refactored ModelExportForm signal/slot connections to the new syntax.
* [Change] Refactored ModelObjectsWidget signal/slot connections to the new syntax.
* [Change] Refactored ModelValidationWidget signal/slot connections to the new syntax.
* [Change] Refactored SQLExecutionWidget signal/slot connections to the new syntax.
* [Change] Refactored TableDataWidget signal/slot connections to the new syntax.
* [Change] Refactored SourceCodeWidget signal/slot connections to the new syntax.
* [Change] Refactored UpdateNotifierWidget signal/slot connections to the new syntax.
* [Change] Refactored OperationListWidget signal/slot connections to the new syntax.
* [Change] Refactored ObjectsTableWidget signal/slot connections to the new syntax.
* [Change] Refactored ObjectsFilterWidget signal/slot connections to the new syntax.
* [Change] Refactored ObjectSelectorWidget signal/slot connections to the new syntax.
* [Change] Refactored ObjectRenameWidget signal/slot connections to the new syntax.
* [Change] Refactored ObjectFinderWidget signal/slot connections to the new syntax.
* [Change] Refactored ObjectDepsRefsWidget signal/slot connections to the new syntax.
* [Change] Refactored NumberedTextEditor signal/slot connections to the new syntax.
* [Change] Refactored NewObjectOverlayWidget signal/slot connections to the new syntax.
* [Change] Refactored ModelOverviewWidget signal/slot connections to the new syntax.
* [Change] Refactored ModelNavigationWidget signal/slot connections to the new syntax.
* [Change] Refactored LineNumbersWidget signal/slot connections to the new syntax.
* [Change] Refactored LayersWidget signal/slot connections to the new syntax.
* [Change] Refactored LayersConfigWidget signal/slot connections to the new syntax.
* [Change] Refactored FindReplaceWidget signal/slot connections to the new syntax.
* [Change] Refactored FileSelectorWidget signal/slot connections to the new syntax.
* [Change] Refactored CustomSQLWidget signal/slot connections to the new syntax.
* [Change] Refactored CsvLoadWidget signal/slot connections to the new syntax.
* [Change] Refactored ColorPickerWidget signal/slot connections to the new syntax.
* [Change] Refactored CodeCompletionWidget signal/slot connections to the new syntax.
* [Change] Refactored ChangelogWidget signal/slot connections to the new syntax.
* [Change] Refactored SQLToolWidget signal/slot connections to the new syntax.
* [Change] Disabled custom font size for hint boxes in BaseObjectWidget, ModelDatabaseDiffForm, ModelFixForm, and ModelWidget.
* [Change] Refactored SwapObjectsIdsWidget signal/slot connections to the new syntax.
* [Change] Refactored ModelValidationHelper signal/slot connections to the new syntax.
* [Change] Refactored ModelRestorationForm signal/slot connections to the new syntax.
* [Change] Refactored ModelFixForm signal/slot connections to the new syntax.
* [Change] Refactored ModelExportHelper signal/slot connections to the new syntax.
* [Change] Refactored MetadataHandlingForm signal/slot connections to the new syntax.
* [Change] Refactored DataManipulationForm signal/slot connections to the new syntax.
* [Change] Refactored DatabaseImportForm signal/slot connections to the new syntax.
* [Change] Refactored DatabaseExplorerWidget signal/slot connections to the new syntax.
* [Change] Refactored BugReportForm signal/slot connections to the new syntax.
* [Change] Refactored SnippetsConfigWidget signal/slot connections to the new syntax.
* [Change] Refactored RelationshipConfigWidget signal/slot connections to the new syntax.
* [Change] Refactored BaseForm::setMainWidget to use the new signal/slot connections to syntax
* [Change] Refactored PluginsConfigWidget signal/slot connections to the new syntax.
* [Change] Refactored GeneralConfigWidget signal/slot connections to the new syntax.
* [Change] Refactored ConnectionConfigWidget signal/slot connections to the new syntax.
* [Change] Refactored Messagebox signal/slot connections to the new syntax.
* [Change] Refactored ViewWidget signal/slot connections to the new syntax.
* [Change] Refactored TypeWidget signal/slot connections to the new syntax.
* [Change] Refactored TriggerWidget signal/slot connections to the new syntax.
* [Change] Refactored TextboxWidget signal/slot connections to the new syntax.
* [Change] Refactored TableWidget signal/slot connections to the new syntax.
* [Change] Refactored SequenceWidget signal/slot connections to the new syntax.
* [Change] Refactored RuleWidget signal/slot connections to the new syntax.
* [Change] Refactored RoleWidget signal/slot connections to the new syntax.
* [Change] Refactored RelationshipWidget signal/slot connections to the new syntax.
* [Change] Refactored ReferenceWidget signal/slot connections to the new syntax.
* [Change] Refactored ProcedureWidget signal/slot connections to the new syntax.
* [Change] Refactored PolicyWidget signal/slot connections to the new syntax.
* [Change] Refactored PgSqlTypeWidget signal/slot connections to the new syntax.
* [Change] Refactored PermissionWidget signal/slot connections to the new syntax.
* [Change] Refactored ParameterWidget signal/slot connections to the new syntax.
* [Change] Refactored OperatorClassWidget signal/slot connections to the new syntax.
* [Change] Minor refactor in CoreUtilsNs::isReservedKeyword.
* [Change] Removed Qt::WindowContextHelpButtonHint from default window flags in BaseForm constructor.
* [Change] Refactored IndexWidget signal/slot connections to the new syntax.
* [Change] Refactored GenericSqlWidget signal/slot connections to the new syntax.
* [Change] Refactored FunctionWidget signal/slot connections to the new syntax.
* [Change] Refactored EventTriggerWidget signal/slot connections to the new syntax.
* [Change] Refactored ElementWidget signal/slot connections to the new syntax.
* [Change] Refactored ElementsTableWidget signal/slot connections to the new syntax.
* [Change] Refactored DomainWidget signal/slot connections to the new syntax.
* [Change] Refactored ConstraintWidget signal/slot connections to the new syntax.
* [Change] Refactored ColumnWidget signal/slot connections to the new syntax.
* [Change] Refactored ColumnPickerWidget signal/slot connections to the new syntax.
* [Change] Refactored CollationWidget signal/slot connections to the new syntax.
* [Change] Refactored BaseObjectWidget signal/slot connections to the new syntax.
* [Change] Refactored BaseFunctionWidget signal/slot connections to the new syntax.
* [Change] Refactored AggregateWidget signal/slot connections to the new syntax.
* [Change] Refactored BaseForm signal/slot connections to the new syntax.
* [Change] Refactored SchemaView signal/slot connections to the new syntax.
* [Change] Refactored TableView signal/slot connections to the new syntax.
* [Change] Refactored TextboxView signal/slot connections to the new syntax.
* [Change] Refactored RelationshipView signal/slot connections to the new syntax.
* [Change] Refactored ObjectsScene signal/slot connections to the new syntax.
* [Change] Refactored GraphicalView signal/slot connections to the new syntax.
* [Change] Refactored BaseObjectView signal/slot connections to the new syntax.
* [Change] Refactored SourceEditorWidget signal/slot connections to the new syntax.
* [Change] Refactored SchemaEditorForm signal/slot connections to the new syntax.
* [Change] Refactored PgModelerCliApp signal/slot connections to the new syntax.
* [Change] Refactored CrashHandlerForm signal/slot connections to the new syntax.
* [Change] Removed the files basetype.h and basetype.cpp.
* [Change] Moved all the code from BaseType to TemplateType.
* [Change] Renamed getCodeDefinition, getAlterDefinition, and getDropDefintion to respectively getSourceCode, getAlterCode, and getDropCode.
* [Change] Moved the element type ids in XmlParser to XmlParser::ElementType.
* [Change] Removed unused constants in SourceCodeWidget.
* [Change] Moved config widget ids constants in ConfigurationForm to ConfigurationForm::ConfWidgetsId.
* [Change] Moved main window view ids to MainWindow::MWViewsId.
* [Change] Moved table buttons and item color constants in ObjectsTableWidget to ObjectsTableWidget::ButtonConf and ObjectsTableWidget::TableItemColor.
* [Change] Moved index attributes (buffering, concurrent, unique) constants to Index::IndexAttribs.
* [Change] Moved relationship break mode ids in ModelWidget to ModelWidget::RelBreakMode.
* [Change] Moved validation info type constants in ValidationInfo to ValidationInfo::ValType.
* [Change] Moved diff type constants in ObjectsDiffInfo to ObjectsDiffInfo::DiffType.
* [Change] Moved diff options constants in ModelsDiffHelper to ModelsDiffHelper::DiffOptions.
* [Change] Moved thread ids constants in ModelDatabaseDiffForm to ModelDatabaseDiffForm::ThreadId.
* [Change] Moved row operation ids constants in DataManipulationForm to DataManipulationForm::OperationId.
* [Change] Moved object attribute ids in DatabaseImportForm to DatabaseImportForm::ObjectAttrId
* [Change] Moved icon types and button ids constants in Messagebox to, respectively, Messagebox::IconType and Messagebox::ButtonsId.
* [Change] Moved tab id constants in RelationshipWidget to RelationshipWidget::TabId.
* [Change] Moved pending operations ids in MainWindow to MainWindow::PendingOpId.
* [Change] Minor adjustment in cached name/signature checking in DatabaseImportHelper::getObjectName.
* [Change] Minor improvement in DatabaseImportHelper::getObjectName to cache names and signatures to avoid repeatedly configuring names on each call.
* [Change] Moved the type function ids constants in Type to Type::FunctionId.
* [Change] Moved the type config constants in Type to Type::TypeConfig.
* [Change] Moved font factor constants in namespace GuiUtils to GuiUtilsNs::FontFactorId.
* [Change] Moved transition table ids in Trigger to Trigger::TransitionTableId.
* [Change] Moved transform functions constants in Transform to Transform::FunctionId.
* [Change] Moved text attributes constants in Textbox to Textbox::TextAttribs.
* [Change] Minor change in ModelWidget::saveModel to create a backup file in the same places as the original model file.
* [Change] Refactored BaseObjectView to use ColorId enum.
* [Change] Moved the Tag::ColorId to the global scope in basegraphicobject.h since the enum usage is not exclusive to tags.
* [Change] Fixed the underlying type of some enums.
* [Change] Moved role type and options constants from Role to, respectively, Role::RoleType and Role::RoleOpts.
* [Change] Moved name pattern constants in Relationship to Relationship::PatternId.
* [Change] Moved SQL type constants in Reference to Reference::SqlType.
* [Change] Moved the reference type constants in Reference to Reference::ReferType.
* [Change] Moved privileges ids constants in Permission to Permission::PrivilegeId.
* [Change] Moved operator class element type constants in OperatorClassElement to OperatorClassElement::ElementType.
* [Change] Moved constants related to argument ids and operators ids in Operator to, respectively, Operator::ArgumentId and Operator::OperatorId.
* [Change] Minor change in databaseimportform.ui, moved the tool buttons that control items selection/collapsing to the top of the section.
* [Change] Moved function ids constants in Operator to Operator::FunctionId.
* [Change] Moved chain type constants in Operation to Operation::ChainType.
* [Change] Moved operation types in Operation to Operation::OperType.
* [Change] Moved function constants in Language to Language::FunctionId.
* [Change] Moved version constants in Extension to Extension::VersionId.
* [Change] Moved the sorting constants of Element to Element::SortOrder.
* [Change] Moved DatabaseModel change log constants to DatabaseModel::LogFields.
* [Change] Moved DatabaseModel metadata attributes constants to DatabaseModel::MetaAttrOptions.
* [Change] Moved the code generation mode constants of DatabaseModel to DatabaseModel::CodeGenMode.
* [Change] Moved CopyOptions constants to CopyOptions::CopyOpts and CopyOptions::CopyMode.
* [Change] Moved SpatialType variation constants to enum SpatialType::VariationId.
* [Change] Moved UserTypeConfig type config constants to enum UserTypeConfig::TypeConf.
* [Change] Moved Constraint columns constants to enum Constraint::ColumnsId.
* [Change] Moved Constraint actions events constants to enum Constraint::ActionEvent.
* [Change] Moved Conversion constants to enum Conversion::EncondingId.
* [Change] Moved constants Collation::Lc* to Collation::LocaleId.
* [Change] Moved Cast::*Type to enum Cast::DataTypeId.
* [Change] Move cast type constants to enum Cast::CastType.
* [Change] Moved BaseTable::*Section constants to BaseTable::TableSection.
* [Change] Constants related to relationship type and label ids were moved, respectively, to enums BaseRelationship::RelationshipType and BaseRelationship::RelationshipLabel
* [Change] Moved the Aggregate::*Func constants to Aggregate::FunctionId enum.
* [Change] Moved all ResultSet::*Tuple constants to the enum ResultSet::TupleId.
* [Change] Moved the Catalog query filter constants to Catalog::QueryFilter enum.
* [Change] Created bitwise operators for enums in enumtype.h.
* [Change] Moved the constants ObjDescriptor, NameLabel, TypeLabel, and ConstrAliasLabel of TableObjectView to TableObjectView::ChildObjectId.
* [Change] Moved the constants Connection::Op* to the enum Connection::ConnOperation.
* [Change] Moved RoundRectItem corner constants to RoundedRectItem::RectCorners.
* [Change] Moved the global enum CollapseMode to BaseTable::CollapseMode.
* [Change] The static attributes ObjectsScene::Layer*Color were moved to LayerAttributes scoped enum.
* [Change] Renamed the enum_cast() to enum_t and moved to enumtype.h.
* [Change] Moved the constexpr attributes that reference toggler buttons to enum AttributesTogglerItem::TogglerButton.
* [Change] Reunited all row editing actions in DataManipulationForm in a single menu under "Edit" action.
* [Change] In SQLToolWidget when requesting the source code display of an object in the objects' tree, the source code pane will be popped up if not being shown.
* [Change] Minor improvement in CsvParser::parserBuffer to replace Windows/MacOs line breaks (\r\n and \r) by Unix only line break (\n).
* [Change] Refactored PhysicalTable::getInitialDataCommands to use CsvParser/CsvDocument.
* [Change] Minor adjustment in CsvParser to raise an exception on the malformed document due to missing close text delimiter.
* [Change] Fixes the loading of complex CSV files in TableDataWidget.
* [Change] Changed the way pgModeler imports CSV files to be used as initial table data.
* [Fix] Minor fix in MainWindow when building with DEMO_VERSION flag.
* [Fix] Fixed a bug in AppearanceConfigWidget that was randomly changing the UI theme when the user tweaks the UI style form.
* [Fix] Additional fix for false-positive diffs when functions were using an array of user-defined types.
* [Fix] Fixed a crash happening in the moment a renamed object got its source reloaded in DatabaseExplorerWidget.
* [Fix] Fixed a bug in CopyOptions that was not initializing attributes in the default constructor.
* [Fix] Minor fix in BugReportForm message box text.
* [Fix] Fixed the tool button style of plugins action in model_actions_tb in MainWindow.
* [Fix] Minor fix in FileSelectorWidget and ObjectSelectorWidget to capture mouse press instead of focus to display the respective selection dialogs.
* [Fix] Fixed a bug in StyledTextboxView that was not emitting s_objectDimensionChanged when (re)configuring it.
* [Fix] Fixed UpdateNotifierWidget style for the dark theme.
* [Fix] Fixed a bug in Catalog::getCatalogQuery that was not placing the custom filter in the right place in the query.
* [Fix] Fixed a bug that was resetting the fade-out state of objects.
* [Fix] Fixed a bug in database import that was failing to import operators in certain cases.
* [Fix] Fixed a bug when importing functions that contain parameters composed of arrays of user-defined types.
* [Fix] Fixed a bug in AppearanceConfigWidget that was not persisting changes in minimum opacity and attributes numbers per page spin boxes.
* [Fix] Minor fix in DatabaseModel::getObject(QString,ObjectType) to search for an object by its signature and name.

v1.0.0-beta
------
*Release date: September 10, 2022*

* [New] Added data dictionary entry for indexes, triggers, and sequences.
* [New] Columns, indexes, triggers, and constraints now have their own method to generate their data dictionaries.
* [New] Created the objects.sch to be reused when generating data dictionaries for columns, constraints, indexes, triggers, and sequences.
* [New] Added split SQL code generation modes to export form.
* [New] Added the ability to export dependencies or children SQL in split mode at CLI.
* [New] Added code generation options to DatabaseModel::saveSplitSQLDefinition.
* [New] Added a fix step in CLI to remove encrypted attribute from <role> tag.
* [New] Added an info message in the FindReplaceWidget reporting the search status (not found, cycle search, replacements made, etc).
* [New] Added a text search widget in SourceCodeWidget.
* [Change] Tab order adjustments in several forms and widget.
* [Change] Adjusted the stylesheet of the data dictionary.
* [Change] pgModeler CLI menu and messages fixed/improved.
* [Change] Improved the interaction between the magnifier tool and the canvas area. Now the user can select multiple objects, move, and control the zoom with the magnifier tool activated.
* [Change] Adjusted the size of the magnifier frame according to the primary screen size.
* [Change] Improved the XMlParser exceptions raised to point the file that generated the error.
* [Change] Dropped the support for PostgreSQL 9.x.
* [Change] Dropped the attribute "Encrypted" from the role form.
* [Change] Connection class now aborts the connection to PostgreSQL 9.x servers.
* [Change] Changed the shortcut key of the magnifier tool from Ctrl + Alt to F9.
* [Change] Changed the "edit item" icon on ObjectsTableWidget.
* [Change] Changed the default size of splash screen on screens with device pixel ratio = 1.
* [Fix] Fix a crash in macOS when right-clicking a blank portion of the canvas when there are objects selected.
* [Fix] Minor fix the warning icon size and position in FileSelectorWidget.
* [Fix] Fixed a bug in BaseObject::isValidName that was not considered valid a name in the format schema."object".
* [Fix] Fixed the diff generation for materialized views.
* [Fix] Fixed a bug in CLI that was not correctly fixing domains in models created in 0.8.2.
* [Fix] Fixed the appearance of the about widget in pgmodeler-se.
* [Fix] Fixed the CLI in such a way to use QRegularExpression correctly on large text buffers.
* [Fix] Minor fix in standalone connection dialog to alert the user about unsaved connection configuration.
* [Fix] Minor fix in ConnectionsConfigWidget to avoid adding connections with duplicated aliases.

v1.0.0-alpha1
------
*Release date: June 21, 2022*

* [New] Added PostgreSQL 15 as the default version for code generation.
* [New] Added a workaround in MainWindow constructor to force the application of UI themes on Windows and Mac.
* [New] Added the methods GlobalAttributes::getConfigParamFromFile and GlobalAttributes::setCustomUiScaleFactor.
* [New] Add support for custom UI scaling and UI icons size selection in AppearanceConfigWidget.
* [New] Created the method Relationship::updateGeneratedObjects.
* [New] pgModeler now saves and restores the QFileDialog instances geometry/state in the file filedlg.conf.
* [New] Created the method QApplication::loadTranslation to reuse translation loading routines.
* [Change] Minor adjustment in assets/schemas/catalog/database.sch in order to retrieve the last system oid from pg_depend.
* [Change] Removed the class CustomMenuStyle since its use is not needed anymore.
* [Change] Refactored MainWindow constructor by creating small methods for specific initialization tasks.
* [Change] Changed the references to STL classes/algorithms by using the namespace std::.
* [Change] Removed global "using namespace std" to avoid breaking the building on mingw64.
* [Change] Replaced QtCompat classes with Qt 6 specific code.
* [Change] Adjusted the layout spacing in all *.ui files.
* [Change] Updated several icon sizes due to Qt 6 improved auto-scale factor.
* [Change] Replaced deprecated attribute QPalette::Foreground in TableDataWidget.
* [Change] Minor improvement in DatabaseExplorerWidget::dropObject to show the complete location of the object being dropped (schema, table, DB).
* [Change] In DatabaseImportHelper::createSchema if the schema to be created is public or pg_catalog, pgModeler will just ignore them because these schemas already exist in the model being imported.
* [Change] When printing a model with page numbers enabled pgModeler now will add the column and row info of the current page being printed.
* [Change] Moved from deprecated QPrinter API to newer one in Qt 6.
* [Change] Adjusted the behavior of ObjectFinderWidget, now "Exact match" option is only enabled when using regular expressions.
* [Change] Adjusted the generation of CDATA elements in XML schema files.
* [Change] Adjusted the syntax highlighting conf files.
* [Change] Adjusted the XML highlighting conf files.
* [Change] Removed the support to partial-match attribute in syntax highlight conf files.
* [Change] Removed partial matching from SyntaxHighlighter.
* [Change] SyntaxHighlighter will raise an error if an invalid regex pattern is detected during file loading.
* [Change] Replaced all QRegExp usages by QRegularExpression due to the deprecation of the former class in Qt 6.
* [Change] Forcing C++17 building and the deprecation of any API before Qt 6.
* [Change] Minor improvement in Relationship::addColumnsRelGenPart to allow the creation of missing columns only as part of the routines in Relationship::updateGeneratedObjects.
* [Change] Minor change in DatabaseModel::validateRelationships to update tables and schemas geometry only at the end of the method.
* [Change] Disabling the extra relationship validation in DatabaseModel::loadModel.
* [Change] Improved the procedure that tries to recreate invalid relationships in DatabaseModel::validateRelationships.
* [Change] Refactored the way relationships are validated in DatabaseModel improving the overall operation speed.
* [Change] Changing the catalog query filter in PgModelerCliApp::importDatabase and ModelDatabaseDiffForm::importDatabase to avoid retrieving and creating system objects unnecessarily.
* [Change] Minor adjustments in pen width factor in RelationshipView::getDefaultPenWidth.
* [Change] Minor change in identifier relationship rendering.
* [Change] Removed the unneeded use of QString("") instantiation in several parts.
* [Change] The delete operation on ModelWidget now displays a wait cursor while running.
* [Fix] Fixed a bug in ConnectionsConfigWidget::openConnectionsConfiguration that was preventing reloading the original conf file when the user clicked cancel.
* [Fix] Fixed a bug in AppearanceConfigWidget that was applying the font style correctly to the source code preview.
* [Fix] Fix splitters handler sizes.
* [Fix] Fixed the QMenu configuration in several parts to use the new API of Qt 6.
* [Fix] Minor fix in LineNumbersWidget to render numbers in the correct height.
* [Fix] Fixed the resizing and positioning of the code completion widget in the first show.
* [Fix] Fixed a signal/slot connection in GeneralConfigWidget.
* [Fix] Fix a crash in ObjectsScene::mousePressEvent when building with Qt 6.
* [Fix] Fixed a typo in the error message in Tablespace::setDirectory.
* [Fix] Minor fix in Catalog::parseRuleCommands.
* [Fix] Minor fix in ModelObjectsWidget::updateObjectsList. Instead of using an empty pattern, we needed to force the wildcard * so the search returns all objects.
* [Fix] Fixed a bug in the column propagation mechanism that was failing to (re)create columns properly when the model was using lots of identifier relationships.
* [Fix] Minor fix in ElementsTableWidget to display "Default" in Sorting and Nulls columns instead of a blank text when no sorting method is defined for an element.
* [Fix] Fixed a bug in PhysicalTable::setColumnsAttribute that was ignoring exclude constraints when determining the use of the last comma in the table's SQL code.
* [Fix] Import process now correctly configures sorting options for excluding constraints and indexes.
* [Fix] Minor fix in RelationshipView to use the method getDefaultPenWidth when configuring lines and descriptors.
* [Fix] Fixed the scattered objects arrangement mode to make tables less sparse over the canvas.
* [Fix] Fixed a crash when trying to swap relationships in SwapObjectsIdsWidget.
* [Fix] Fixed blurry icons on TaskProgressWidget.

v1.0.0-alpha
------
*Release date: April 11, 2022*

* [New] pgModeler 1.0 will now copy compatible config files from 0.9.x if the older configuration dir is found on the first execution.
* [New] Added extra constants to ObjectsTableWidget to control the coloring of items in DataManipulationForm according to the theme in use.
* [New] Added an extra tab in Messagebox stack trace info called "Extra info" that displays only error's extra info to quickly help to identify any problem.
* [New] AppearanceConfigWidget now switches between light/dark themes on-the-fly.
* [New] Added custom appearance config files in conf/themes for each theme color.
* [New] Added support for customizing objects' shadow color.
* [New] Textboxes are now rounded at corners.
* [New] Created the files ui-medium.conf and ui-small.conf that controls the ui style in medium (2k) and small (1080p and below) screen sizes.
* [New] Added the class CustomMenuStyle that configures a custom size for QMenu icons.
* [New] Added an implementation for SimpleColumn::operator = to avoid compiler warnings.
* [New] Added an icon for update found notification in MainWindow.
* [New] Adding cool new icons and project logo.
* [New] CLI now restores the changelog during the fix operation.
* [New] Added the instruction $$QMAKE_LIBS_EXECINFO to apps/pgmodeler/pgmodeler.pro
* [Change] Minor adjustment in pgmodeler.pri to accept custom XML_??? and PGSQL_??? vars on Linux
* [Change] Removed the code generation of rules/indexes/triggers in the "Code preview" tab at ViewWidget since these objects' codes aren't appended to the parent view's code.
* [Change] Removed the icon of the QToolBar's extended button via CSS.
* [Change] ColorPickerWidget color button is now resized according to the tool button icon size.
* [Change] ObjectSelectorWidget was simplified to keep UI more uniform. Now it uses a QLineEdit instead of a QPlainTextEdit.
* [Change] Removed all files suffixed as _grp.png from resources and code.
* [Change] The routine that applies the UI stylesheet was moved from PgModelerApp to AppearanceConfigWidget.
* [Change] Improved the Messagebox resizing during the first show.
* [Change] Disabling Qt 5.15.x and below due to the starting of the transition to Qt 6.x
* [Change] Minor layout adjustment in pgmdoeler-se/SchemaEditorForm.
* [Change] Gridlines and delimiters lines now follow the objects' border width.
* [Change] Adjusted the colors of database objects in the dark theme.
* [Change] Minor adjustment in cursor icon when adding new graphical objects to the model.
* [Change] Messagebox will hide "Extra info" tab when the exception does not carry extra error info.
* [Change] ObjectsScene's relationship addition line now follows the page delimiter colors.
* [Change] Updated icns files in apps/pgmodeler/res.
* [Change] The configuration files directory in macOS is now named pgmodeler-1.0 to follow the same pattern as in other OSes.
* [Change] Removed the use of attributes Qt::AA_UseHighDpiPixmaps and Qt::AA_EnableHighDpiScaling.
* [Change] Floating widgets like LayersConfigWidget, AboutWidget, ChangelogWidget, and DonateWidget had CSS changed to obey QPalette changes when using themes.
* [Change] pgModeler default UI theme is now the dark one.
* [Change] Moved the constants ProtRowBgColor, ProtRowFgColor, RelAddedRowBgColor, RelAddedRowFgColor from GuiUtilsNs to ObjectsTableWidget.
* [Change] Update the signal/slot connections in AppearanceConfigWidget to the new syntax connect(obj, &Class::signal, obj1, &Class1::slot) (Experimental)
* [Change] Updated the relationship sample images in RelationshipConfigWidget.
* [Change] Updated the icons of the operations' modes in ModelExportForm.
* [Change] Minor component size adjust in GeneralConfigWidget.
* [Change] Removed unused stylesheet images.
* [Change] The configuration files renamed. Changes pgmodeler.conf -> general.conf, objects-style.conf -> appearance.conf.
* [Change] AppearanceConfigWidget now can handle settings in the file appearance.conf.
* [Change] Minor improvement in BaseConfigWidget::loadConfiguration.
* [Change] DataManipulationForm now deactivates tool buttons' texts according to the dialog height compared to the screen's height.
* [Change] Moved the appearance settings from GeneralWidget to AppearanceWidget.
* [Change] Minor adjustments in sample models.
* [Change] Improved the logic used to load extra stylesheet files in PgModelerApp.
* [Change] Changed the default source code font to 12pt.
* [Change] Adjusted the sizes of descriptors in RelationshipView, BaseTableView, and AttributesTogglerItem to reflect the new default font size.
* [Change] Changed default font size in canvas to 11pt.
* [Change] pgModeler 1.0 configuration directory was renamed from "pgmodeler" to "pgmodeler-1.0" since some config files are now incompatible with 0.9.x.
* [Change] Updated the Windows icons for the apps.
* [Change] Minor size adjustments in pgmodeler-se subproject.
* [Change] Changed the configuration dir name to pgmodeler-1.0 to avoid conflicts with 0.9.x confs.
* [Change] UI stylesheets are now selected depending on the screen size.
* [Change] Changed default icon size of combo boxes.
* [Change] Several layout margins/spacing adjustments in UI files.
* [Change] Size adjustments in ui/settings/*.ui
* [Change] Size adjustments in ui/tools/bugreportform.ui,databaseexplorerwidget.ui,metadatahandlingform.ui,modeldatabasediffform.ui
* [Change] Adjusted the size of icons in tool buttons.
* [Change] Standardization of layout spacing/margin in widgets.
* [Change] Tool buttons resize and layout spacing adjustments.
* [Change] Minor ui component resizes in several forms/widgets.
* [Change] Adjusted sizes in mainwindow.ui welcomewidget.ui.
* [Change] Adjusted ui components on sqlexecutionwidget.ui, modelobjectswidget.ui, operationlistwidget.ui.
* [Change] Adjusted the splash screen asset.
* [Change] Minor icons size adjustment in mainwindow.ui, sceneinfowidget.ui, welcomewidget.ui, modelnavigationwidget.ui.
* [Change] Minor update on toolbars title in MainWindow.
* [Change] Removed *_big* icons.
* [Change] Renamed general_tb to tools_acts_tb and controls_tb to model_acts_tb.
* [Change] Adjustment in the toolbars icon sizes in MainWindow.
* [Change] Adjustment in the stylesheets for the new UI style.
* [Change] Improved the mechanism that guesses if an FK relationship can be rendered as 1:1 or 1:N.
* [Change] Minor adjust in CLI so the "hide-ext-attribs" property in models created before 0.9.2 can be correctly replaced by "collapse-mode" during the model fix operation.
* [Change] PostgreSQL 14 is now the default version for code generation.
* [Change] Disabling the usage of the flag QGraphicsItem::ItemClipsChildrenToShape in BaseTableView to avoid wrong clipping of column items during export to PNG.
* [Change] The workaround in CLI that forces the loading of the offscreen plugin will now be triggered only in the absence of $DISPLAY env var.
* [Change] Isolated the variables common to executables subproject in the file apps/apps.pri.
* [Fix] Fixed the detection of ENTER/RETURN press to find objects in ObjectFinderWidget.
* [Fix] Fixed a bug in ModelWidget that was causing the scene to not recover from blocked signal status if an exception was thrown in ModelWidget::protectObject.
* [Fix] Minor fix in different places that QScreen instances are used. Instead of getting the screen where the active window is, we get the primary screen of the application.
* [Fix] Minor fix in CodeCompletionWidget to resize correclty according to the contents.
* [Fix] Fixed a critical bug that was preventing the importing of data types in CamelCase form.
* [Fix] Minor fix in SyntaxHighlighter to set the correct height of the parent QPlainTextEdit when using single-line mode.
* [Fix] Fixed the configuration of the shadow object in RelationshipView.
* [Fix] Fixed a bug in RelationshipWidget that was not allowing the duplication of attributes and constraints.
* [Fix] Fixed a bug in AttributesTogglerItem that was not resizing buttons when the user changed the font factor.
* [Fix] Fixed an exception in ObjectFinderWidget when searching using attributes different from the name, comment, and schema.
* [Fix] Fixed the long name of foreign-data wrappers.
* [Fix] Fixed a bug when comparing serial and integer columns on diff that was not reusing sequences or changing types properly.
* [Fix] Fixed the SVG exporting process avoiding truncating the resulting graphics.

v0.9.4
------
*Release date: December 17, 2021*

* [New] Added a fix step in CLI to reconstruct enum types XML code in new format.
* [Change] Changed the way XML code for enum types is generated and loaded.
* [Change] Minor improvement in reverse engineering to import enum types in the new format.
* [Change] Minor adjust in the demo version warning message.
* [Change] Minor adjustment in permission id generation. Now the object's signature is used instead of name.
* [Change] Minor refactor in the reconstruction of XML related to enum types in CLI.
* [Fix] Minor fix in deployment script to support new libicui versions.
* [Fix] Minor fix in order to set read/write permissions when copying configuration files with read-only permissions.
* [Fix] Minor fix in CLI to avoid the usage of '-platform offscreen' when calling the application.
* [Fix] Fixed a bug in file selectors on ModelExportForm that was not allowing to select output folders on Windows and macOs.

v0.9.4-beta1
------
*Release date: November 08, 2021*

* [New] Added extra PostGiS data types to PgSQLType.
* [New] Created the method PgSqlType::isPostGiSType() which returns true if the current type is a PostGiS one.
* [New] Created the method PgSqlType::reset() that clears some attributes of the type.
* [New] Added the built-in type pg_lsn in order to make databases using timescaledb extension to be imported correctly.
* [Change] Minor adjustment in windowsdeploy.sh to create zip packages.
* [Change] Minor adjustment in demo version warning messages.
* [Change] Adjusted the catalog query filters in ModelDatabaseDiffForm to retrieve system and extension objects according to the checkboxes "Import system objects" and "Import extension objects".
* [Change] Adjusted the reverse engineering in such a way that the table children will follow the SQL disabled state of their parent tables.
* [Change] Allowing importing pg_lsn attributes in usertype.sch.
* [Change] Allowing the use of commas in enum type labels.
* [Change] Improved the output of model fix operation in CLI.
* [Change] The constants DataSeparator, UnescValueStart, UnescValueEnd, WildcardChar, FilterSeparator, FilterWildcard, FilterRegExp were moved from CoreUtilsNs to UtilsNs in order to be used in the parsers module.
* [Change] In SchemaParser the meta char $ds (dollar sign) was renamed to $ms (money sign). Also, a new meta char $ds (data separator) was added and translates to the special data separator character UtilsNs::DataSeparator.
* [Change] Minor adjustments in table.sch and foreigntable.sch schema files.
* [Change] Improved DatabaseModel::getUserDefTypesReferences in order to get all references to postgis data types.
* [Change] Improved ModelValidationHelper in order to set postgis extension a default comment when automatically creating it.
* [Change] PgSQLTypeWidget now can be configured in such a way to disallow the configuration of type qualifiers like length, precision, interval, and some other. This is useful when configuring data types for objects that don't require such attributes in the data type like parameter, aggregates, transforms, casts, operators.
* [Change] Changed the behavior of Parameter::getCodeDefinition when generating SQL code. Now, any type qualifier (except dimension descriptor []) will be discarded.
* [Fix] Fixed the catalog query that lists policies.
* [Fix] Added a minor workaround in DatabaseImportHelper::getType in order to treat the "any" pseudo-type correctly.
* [Fix] Minor fix in DatabaseImportForm in order to disable option buttons when no connection is selected.
* [Fix] Fixed a segfault when importing domains with long expressions.
* [Fix] Fixed the broken SQL generation for tables with columns/constraints disabled.
* [Fix] Fix the name of the checkbox related to updates checking to avoid breaking the building when enabling NO_UPDATE_CHECK via qmake.
* [Fix] Fixed the shortcuts of Copy/Paste actions in DataManipulationForm.

v0.9.4-beta
------
*Release date: September 20, 2021*

* [New] Added the shortcut Alt+C to trigger the cancel action in DatabaseImportForm ModelDatabaseDiffForm and ModelExportForm.
* [New] Added support for the generation of GRANT/REVOKE commands to control roles memberships via the diff process.
* [New] Added a fix step to reconfigure roles membership considering the deprecation of Role:RefRole attribute.
* [New] Added support for included columns on indexes.
* [New] Added the ability to reference view columns on indexes.
* [New] Added support to use view's SimpleColumn in ColumnPickerWidget as well as Index.
* [New] Added .dtd extension to SchemaEditorForm::showFileDialog.
* [New] Created the widget ColumnPickerWidget by isolating the code that handles constraint columns in ConstraintWidget for reusing in other portions of the tool.
* [New] Added the method Constraint::addColumns().
* [New] Two buttons were added aside the date input fields in partial diff tab (ModelDatabaseDiffForm) in order to allow the quick assignment of the first and last modification dates.
* [New] Added support for toggling update notifications for alpha/beta versions.
* [New] Added support for save and restore the tree state in DatabaseExplorerWidget.
* [Change] Minor improvement in ModelsExportHelper by adding more breaking points related to export canceled.
* [Change] Minor improvement in ObjectRenameWidget in such a way to fill the new name field with the object's name in the first show when a single object is selected for renaming.
* [Change] Dropped the support for MemberOf (Role::RefRoles) in order to make the diff between two roles more precise.
* [Change] In RoleWidget the tab "Member Of" now works only as a convenience feature that adds the role being edited to the ones listed in that tab.
* [Change] Restored the file pgmodeler.appdata.xml.
* [Change] Refactored BaseObject::isCodeDiffersFrom (and its variants in children classes) to use QStringList instead of vector<QString> as parameters.
* [Change] Allowing swap cluster level object ids in SwapObjectsIdsWidget when the objects are of the same kind.
* [Change] Minor refactoring in RoleWidget, Role and DatabaseModel to use Role::RefRole, Role::MemberRole and Rele::AdminRole constants as loop indexes.
* [Change] Minor adjustment in demo features.
* [Change] Minor refactoring in Role::setRoleAttribute.
* [Change] Minor improvement in ModelExportHelper::exportToDbms to send the SQL command via a signal when creating roles and the database.
* [Change] Improvements in pgmodeler-se in such a way to control syntax highlighting settings per open editor.
* [Change] Avoided the use of a working dir in .desktop file when installing mime types system-wide.
* [Change] Improved the way layers rectangles are updated after undoing/redoing operations in OperationListWidget.
* [Change] Isolated SimpleColumn struct on its own file for reusing in other modules.
* [Change] Moved the constants ProtRowBgColor, ProtRowFgColor, RelAddedRowBgColor, RelAddedRowFgColor from BaseObjectWidget to GuiUtilsNs.
* [Change] Changed the default match type in foreign key constraint to MATCH SIMPLE.
* [Change] Minor adjustment in the ModelWidget::updateSceneLayers to force the update of all schemas.
* [Change] Avoided a lambda function call when checking updates to prevent segfaults.
* [Change] Exceptions raised in BugReportForm when the "lastmodel" file isn't found are now ignored.
* [Change] Minor change in ModelValidationHelper to use table's signature instead of name during the name validation process.
* [Change] Avoided calling the functions xmlInitParser() and xmlCleanupParser() more than once per app execution in XmlParser class to avoid unexpected behavior.
* [Change] Minor adjustment in viewport drawing mode in order to redraw objects by bounding rect changes.
* [Change] Minor improvements in the changelog widget in order to display the first modification date.
* [Change] Refactored Connection::requestCancel in order to avoid using PQrequestCancel which is deprecated.
* [Fix] Fixed a glitch when drawing StyledTextboxView instaces.
* [Fix] Fix the diff between columns using PostGiS datatypes.
* [Fix] The attributes Qt::AA_UseHighDpiPixmaps and Qt::AA_EnableHighDpiScaling are now correctly set before the instantiation of Application based classes.
* [Fix] Fixed a crash in diff process caused by the ModelExportHelper instance when canceling the export to DBMS when an exception was raised.
* [Fix] Fixed a crash in SwapObjectsIdsWidget when the user tried to swap objects using arrow keys.
* [Fix] Fixed the tab names in RoleWidget.
* [Fix] Minor fix in DatabaseExplorerWidget to display view's children source code.
* [Fix] Fixed the building of IndexWidget on Qt 5.14 and below.
* [Fix] Fixed the problem on ModelDatabaseDiffForm that was not accepting pressing Return/Esc to respectively run the diff and close the dialog due to a conflict with the event loop instance in ModelDatabaseDiffForm::exec().
* [Fix] Minor fix in the configuration of ObjectGroupId element in DatabaseImportForm::updateObjectsTree in order to avoid crashes when no root element is specified.
* [Fix] Fixed the generation of aggregate functions when they use functions that contain parameters in which data type have spaces (double precision, character varying).
* [Fix] Fixed a segfault that can happen when changing the number of parameters of a function already associated with an aggregate.
* [Fix] Additional fix for layers not being correctly loaded from dbm file.

v0.9.4-alpha1
------
*Release date: July 19, 2021*

* [New] The bulk editing form now can be confirmed by the Ctrl+Return shortcut.
* [New] The CLI now is capable of fixing models created before 0.9.4-alpha1 that have columns, functions, and other objects that reference extension types with simple names instead of schema-qualified names.
* [New] Added an option to GeneralConfigWidget to toggle the displaying of schema names of columns data types on design view. This option affects only the rendering of objects, the code generation will keep displaying user-defined type names in a schema-qualified way.
* [New] Created the DeletableItemDelegate which draws a delete button aside combobox items.
* [New] Added the ability to drop databases from the database listing in SQLToolWidget.
* [New] Added support to strikeout attribute in SyntaxHighlighter.
* [New] Created an auxiliary tool called pgModeler Schema Editor in order to help to edit *.sch files as well as validate their syntax.
* [New] GeneralConfigWidget now restores the sch-highlight.conf to default.
* [New] Added the environment variable name for schema editor app in GlobalAttributes.
* [New] Added a specific icon for CLI on Windows.
* [New] Added the methods SchemaParser::getCurrentColumn and SchemaParser::getCurrentLine.
* [New] Added signal s_hideRequested to FindReplaceWidget to notify connected slots that the close button on that widget was clicked.
* [New] Added an attribute capture-nearby to the word-separators tag that is read by SchemaParser to indicate if word separators must be capture altogether in the same word if they appear contiguously.
* [New] Added options to create only missing config files or recreate all config file in CLI.
* [New] Added support for saving SQL definition in split files.
* [New] Added support for split SQL generation in CLI.
* [New] Added the ability to cancel the SQL file export process in GUI.
* [New] Created the methods UtilsNs::saveFile and UtilsNs::loadFile in order to replace repetitive save/load code by them.
* [New] Created the function PgSqlVersions::parseString that checks the forced PostgreSQL version is valid, returning a fallback version when needed or raising an error if the version is malformed or is unsupported.
* [New] Added new metacharacter to SchemaParser: $hs for #, $ps for %, $at for @, $ds for $.
* [New] Added the option --force to the mime type handling operation on CLI.
* [Change] Changed the behavior of column click on DataManipulationForm. Now, the sorting is triggered by holding control and clicking the desired column. Clicking a column without holding control will select all the items in that column.
* [Change] Changed the behavior of extension types registration, now they will be forcibly registered with complete name (schema.name) in order to follow the same rule of user-defined data types so the reverse engineering can work properly for both types handled by extensions and those not.
* [Change] Improved the type searching on PgSqlTypeWidget in such a way to display a popup with the data types matching the text typed by the user.
* [Change] Improved TableObjectView to be rendered with/without schema names appended to user-defined data type names.
* [Change] Minor adjustment on icon size of QTreeWidget instances.
* [Change] Improved the NewObjectOverlayWidget in such a way to allow only one action to be executed at a time.
* [Change] Several clean-ups in the source code root. Removed unused files.
* [Change] The subproject libpgmodeler was renamed to libcore and now moved to libs/
* [Change] The subproject libpgmodeler_ui was renamed to libgui and now moved to libs/
* [Change] The subproject libobjrenderer was renamed to libcanvas and now moved to libs/
* [Change] The main executable subproject was renamed to pgmodeler and now moved to apps/
* [Change] The main-cli executable subproject was renamed to pgmodeler-cli and moved to apps/
* [Change] The crashhandler subproject was renamed to pgmodeler-ch and moved to apps/
* [Change] Created a subfolder called "assets" to store all deployed files that aren't libraries or executables: conf, lang, schemas, samples, etc.
* [Change] Refactored all .pro/.pri files in order to reference pgModeler libraries from variables $$LIB[LIBNAME] instead of relative paths.
* [Change] Renamed all icons that were in Portuguese to English.
* [Change] In SnippetsConfigWidget replaced the SQL highlight by Schema micro-language highlighting.
* [Change] Renamed BlockInfo class to TextBlockInfo and moved to a dedicated source file.
* [Change] Changed the workflows to build any *fix, *change, *support branches.
* [Change] Minor improvement on QMainWindow to resize general toolbar buttons.
* [Change] Widgets that were emitting the signal s_visibilityChanged(bool) only on hide event no emit s_hideRequested instead. The s_visibilityChanged(bool) is now reserved to widgets that need to notify visibility changes passing the current state to slots (hidden/displayed) in for of a bool parameter.
* [Change] Minor hint text typos fixes.
* [Change] Improved the syntax error messages in SchemaParser.
* [Change] Minor adjustment in the minimum width of the general toolbar when resizing the buttons.
* [Change] BaseForm::setMainWidget(QWidget *) version will use the same icon as the widget being inserted.
* [Change] Adjusting the general toolbar width according to the screen DPI.
* [Change] Renamed the namespace PgModelerUiNs to GuiUtilsNs.
* [Change] Renamed the namespace PgModelerNs to CoreUtilsNs
* [Change] Source files and ui files in libgui were reorganized by contexts (widgets, tools, settings, utils, dbobjects)
* [Change] Applied an automatic indentation to all schema files.
* [Change] pgModeler will now create missing config files in the startup.
* [Change] Replaced duplicated code that saves files and raises exception by UtilsNs::saveFile
* [Change] The class Schema now has its own id interval (4000 - 4999) in order to make the split code generation be done properly.
* [Change] Minor adjustment in AppearanceConfigWidget to enable scrollbars in the viewport so users on small screens can see the entire model.
* [Fix] Fixed the crash handler and bug report form in such a way to load the last modified model opened before their execution.
* [Fix] Fixed the column positions in ObjectDepsRefsWidget.
* [Fix] Fixed a crash when the database import process fails to create an inheritance relationship somehow.
* [Fix] Fixed a bug in the database import process that not correctly setting up the data type name of columns when the types were handled by extensions. Types handled by extension don't need to have the schema name prepended.
* [Fix] Minor fix in database import process in such a way to properly update the table rendering to hide schema names from title box.
* [Fix] Fixed a malformed SQL code when configuring timestamptz in PgSqlType.
* [Fix] Fixed the reference to libutils in deploy scripts.
* [Fix] Fixed doxygen references for libraries and executables.
* [Fix] Fixed the path to globalattributes.cpp in deploy scripts.
* [Fix] Added missing file windows_ico.qrc.
* [Fix] Fixed a bug in SyntaxHighlighter that was wrongly highlighting the entire document when a multiline group had a line break as the final expression.
* [Fix] Fixed the syntax highligting files (xml, sql).
* [Fix] Minor fix in CLI in order to associate .sch files to the scheditor.
* [Fix] Fixed a bug that was causing all layers to be active even if there was some inactive (invisible) when adding a new layer.
* [Fix] Fixed the method PgModelerCliApp::handleWindowsMimeDatabase to properly insert sch file association in Windows registry.
* [Fix] Fixed the "iCCP: CRC error" in PNGS at libs/libgui/res/icons
* [Fix] Minor fix in SyntaxHighlighter in order to return unformatted word when the group can't be forcibly defined in all exception cases when the word doesn't match any group.
* [Fix] Fixed the description of the option --output in CLI.
* [Fix] Fixed a bug in CLI that could lead to segfault when using diff option and a database model as input.
* [Fix] Fix broken build on Qt 5.11.x.
* [Fix] Minor fix in PgSQLTypeWidget to capture the text changing signal on data type combo so the type format input can be properly updated when the user types the desire data type.

v0.9.4-alpha
------
*Release date: May 12, 2021*

* [New] Added compatibility code that allows loading models created in versions before 0.9.4-alpha without losing layers and active layers information.
* [New] Added an option to the metadata handling operation that serves to indicate if duplicated objects (textboxes, tags, generic sql) must be merged or not.
* [New] Added support for configuration parameters and transform types on functions and procedures.
* [New] Added the compatibility method QtCompat::horizontalAdvance(QFont, QString).
* [New] Added the ability to rename the default layer.
* [New] DatabaseModel now stores in XML code all the new settings related to layers (rect and name colors, rect and names display status).
* [New] Added several methods in ObjectsScene to handle layer colors that helps to save and restore colors from database model files.
* [New] Add support for custom layer colors in LayerConfigWidget.
* [New] Added the class LayerItem to create custom items for displaying layout rects in canvas.
* [New] Added an option to toggle layers rectangles and names from LayerConfigWidget.
* [New] Added support for setup, in general settings, the custom color for grid lines, canvas area and page delimiter lines.
* [New] Added support for multiple layers on the model.
* [New] Added support for PARALLEL attribute to functions.
* [Change] Minor improvement on "Forced filtering" option on ObjectsFilterWidget in order to use a list widget instead of single actions for each object type.
* [Change] Adjusted the default Qt version to 5.15.2 in the deployment scripts.
* [Change] Minor adjustments on Messagebox resizing during first display.
* [Change] Minor improvement on Messagebox in order to display the stack trace toggle button in a more intuitive way.
* [Change] Changed the behaviour of the method ObjectsScene::removeLayers, now it'll allow the resetting or not the layers of the objects.
* [Change] Minor adjustment on QTableWidget vertical header size on DataManipulationForm, ObjectsTableWidget and SqlExecultionWidget.
* [Change] Minor size adjustment policy in the comboboxes in DataManipulationForm, ModelNavigationWidget and ModelValidationWidget.
* [Change] Minor improvement in DatabaseImportHelper::setBaseFunctionAttribs in such a way to discover the name of the functions transform types from their oids.
* [Change] Isolated the duplicated code in DatabaseModel::createFunction and DatabaseModel::createProcedure in the method DatabaseModel::setBasicFunctionAttributes.
* [Change] Adjusted the minimum size of the main window to 640x480 so it can fit in smaller screens.
* [Change] Minor refactoring on SchemaView::configureObject to avoid unecessary calculations when the layers are not visible.
* [Change] Added forced updates of schema rectangles after setting layers to children objects.
* [Change] Change on the search path for PluginsDir to use getPathFromEnv().
* [Change] LayersConfigWidget will now emit a signal when toggling layer rects/names so the overview widget on mainwindow is properly updated.
* [Change] DatabaseModel will now create a default layer in the constructor.
* [Change] The database model will add the default layer to the loaded model if there isn't a single layer identified.
* [Change] DatabaseModel when loading older database model files will generate random layer colors.
* [Change] The scene now will update layer rects when catching the signal s_objectDimensionChanged from tables and textboxes.
* [Change] Adjust the ObjectsScene to resize layer rects when one or more layers are removed.
* [Change] Improved the window title of DataManipulationForm in sucha a way to display the currently browsed table.
* [Change] Improved the selection of layers to be associated to object in context menu (Quick > Set layers). Now a floating widget is use to select multiple layers for the object selection in the canvas area.
* [Change] Updated the sample models to use "layers" attributes on graphical objects.
* [Change] Changed the value separator for attributes layers and active-layers on <dbmodel> from semi-colon to comma.
* [Change] pgModeler CLI will now rename "layer" attributes to "layers" in order to reflect the multiple layers support for older models.
* [Change] Minor copyright updates.
* [Change] The option "Truncate before alter columns" in the diff process was removed since its use is discouraged and dangerous in some situations.
* [Fix] Minor fix in the table's catalog query in order to avoid setting up a partitioning strategy when the table is not a partitioned one.
* [Fix] Minor fix in database import in such a way to update scene layers info before adding it to the main window.
* [Fix] Fixed the layers configuration of example.dbm.
* [Fix] Minor fix in function.sch in order to generate transform types only for PostgreSQL >= 9.5.
* [Fix] Fixed DatabaseModel::getUserDefTypesReferences in such a way to detect that procedures and functions are referencing a user-defined type from their list of transform types.
* [Fix] Fixed ObjectsTableWidget in such a way to control whether the ResizeColsButton can be displayed or not via ObjectsTableWidget::setButtonConfiguration.
* [Fix] Minor fix to avoid black canvas when using settings from 0.9.3 or below on 0.9.4-alpha.
* [Fix] Minor fix in BaseObjectView in such way to avoid the adjustment of its position to the scene grid when the object is not selected. This can avoid undesired adjustment when moving a schema box and having only a portion of its children selected.
* [Fix] Minor fix in ObjectsScene in such a way to update layer rects when the grid alignment option is toggled.
* [Fix] Fixed the buttons shortcuts on LayerConfigWidget.
* [Fix] Minor fix on SchemaView to compute the last position any time the object is reconfigured.
* [Fix] Minor fix in BaseObjectWidget to avoid mark the object as modified in finishConfiguration() before adding it to the parent.
* [Fix] Fixed the key combination to trigger the magnifier tool.
* [Fix] Fixed LayerItem in order to build in Qt 5.9.
* [Fix] Minor fixes in ModelWidget, MainWindow to update scene layer settings correctly.
* [Fix] Fixed the CLI in such a way to update scene layer settings so the exporting processes can generate graphical files correctly.
* [Fix] Minor fix in LayersConfigWidget in such a way to mark the correct active layers when setting up the model to operate on.
* [Fix] Fixed a bug in the layers removal process that was causing objects to be placed in a incorrect layer index after the removal.
* [Fix] Fixed the "Fix" menu in main tool bar.
* [Fix] Fixed a bug in ModelWidget that was causing overlay to be displayed when a textbox object was selected.
* [Fix] Fixed wrong text in collationwidget.ui.
* [Fix] Fixed a bug during the loading of changelog entries.
* [Fix] Fixed the display of warnings during export in ModelExportForm.
* [Fix] Fixed the display of warning messages during export in ModelDatabaseDiffForm.
* [Fix] Fixed a malformed SQL code for triggers.
* [Fix] Fixed typos in README.md.
* [Fix] Fixed the broken build on Qt 5.9.x.
* [Fix] Fixed a bug that was preventing special PKs to be properly configured.
* [Fix] Fixed the SQL generation of functions based on internal language.

v0.9.3
------
*Release date: December 30, 2020*

* [New] Added support for deterministic and provider attributes in collations.
* [New] Added missing check constraints in the data dictionary.
* [New] Added support for selecting all relationships of a table at once via right-click on a table > select relationships.
* [New] Added extra search fields in ObjectFinderWidget. Now it's possible to search relationships by the involved tables (source and destination table), constraints by their columns (source and referenced columns), and relationships by the foreign keys related to them.
* [Change] The warning message about the unchecked option related to drop missing objects now pops up only when there are partial diff filters configured.
* [Change] Improved the extended fade in/out options for tables/views/foreign tables.
* [Change] The partial diff filters generated from changelog will ignore table children objects in order to avoid the wrong generation of diff code.
* [Change] Minor refactoring in DatabaseModel::addChangelogEntry in order to use new attributes to define changelog operations.
* [Fix] Fixed a crash when trying to select children objects of a schema that has no rectangle defined.
* [Fix] Fixed a crash during the importing of domain objects.
* [Fix] Fixed a crash during the importing of objects into the current model in very specific cases when the model had some tables/views referencing columns added by relationship.
* [Fix] Fixed the full diff switching when there are no filtered objects in ModelDatabaseDiffForm.
* [Fix] Fixed a bug in DatabaseModel::addChangelogEntry that was causing the registration of empty signature for some objects.
* [Fix] Fixed a bug in CLI that was not accepting the use of --list-conns parameter
* [Fix] Fixed the data dictionary generation in such a way to include nextval(sequence) calls in the "default value" column.
* [Fix] Fixed a bug when configuring spatial data types during reverse engineering/diff.
* [Fix] Fixed a bug in Catalog::parseDefaultValues that was causing ARRAY[] values to be wrongly split.
* [Fix] Fixed a bug that was causing infinite validation of imported sequences.

v0.9.3-beta1
------
*Release date: October 5, 2020*

* [New] Added the version descriptor for PostgreSQL 13.
* [New] Added support for procedures in design, import and diff processes.
* [New] Added support for transforms in design, import and diff processes.
* [New] Added an entry in NewObjectOverlayWidget for procedures.
* [New] Added a custom version of addParameter to Procedure in order to validate the usage of out parameters.
* [New] Added a tool button for transform objects in NewObjectOverlayWidget
* [New] Added a unit test to verify schema files syntax (sql, xml, alter).
* [New] Added the method Cast::setName to override the default behavior of BaseObject::setName.
* [New] Added the class PgModelerUnitTest that must be inherited so the child test class can have access to schema files path.
* [New] Added support for modifying attributes toggler colors from appearance settings.
* [New] Tag objects now include attribute toggler colors.
* [Change] Changed the behavior of the generation of SQL code for database object, now it'll respect the SQL disabled status of the object.
* [Change] The ModelExportHelper will abort the export process if the SQL code of the database object is disabled.
* [Change] The database model is now flagged as modified everytime the objects are swapped.
* [Change] Improved the ObjectSelectorWidget in order to save/restore the geometry of internal ModelObjectsWidget instances.
* [Change] Ajusted the Qt version check in QFontMetricsCompat and QPlainTextEditCompat in order to avoid deprecation warnings.
* [Change] pgModeler will alert about a possible data/work loss if the user is trying to save a model in which there're other instances loaded other tabs.
* [Change] Refactored FunctionWidget in such way to make it a subclass of BaseFunctionWidget.
* [Change] Removing unused method DatabaseModel::removeObject(unsigned,ObjectType).
* [Change] Moved the DTD defintion of parameter tag to a dedicated file to be shared betwen function.dtd and procedure.dtd.
* [Change] Making the class Function be a direct child of BaseFunction.
* [Change] Moved the common code between functions and procedures to a base class called BaseFunction.
* [Change] Replaced the attributes PhysicalTable::DataSeparator and DatabaseExplorerWidget::ElemSeparator usages by PgModelerNs::DataSeparator.
* [Change] Minor improvement on data dictionary in order to add the current year in the footer.
* [Change] ModelExportHelper now is capable of indetifying a transform and procedure objects being exported.
* [Change] Minor fields sizes adjustment in pgsqltypewidget.ui
* [Change] Improved the schema files syntax tests to include all folder under / schemas.
* [Change] Refactored the schema files in order to remove code duplication related to ddl-end token.
* [Change] Minor improvement in ConfigurationForm by adding a splitter between config items (left) and settings page (right).
* [Change] Minor improvements on objects rendering in order to consider screen dpi when configuring objects border sizes.
* [Change] Minor refactoring in the parameter/signature generation in class Function.
* [Change] Making the NewObjectOverlay less transparent in order to enhance reading.
* [Fix] Fixed a bug in constraint.sch that was avoiding the correct importing of exclude of constraints.
* [Fix] Fixed source file name for PgModelerCliApp.
* [Fix] Minor fix in the SyntaxHighlighter in order to highlight correctly multline blocks (specially comments).
* [Fix] Fixed a bug in DatabaseImportHelper that was causing failure when importing some objects' permissions.
* [Fix] Minor fixes on the CLI menu.
* [Fix] Fixed a bug on MainWindow that was wrongly showing the main menu bar in certain cases at startup.
* [Fix] Minor fix in BaseFunction::createSignature in order to remove OUT keywords from signature.
* [Fix] Fixed a bug when importing triggers in which functions arguments contain json/jsonb values. Now values are properly formatted.
* [Fix] Fixed a bug in XmlParser::convertCharsToXMLEntities that was not converting json/jsonb default values correctly breaking the entire XML code of the database model.
* [Fix] Fixed a bug in Parameter class that was causing default values to be ignored.
* [Fix] Fixed a bug in SchemaParser related to exceptions being raised wrongly in expressions evaluation.
* [Fix] Fixed the objects style template config files.
* [Fix] Removed some deprecation warnings when building on Qt 5.15.1.
* [Fix] Fixed a bug in ModelFixForm that was passing arguments to CLI in wrong format.
* [Fix] Minor fix in some project files to remove unnused include paths.
* [Fix] Fixed a bug in CLI that was ignoring input-db parameter when doing diff.
* [Fix] Minor fix in the graphical objects rendering in 4k screens when QT_AUTO_SCREEN_SCALE_FACTOR is set to. Now they are rendered in acceptable proportions.
* [Fix] Minor fix in ModelsDiffHelper in order to avoid generating ALTER...ADD COSTRAINT related to constraints (check and unique) in which parent table is also being created.


v0.9.3-beta
------
*Release date: July 10, 2020*

* [New] Added GitHub actions scripts in order to test the building on each platform. On Linux and macOs the build will occur on all Qt versions from 5.15 to 5.9, on Windows the build happens only on Qt 5.15 (due to the usage of msys2 that uses the most recent version of the framework).
* [New] Added default constructors in the form "Class(const &)" for Connection, Parameter and TypeAttribute in order to avoid deprecated-copy warnings on GCC9.
* [New] The CLI now validates the mixing of options of different operation modes.
* [New] Added support for partial diffs between a model and database or between two databases.
* [New] Created an internal changelog on DatabaseModel to register object's modification over time being useful for partial diff operations.
* [New] Added the widget called ChangelogWidget to control the model's internal changelog settings in the design view.
* [New] Added support for save/load changelog from model file.
* [New] The CLI now supports partial diffs too.
* [New] Create the method ModelsDiffHelper::getRelationshipFilters.
* [New] Added a warning message prior to the partial diff without using "Do not drop missing objects" option.
* [New] Created a method ObjectsFilterWidget::setModelFilteringMode that disables some options when the filtering widget is being used to filter database model objects.
* [New] Added cast and user mapping to the list of filterable objects.
* [New] Added a hint text in ObjectsFilterWidget to inform about exact match searching.
* [New] Added the methods Catalog::clearObjectFilter and Catalog::clearObjectFilters.
* [New] Added support for DatabaseImportHelper to create a sequence assigned to a column via default value if automatic dependencies resolution is enabled.
* [New] Created the namespace QtCompat that will reunite all the specific code that isn't compatible between different Qt versions. The goal of this namespace is to store in a central point all the code that uses QT_VERSION_CHECK to perform different operations depending on the current Qt version.
* [New] Added the method SchemaParser::getExpressionResult in order to perform comparisons between QVariant values correctly due to the QVariant operators deprecation in Qt 5.15.
* [New] Added the signal ObjectsFilterWidget::s_filtersRemoved that is emitted whenever all filters are removed.
* [Change] Modified the script wingetdate.bat in order to return the build number based on the current local date correctly and renamed it to getbuildnum.bat.
* [Change] Updated to 5.15 the Qt version used by default in linuxdeploy.sh and macdeploy.sh.
* [Change] Replacing verbose QString() calls by "" for string initialization purposes.
* [Change] Improved the sequences assignments to columns in DatabaseImportHelper::assignSequencesToColumns.
* [Change] Improved the method ModelsDiffHelper::setFilteredObjects in order to treat tables/view/foreign tables properly by appending their children objects to the creation order list of filtered objects.
* [Change] Adjusted the Catalog::setObjectFilters in order to configure exact match filters when any wildcard filter provided has no *.
* [Change] Adjusted the partial diff process between a model and a database in which there's one or more many-to-many relationships filtered.
* [Change] Minor adjustment on objects grids in SwapObjectsIdsWidget, DatabaseImportForm and ModelDatabaseDiffForm by changing the columns order.
* [Change] Improved the method ModelsDiffHelper::setFilteredObjects in order to detect the proper creation order of the provided objects.
* [Change] Minor change in ObjectsFilterWidget by making the action "Only matching" checked by default.
* [Change] Minor improvement on CLI to accept the value "all" for the paramenter --force-children in order to force all table children at once.
* [Change] BaseObject::configureSearchAttributes now configures the signature attribute as a formatted string.
* [Change] Minor improvement on ObjectFinderWidget::updateObjectTable in order to optionally create checkable items in the first column.
* [Change] Attributes related to object filters were moved from Catalog to PgModelerNs in order to be shared between Catalog and DatabaseModel classes.
* [Change] Minor improvement in Connection::getConnectionId in order to return an optional HTML formatted string.
* [Change] Isolated the method DatabaseImportForm::listFilteredObjects in a static one in order to reuse it on ModelDatabaseDiffForm.
* [Change] Improved the objects filtering in such way to allow filter by name or signature.
* [Change] Improved the UI of ObjectsFilterWidget by moving all options to a popup menu.
* [Change] Several changes in all catalog queries in order to support signature matching.
* [Change] Minor adjustemnt in Catalog::getFilterableObjectNames.
* [Change] Adjusted the appimages building process.
* [Change] Changing the default font size of graphical objects to 10pt in order to try to solve the intermittend issue of disappearing texts.
* [Change] Updated the French translation.
* [Fix] Fixed a regression in ObjectFinderWidget that was not opening objects form with double-click on an item on the results grid.
* [Fix] Fixed a bug when drawing relationships in FK to PK connection mode. Now when one of the tables is collapsed the center points of both are used as connection points to the relationship.
* [Fix] Fixed a crash in ModelDatabaseDiffForm while enabling/disabling the partial diff tab.
* [Fix] Fixed the diff process for inheritance relationships created for two existing tables.
* [Fix] Fixed a crash in LayersWidget when renaming a layer.
* [Fix] Minor fix in DatabaseImportHelper::assignSequencesToColumns.
* [Fix] Fixed a wrong comparison in Catalog::getObjectsOIDs.
* [Fix] Minor fix in ConstraintWidget that was trying to validate FK relationships for generated tables of many-to-many relationships.
* [Fix] Fixed a bug in Catalog::getObjectsOIDs that was executing a catalog query for a certain object when it was not being filtered.
* [Fix] Minor fix in ModelDatabaseDiffForm tabs enabling/disabling steps.
* [Fix] Fixed the QProcess usage in ModelFixForm due to QProcess::start() deprecation in Qt 5.15.
* [Fix] Fixed a crash when trying to move several objects to a layer from the object finder widget and in the selected set one or more objects aren't graphical ones.

v0.9.3-alpha1
------
*Release date: May 20, 2020*

* [New] Added support for creating initial configuration files from CLI.
* [New] The CLI is now capable of using the objects filtering in reverse engineering.
* [New] Added a warning message when trying to import a big database.
* [New] Added support for object filtering in reverse engineering dialog.
* [New] Adding support for AppImage building for Linux.
* [New] Added support for importing PostgreSQL 12 generated columns.
* [New] Adding support for PostgreSQL 12 generated columns.
* [New] The ModelNavigationWidget now will display a save icon if the current model is pending a save operation.
* [New] pgModeler now guesses if a FK relationship consists in a one-to-one based upon the foreign keys and unique keys settings.
* [New] Improved the rendering of FK relationships. Now it recognizes the minimum cardinality by using the right crow's foot descriptor o|< or ||< in the destination entity.
* [New] Added support for the conversion of 1-1 and 1-n relationships, allowing the user to perform modifications over objects generated by those relationships.
* [New] Added "virtual" keyword in classes that inherit others so the destruction order can be followed correctly.
* [New] Adding support for copy model validation widget output as text as well preview it as texts.
* [New] Added support for show/hide all layers in LayersWidget.
* [New] Added the paramenter application_name to Connection in order to indentify connections created by pgModeler when querying pg_stat_activity.
* [New] Added support for save/restore Z value attribute in MetadataHandlingForm.
* [New] Created an improved and reusable file selector widget.
* [Change] Improvements done in DatabaseImportHelper in order to speed up the listing of types as well the importing user-defined types on demand.
* [Change] Refactored the way default value fields states are setup by user's actions.
* [Change] Adjusted the default size of DatabaseImportForm.
* [Change] Avoiding retrieve all types (system and user defined) at once before the reverse engineering in DatabaseImportHelper (Experimental).
* [Change] Minor improvement on DatabaseImportHelper in order to display the correct message while assingning sequences to columns.
* [Change] Improved the catalog queries in order to retrive parent oids and type when using the list queries.
* [Change] Minor updates on windeploy.sh and macdeploy.sh
* [Change] Minor improvement on DatabaseImportHelper in order to speed up the objects creation.
* [Change] Improved the execution time for the catalog query of data types.
* [Change] Improved the catalog query for sequences.
* [Change] Avoiding the diff process to generate default value changes for generated columns.
* [Change] Minor improvement on DatabaseModel::validateRelationships.
* [Change] Improved the ModelWidget::convertRelationship1N() in order to avoid segfaults in certain cases.
* [Change] Minor adjustment in PhysicalTable::restoreRelObjectsIndexes in order to diminish memory usage.
* [Change] Minor improvement on update icon.
* [Change] Minor improvement in LayersWidget in order to allow manual resizing using the mouse.
* [Change] Improved the following classess in order to use the new file selector widget: ModelExportForm, ModelFixForm, MetadataHandlingForm, ModelDatabaseDiffForm, GeneralConfigWidget, PluginsConfigWidget, CsvLoadWidget, CrashHandlerForm and BugReportForm.
* [Fix] Minor fix in OperationList in order to update FK relationships related to FKs that contains the column being handled by the list.
* [Fix] Fixed a bug when retrieving the refs to some objects in DatabaseModel.
* [Fix] Minor fix in CLI menu and error messages.
* [Fix] Fixed the role importing process. Now referenced roles are auto resolved.
* [Fix] Fixed the dependency checking for user defined types in DatabaseModel.
* [Fix] Fixed a bug when loading extension that has no schema specified.
* [Fix] Minor fix in DatabaseImportHelper::createFunction in order to solve eventual duplicated parameter names when importing some functions.
* [Fix] Fixed a bug that was causing global name patterns for relationships to never be used even if they were properly configured.
* [Fix] Minor fix in ModelObjectsWidget and ModelWidget to provide the correct information about objects' selection in SceneInfoWidget.
* [Fix] Fixed a bug in RelationshipWidget that was causing the cursor never to be restored to its original icon when an exception occurred during relationship editing.
* [Fix] Fixed a crash in some situations when closing models.

v0.9.3-alpha
------
*Release date: March 26, 2020*

* [New] Added support for multiselection in ModelObjectsWidget.
* [New] Added an optional parameter in PgModelerNs::generateUniqueName to avoid comparing the input object.
* [New] Added support for objects' bulk renaming.
* [New] Added support for multiselection in ObjectFinderWidget.
* [New] Added support for middle button click on source code panel in order to paste the selected code in the current open SQL execution widget.
* [New] Using the CRTP (Curiosly Recurring Template Pattern) approach to create a template type based on BaseType in such way that the static member from that template but used on all derived classes of TemplateType can be unique for each derived class.
* [New] Added a "Stacking" action in popup menu that is used to control the z value of graphical objects.
* [New] Adding support for move objects on the Z stack up and down.
* [New] Added support for CLI to write system wide mime database on Linux and Windows.
* [New] Added a method GlobalAttributes::setSearchPath so the application can setup the path in which the internal folders (schemas, lang, etc) can be found.
* [New] Added the ability to remove owners from objects from Quick Actions > Change owner.
* [New] Added an fix step that removes IN keyword from functions signatures.
* [Change] Refactored the method DatabaseModel::getObjectDependecies by splliting it into small portions that handles specific object types dependecies.
* [Change] The DatabaseModel::getObjectReferences was splitted in other get[object]References() in order to facilitate maintainance.
* [Change] Minor adjustments in order to make the objects listing a bit faster for large databases.
* [Change] Minor improvement on ObjectRenameWidget in order to apply renaming in reverse id order of the object so objects invalidations (due to relationships revalidations) diminishes.
* [Change] In DatabaseModel the invalidated special objects are now stored in a list which is destroyed only when the model is destroyed too. This will avoid segfaults in some cases when the objects in that list are still being referenced in the operation list (Experimental).
* [Change] Minor adjustment in ModelObjectsWidget and ObjectFinderWidget in order to emit the signal ModelWidget::s_sceneInteracted so the left menu at MainWindow can be updated accordingly.
* [Change] Minor change in PgModelerNs::generateUniqueName in order to remove quotes from names before perform the comparison.
* [Change] Refactored ObjectRenameWidget in order to support multiple objects renaming.
* [Change] Refectored all the classes derivated from BaseType moving them to dedicated sources files in order to make them reuse TemplateType class isolating the related keywords, improving maintainance.
* [Change] Removed the class LanguageType since it was replaced by namespace DefaultLanguages namespace.
* [Change] Isolated the default language names into a namespace called DefaultLanguages and removed any reference to LanguageType in the code.
* [Change] Improved the DataManipulationForm in such way that the user can sort results by clicking the column names in the result grid. This will cause a new query to be performed by using the clicked column as sorting criteria.
* [Change] The current model's popup menu is now used as the "More" actions menu at MainWindow > general toolbar.
* [Change] Refactored the ModelWidget::configurePopupMenu separating some portions in new methods to make maintainance more easy.
* [Change] Moved the 'Fix model' and 'Objects metadata' actions to the general toolbar (left) at main window under the menu "Fix".
* [Change] The class HintTextWidget was removed and replaced by simple html QToolTips.
* [Change] Small change in the icon related to new tab opening in SQLToolWidget.
* [Change] Minor adjustment or rows resizing policy on datamanipulationform.ui.
* [Change] Minor adjustment on updatenotifierwidget.ui to allow link opening.
* [Change] Refined the installer wizard pages.
* [Change] Improved the installer script to work better on Windows.
* [Change] Improving the installer on Linux and Windows to select the installation for all users or current user.
* [Change] Improved the linuxdeploy.sh in order to remove the need to use the startup scripts by the binaries in the installer.
* [Change] Fixed the installscript.qs in order to correctly update the mime database on Linux.
* [Change] Minor improvements on the usability of swap objects ids form.
* [Change] Separated the code that handles mime db on linux and windows, using a  #ifdef switch to call the correct method depending on the platform.
* [Change] Adjusting the linker options to make the executables search for libraries in PRIVATELIBDIR without forcing the usage of LD_LIBRARY_PATH.
* [Change] The class Application was moved to libutils and has a basic implementation of just initialize the search path on GlobalAttributes.
* [Change] Rename the classes Application and PgModelerCli to respectively PgModelerApp and PgModelerCliApp and they now inherits the new Application class in order to take advantage of the GlobalAttributes::setSearchPath.
* [Change] The crash handler application is now executed under a Application instance not a QApplication anymore in order to use GlobalAttributes::setSearchPath.
* [Change] Changed the namespace GlobalAttributes to a singleton in order try to solve problems with fallback paths.
* [Change] Updated deployment scripts to use Qt 5.14.x
* [Change] Changing the exposed main window from QMainWindow to MainWindow in PgModelerPlugin interface.
* [Change] Moved the method SchemaParser::convertCharsToXMLEntities to XmlParser.
* [Change] Minor buttons position adjustments on WelcomeWidget.
* [Change] Minor refactoring on OperationListWidget in order to show a more compact text when displaying operations done over objects.
* [Change] Removed the workaround code that disables the ALTER SET OWNER  when the role associated to it is disabled and the object itself not. The best approach is to dissociate the role from the object so the ALTER instruction is not generated.
* [Fix] Fixed a shortcut duplication in DatabaseExplorerWidget
* [Fix] Fixed the shortcut for partitioning relationships in new object overlay
* [Fix] Fixed a minor bug in MainWindow that was causing the bottom widgets bar to be displayed even when object finder widget and model validation widget weren't visible.
* [Fix] Fixed a bug that was causing copy options to stay hidden for copy relationships in the relationship form.
* [Fix] Fixed the importing of extensions on PostgreSQL 9.4 and below.
* [Fix] Fixed the name uniqueness generation when the name is truncated when exceeding 63 bytes.
* [Fix] Fixed a bug that was preventing the recent menus cleaning to be persisted in the conf file.
* [Fix] Fix a bug in the validation of collation objects.
* [Fix] Fixed a crash in ObjectRenameWidget when renaming several objects in which there was the need to revalidate relationships;
* [Fix] Fixed the method PgSqlType::getTypes in order to avoid returning the first (null) type.
* [Fix] Fix the importing/diff of columns based on PostGiS data types.
* [Fix] Fix a crash in UpdateNotifierWidget when compiling pgModeler using Qt 5.14.x.
* [Fix] Minor fix in PgSQLTypeWidget to avoid show an error message box when the user types an invalid data type name.
* [Fix] Additional fix on XmlParser::convertCharsToXMLEntities in order to replace special char to xml entities correctly.
* [Fix] Minor fix in the update notifier widget display position.
* [Fix] Minor fix in diff regarding to the comparison on column/domain default values. Now the values are compared in insensitive case.
* [Fix] Fixed the Windows installer in order to create the start menu item  correctly (system wide).
* [Fix] Minor fix on CLI in order to allocate export/import/diff helpers and config widgets only when they are required (when one of the options related are used).
* [Fix] Fixed a bug that was allowing the applying changes of the forms on relationship added objects by hitting ENTER causing crashes.
* [Fix] Fixed the bug that was causing model restoration to be displayed when the application was started from a double click on a model file.
* [Fix] Fixed a bug in the metadata handling that was not restoring the collapse mode of tables.
* [Fix] Fixed a crash on UpdateNotifierWidget after upgrading to 5.14. A wrong deletion was being performed where  deleteLater() needed to be used instead
* [Fix] Removing lots of Qt 5.14 deprecation warnings in the code.
* [Fix] Fix a bug that was causing wrong replacements during the reverse engineering of functions.
* [Fix] Fixed the method XmlParser::convertCharsToXMLEntities in order to avoid chars replacements within <![CDATA entities.
* [Fix] Fixed the oversized tooltips on Windows.
* [Fix] Minor warning message fix on pgmodeler.pro.
* [Fix] Fixed a bug in AppearanceConfigWidget and objects-style.sch that was causing problems while saving the objects styles.

v0.9.2
------
*Release date: December 26, 2019*

* [New] Add support for data dictionaries generation in HTML format in ModelExportForm.
* [New] Added options to control data dictionary generation in CLI.
* [New] Plugins now can optionally be associated to a menu action or not. Generally, an plugin not associated to a menu action is automatilly executed during the startup (see PgModelerPlugin::initPlugin).
* [New] Added a missing model fix step on CLI that removes the IN keyword from functions signatures.
* [Change] Making BaseRelationship::getReferenceForeignKey() public.
* [Change] Isolated duplicated code in MainWindow::isToolButtonsChecked that checks if any tool button of the bottom or right widget bars is checked.
* [Change] Removing the plugins from core code.
* [Change] Modified pgmodeler.pro to include plugins folder when present in the source root (either in debug or release mode).
* [Change] Ignoring plugins folder in the core code.
* [Change] Minor improvement on ModelWidget::rearrangeSchemasInGrid in order to consider the amount of tables to determine the minimum grid size used to rearrange table.
* [Change] Minor text adjustments in CLI.
* [Change] Added an additional checking during relationship creation in order to avoid the creation of 1-* or n-n relationships involving partition tables.
* [Change] Removing the restriction to create 1-1, 1-n and n-n relationships in which the involved tables are partitioned ones.
* [Change] Disabling the SQL statment ALTER...OWNER TO in the object's SQL when the owner role has its SQL disabled but the object itself not. This will avoid reference errors when validating/exporting code.
* [Fix] Fixed the "Save" action enabled state according to the current model's modified state.
* [Fix] Fixed a crash when the user tried to edit connections in ModelDatabaseDiffForm and right after select a connection in the "Compare to" field.
* [Fix] Fixed the generation of escaped comments for columns.

v0.9.2-beta2
------
*Release date: November 1st, 2019*

* [New] Added support for foreign tables.
* [New] Added the method DatabaseModel::getObject(QString,vector<ObjectType>) to return the first ocurrency in the lists related to the provided types.
* [New] Added support for select all foreign tables in ModelWidget.
* [New] Created the method BaseTable::isBaseTable to help determining if an object type is of Table, View or ForeignTable.
* [New] Added extra support for foreign tables on DatabaseExplorerWidget and DataManipulationForm.
* [New] Added the method DatabaseImportHelper::createColumns in order to isolate the code to create table/foreign table columns during reverse engineering.
* [New] Added support for foreign table importing as well its children objects (columns, constraints, triggers).
* [New] Added server and options fields on TableWidget for foreign table.
* [New] TableWidget now fully supporting the edition of foreign tables.
* [New] Added the ability to TableView to render ForeignTable objects.
* [New] Added support for swap objects ids using arrow keys by changing creation order between two close objects on the grid.
* [New] Added support for save diff settings in form of presets.
* [New] Added an option to BaseObject to force the comment escaping making multilined comments be presented as single lines but without changing their semantics.
* [New] Adding support for parse escaped text delimiters and value separators in CSVLoadWidget.
* [Change] Avoiding the usage of "Generate ALTER commands" option when the table is a partition or partitioned.
* [Change] Avoid unchecking PK checkboxes on TableWidget when adding a new column.
* [Change] Validating the dialogs geometry regarding to screens geometry in order to avoid their appearance in an invalid position (out of range).
* [Change] Limiting the size of the exceptions stack in 50 elements.
* [Change] Improved the GenericSQL::isObjectReferenced in order to check if, when a referenced object is a column, the provided object is the same as the parent of that referenced object.
* [Change] Restricting the usage of "Default partition" option for partitioning relationships when the partition table is a foreign table.
* [Change] Making ModelObjectsWidget update the foreign table subtree.
* [Change] Making ReferenceWidget to use foreign tables from referenced table selector.
* [Change] Improved the Relationship class to accept foreign tables.
* [Change] Improved the DatabaseModel::getObjectDependecies and DatabaseModel::getObjectReferences to include ForeignTable.
* [Change] Updated View class to accept reference foreign table columns.
* [Change] Moved the original implementation of DatabaseModel::createTable to a template method createPhysicalTable in order to reuse it to create tables and foreign tables.
* [Change] ForeignObject is not a BaseObject child anymore due to multi inheritance problem that it can generate in ForeignTable class.
* [Change] Minor ajustment in Relationship class to accept PhysicalTable in its constructor so relationships can be used by ForeignTable class as well.
* [Change] Isolated the code common to Table and ForeignTable classes in a parent class named PhysicalTable.
* [Change] Disabling mime type update on installers.
* [Change] Minor adjustment on SQLExecutionWidget::generateBuffer to escape line breaks and separators when generating CSV buffer.
* [Change] Removing unused code from NumberedTextEditor.
* [Change] Changed the way temp source file is handled by the tool in order to solve a sharing violation problem on Windows.
* [Change] Minor adjustment on HtmlItemDelegate to avoid creating local variables in paint() everytime the method is called.
* [Fix] Fixed an bug in SQL generation of columns that was wrongly removing a comma from decimal data types.
* [Fix] Fixed some catalog queries in order to support PostgreSQL 12.
* [Fix] Fixed ObjectsScene to remove foreign tables during destruction.
* [Fix] Fixed the View::getObjectCount in order to return 0 when the provided object type is invalid.
* [Fix] Fixed the ObjectFinderWidget in order to fade out correctly the listed/not listed elements.
* [Fix] Fixed the quick actions menu at ModelWidget in order to include the "Edit data" action for foreing tables.
* [Fix] Fixed the DatabaseImportHelper::assignSequencesToColumns in order to correctly assign sequences to foreign table columns.
* [Fix] Fixed a crash in ModelValidationHelper that was caused by wrong checking when validating generic sql objects.
* [Fix] Fixed the source code display for tables and foreign tables on DatabaseExplorerWidget.
* [Fix] Fixed the CodeCompletionWidget to display foreign table children.
* [Fix] Fixed the loading of generic SQL objects when a referenced object was a column.
* [Fix] Additional fixes to correctly reference foreign tables and tables in the portions were both classes are acceptable/desirable.
* [Fix] Fixed the diff process in order recognize partition foreign tables attach/detach.
* [Fix] Fixed the generation of XML code of partitioning relationships.
* [Fix] Fixed the Relationship::getAlterRelationshipDefinition to include foreign tables.
* [Fix] Fixed the preset.sch since it was not registering the "Reuse sequences" options correctly.
* [Fix] Improved the DataManipulationForm in order to restore the columns visibility when retrieving again the data of the current table.
* [Fix] Fixed the drop action in DatabaseExplorerWidget for user mappings.
* [Fix] Fixed the ModelWidget::rearrangeTablesInGrid to adjust position of foreign tables.
* [Fix] Fixed the generation of SQL code of partition tables in order to avoid the inclusion of partitioned tables columns on the code.
* [Fix] Fixed a crash on DatabaseImportHelper when destroying detached inherited columns.
* [Fix] Fixed the object duplication operation in model widget to accept duplicate foreign table children objects.
* [Fix] Fixed the creation of relationships involving foreign tables.
* [Fix] Fixed the permission object in order to support foreign tables.
* [Fix] Fixed the model export helper to support foreign tables.
* [Fix] Fixed ModelWidget to show missing popup actions for foreign tables.
* [Fix] Fixed the NewObjectOverlayWidget to display tool buttons related to foreign table children.
* [Fix] Fixed the SchemaView::fetchChildren to include foreign tables in the returned list.
* [Fix] Fixed the ModelValidationHelper to validate foreign tables and table-view relationships.
* [Fix] Fixed ColumnWidget and ConstraintWidget to accept foreign tables usage.
* [Fix] Fixed a regression when importing permissions related to functions.
* [Fix] Fixed the OperationList to handle foreign tables.
* [Fix] Fixed create methods in DatabaseModel related to table children object in order to accept foreign tables.
* [Fix] Fixed the SwapIdsWidget postion/size saving and restoration.
* [Fix] Minor tooltip fix in GeneralConfigWidget.
* [Fix] Minor fix in default conf files removing unused tag attribute.
* [Fix] Additional fix to the extension import to correctly indetify it as a data type handler.
* [Fix] Fixed a bug on Role's SQL generation due to a missing attribute.
* [Fix] Additional fix in GeneralConfigWidget to save correctly the settings of dockwidgets.
* [Fix] Minor fix in DatabaseModel to write ddl end token in the appended/prepended custom code.
* [Fix] Fixed a false-positive result when doing the diff between two fuctions that contains the same signature.
* [Fix] Fixed the diff generation for Role objects when setting up an blank password.
* [Fix] Fixed the "Prepend at the beggining of the model" option behavior on CustomSQLWidget.
* [Fix] Fixed the catalog query for extension in order to indetify correctly if this object handles a data type.
* [Fix] Fixed the Variadic option enabling on ParameterWidget.
* [Fix] Fixed the saving of validator widget settings. There was a conflict between pgsql-ver attributes present on the validator settings and the default attribute added by the schema parser. The solution was to use a different attribute (version) in the validator.
* [Fix] Fixed the "Clear items" action in popup menu at DataManipulationForm.
* [Fix] Fixed the importing of tables and views in such way to automatically create correctly the domains referenced by their columns.
* [Fix] Fixed a bug on ModelDatabaseDiffForm that was causing the form to be reseted on the middle of a diff process when the user minimize the diff dialog causing unexpected behavior (or even crashes sometimes).
* [Fix] Minor fix in AboutWidget.
* [Fix] Minor fix in ModelDatabaseDiffForm to avoid using uniform items height in output widget.

v0.9.2-beta1
------
*Release date: September 2, 2019*

* [New] Added a routine to write a backup file during the saving of model files in order to avoid data loss in some rare cases.
* [New] Added a new action on DataManipulationForm to clear the selected items by pressing Ctrl+R.
* [New] Enabling/disabling the save model action according to the model's modified status.
* [New] The main window instance is now exposed to plugins to make the extra features development more flexible.
* [New] Avoiding selecting table items in BaseTableView::mousePressEvent which doens't contain an underlying (source) object.
* [New] Adding support for duplicate several table objects at once.
* [New] Added support for table children multiselection by using Ctrl+Shift+Click on them.
* [New] Added support for load the generated diff code in the SQL tool and automatically browsing the destination server for manual diff applying.
* [New] Added a button to create a new data manipulation form from within another data manipulation form.
* [New] Added a confirmation message before closing a database explorer widget.
* [Change] Minor adjustments in PgModelerUiNs::createOutputListItem and PgModelerUiNs::createOutputTreeItem to better display formatted messages.
* [Change] Minor improvement on CSV text copying to exclude from the buffer the hidden columns.
* [Change] Adjusted the font weight on WelcomeWidget (only for Linux).
* [Change] Minor adjustment on tool buttons font size on DataManipulationForm.
* [Change] Improved the UI of the DataManipulationForm by rearranging the tool buttons to the left of the dialog.
* [Change] Improved the clipboard text pasting on DataManipulationForm in order to parse the text as CSV when Ctrl+Shift+V is pressed.
* [Change] The filter input on SQLExecutionWidget is now automatically focused when toggling the filter widget.
* [Change] Changed the default behavior of result set filtering to Qt::MatchContains on SQLExecutionWidget.
* [Change] Avoiding showing the "Swap ids" action in popup menu when we have only columns/constraints selected
* [Change] Minor adjustment on SceneInfoWidget to avoid display dimensions of selected columns/constraints on the canvas.
* [Change] Renamed the method BaseGraphicObject::getReceiverObject to getOverlyingObject (to make sense with its counter part BaseObjctView::getUnderlyingObject)
* [Change] Avoiding clear the whole scene selection when the table is already selected and the user requested popup menu.
* [Change] Renamed BaseObjectView::getSourceObject to getUnderlyingObject.
* [Change] Improved the object protection action in ModelWidget in order to accept the multi item selection on tables.
* [Change] Changed the way the PSVI attribute is handled in xmlparser to retrieve big line numbers on a xml document.
* [Change] Improved the diff code preview allowing the user to close the dialog without going back to previous tab and click 'Cancel'.
* [Change] Minor adjustment on buttons shortcuts/tooltips on DataManipulationForm.
* [Change] Changed the way the model saving timers are controlled to avoid infinity savings on the model (when the auto saving option is disabled) making the application unusable.
* [Change] Making the mime update operation return exit code 0 even if there was errors (still displayed in the console). This will avoid the installer to abort installation when the dbm mime update fails.
* [Change] Avoiding applying fillfactor to gin indexes.
* [Change] Moved the method DatabaseImportHelper::parseIndexExpressions to Catalog class.
* [Fix] Fixed a broken diff code generation for policies.
* [Fix] Fixed the object duplication action when only a single table object is selected.
* [Fix] Fixed a bug in ModelWidget::removeObjects that was causing the removal of relationship added columns
* [Fix] Fixed the ModelWidget::configurePopupMenu to consider the table's multi item selection.
* [Fix] Fixed a false-positive diff result when dealing with timestamptz.
* [Fix] Fixed the progress calculation of dbm files loading.
* [Fix] Fixed a bug that was causing the generation of broken view code.
* [Fix] Fixed the display of index expressions on DatabaseExplorerWidget.
* [Fix] Minor fix on SQLExecutionWidget to avoid results grid to overlap command execution history widget in certain cases.

v0.9.2-beta
------
*Release date: May 31, 2019*

* [New] Added support for user mapping.
* [New] Added support for foreign server.
* [New] Added support for foreign data wrapper.
* [New] Added support for reduced verbosity on diff, export and import processes in order to improve performance.
* [New] Adding missing tootip on ObjectFinderWidget.
* [New] Generic SQL objects now support dynamic references to objects which can be used in the definition code.
* [New] Added support for compare foreign servers on diff process.
* [New] Created a generic getAlterDefinition on ForeignObject.
* [New] Added ForeignServer toolbutton in NewObjectOverlayWidget.
* [New] Added support for the reverse engineering user mapping objects.
* [New] Added support for the reverse engineering foreign server objects.
* [New] Added code snippets for foreign data wrapper and foreign server.
* [New] Added support for diff user mapping.
* [New] Added support for diff foreign data wrappers.
* [New] Added support for set permissions to foreign data wrapper.
* [New] Added the WRAPPER, SERVER and MAPPING key words to sql-highlight.conf.
* [New] Added the method PgSqlType::isExactTo in order to do a full comparison (all attributes) between two data types.
* [New] Added the ability to view references to store referenced tables. This feature will cause relationships to be created between the view and the referenced tables. This is useful when we're using reverse engineering feature in which, in previous versions, couldn't determine the tables that were linked to a view. Now, with this feature a relationship is created between the view and all involved tables.
* [New] Added missing data type macaddr8.
* [New] Enabling quick clear button on several input fields.
* [New] Added support for result set filtering in the SQL execution widget.
* [New] Adding a column labeled "Comment" in TableWidget and ViewWidget to hold comments of children objects.
* [Change] Changed the shortcut of run SQL action in SQLExecutionWidget to F5.
* [Change] Changed the shortcut of tree update action in DatabaseExplorerWidget to F6.
* [Change] Change "New object" action in popup menu in order categorize object types when clicking the database object diminishing the amount of items displayed on the screen.
* [Change] Improved the object search mechanism in such way that various attributes of the object can be matched. New searchable attribute may be added in the future.
* [Change] Added missing code documentation.
* [Change] Minor adjustment on ForeignDataWrapper::getAlterDefinition.
* [Change] Minor improvement on ModelDatabaseDiffForm to show the connection id of the databases being imported in the output tree.
* [Change] Formatting server objects' attributes on DatabaseExplorerWidget.
* [Change] Minor adjustments on the icons of the buttons in ObjectsTableWidget.
* [Change] Improved the method DatabaseModel::getObjectReferences to detected foreign data wrappers as functions' references.
* [Change] Minor code refactoring on Table and View classes.
* [Change] Renamed the method Exception::getErrorType to Exception::getErrorCode.
* [Change] Improved the ModelValidationWidget in such way that is possible to operate over objects on the output list through their respective context menu (the same as in the ModelWidget).
* [Change] Now its possible to trigger the swap ids dialog for two selected objects, causing their ids to be swapped more quickly.
* [Change] Minor refactor on schema files.
* [Change] Minor attributes refactoring on several classes.
* [Change] Minor change in the PgSqlType constructor by turning some parameters optional in order to facilitate the creation of array only types.
* [Change] Minor update on disclaimer text at start of the source files.
* [Change] Allowing copied object to be pasted multiple times. This feature works only with copy/paste operation without remove the pasted objects from the clipboard, for cut/paste the behaviour is unchanged.
* [Change] Increased the maximum limit of SQL history.
* [Change] Updated the windeploy.sh and the installer scripts.
* [Change] Adjusted the installer scripts.
* [Change] Changed the windows deploy script to use Qt Installer Framework.
* [Change] Adjusted the deploy script to use Qt 5.12.
* [Change] Fixed the windows deploy script to use newer version of the compiler in 64 bits environment.
* [Change] Minor improvements in SQLToolWidget and SQLExecutionWidget to avoid segmentation faults when trying to close a execution tab while the command is still running.
* [Change] Adjusted the resize parameters in DataManipulationForm to avoid wrong dialog resizings mainly on Windows.
* [Fix] Fixed a bug in DataManipulationForm that was deleting new rows wrongly.
* [Fix] Fixed a bug that was causing domain constraints not to be extracted correctly during reverse engineering.
* [Fix] Fixed a bug that was causing a fk relationship not to be deleted if the fk tied to it was changed by the user.
* [Fix] Fixed a bug on CLI that was not fixing broken models correctly when they had no role declaration.
* [Fix] Fixed a bug that was causing tables not to be moved on the canvas using mouse.
* [Fix] Fixed a crash related to destruction of special objects on DatabaseModel::destroyObjects.
* [Fix] Fixed a bug that could crash the application when no language was specified to a funcion and the SQL/XML code was being generated.
* [Fix] Minor fix a bug on index importing.
* [Fix] Minor fix on View::isReferencingTable.
* [Fix] Fixed a crash when a query executed in SQLExecutionWidget was a DDL one or was not returning results.
* [Fix] Fixed a bug in CLI that was failing to fix model in certain cases.
* [Fix] Minor fix on buttons tooltips.
* [Fix] Fixed a bug that was causing syntax error if the last column of a table had the SQL code disabled.
* [Fix] Fixed a bug on diff process due to a missing attribute on the generation of diff code for inheritance relationships.
* [Fix] Fixed a bug when rendering several self relationships attached to the same table.
* [Fix] Fixed the CLI in order to restore the layers information when fixing a broken model.
* [Fix] Fixed a bug in object finder that was causing objects from a hidden layer to be displayed causing inconsistency on the layer state.


v0.9.2-alpha1
------
*Release date: December 17, 2018*

* [New] Added support for scene layers.
* [New] Added support for view's columns importing in DatabaseImportHelper.
* [New] Added the ability to load view columns from database model file in DatabaseModel::createView.
* [New] Added a tab "Columns" in ReferenceWidget where the user will be able to insert columns to be used as view columns.
* [New] Added support for pagination of tables and views columns pagination.
* [New] Added a pagination toggler action on context menu at ModelWidget.
* [New] Added a fix step on CLI to remove the deprecated attribute hide-ext-attribs from tables and views xml code.
* [New] Added a configuration option to control attributes per pages in tables and views.
* [New] Added support for save collapsing states and current attributes pages to the database model file.
* [New] Added constants to reference child objects of TableObjectView.
* [New] Added the class TextPolygonItem which can be used to draw a text over a background polygon.
* [New] Added support for OLD/NEW tables aliases on triggers.
* [New] Added a hint text on RelationshipWidget to document the correct usage of default partitions.
* [New] Added support for partition attaching/detaching detection in diff process.
* [New] Added auxiliary methods in Table class in order to add/remove and retrieve partition tables.
* [New] Added support for importing partitioned/partition tables on DatabaseImportHelper.
* [New] Added a missing validation in Relationship to avoid creating other types of relationships involving partitioned or partition tables.
* [New] Added support for specify partition bounding expression on partitioning relationships.
* [New] Added support for resize grid cells to fit contents on ObjectsTableWidget.
* [New] Added a tab "Partition keys" that will handle partitioning configuration on TableWidget.
* [New] Added a method in ObjectsTableWidget to hide some horizontal header sections.
* [New] Added some validations when creating partitioning relationships.
* [New] Added support for hide columns on data manipulation dialog.
* [New] Added a transient attribute to objects DatabaseModel, Table and View in order to give a hint on the maximum count of objects held. This attribute is used to preallocate the vectors which store the children objects in order to avoid excessive memory allocation/deallocation due to vector resizing.
* [New] Added a column labeled "Alias" on all objects tables in TableWidget so the aliases of children can be displayed.
* [New] Added support for adding tabs via shortcut or corner button in the SQL Execution panel.
* [Change] Minor adjustments on MainWindow to make the overview widget to update its contents whenever the active layers change on the current model.
* [Change] Minor adjusment in ObjectsScene::addItem to make the item (in)visible according to the visibility of its related layer.
* [Change] Minor fix in AttributesTogglerItem in order to consider the parent's opacity during painting.
* [Change] Minor fixes in OperationList in order to force views to be updated correctly when operating over a table which is referenced by those objects.
* [Change] Minor adjustments on SchemaView and BaseTableView (and its children classes) to update the geometry when they switch from invisble to visible state.
* [Change] Changed views in such way so they can use the struct SimpleColumn to represent their deduced columns.
* [Change] Improved the update of views when referenced columns and tables change their structure.
* [Change] Improved database model loading times by avoiding the rendering of tables while the children objects (indexes, trigger, rules, etc) are being added.
* [Change] Removed the several operators ~ overloading that statically cast enums to their underlying type and created a template function called enum_cast in C++14 syntax.
* [Change] The zoom in/out level is now sensible on how much the user rolls the mouse wheel.
* [Change] Move the default implementation of configureObjectShadow and configureObjectSelection from BaseObjectView to BaseTableView.
* [Change] Disabling configureObjectSelection and configureObjectShadow on TableObjectView and RelationshipView.
* [Change] Minor adjustment on protected icon position on TableTitleView and TextboxView.
* [Change] Minor performance adjustments in ModelWidget.
* [Change] Minor improvement in TextboxView to use only a TextPolygonItem to hold text and the object's rectangle instead of a box and a text items.
* [Change] Replaced the sql_info_txt and sql_info_box items by a single instance of TextPolygonItem to denote SQL disabled status.
* [Change] Replace the tag_body and tag_name elements on BaseTableView by the tag_item which is a instance of TextPolygonItem.
* [Change] Improved the TableObjectView to avoid adding extra scene items.
* [Change] Improved the TableTitleView to avoid adding children items. A custom paint() method now draws them.
* [Change] Removing unused fr_FR UI translations.
* [Change] Minor update on known issues sections at README.md.
* [Change] Renamed the namespace ParsersAttributes to Attributes and its attributes has been refactored.
* [Change] Refactored all static const attributes of the classes present in pgsqltypes.h.
* [Change] Renamed PgModelerNS to PgModelerNs.
* [Change] Renamed PgModelerUiNs to PgModelerUiNs.
* [Change] Renamed XMLParser to XmlParser.
* [Change] Removing uneeded temporary QString instance created from Exception::getErroMessage call before throwing exceptions.
* [Change] Refactored static const attributes of BaseObject.
* [Change] Refactored the items in the enum ObjectType by removing the prefix 'OBJ'.
* [Change] The enums ErrorType and ObjectType were transformed into scoped enums. Also the ErrorType enum was renamed to ErrorCode.
* [Change] Code refactoring done in order to make it more close to C++14 in order to take advantage of new features introduced by that standard.
* [Change] Removed unused labels and fixed warning frame on ModelWidget.
* [Change] Minor improvements on table's attributes displaying on DatabaseExplorerWidget.
* [Change] Improved the diff process in such way to avoid generating unnecessary/noise commands related to changing types of columns to integer and setting nextval() call as default value.
* [Change] Partition tables are now displayed in the "Tables" tab at TableWidget.
* [Change] Removed the cached catalog query test feature from Catalog.
* [Change] Fine tuning on the validation of the entities used in the partitioning relationship creation.
* [Change] Forcing the partitioning relationship to be invalidated when the reference table (partitioned) partitioning type is set to null (no partitioning).
* [Change] Move the FK settings, copy options and name patterns group boxes to a dedicated tab on RelationshipWidget.
* [Change] Improved the models destruction when closing the application.
* [Change] Improved the Index/Exclude/ParitionKey elements handling by creating a generic form/grid that handles these kinds of objects (ElementsTableWidget).
* [Change] Modified the RelationshipWidget in order to handle partitioning relationships.
* [Change] Modified RelationshipConfigWidget in order to write name partterns for partitioning relationships.
* [Change] Improved the column copying and validation on class Relationship to include partitioning relationship logics.
* [Change] Improved the reverse engineering performance by avoiding update relationships as they are being imported.
* [Change] Improved the object duplication feature in ModelWidget.
* [Change] When the model is loaded it is copied to the temporary models storage as a first version of the temporary dbm file.
* [Change] Simplified the temporary models saving process by removing the thread that was controlling it. Actually the thread was unnacessary because the process was being executed in the main thread no matter if there was another thread to control the saving.
* [Change] Minor adjustment on the hint text resizing.
* [Change] Increasing to 5 minutes the period in which the temporary model saving will be executed.
* [Change] pgModeler will now use the official docs url in the help buttons.
* [Fix] Fixed a minor bug that was preventing the copy action to be enabled in DataManipulationForm.
* [Fix] Fixed some sample models to remove deprecated attributes.
* [Fix] Fixed a crash while renaming view's children objects.
* [Fix] Fixed the rendering of views which contain only a single reference that is the whole object's defintion.
* [Fix] Fixed the column name deduction for recursive views.
* [Fix] Fixed a bug that was causing crashes while configure new constraints on tables.
* [Fix] Fixed the view's resizing.
* [Fix] Fixed a regression in schema's rectangle selection.
* [Fix] Fixed the StyledTextboxView bounding rectangle.
* [Fix] Fixed an artifact when user switched on and off the compact view.
* [Fix] Fixed the Linux deploy script.
* [Fix] Fixed the macOs deploy script.
* [Fix] Fixed some compilation problems on macOs due to the usage of C++14.
* [Fix] Fixed some compilation problems on Windows due to the usage of C++14.
* [Fix] Fixed a bug in DatabaseModel::destroyObjects that could lead to segfault when the destroyed model had permissions on it.
* [Fix] Fixed a bug when importing columns which data types is some user defined type in form of array, e.g., custom_type[].
* [Fix] Fixed a bug in SchemaParser that was causing only the first %set line in a if block to be parsed no matter that there were others %set below the same block.
* [Fix] Fixed the tooltip of some graphical objects by adding their comments and aliases.
* [Fix] Fixed the catalog query for tables to select partitioned tables correctly.
* [Fix] Fixed the catalog query for types to avoid selecting partitioned tables as being data types.
* [Fix] Fixed a bug that was causing special primary key configured on a relationship to make the original primary key of the table to disappear after disconnect the relationship. Now pgModeler stores in memory the original PK prior the connection of relationship and creation of the special PK. When disconnected the relationship, the original primary key is restored on its parent table.
* [Fix] Fixed the creation of elements (index, exclude, patition key) on DatabaseModel.
* [Fix] Fixed the class Relationship to reuse compatible columns when handling partitioning relationships.
* [Fix] Fixed the diff process in such way to create new columns with their respective COMMENT ON statement when present.
* [Fix] Fixed the detection of comment changes for columns on diff process.
* [Fix] Fixed the order of recent models saved on the file pgmodeler.conf.
* [Fix] Fixed a bug when creating a view reference as the whole view definition.
* [Fix] Minor tooltip fix on DatabaseExplorerWidget.
* [Fix] Making pgModeler honor the columns arrangement in primary keys.
* [Fix] Fixed a bug that was causing FK relationship deletion to crash the application sometimes.
* [Fix] Some fixes were done in the ModelOverviewWidget in order to support large models without exceed the screen size when configuring the size of the overall widget.
* [Fix] Fixes a bad erase in View::removeReference.
* [Fix] Fixed some bugs related to dialog size restoration in DataManipulationForm and TableWidget.

v0.9.2-alpha
------
*Release date: August 20, 2018*

* [New] Added the support for cancelling SQL execution in SQLExecutionWidget.
* [New] Added support for save/restore the dialogs sizes and positions.
* [New] Added support for truncate tables in DataManipulationForm.
* [New] Added support for aliases on some graphical objects that is used in the compact view mode.
* [New] Added support for save/load object's metadata containing aliases information.
* [New] Added support for compact view of the model where graphical objects can have a more friendly name for a reduced view as well for those who don't need to see details about tables (clients of the business, for instance).
* [New] Added support for sequence options for identity columns.
* [New] Added the ability to paste CSV text from clipboard into the TableDataWidget.
* [New] Added support for bulk data edit in TableDataWidget.
* [Change] Added missing copy options on copy relationships.
* [Change] Minor adjustments on the item delegates in order draw text in the right alignment.
* [Change] Minor adjustment on buttons style in DatabaseExplorerWidget, DataManipulationForm and SQLExecutionWidget.
* [Change] Minor adjustment on OperationList::removeFromPool to avoid throw an exception when an invalid index is passed.
* [Change] Changed the behaviour of the fade in/out of relationships linked to a table by applying the effect on the other tables that are related to the selected one.
* [Change] Refactored the view editing dialog by moving the references handling form to a dedicated modal dialog.
* [Change] Improved the model loading from file by blocking signals of relationships avoiding excessive/repetive rendering of objects. The whole model is fully rendered when the file was completely loaded.
* [Change] Minor adjustment on constraints rendering at extended attributes section of tables.
* [Change] French translation update.
* [Change] Updated the other lang dictionaries with the new text brought by new releases.
* [Change] Removing icons at the top of the dialogs: DatabaseImportForm, MetaDataHandlingForm, ModelDatabaseDiffForm, ModelExportForm, ModelFixForm.
* [Change] Minor adjustments in the features of the demo version.
* [Change] Minor adjustments in the UI stylesheet.
* [Change] In DatabaseExplorerWidget the root item will come automatically selcted when browsing a database.
* [Change] Minor performance tuning when handling big models.
* [Change] Added some statistics attributes for tables on DatabaseExplorerWidget.
* [Change] Minor adjustment in NewObjectOverlayWidget by putting the tool buttons under categories.
* [Fix] Fixed a bug in ObjectFinderWidget that was forcing schemas rectangles to appear even if the flag indicating them to be visible was set to false.
* [Fix] Fixed the editing form cancel operation. Now operations done when the form was active are undone correctly.
* [Fix] Fixed a bug that was preventing to create a view containing the same name as a table but in different schema.
* [Fix] Fixed a regression that caused notices not to be shown in the output panel at SQLExecutionWidget.
* [Fix] Fixed the query catalog for policies which was causing syntax error when combining import system objects and extension objects options.
* [Fix] Fixed the disabling of some actions related to design when switching to manage view.
* [Fix] Minor fix on stylesheet in order to display the extended button on general toolbar.
* [Fix] Fix a shortcut conflict in DataManipulationform.
* [Fix] Fixed the offset of strings composing the StorageType.
* [Fix] Minor form size adjustments.
* [Fix] Minor fix in sqlexecutionwidget.ui to force the exhibition of grid headers
* [Fix] Minor fix in SQLExecutionWidget allowing the output widget to be resized to a size lower than the default one.
* [Fix] Fixed the tab order in PolicyWidget.
* [Fix] Fixed the generation of Database object source in DatabaseExplorerWidget.
* [Fix] Fixed the method BaseObjectWidget::setRequiredField to make object selector fields as required correctly.
* [Fix] Minor fix in HintTextWidget to stay on top of all widget when being displayed.
* [Fix] Fixed a bug that was not quoting extension name when needed.
* [Fix] Fixed a crash when trying to remove a fk relationship when it was created from a foreign key which references protected columns (added by relationship).
* [Fix] Fix a crash when importing CSV files into DataManipulationForm.
* [Fix] Minor typo in TableDataWidget.
* [Fix] Minor fix on schema file sql/table.sch.

v0.9.1
------
*Release date: May 14, 2018*

* [New] Added support for line selection by clicking and moving the mouse over the line numbers widget in any source code field.
* [New] The validator now checks if the model has columns referencing spatial data types and creates the postgis extension automatically when fixing the model.
* [New] Added support for RESTART IDENTITY on truncate tables in DatabaseExplorerWidget.
* [New] Added an custom option checkbox in Messagebox for general purpose usage.
* [New] Added support for diff operation in CLI.
* [New] Added support for import database from CLI.
* [New] Adding missing types regrole and regnamespace.
* [Change] Improved the copy/duplicate operation in order to copy rules, index, trigger and policies together to their parents.
* [Change] Added column names to the code completion widget used in the filter widget at DataManipulationForm.
* [Change] Improved the SQLExecutionWidget in such way that it'll display large amount of data more quickly and consuming less memory.
* [Change] Minor improvement in SQLExecutionWidget to show the amount of time took to run a query.
* [Change] Minor improvement in the text find widgets in SQL tool in order to make them closable via dedicated button.
* [Change] Improved the set tag operation in ModelWidget in order to cleanup the assigned tags to a set of objects.
* [Change] Minor improvement on DatabaseExplorerWidget to show the rls attributes labels correctly in the attributes grid.
* [Change] Refactored all the CLI options.
* [Change] Minor change in Connection::generateConnectionString in order to put the dbname param in the start of the string.
* [Change] Improved the performance of the row duplication action in DataManipulationForm.
* [Change] Minor improvement in order to update the schemas boxes when the tables have their extended attributes box toggled.
* [Change] Improved the performance of "Move to schema" operation.
* [Change] Added an busy cursor while closing a model.
* [Change] Improved the object selection in object finder.
* [Change] Changed the behaviour of select and fade buttons in ObjectFinderWidget in such way to enable the user to select/fade the objects in the listing (or not included in the results).
* [Fix] Fixed a bug when import identity columns in certain cases when the identity column was followed by another column which data type was not accepted for identity, e.g, text after smallint.
* [Fix] Fixed the check boxes disabling when dealing with identifier relationships.
* [Fix] Disabled the drag & drop for items in the side listing at ConfigurationForm.
* [Fix] Fixed the tab behavior on comment box in all editing forms of database objects.
* [Fix] Fixed the catalog query for user defined types.
* [Fix] Fixed the import of user defined types which names contains uppercase characters.
* [Fix] Minor typo fixes in CLI.
* [Fix] Fix window scaling on HiDPI/Retina screens.
* [Fix] Minor fix in Connection::getConnectionId in order to omit port when that parameter is not configured in the connection.
* [Fix] Fixed a bug in ModelExportHelper that was failing to remane the database when the command appeared.
* [Fix] Fixed a bug in CollationWidget that was referencing the collation attributes LC_??? using the wrong constant.
* [Fix] Fixed the behaviour of the message box that warns about the need of validate the model prior to export, save or diff. Now rejecting the dialog (i.e. closing it) will be considered that the user wants to proceed with the pending operation even with an invalid model.
* [Fix] Fixed the import of comments for constraints,triggers, index and rules.
* [Fix] The value input in BulkDataEditWidget will be focused as soon as the widget appears.
* [Fix] Fixed a bug in the aggregate import process.
* [Fix] Minor fix in DataManipulationForm to avoid the generation of a where clause when the filter is filled only with spaces.
* [Fix] Minor fix in the magnfier tool to use the same render hints as the canvas viewport.
* [Fix] Fixed a bug in the diff process that was trying to recreate the whole database when the "Force recreation" option was set.
* [Fix] Fixed a bug when showing the source of tables in DatabaseExplorerWidget when these objects have permissions assigned.
* [Fix] Adjusting tables position when the parent schema is moved and the alignment to grid is enabled.
* [Fix] Minor fix in the CLI menu.
* [Fix] Fixed the saving process for large models by stopping the threads related to temp models saving while the model file is being written.

v0.9.1-beta1
------
*Release date: April 6, 2018*

* [New] Added the ability to create multiples one-to-many and many-to-many relatationships between the same pair of tables.
* [New] Added the ability to use more special ascii chars in the middle of object names.
* [New] Added missing SQL keywords into sql-highlight.conf
* [New] Added support for multi line comments in UI.
* [New] Added code snippets for CREATE and ALTER policy.
* [New] Added full support for row level security (RLS), including export, import and diff of this kind of object.
* [New] Added the method DatabaseExplorerWidget::formatPolicyAttribs in order to display some attributes values correctly.
* [New] Added support for bulk data editing in DataManipulationForm.
* [New] Added an option to diff process to force the generation of DROP commands for columns and constraints even if the missing objects need to be preserved. This is useful to work with partial models and the user need to remove columns/constraints and preserve the rest of objects.
* [New] Added the ability to generate diff code to Enable/Force RLS attribute of tables.
* [New] Added support for RLS on tables.
* [New] Added the support for detect identity columns in diff.
* [New] Added support for identity columns (PostgreSQL 10).
* [New] Added the support for BYPASSRLS option on roles.
* [New] Added support for IS_TEMPLATE and ALLOW_CONNECTIONS options in database object.
* [New] Added the procedures to fix old style domains in CLI.
* [New] Added support for multiple check constraint in domains.
* [New] Added support for sort items alphabetically (ascending) or by oid in DatabaseExplorerWidget.
* [Change] Changed the input mode of the password field in ConnectionsConfigWidget in order to hide the passwords in the form. NOTE: the passwords are still in plain text in the config file.
* [Change] Moved extensions from schema level to database level in order to reproduce better the PostgreSQL's behavior.
* [Change] The filter widget is now toggled in DatabaseExplorerWidget via filter menu.
* [Change] Minor adjustments on forms sizes.
* [Change] In GeneralConfigWidget when restoring default settings the default settings for syntax highlight are restored as well.
* [Change] pgModeler will not try to create the plugins path anymore. This will avoid constant error messages during startup. Now, it'll silently ignore the absence of that folder and skip the plugin loading.
* [Change] Minor improvements on catalog queries for index, trigger, rule, policy, constraint in order to use the comment catalog query.
* [Change] Removed an uneeded form adjustment code in Table::openEditingForm.
* [Change] Minor improvements on DatabaseModel::getCreationOrder.
* [Change] Improved the source editing in external application. The use is informed about the app running state and the contents for the source editor field are locked until the user closes the external app.
* [Change] Improved the model loading on macOs in such way to avoid showing the visual objects creation while they are being loaded from file.
* [Change] Improved the reverse engineering and diff process to accept the new attributes of database object.
* [Fix] Fixed the query catalog for built-in types to include the types related to domains.
* [Fix] Fixed the Extension::setSchema method to accept null schemas.
* [Fix] Fixed the generation of XML code for casts.
* [Fix] Fixed the extension creation, allowing only one instance of the named extension per database no matter the schema used to allocate its children objects.
* [Fix] Minor fix in ObjectDepsRefsWidget to correctly list the indirect references.
* [Fix] Fixed a bug when dropping Functions in DatabaseExplorerWidget.
* [Fix] Improved the import of sequences in such way to avoid unsolvable reference breaking.
* [Fix] Fixed a bug that cause the disabling of connections for database models created prior to 0.9.1-beta1.
* [Fix] Fixed a bug on import process that was wrongly creating types derivated from tables/sequence/views causing duplication problems during validation.
* [Fix] Fixed a crash on macOs when opening a second model.
* [Fix] Fixed the import of sequences which now assigns owner columns correctly. If the owner column is an identity one the SQL code of the sequence is disabled by default which will not cause confusion in the diff process trying to drop it in some cases.
* [Fix] Fixed an issue in diff process that was generating a malformed DROP command for extensions.
* [Fix] Minor fixed in the "Filter by OID" feature in DatabaseExplorerWidget and DatabaseImportForm.
* [Fix] Fixed the diff for domains which contain multiple check constraints.
* [Fix] Fixed a bug that was not selecting the correct spatial type in the widget.
* [Fix] Fixed a conflict of shortcuts in DatabaseExplorerWidget. Now F5 updates a leaf/subtree and Alt+F5 performs quick refresh of the tree.
* [Fix] Fixed a problem with sqlexecutionwidget.ui that is not building properly in Qt 5.10.

v0.9.1-beta
------
*Release date: January 26, 2018*

* [New] Added support for GROUP BY/HAVING clauses in Views by adding a new kind of reference. Proper changes done in ViewWidget to allow configuring those clauses.
* [New] Added the method Catalog::isSystemObject(oid) which indicates if the provided OID is related to a system object.
* [Change] Minor adjustment in the copy/paste operation to generate suffix in the pasted objects only when there're conflics.
* [Change] Removed the port range limitation in connection configuration dialog.
* [Change] Updated the default version of Qt and PostgreSQL to, respectively, 5.9.3 and 10.1 in deployment scripts.
* [Change] Changed the method PgSQLType::getTypeName by adding a bool parameter so the name can be returned with dimension descriptor (when dimension is > 0). Useful for configuring operator's signatures.
* [Fix] Fixed the drop action for materialized views in database explorer.
* [Fix] Fixed a crash when importing extension objects.
* [Fix] Fixed the generation of operator's signature that must consider dimensions of the arguments' types.
* [Fix] Fixed the bounding rect calculation for relationship instances when one or more labels are hidden.
* [Fix] Fixed the SVG & PNG export to properly determine the area to be drawn in the destination graphics file.
* [Fix] Fixed a crash when adding attributes into many-to-many relationships.

v0.9.1-alpha1
------
*Release date: November 30, 2017*

* [New] Added the ability to compare two databases, and not only a model and a database, in diff tool.
* [New] Added the relationship creation buttons on the object overlay when a single table is selected.
* [New] Added the "Relationship" action in "New" submenu on table's popup menu so the user can create relationships using the selected table as source. This avoids the need to use blank areas of the canvas to start creating relationships.
* [New] Improved the data manipulation dialog in such way that when dealing with deletes in tables without PK, tuples with NULL values can be correctly considered.
* [New] Improved the validations on ResultSet class.
* [New] Added a method to indicate if a column value is null in ResultSet.
* [New] Added support for fade in/out objects in object finder in order to highlight the graphical objects retrieved from the search.
* [New] Added an attribute in pgmodeler.conf to store the current status of the "Fade in" button in object finder widget.
* [Change] Minor improvement in the diff generated metadata.
* [Change] Increased the maximum allowed amount of lines in command history.
* [Change] Minor adjustment on diff tool so the connections combo can be correctly updated when the user edit connections from within that form.
* [Change] Improved the progress info of diff process so it can be more accurate.
* [Fix] Fixed the way PostgreSQL 10+ version is returned from Connection::getPgSQLVersion.
* [Fix] Fixed the sequence importing on PostgreSQL 10.

v0.9.1-alpha
------
*Release date: October 20, 2017*

* [New] Added support for crow's foot notation.
* [New] Added the crow's foot notation switch in RelationshipConfigWidget.
* [New] Added the grid arrangement in the arrangment menu at MainWindow.
* [New] Added the schema arrangement (scattered).
* [New] Added an action to toggle schemas rectangle on ModelWidget.
* [New] CLI now loads the relationship and general settings to reflect relationship styles in export modes.
* [New] Added support for connect relatinship on tables' edges when using classical notation.
* [New] Added support for apostrophes in the middle of object's name.
* [Change] Removed the controls related to arragement in DatabaseImportForm.
* [Change] Minor adjustments in tables' spacing in auto arrangement process.
* [Change] Minor improvement on SQLExecutionWidget and DataManipulationForm in order to make possible to paste csv buffer from SQLExecutionWidget to DataManipulationForm.
* [Change] Improvements done in the Spanish UI translation.
* [Change] Changed the position of the zoom info icon in SceneInfoWidget.
* [Change] Minor adjustments in the pen width of relationship lines and objects borders.
* [Change] Minor improvement when aligning objects to grid forcing the relationships updating.
* [Change] Minor arrangement of the connection modes in RelationshipConfigWidget.
* [Change] Improved the performance of (de)selection of several objects at once in ModelWidget and ObjectsScene.
* [Change] Removed unused attributes from BezierCurveItem.
* [Change] Improved the BezierCurveItem class to enable the drawing of inverted curve by inverting its bounding rect.
* [Change] Improved the import of index objects.
* [Change] Minor tweak to enable clipboard usage in macOS when copying data in DataManipulationForm.
* [Fix] Fixed a bug in ObjectsScene that was not emitting signals of deselection correctly.
* [Fix] Fixed a bug in SQLToolWidget that was not cleaning up the source code pane when all databases were disconnected.
* [Fix] Fixed a bug that was causing the diff process to try to remove the not null constraint of a primary key.
* [Fix] Fixed a bug that was causing relationship line to be wrongly constructed in case the tables bounding rects don't intercepted.
* [Fix] Fixed a bug that was recognizing the creation of a constraint but wasn't generating the SQL in diff process.
* [Fix] Minor fix in order to avoid the inheritance/dependency descriptor to be rotated to the wrong size when curved lines are being used.
* [Fix] Fixed the generation of sql comments for database and tablespace.
* [Fix] Minor fix in example.dbm
* [Fix] Fixed the configuration of bidirectional fk relationships when crow's foot is enabled.
* [Fix] Fix a bug in GeneralConfigWidget that was reverting the grid optins everytime the user applyed settings.
* [Fix] Fixed the genaration of index elements containing expressions.
* [Fix] Fixed the import of operators and operator classes.
* [Fix] Fixed the generation of operator signature by removing the length/precision of the types.
* [Fix] Minor fix in CSVLoadWidget::loadCsvFromBuffer in order to preserve the line breaks avoiding the creation of unecessary lines.
* [Fix] Fixed the import of exclude constraint.
* [Fix] Fixed the import of timestamp(0) type.

v0.9.0
------
*Release date: September 1st, 2017*

* [New] Added the ability to paste text from clipboard to data grid in DataManipulationForm.
* [New] Created the method CsvLoadWidget::loadCsvFromBuffer to make the code that extract csv document from string buffer reusable by other classes.
* [New] Added a new sample model donated by the maintainers of 3D City DB project.
* [New] Added the language "internal" to the set of system languages available when creating a new model.
* [New] Added support for override the default language settings via GeneralConfigWidget.
* [New] Added support for toggle curved relationship lines in GeneralConfigWidget.
* [Change] Improved the MetadataHandlingForm enabling user to only extract metada to a backup file.
* [Change] Small update on sample models.
* [Change] Minor adjustments in the graphical points when relationships are selected.
* [Change] Adjusted the calculation of the descriptor object.
* [Change] Fixed the rotation of the descriptor object for identifier relationship when curved lines are activated.
* [Change] Minor code documentation.
* [Change] Moved the class BezierCurve to its own source files.
* [Change] Improved the way bezier curves are generated for relationships.
* [Change] Changed the default action to reset label's position from middle button click to Alt + Ctrl + left click.
* [Change] Minor enhancement in auto arrange feature to avoid breaking lines when curved relationship lines are enabled.
* [Change] Minor size adjustment in GeneralConfigWidget.
* [Change] Minor update in README.md.
* [Change] Minor size adjustment for DatabaseImportForm.
* [Change] Minor adjustment in the methods which automatically resize dialogs depending on the resolution.
* [Change] Changed the default font for objects and source code.
* [Fix] Minor fix in RelationshipView to hide the circles at end of lines for self relationships.
* [Fix] Fixed the catalog query for event triggers.
* [Fix] Fixed the icons and labels of the "Select all" submenu in ModelWidget.
* [Fix] Fixed a rare crash when configuring self relationships.
* [Fix] Minor fix when rendering self n:n relationships.
* [Fix] Minor fix in the HintTextWidget to resize more properly according to the held text.
* [Fix] Minor adjustment in SceneInfoWidget.
* [Fix] Minor fix in default confs for source code font style.
* [Fix] Fixed the generation of objects style configuration file that was missing constraints settings.
* [Fix] Fixed a bug in the index / exclude constraint import which was not creating expressions of these objects correctly and sometimes trucating them.

v0.9.0-beta2
------
*Release date: July 1st, 2017*

* [New] Added an action to reset labels distance in BaseRelationship and ModelWidget.
* [New] Added a widget that shows some info about the canvas and the selected objects at the bottom of main window in design view.
* [New] Enabled the usage of snippets in other portions of the software like GenericSQLWidget, FunctionWidget, ViewWidget, CustomSQLWidget.
* [New] Added the ability to quickly jump to the tables related to a relationship.
* [New] Added support for select all objects in the canvas by type (table, view, textbox, schema, relationship).
* [New] Added support for bulk relationship points removal.
* [New] Added a magnifier tool so the user can visualize objects when the zoom is too small. This tool allows the user to click to select or activate the context menu over the objects.
* [New] Added support for generic sql objects that serve as an improved way to use custom SQL.
* [New] Added support for handle metadata related to generic sql objects.
* [New] Added the first object auto-arrange algorithm.
* [Change] pgModeler will now accept (connect) to a PostgreSQL server even if the version of the server is not supported falling back to the most recent supported.
* [Change] Minor improvements on DatabaseImportForm, ModelExportForm, ModelDatabaseDiffForm and MetadataHandlingForm to toggle uniformRowsHeight of the output tree at the start and the end of each process to avoid slowdowns and allow the items to be resized correctly when expanded.
* [Change] Changed the way color are stored in Tag.
* [Change] Minor adjustement on position info of objects in canvas.
* [Change] Improvements on import/diff/export performances by adjusting the way the output widget handles items height.
* [Change] Move the code from MainWindow::showEvent to the constructor of that class so all operations that load resources and restore configurations can be performed prior the window display.
* [Change] Improved the ModelOverviewWidget to handle huge models better.
* [Change] Improved the objects swapping by adding an objects grid where user can interact with it in order to choose which objects to swap.
* [Change] Improved the schema parser to allow comparisons forcing the cast of values to float or int in the expression by using special letters attached to logical operators.
* [Fix] Fixed the DatabaseModel::destroyObjects in order to include missing object types and avoiding leaks.
* [Fix] Several memory leaks removed in different portions of the application.
* [Fix] Minor fix in RelationshipView to show the line circles for n:n relationships.
* [Fix] Minor warning fixes related to unused variables/values.
* [Fix] Minor fix in Catalog that was trying to retrieve catalog info for generic sql objects.
* [Fix] Fixed a bug when zooming out using wheel that was causing duplicated zoom in a single wheel turn.
* [Fix] Minor fix in TableView and TableObjectView to present tables in a more compact fashion minimizing the space used in the canvas.
* [Fix] Minor typo fix in swapobjectsidswidget.ui.
* [Fix] Minor fix in lambdas slots usages.
* [Fix] Fixed a bug in generateTextBuffer method in SQLExecutionWidget.

v0.9.0-beta1
------
*Release date: May 13, 2017*

* [New] Added the ability to standalone dialogs like import, diff, export and others to be resized according to the screen dpi and resolution.
* [New] Added an experimental routine that will resize windows according to the current screen resolution and font dpi.
* [New] Added support for browse referrer and referenced in DataManipulationForm.
* [New] Added an item under table items that stores the referrer tables in the DatabaseExplorerWidget.
* [New] Added the method BaseObjectView::getScreenDpiFactor to help resize scene objects according to the screen dpi/resolution.
* [Change] Minor adjustment on readonly items regarding to referenced and referrer tables in DatabaseExplorerWidget.
* [Change] Improved the tabs handling in SQLToolWidget in order to avoid confusion related to which database is being managed or queried currently.
* [Change] Improvements done in the context menu at DataManipulationForm to include the key actions related to the control buttons at the top right portion of the dialog.
* [Change] Improved the external script handling in SQLExecutionWidget.
* [Change] Applied automatic resize for TaskProgressWidget.
* [Change] Improvement done in model restoration dialog that is now displayed after the main windows is exposed.
* [Fix] Fixed a problem in UpdateNotifierWidget that was receiving error 403 from the site.
* [Fix] Fix a bug in DataManipulationForm that was causing order by clause to be nullified by comments added in the filter field.
* [Fix] Fixed a regression in permission code generation.
* [Fix] Fixed a bug in the generation of grant/revoke commands for columns.
* [Fix] Fixed a bug that was causing the sorting options of index elements to be wrongly hidden.
* [Fix] Minor fix in the site url.
* [Fix] Minor fix in the filter toggling action in ObjectFinderWidget.

v0.9.0-beta
------
*Release date: April 4, 2017*

* [New] Added support for indexes in Views.
* [New] Added the support for edit/load the source code in NumberedTextEditor in external application.
* [New] Added the ability to save/load metadata related to fade out status and extended attributes display status.
* [New] Added the ability toggle the extended attributes area in tables and views. The toggle status is persisted in the model file and restores during loading
* [New] Added constraints to the extended attributes section in the tables at canvas area in order to improve the quick access to these objects.
* [New] Enabled the importing of view's indexes.
* [New] Fade status is now persisted in the dbm file and restored during loading.
* [New] Added the ability to control zoom factor from overview widget.
* [New] Added a shortcut for "Duplicate" action in design view.
* [New] Added support for (back)slash char in object's names.
* [New] Enabled the usage of NewObjectOverlayWidget for views.
* [Change] Changed the default characters used to escape values in DataManipulationForm and TableDataWidget from {} to // due to problems with json data.
* [Change] Improved the file manipulation in SQLExecutionWidget. Added option to save the commands to the current file or in another file (save as).
* [Change] Minor improvements done in Linux deployment script to support multiarch systems.
* [Change] View's children (indexes, rules, triggers) are now listed under their respective parent view in DatabaseExplorerWidget.
* [Change] Minor improvement in ElementsWidget to disable/hide columns combo when creating index elements for a index associated to a view.
* [Change] Improved the diff between the complete database and a partial model representing it.
* [Fix] Minor fix in AppearanceConfigWidget in order to set the font color correctly.
* [Fix] Minor fix in the default file objects-style.conf
* [Fix] Added the missing support for drop event triggers from database model.
* [Fix] Fixed the drop cast command generation.
* [Fix] Minor fix in windows deploy script to use newer PostgreSQL.
* [Fix] Minor fix in template connections.conf file.
* [Fix] Minor fix in config files related to installer generation (Linux).
* [Fix] Minor fix in paste operation to restore the viewport position in design view.
* [Fix] Minor fix in diff process to detect view's index changes.
* [Fix] Fixed a bug in EventTrigger that was causing unknown exception to be thrown.
* [Fix] Fixed a bug on RoleWidget that was preventing roles to be removed from "Members Of" tab.
* [Fix] Minor fix in mouse cursor override operations.
* [Fix] Fixed a bug when importing functions and composite types that somehow depend upon array types.
* [Fix] Fixed a bug in function importing that was causing default values of parameters to be placed in the wrong position.

v0.9.0-alpha1
------
*Release date: February 07, 2017*

* [New] Added support for object moving via arrow keys in canvas area.
* [New] Added support for easily create primary keys just by checking the desired columns in table's editing form.
* [New] Added support for use middle button to handle panning mode.
* [New] Added a more user friendly message at startup whenever a missing or corrupted configuration file is detected. The user is now presented to an option to restore default settings for the problematic file.
* [New] Now any default file restored in ConfigurationForm has a backup saved into the directory 'backups' inside the configuration storage.
* [New] Added support for hide the database explorer widget in SQL tool via splitter handler.
* [New] Added a method to disable the custom context menu of the class NumberedTextEditor.
* [New] Added support for object fading in ModelWidget.
* [New] Added the support for persist the object opacity factor in config file.
* [New] Added the method PgModelerUiNS::getIconPath() in order to retrieve icons from resource.
* [New] Added support for column, constraint, trigger, rule and index duplication in TableWidget.
* [New] Added support for item duplication in ObjectTableWidget.
* [New] Added a loading cursor when the user opens the DataManipulationForm.
* [New] The database explorer now creates the root item in the tree as the server itself which contains data related to this latter.
* [New] Added the support for parenthesis in the middle of objects' names.
* [Change] Improvements done in the SQL history at SQL execution widget. Now the command history is saved into a specific file and restored when the application starts.
* [Change] Minor improvement in DataManipulationForm to show a wait cursor while filtering results.
* [Change] Minor improvements in GeneralConfigWidget. Added an readonly input that exposes the path to the current user's configuration storage.
* [Change] Improvements done in the object duplication feature.
* [Change] Remove hardcoded icon paths in the code.
* [Change] Improved the PgSQLTypeWidget to enable the length, precision, dimension fields as the user types the desired datatype. This will avoid jumping to the wrong field when pressing tab.
* [Change] Updated the urls related to download and donation of the new site.
* [Change] Changed the url to check for updates in GlobalAttributes to point to the new site.
* [Change] Improvements done in the linux deploy script to use Qt 5.6.2.
* [Change] Minor tweaks done in order to minimize the diff detection related to default values of columns.
* [Change] Changed the default framework version used in the windows deployment script to 5.6.2.
* [Change] Dropped the automatic LC_COLLATE and LC_CTYPE generation in Database object. Since this was causing more problems than helping when import the database and validating/exporting it.
* [Change] In DataManipulationForm the filter input field is automatically focused when the filter toggle button is activated.
* [Fix] Fixed a bug when using diff to create columns and update constraints.
* [Fix] Fixed a bug that was duplicating the action "New" in the main window's side bar.
* [Fix] Fixed a problem when importing database that contains citext extension installed in pg_catalog.
* [Fix] Minor fix in ConnectionsConfigWidget that was causing duplicated connections to share de same host info wrongly.
* [Fix] Restored the input data type handling in AggregateWidget.
* [Fix] Fixed the oldsample.dbm model.
* [Fix] Fixed a crash when restoring objects' metadata from backup file.
* [Fix] Fixed a bug that was preventing inheritance relationships to be created when the same pair of tables existed in different schemas.
* [Fix] Fixed a bug that was causing column name patterns to be used wrongly in many-to-many relationships.
* [Fix] Fixed a bug that was preventing the automatic closing of tabs related to a dropped database in manage view.
* [Fix] Fixed a bug that was causing the duplication of permissions during the database import which was leading to the complete failure of the entire process.
* [Fix] Fixed the problem with invalid type error when trying to edit a 'timestamp with timezone' column.
* [Fix] Fixed a bug in Relationship that was not setting NOT NULL flag for columns of the multi-valued primary key of many-to-many relationships causing the diff process to fail in some specific cases.
* [Fix] Fixed a regression in RelationshipWidget that was not showing advanced object's form.
* [Fix] Fixed the constraint codes display in TableObjectView. Now self relationships do not mark the primary key field as foreign keys.
* [Fix] Fixed a regression that was not properly disabling the apply button in editing forms when the handled object was protected somehow.
* [Fix] Minor typos fixes in some widgets.

v0.9.0-alpha
------
*Release date: October 18, 2016*

* [New] Enabling pgModeler to connect to PostgreSQL 9.6 servers.
* [New] Added the option to ignore error codes during the export process in CLI.
* [New] Added the ability to ignore extra errors by their codes in ModelExportForm and ModelDatabaseDiffForm.
* [New] Added the ability to load data from CSV file into TableDataWidget and DataManipulationForm.
* [Change] Minor update in snippets.conf by adding a SELECT * command.
* [Change] Removed deprecated exception ERR_ASG_ZERO_LENGTH.
* [Change] Improvements done in CodeCompletionWidget so that the completion can be more accurate mainly when using the form [schema].[table].
* [Change] Methods responsible for dropping and exploring data were moved from SQLToolWidget to DatabaseExplorerWidget.
* [Change] Improved the error output in DatabaseImportForm, ModelDatabaseDiffForm, ModelExportForm and Messagebox.
* [Change] TableDataWidget widget now can have the column names changed freely not only when there are invalid ones.
* [Change] Removed codename from AboutWidget.
* [Fix] Fixed a bug in PgSQLType and PgSQLTypeWidget that was not properly setting length = 1 in character, varchar and numeric data types.
* [Fix] Fixed a bug that was leading to stack overflow when generating object's sql plus its dependencies in huge models.
* [Fix] Fix the structure of the sample model pagila.dbm.
* [Fix] Minor fix in diff proccess in order to permit the comparison between a column added by relatinship and other that is not but share the same name.
* [Fix] Fixed a bug that could cause crashes when editing connections in DatabaseImportForm or ModelDatabaseDiffForm.
* [Fix] Fixed a crash when the user modified a connection on the fly with the SQL tool activated and trying to resume his work in database management.
* [Fix] Fixed the tab order in ConnectionsConfigWidget.
* [Fix] Fixed a bug in ModelDatabaseDiffForm that was running the export thread several times.
* [Fix] Fix the generation of truncate commands in the diff when the types of columns are incompatible.
* [Fix] Fixed a bug that was generating broken sql for tables when these objects have no constraints.
* [Fix] Fixed a bug in diff that was not detecting column types length changes.

v0.8.2
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: June 3, 2016*

* [New] Created the PlainTextItemDelegate replacing the ReadOnlyItemDelegate where needed.
* [New] Added the ability to the table to create insert commands from the initial data buffer.
* [New] Added the support for interpret initial-data tag in DatabaseModel::createTable.
* [New] Create the attribute initial-data for Table in order to store the initial set of values in a CSV-like buffer.
* [New] Created the form to handle table's initial data.
* [New] Added the ability to duplicate rows in DataManipulationForm.
* [New] Added shortcuts to tabs in TableWidget.
* [New] Added the ability to clear and copy text from history to the sql command input field using middle mouse button in SQL tool.
* [New] Added the ability to set the default connection for operations import, export, diff and validation in ConnectionsConfigWidget.
* [New] Added the usage of default connections in ModelValidationWidget.
* [New] Added support for save/load default connections in the ConnectionsConfigWidget.
* [New] Added attributes to the Connection class in order to control wheter the connection is the default for export, import, diff or validation operations.
* [New] Added the ability to save the current grid options to the pgmodeler.conf file.
* [New] Added a reference the svg library to the deployment scripts.
* [New] Added support for export model to SVG file in UI and CLI.
* [New] Added the support for change case and identation of the selected text in NumberedTextEditor using context menu or shortcuts.
* [New] Added a method PgModelerUiNS::createOutputListItem which created list items with an icon and text.
* [New] Connections now can have a timeout between command executions. When this timeout exceeds the next command is not executed. This is a workaround to avoid the crash of the program due to connections being (unexpectedly or not) closed by the server.
* [New] Added the ability to show connections notice/warning in SQL tool.
* [New] Added an step during the connections.conf loading to fix the connection timeout attribute automatically.
* [Change] The class ReadOnlyItemDelegate was removed due to the introduction of PlainTextItemDelegate.
* [Change] Minor adjustments in queries generated in DataManipulationForm in order to use PostgreSQL like escaping E'' permitting user to use special chars in the middle of values.
* [Change] Simple quotes (') in DataManipulationForm and TableDataWidget will be automatically replaced by double quotes ('') in order to avoid broken commands.
* [Change] Updated all translation dicts with new terms to be translated by their mainterners.
* [Change] Changes in demo version enabling a limited usage of diff and import features.
* [Change] Improved the way QTableWidgets instances are emptied.
* [Change] Improvements done in order to correctly enable the column/row control buttons in DataManipulationForm according to the selected items.
* [Change] Improved the duplication/delete operation in DataManipulationForm. These operations will happen only if the user selects the entire row.
* [Change] Renamed the method DataManipulationForm::insertRow to addRow.
* [Change] Changed the icons for Add, Delete rows in DataManipulationForm.
* [Change] Improved the metadata handling form. Now the user just need to choose from which model to extract the metadata and the form will do the rest (extract and apply) in one step.
* [Change] Minor improvement in ModelsDiffHelper in order to avoid the generation of useless SQL code (SET statments) when no effective changes were found in the process.
* [Change] Minor improvement in ObjectSelectorWidget to change the object selection dialog title according to the handled object type.
* [Change] Minor improvement in ObjectSelectorWidget to adjust the size of input according to the installed syntax highlighter.
* [Change] Renamed the method Connection::setCommandExecTimeout to setSQLExecutionTimout.
* [Change] Minor improvements in SQLExecutionWidget to use the command execution timeout.
* [Change] Improvements on SQLExecutionWidget to enable the connection stay open in order to permit the usage of commands START TRANSATION, COMMIT and ROLLBACK.
* [Change] Changed the icon for info message boxes.
* [Fix] Minor typos fixed in UI components.
* [Fix] Fix a bug in the Catalog class that was generating broken catalog queries for PostgreSQL releases under 9.3.
* [Fix] Fixed a bug in DataManipulationForm that was not quoting columns in the generated UPDATE commands.
* [Fix] Fixed some tooltips and shortcuts.
* [Fix] Fixed a bug in import process related to permission creation. Now pgModeler removes extra backslash from role's name to avoid it not to be found in the model.
* [Fix] Fixed the doxygen 'brief' instructions in code documentation.
* [Fix] Fixed a bug that was generating broken table's SQL when the object has one or more inherited column.
* [Fix] Fix on the history text copy. Added the correct mouse button (middle) that triggers the copy from the history to the sql input field.
* [Fix] Escaping properly columns' comments.
* [Fix] Minor fix in RelationshipWidget input fields related to receiver and reference tables.
* [Fix] Minor fixes in QPlainTextEdit instances where the SyntaxHighlighter class is used to adjust the height of the field according to the font size.
* [Fix] Minor fix in NumberedTextEditor::showContextMenu to use the cursor's postion only when the menu is executed.
* [Fix] Fixed the way instances of ResultSet are copied.

v0.8.2-beta1
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: March 31, 2016*

* [New] Added missing PostGiS types.
* [New] Adding the ability to save/load more metadata from objects like model's last zoom and position, protection status, sql disabled status, tags, textboxes and others.
* [New] Created the method BaseGraphicObject::isGraphicObject.
* [New] Added the ability to show the object's source in the SQL tool.
* [New] Added missing encodings descriptors KOI8 and KOI8R.
* [New] Added the method PgSQLType::isPolymorphicType that indicates if the type is one of the any*.
* [New] Added a new method PgSQLType::canCastTo() that indicates if a type can be casted to another.
* [New] pgModeler now stores and restore the state of attributes grid and source pane in SQLTool.
* [New] Added the ability to load dummy items contents by clicking them in DatabaseExplorerWidget.
* [New] DatabaseExplorerWidget now loads the small set of object in order to improve performance. The user have the ability to load the full database by using the refresh button actions.
* [New] Added the ability to retrieve children objects of schemas or tables on demand (as their items are expanded) improving the performance.
* [New] Created an specific icon for refresh database tree.
* [New] Added a version of the method DatabaseImportHelper::getObjects that accepts a list of types and returns a list of attributes.
* [New] Added an new version of Catalog::getObjectNames that retrieve the names according to a list of object types.
* [New] Created a second version of PgModelerUiNS::configureWidgetFont that accepts a custom factor value.
* [New] Adding the ability to BaseForm to use a ScrollArea when the size of the widget exceeds the 2/3 of the user's screen.
* [New] Created template methods to show editing forms according to the kind of database object in ModelWidget.
* [New] Created a signal BaseObjectWidget::s_closeRequested to tell the parent form close after successfuly edit an object.
* [New] Created template methods in RelationshipWidget and ViewWidget to handle child object manipulation.
* [New] Created a template method TableWidget::openEditingForm to handle the editing form for children objects.
* [New] Added support for placeholder objects when moving graphical objects improving performance mainly when moving tables and relationships avoiding excessive update operations.
* [New] Added an option to protect schema's children when protecting the schema itself.
* [New] Added the abitily to diff partial models without drop the not imported ones from the original database.
* [Change] CentralWidget renamed to WelcomeWidget.
* [Change] Adjusted the build in Windows to use qt 5.5.1 and PostgreSQL 9.5.
* [Change] Changed the help action from Wiki to Support and pointing it to GitHub's issues page.
* [Change] Minor tweak in GeneralConfigWidget to permit current line to be highlighted even if the text preview field is readonly.
* [Change] NumberedTextEditor will not highlight the current line if it is readonly.
* [Change] Since label's dtd was moved to its own file the dtd for relationship now includes it.
* [Change] Sorting spatial types in PgSQLTypeWidget.
* [Change] Minor adjustments in central frame of the dialogs modelfixform.ui and modelrestorationform.ui.
* [Change] Replaced the taskprogress widget usage from undo/redo operations by a simple busy cursor.
* [Change] Removing duplicated alignment descriptors in generated warning frames at BaseObjectWidget.
* [Change] Minor improvements in DatabaseExplorerWidget. Now each operation that take some time the cursor will be changed to a "wait" icon.
* [Change] Minor improvements in table SQL generation. Inherited columns will be included in the table's code but in commented lines.
* [Change] Minor fix in catalog query for operators. Unary operators will come with NONE key work in the side that the type is missing.
* [Change] Minor update in sql-highlight.conf.
* [Change] DatabaseImportForm::updateObjectsTree now removes 'without time zone' type modifier from parameters.
* [Change] Minor adjustments on table headers in objectdepsrefswidget.ui.
* [Change] Improvement on DatabaseImportForm::listObjects method. Now user can opt in load the full database or load only the cluster level ones and creating dummy items as children of these ones.
* [Change] Minor size adjustments in AboutWidget.
* [Change] Added a default title for ConnectionsConfigWidget.
* [Change] Removed the static text 'pgModeler - ' from message box title.
* [Change] Changed the default Qt version to 5.5.1 in linuxdeploy.sh.
* [Change] Minimum size adjustments for toolbuttons and icons in DataManipulationForm, ModelValidationWidget and SQLExecutionWidget.
* [Change] Minor improvement on DatabaseModel::getCreationOrder in order to include generated relationship constraints instead of the relationship themselves (useful in diff process)
* [Change] Removed the forced frame shape for BaseForm central widget (Windows only).
* [Change] Several size adjustments in all editing forms and dialogs.
* [Change] Forcing the usage of Fusion style if the user does not provide a custom style.
* [Change] Refactored the usage of SwapObjectsIdsWidget instance in ModelValidationWidget. The aggregate BaseForm instance in SwapObjectsIdsWidget was removed and locally created in ModelValidationWidget.
* [Change] Removed the default window title from database objects widgets.
* [Change] Decoupled the BaseForm instance from all BaseObjectWidget and their subclasses. Now editing forms are constructed in ModelWidget::showObjectForm.
* [Change] Changed the attribute that controls connection timeout in connections.conf from 'connect_timout' to 'connection-timeout'.
* [Change] Minor adjustment on connection configuration dialog size.
* [Change] Remove the maximum size restrictions from all editing forms in order to better adapt the user's font settings.
* [Change] Minor adjustments on bottom margins at mainwindow.ui.
* [Change] Adjustments on warning and hint messages in editing forms.
* [Change] Changed the size constraints for tool buttons.
* [Change] Done modifications in order to avoid the usage of fixed font size and fixed colors in some widgets.
* [Change] The option to control render smoothness in canvas now does not requires application restart.
* [Change] Improved the CLI fix mode when dealing with broken operator classes, index and exclude constraints.
* [Change] Improving the presentation of operator classes and families. Now the index access mode comes attached to their names in tree views.
* [Change] Added a message to export progress when an object is renamed due to the option 'use temp. and uniq. names'.
* [Change] Moved the "save model" button from "Report" tab to "Database model" tab in CrashHandlerForm.
* [Change] Forcing the tree item update in model objects widget when activating some of actions in the popup menu.
* [Change] Added colon character (:) as a valid one to appear in the middle of object's names.
* [Fix] Added missing dtd file label.dtd.
* [Fix] Fixed the label for inheritance relationship action in ModelWidget.
* [Fix] Fixed the object schema file for metadata generation.
* [Fix] Fixing the splash screen display in MacOSX.
* [Fix] Fixed the tooltip of toggle buttons in SQLTool.
* [Fix] Minor fix when generating the database source code for visualization in SQLTool.
* [Fix] Fixed a bug in import process that was not properly creating dependency objects when auto resolve deps was checked.
* [Fix] Fixed the fonts of hint boxes in datamanipulationform.ui.
* [Fix] Fixed the conversion catalog query and import process.
* [Fix] Fixed the aggregate validations related to assigned functions in order to permit the import of system aggregates.
* [Fix] Minor fix in catalog query for triggers.
* [Fix] Fixed the import of triggers, index, rules to automatically create their parent table if they are not yet created (when using auto resolve deps).
* [Fix] Fixed the reference to "any" type.
* [Fix] Fixed the attribute used as function for casts in DatabaseImportHelper::createCast.
* [Fix] Additional fixes to cast code generation.
* [Fix] Minor fix in the cast SQL schema file.
* [Fix] Fixed the OID filtering in DatabaseExplorerWidget.
* [Fix] Fixed the code display in SourceCodeWidget.
* [Fix] Fixed the drop command generation for extension objects.
* [Fix] Fixed a bug that was not updating relationships when importing objects to the current database model.
* [Fix] Fixed the deployment script linuxdeploy.sh making it to copy additional Qt libs.
* [Fix] Fixed the path to Qt installer framework in linuxdeploy.sh.
* [Fix] Additional fix in linuxdeploy.sh to retreive the current compiled version.
* [Fix] Fixed a bug when initializing PgSQLType instances when the provided type name is a user defined one. Random precision/dimension is not created anymore.
* [Fix] Changed the font factor used by WelcomeWidget buttons.
* [Fix] Minor fix in deployment script (Mac OSX).
* [Fix] Fixed the object search when using exact match option.
* [Fix] Fixed a bug that was not creating new columns in cases when the option to keep missing objects was set.
* [Fix] Fixed the diff process that was not processing 1-1 or 1-n relationships correctly.
* [Fix] Minor fix in ConnectionsConfigWidget setting up the correct buttons of the parent form in method openConnectionsConfiguration.
* [Fix] Minor fix on message box default size of buttons.
* [Fix] Fixed some signal connection warnings in BaseForm.
* [Fix] Disabling the apply button correctly when the object is protected.
* [Fix] Fixed a bug when pressing ESC key in the middle of object's movement that was canceling it.
* [Fix] Fixed a bug when selecting a protected object using right button. The parent object is not selected incorrectly anymore.
* [Fix] Fixed a bug when importing operator classes that was generating incorrect XML code for this kind of object prior the creation in the output model.
* [Fix] Additional fix in SchemaParser::convertCharsToXMLEntities to correctly replace char in operator's names.
* [Fix] Fixed the sample files to use the new way to reference opertor classes and families.
* [Fix] Additional fix for code generation of operator classes.
* [Fix] Fixed the loading of index, exclude constraint and operator classes when referencing some operator class or family.
* [Fix] Fixed the code generation for operator classes to use 'signature' attribute.
* [Fix] Fix a bug in the method SchemaParser::convertCharsToXMLEntities that was not replacing char by entities in certain cases.
* [Fix] Minor fix in SourceCodeWidget to correctly show the progress dialog in the right position on screen.
* [Fix] Added a validation in enum types avoiding user to include enum ids with invalid chars.
* [Fix] Added a validation when creating operator classes that references an operator family by the name instead of signature.
* [Fix] Minor fix in operator class form to install syntax highlighter in the operator family selector.
* [Fix] Improved the method DatabaseImportHelper::getObjectName in such way to be able to return the operator families signatures.
* [Fix] Fixed operator class code generation to use operator family's signature instead of name.
* [Fix] Fixed the code generation for Operator family. Reduced form code (XML) is generated with "signature" attribute instead of "name"
* [Fix] Fixed the object search in DatabaseModel. Now duplicated operator families are accepted, the desambiguation term will be the indexing mode.
* [Fix] Fixed the creation of operator classes. Now operator families are referenced by their signature not the name.
* [Fix] Fixed a bug when importing functions with unamed parameters.
* [Fix] Minor typos fixes in PgSQLType source.
* [Fix] Fixed some leaks and crashes when canceling the creation of new tables, views or relationship from their editing forms and closing the application.
* [Fix] Fixed a bug related to quoted name validation that was wrongly raising errors related to long names. The validation of name size now discards the quotes from the count.

v0.8.2-beta
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: January 12, 2016*

* [New] Added version descriptor for PostgreSQL 9.5 enabling pgModeler to connect to it.
* [New] Added access method BRIN for indexes, operator classes and operator families as an initial support for PostgreSQL 9.5.
* [New] Added event "table_rewrite" for event triggers as an initial support for PostgreSQL 9.5.
* [New] Added "Diff" action to File menu.
* [Change] Minor improvement in DataManipulationForm adding the shortcut of "Copy selection" button to its tooltip.
* [Change] Improvements on DataManipulationForm on how pk columns are handled and used in the generated DML commands for UPDATE and DELETE.
* [Change] Minor improvement on ModelRestorationForm when listing temp models.
* [Change] Changed the hint text for "Disable render smoothness" option.
* [Change] Changed the hint text for "Validate before save, export and diff".
* [Change] Minor improvements on SQLExecutionWidget and DataManipulationForm to scroll items in the results grid by pixel not per item.
* [Fix] Fixed a bug in operations that convert integer to serial and vice-versa.
* [Fix] Fixed some header items text alignment.
* [Fix] Fixed a crash when loading a broken model. Instead of show the error message related to corrupted file pgModeler was being aborted.
* [Fix] Minor fix in hint text in SourceCodeWidget and ModelDatabaseDiffForm.
* [Fix] Fixed shortcut conflicts in MainWindow.

v0.8.2-alpha1
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: November 13, 2015*

* [New] Added an additional step in import process to validate inheritance relationships to avoid incomplete tables.
* [New] Added an additional relationship validation in model loading process when there are inheritances. This will avoid incomplete columns and validation errors related to "permanent invalidation state".
* [New] Created an exclusive exception code when a parent table is not found in the imported set. This error is raised during inheritances creation.
* [New] Added the signal s_connectionsUpdateRequest in DatabaseImportForm, ModelExportForm, ModelDatabaseDiffForm, ModelValidationWidget and SQLToolWidget in order to inform the main window that user has changed connections in those forms.
* [New] Added the ability to configure connections without using the main configuration form. Now the user is able to do this task by using the "edit connections" option in any combo related to connections.
* [Change] Minor adjustments in diff process messages.
* [Change] Minor adjustments on ModelValidationWidget, ObjectFinderWidget and SQLExecutionWidget resize event to change the shape of toolbuttons in order to avoid truncate texts when the window size is too small.
* [Change] Replaced the explicit hint texts from ModelValidationWidget by HintText instances.
* [Change] Minor adjustment on widgets that are used to set connections.
* [Fix] Fixed the way objects are destroyed when a model is closed diminishing the time consumed by that operation and the chances of crashes after their destruction.
* [Fix] Additional fix in database import feature. Inheritances will be automatically created when "auto resolve dependencies" is checked.
* [Fix] Fixed a crash when importing a database that contains big tables that handles multiple inheritances.
* [Fix] Fixed a crash in Windows version. A missing initialization in OperatorClassElement was leading to segmentation fault.
* [Fix] Fixed a bug in table and view editing form that was permitting to confugure permissions to new objects before create them in the model.
* [Fix] Fixed a problem in UpdateNotifierWidget when the server returns http status code 302.

v0.8.2-alpha
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: October 05, 2015*

* [New] Added a toggle button in SQL Execution to show/hide the output pane.
* [New] Added the method Permission::isSimilarTo that returns true when a provided permission has the same semantics as the caller permission.
* [New] Added missing keywords CASE, ELSE, QUERY, ELSIF, RAISE, EXCEPTION, TG_OP to sql-highlight.conf
* [New] Columns that compose primary key and unique key are exposed as children of the constraint in the object tree at DatabaseExplorerWidget.
* [New] Foreign key objects selected in DatabaseExplorerWidget now expose, in two children items, the source and referenced tables/columns.
* [New] Added a confirmation message in DataManipulationForm to avoid lose uncommited changes before retrieve data.
* [New] NumberedTextEditor now is able to set a custom tab width.
* [New] Added a configuration option for custom tab width in GeneralConfigWidget.
* [New] Added a nl_NL (Dutch - Netherlands) UI translation.
* [New] Created a mechanism to make default values of columns in the form nextval(sequence) be transformed in a link between the sequence and the column in the import process. This will diminish the divergences raised by the diff process.
* [New] Added a readonly item delegate for attributes grid to permit user to copy contents or navigate through values using keyboard.
* [Change] Changed the initial data limit in DataManipulationForm from 100 to 1000.
* [Change] Removed the default protected status of public schema in sample models.
* [Change] The system schema public now can be protected/unprotected as well moved through the canvas area.
* [Change] Changed the method DatabaseModel::getPermissionIndex to search permissions looking into their contents and not only by their internal references.
* [Change] Improvements on diff process to avoid include already existent permissions.
* [Change] Improvement in diff process to avoid generate code for an unmodifiable object when its code doesn't differs from the same object in database.
* [Change] Added an option to DatabaseImportHelper to avoid the fk relationship updates. This will reduce the time to perform the import step in diff process.
* [Change] The diff is now capable to detect differences in functions source code and recreate them.
* [Change] Minor enhacement in DataManipulationForm to show the query time when retrieving data.
* [Change] Minor adjustments on tooltips of buttons in SQLToolWidget and DatabaseExplorerWidget.
* [Change] Minor size adjustment in ColumnWidget.
* [Change] Minor improvement on SyntaxHighlighter to optionally use the same tab size as NumberedTextEditor.
* [Change] Minor improvement on BaseObjectWidget to avoid install event filter in QPlainTextEdit and NumberedTextEditor.
* [Change] Replaced the QPlainTextEdit instance for source code input in FunctionWidget by a NumberedTextEditor instance.
* [Change] Minor message adjustments on SQLExecutionWidget.
* [Change] Minor adjustment on relationship invalidation message in ModelValidationWidget.
* [Change] Removed unused code from ModelValidationHelper.
* [Change] If a 1:1 or 1:n relationship is removed from the model the receiver table will have its fk relationships updated. This is useful to recreate fk relationships that were replaced by the removed relationship.
* [Change] Minor forms adjustments to resize command buttons depending on the size of texts.
* [Change] More improvements in the diff process when dealing with foreign keys creation.
* [Change] Changed the characters used to specify function call or any unescaped values in DataManipulationForm from <> to {}
* [Change] When using snippets in the SQL execution field the current code will not be cleaned up, instead the snippet will be appended to the current code.
* [Change] Removed the automatic view switching when saving the model.
* [Change] Minor adjustment on buttons positions at NewObjectOverlay.
* [Change] Minor message update in MainWindow::saveModel.
* [Change] pgModeler now indicates the name of unsaved models before quit.
* [Fix] Fixed a regression in trigger drop action in DatabaseExplorerWidget.
* [Fix] Fixed a severe bug that was not configuring the connection correctly when adding a new SQL input field from the current browsed database in SQL tool. The bug could cause user to manage a different database other than the one desired.
* [Fix] Fixed the "Find" button tooltip in SQLExecutionWidget
* [Fix] Minor fix when showing system objects in ModelObjectsWidget.
* [Fix] Fixed the view's SQL generation trimming the SQL that defines it to avoid differences between the model's view and the one generated after export. This will cause less divergences in when diff'ing the model and database.
* [Fix] Minor fixes in the *::getAlterDefinition() methods do avoid crashes due to null objects handling.
* [Fix] Fixed a crash when generating SQL code for recursive views.
* [Fix] Minor fix to correclty show the temporary models save progress at the bottom of main window.
* [Fix] Minor fixes in the validation process to force graphical objects updates and object's tree updates to reflect the new ids.
* [Fix] Minor fixes in the object naming. Now pgModeler will accept dollar signs in any portion of the string or even numbers as object's name but this will automatically quote the name to avoid errors.
* [Fix] Fixed the generation of DROP commands for triggers and rules.
* [Fix] Fixed a bug in Index and IndexWidget that was permiting btree index elements to have sorting attributes which is not valid according to PostgreSQL rules.
* [Fix] Fixed a bug in CodeCompletionWidget that was not retrieving objects with quoted names.
* [Fix] Minor fix in DataManipulationForm to clear the changed rows list after save the modifications.
* [Fix] Fixing the tab index in generalconfigwidget.ui.
* [Fix] Added a workaround to avoid crashes and leaks related to relationship disconnection and validation.
* [Fix] Minor fix in ModelsDiffHelper to avoid diff generation errors related to the missing 'fk-defs' attribute.
* [Fix] Fixed a crash when trying to create a new foreign key after connect two tables using a 1:1 or 1:n relationship.
* [Fix] Translated the pt_BR (Brazilian Portuguese) word found in the code.
* [Fix] Minor fix in BaseObject to permit the usage of swapObjectsIds method from ModelWidget class.
* [Fix] Fixed a bug that was duplicating some foreign key creation code in diff process.
* [Fix] Fixed a bug in the diff process that was dropping columns linked to sequences when these ones were dropped.
* [Fix] Fixed a bug when disabling table's SQL code from model widget. The FK constraints are now enabled/disabled correctly.
* [Fix] Fixed a bug in import process that was wrongly prepending schema's name in types related to tables.
* [Fix] Minor fixes in doxygen.conf.
* [Fix] Fixed the feature to convert to sequence a serial column in order to diminsh breaking references between the column's parent table and the newly created sequence.
* [Fix] Spelling fixes in es_ES (Spanish) UI translation.
* [Fix] Fixed a bug that was not setting up the object's schema correctly when creating new table or view inside a selected schema.
* [Fix] Fixed a bug in DatabaseModel::storeSpecialObjectsXML that was causing crashes when closing a model.
* [Fix] Fixed the model loading from recent list in order to expose the "fix model" message box in case of errors.
* [Fix] Minor adjustment on the generated diff code to include foreign keys definitions at the end of the script.
* [Fix] Minor adjustment on ui-style.conf to minimize the problems with dark themes.
* [Fix] Fixed a problem with validation that was trying to validate foreign keys without need.
* [Fix] Fix a bug that was preventing "deferrable" attribute for constraint triggers to be used in SQL definition.

v0.8.1
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: July 30, 2015*

* [New] Added the ability to create objects from within the object selectors to shorten up the time spent to create a new objects in the model. The only exception of for selectors in SwapObjectsIdsWidget and table/column selectors in ViewWidget.
* [New] Added a new method OperationList::isObjectRegistered to check if an object is registered in the list.
* [New] Created a test class for SyntaxHighlighter.
* [Change] Code optmizations done in SyntaxHighlighter removing duplicated or unused code.
* [Change] Minor adjustments on splitters at DatabaseExplorerWidget and SQLExecutionWidget.
* [Change] Changed the way the s_objectAdded signal is captured in ModelObjectsWidget::selectObject in order to work properly in Windows.
* [Change] Removed unnecessary message box in BaseObjectWidget.
* [Change] Changed the connection mode of the procedures to clear the operation list when a fix is applied in the model by the validation.
* [Change] More improvements related to the operation list management when editing/creating objects through forms.
* [Change] Improvements on BaseObjectWidget when canceling the creation/edition of objects and there are chained operations in the operations list.
* [Change] Additional improvements on TableWidget removing unneeded connection to signals.
* [Change] Improvement in TableWidget to register a new table in the operaions list after the user create a dependency object (schema, tag, role, etc) to avoid undo/redo errors.
* [Change] Additional improvements on ObjectSelectorWidget and ModelObjectsWidget to automatically set focus on the created object.
* [Change] Enhanced the generated view code. Now it should be more readable.
* [Fix] Fixed a bug related to unlogged tables exporting.
* [Fix] Fixed a small bug in SyntaxHighlighter that was not highlighting properly new lines and text after multline comments.
* [Fix] Fixed a bug that was preventing comments to be properly quoted (issue #710)
* [Fix] Fixed the SyntaxHighlighter constructor call throught the classes.
* [Fix] Minor fixes on xml-highlight.conf
* [Fix] Minor fix in the import process to remove quotes from enum type attributes to avoid erroneous modification of attributes when diff a model and database.
* [Fix] Fixed the disabled close button on editing forms (Windows only)
* [Fix] Fixed a bug in the object selector that was preventing the selector dialog to be shown when user click in the input field.
* [Fix] Fixed a regression in method DatabaseModel::getObject(QString,ObjectType) when dealing with operator class or family.
* [Fix] Fixed the configuration of Quick menu for relationship added objects. The disable/enable sql action is not visible anymore.
* [Fix] Fixed possible memory leaks when creating new objects through editing forms.
* [Fix] Fixed a bug on the object name validation that was permitting user to specify object names containing only numbers.
* [Fix] Fixed a bug when generating ENUM types without enum attributes.
* [Fix] Fixed the generation of DROP command for trigger and rules.
* [Fix] Fixed a crash when closing the model. This crash was related to removing permissions from special objects and right after destroy the remaining permisisons.
* [Fix] Fixed the method Permission::getSignature to avoid generating the same signature for different objects (permissions must be unique).

v0.8.1-beta1
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: June 30, 2015*

* [New] Objects can be now renamed in database explorer except for databases and casts.
* [New] Added an initial Spanish (es_ES) UI translation (review needed).
* [Change] Dropped the startapp script in Mac OSX. All executables now are able to run without explicitly use that script.
* [Fix] Fixed the snippet related to object renaming in snippets.conf.
* [Fix] Fixed a small bug in schema parser that was ignoring the usage of '$oc' metacharacter in some cases.
* [Fix] Fixed shortcut for delete command in data manipulation form.
* [Fix] Fixed the deployment script on Mac OSX to make the CLI and crash handler find the core libraries properly.
* [Fix] Fixed a bug that was generating ALTER ROLE commands without the semicolon.

v0.8.1-beta
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: June 04, 2015*

* [New] Added the ability to handle databases in different connections at once without the need to disconnect from a server and connect to another.
* [New] Added an option to preserve database name (do not rename) in diff process.
* [New] Added a numbered code editor in ModelDatabaseDiffForm.
* [New] Created a basic structure to start building unit tests based upon Qtest for new features and fixes.
* [New] Added an automatic quotation mechanism for PostgreSQL's reserved keywords when they are begin used as object's name.
* [New] Created the function createNumberedTextEditor in PgModelerUiNS to be used as a factory of NumberedTextEdit instances.
* [New] Added a font preview widget in GeneralConfigWidget.
* [New] Added the ability to configure line numbers font/bg colors and highligt color in GeneralConfigWidget.
* [New] Created a class ReadOnlyItemDelegate to avoid the edition of items in data grid / tree widgtes.
* [New] Added a donate widget in MainWindow.
* [New] Created a new class NumberedTextEdit which goal is to display source code with line numbering and highlight color.
* [New] Created a html based delegate item in order to render tree items with html format.
* [New] Added a fallback value for environment values not set during startup.
* [Change] Minor improvement on ConnectionsConfigWidget when user left a connection open for editing and don't save it. The software will ask for save the connection.
* [Change] Improvements done to DataManipulationForm in order to avoid let connections open leading to application crash when the connection timeout is reached.
* [Change] Added a new constructor to Connection class that accepts a map of connection params.
* [Change] Removed the installation of PGMODELER_TMP_DIR environment variable in installer.
* [Change] Removed the usage of TEMPDIR variable in pgmodeler.pri
* [Change] Moved the code to create temp dir to Application class.
* [Change] Temporary folder now is isolated in pgModeler's configuration dir at user's home (platform specific) to avoid a user to see temp files from another.
* [Change] Removed the "stay on top" behavior on ModelOverviewWidget to avoid the blocking of application dialogs is some systems.
* [Change] Minor improvement on DataManipulationForm when creating the window title based upon the connected database.
* [Change] Disabled the cascade drop for roles and tablespaces (not supported) in DatabaseExplorerWidget.
* [Change] Minor improvement on SQLToolWidget to remove the excessive usage of buttons to connect and browse databases.
* [Change] Removed the "connect" button from ModelDatabaseDiffForm. Now the databases are listed when the connection is selected in combo.
* [Change] Removed the "connect" button from DatabaseImportForm. Now when selecting a connection the databases will be listed.
* [Change] Minor improvement on DataManipulationForm to load the first 100 rows of the current table each time the index of combobox changes.
* [Change] Moved the database drop button from SQLToolWidget to DatabaseExplorerWidget. Now each instance of the latter class emits a signal to SQLToolWidget to perform the db drop.
* [Change] Removed the mothod ModelsDiffHelper::setDiffOptions and create setDiffOption and a set of OPT_xxx constants to access the diff options.
* [Change] Minor improvement on SQLToolWidget to force focus of the current SQLExecutionWidget instance.
* [Change] Replaced the QPlainTextEdit instance in SnippetsConfigWidget by NumberedTextEditor.
* [Change] Added instances of NumberedTextEditor to CustomSQLWidget to be used as appended/prepended code input.
* [Change] Changed the default font size of source code editors to 10pt.
* [Change] Minor size adjustments on aboutwidget.ui and donatewidget.ui
* [Change] Improved the windeploy.sh to build binaries in x86 and x64 at once.
* [Change] Minor improvement on linuxdeploy.sh to build demo and full versions at once.
* [Change] Avoiding the copy of ui-style.conf during startup.
* [Change] Attributes and general functions in namespaces are now declared as extern in .h and defined in .cpp to force them to be initialized once.
* [Fix] Minor fix on windeploy.sh when building all releases at once.
* [Fix] Fixed the demo version build error.
* [Fix] Fix a crash when recreating Views that have child objects (rules or triggers) in DatabaseModel::storeSpecialObjectsXML.
* [Fix] Fixed a typo when displaying column default values as nextval(seqname).
* [Fix] Fixed a bug on DatabaseExplorerWidget that was selecting the wrong database when opening the data grid.
* [Fix] Fix a bug on DataManipulationForm that was preventing user to delete rows in tables with no primary key.
* [Fix] Minor signal message correction in DatabaseImportHelper.
* [Fix] Minor hint text correction in ModelDatabaseDiffForm.
* [Fix] Fix a bug on diff process that was not detecting precision/length changes in column types.
* [Fix] Fixed a bug in CLI that was not considering the PWD when dealing with input/output files.
* [Fix] Fix a crash in PgModelerCLI when exporting model to PNG.
* [Fix] Minor fix in SyntaxHighlighter to apply font changes in live objects instead of only apply font changes in new objects (requires the use of rehighlight())
* [Fix] Fixed a bug on SyntaxHighlighter that was rehighlighting endlessly a document leading to application crash.
* [Fix] Minor adjust on pgmodeler executable to find libs at runtime (Mac OSX).
* [Fix] Fixed a crash when creating a new table containing a new index.
* [Fix] Fixed a bug that was permitting to set precision/scale to a user-defined type.
* [Fix] Minor fix on catalog query that retrieve types. Now types derivated from materialized views are not listed.
* [Fix] More fixes related to inherited columns and check constraints. Now diff infos containing modification for these kind of objects are discarded.
* [Fix] Fixed the import of inherited check constraints.
* [Fix] Minor fixes on DatabaseImportForm, ModelDatabaseDiffForm, ModelValidationWidget to use the html item delegate.
* [Fix] Minor fix on model export and validation progress.

v0.8.1-alpha1
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: April 20, 2015*

* [New] Added the ability to import objects from an existent database to a currently working model.
* [New] Improvements on DatabaseImportHelper to dump the objects attributes in debug mode or to the log file when "ignore errors" is checked.
* [New] Added a fix step in CLI to fix functions signatures that includes OUT parameters.
* [Change] Minor adjustment on model validation progress and output.
* [Change] Minor adjust on model export progress on CLI.
* [Change] Minor improvements on objects validation process when dealing with broken relationship config as well with FKs referencing a column created before it.
* [Change] Minor improvement on object selector widget to show the constraint name correctly.
* [Change] Change in user's configuration copy process. Now only configuration files are copied, folders are not copied anymore.
* [Change] The CLI now references template configuration files instead of user's when configuring file association.
* [Change] The settings related widgets now references template configuration files in order to load defaults or save settings.
* [Change] Improved the way threads are handled in ModelDatabaseDiffForm.
* [Change] Changed the way thread is used in ModelValidationWidget. Now it'll be created in each time the "Validate" button is clicked.
* [Change] Removed unused actions in MainWindow.
* [Change] Removed sleepThread method from ModelExportHelper, ModelValidationHelper and DatabaseImportHelper.
* [Change] Disabled the tabChangeFocus property of sql input field in SQLExecutionWidget.
* [Change] Minor adjust on tag objects positioning.
* [Change] Fixed the generation of function's signature in Function. Now OUT parameters aren't included.
* [Change] Fixed the generation of function's signature in DatabaseImportHelper. Parameters signaled with 't' in their modes aren't included.
* [Change] Restored the old behavior of Connection class to append only error code as extra info in exceptions to avoid break other features that need that code.
* [Fix] Added a proper quotation to the command used to trigger the crash handler.
* [Fix] Minor fix when displaying the row amount for the selected table on database explorer.
* [Fix] Added a patch into swap id dialog when dealing with broken relationships.
* [Fix] Minor fix on relationship line configuration when redrawing them during the relationship validation process.
* [Fix] Minor fix when drawing views with extended attributes.
* [Fix] Minor fix when enabling/disabling zoom buttons when the current zoom reaches the minimum or maximum values.
* [Fix] Minor fix on thread operations on Windows system.
* [Fix] Fixed the export to SQL and PNG in thread mode.
* [Fix] Added missing breaking points in ModelsDiffHelper to help the thread to shutdown.
* [Fix] Fixed the UI hang up when exporting a huge model by using BlockingQueueConnection in some slots to avoid thread consume all CPU resources.
* [Fix] Fixed a endless loop in PgModelerUiNS::formatMessage.
* [Fix] Minor fix in the generation of ALTER TABLE .. ADD COLUMN command.
* [Fix] Fixed current tab index in AboutWidget.
* [Fix] Minor fix on SwapObjectsIdsWidget to invalidate the database forcing the validation process.
* [Fix] Fixed the operator name validation.
* [Fix] Adding missing flag icons in AboutWidget.

v0.8.1-alpha
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: April 02, 2015*

* [New] Added a "Contributors" section in "About pgModeler" dialog.
* [New] Introduced the NO_UPDATE_CHECK variable in qmake to turn off update verification code specifically for package maintainers usage.
* [New] Generated installers has the ability to install .dbm file association (Windows and Linux).
* [New] Introduced a new env var PGMODELER_TMPL_CONF_DIR to override the template configuration location.
* [New] The "plugins" folder is created automatically at startup if does not exits.
* [New] Added the ability to show original SQL code, dependencies and children's code for test purposes in source code preview dialog.
* [New] Added the method DatabaseModel::getCreationOrder(BaseObject *) that retrieves all objects needed to create a certain object.
* [New] Added an action to save SQL code to file in source code preview dialog.
* [New] Added an option to list indirect refereces to an object in ObjectDepsRefsWidget.
* [Change] Minor adjust on Windows installer script.
* [Change] Deployment scripts on all platforms now uses PostgreSQL 9.4 and Qt 5.4.1 by default.
* [Change] Replaced float data types to double in libobjrenderer and libpgmodeler_ui classes to avoid lost of precision mainly when handling graphical objects.
* [Change] File association procedures were moved to CLI. Now the user can install/remove file association by using '--dbm-mime-type' option (Windows and Linux).
* [Change] Minor adjustments in PgModelerCLI menu.
* [Change] Removed unused variables and commented code throughout the code.
* [Change] The build process now uses libxml2 from PostgreSQL installation (Windows).
* [Change] Minor adjustment on how the duplicated elements are removed from lists in the methods __getObjectDependencies(), __getObjectReferences(), findObject() in DatabaseModel class.
* [Change] Removed uneeded "using namespace" statements.
* [Change] Adjustment on start-pgmodeler.sh script to use pgmodeler.vars as source of needed environment variables (Linux).
* [Change] Improvements on how objects are recreated using the "recreate unmodifiable" option on Diff process.
* [Change] Enhanced the control of database explorer widgets and the SQL execution panes related to them.
* [Change] Added a clear error message when required fields are not set when creating/updating object.
* [Change] Installation folder/files arrangement reverted to previous settings in order to avoid "DLL entry point errors" errors (Windows).
* [Change] Minor change in pgmodeler.pri to set default output paths according to FSH standard (Linux).
* [Fix] Minor fixes and adjustments on the deployment script and installer configuration file (windows).
* [Fix] Minor fixes when dealing with CLI and crash handler calls inside code (MacOSX).
* [Fix] Minor fix on range based loops using auto keyword.
* [Fix] Fixing invalid shebang in shell scripts (Linux).
* [Fix] Fix tab order in ConnectionsConfigWidget.
* [Fix] Minor fix when resizing HintTextWidget instances.
* [Fix] Fixed a bug when importing constraint triggers.
* [Fix] Minor bug fix when dropping table children objects in database explorer.
* [Fix] Minor fix when generating XML code for permissions.
* [Fix] Minor bug fix on database explorer at manage view to avoid left opened connections.
* [Fix] Added a patch in model fix process to correctly move indexes/triggers/rules from within tables to outside their xml definition.
* [Fix] Fixed a bug when configuring encoding for database. Now the "Default" value can be used normally.
* [Fix] Fixed a bug on model fix process that was removing empty lines (only with breaks) from functions definitions as well from other objects.
* [Fix] Fix crash when converting a serial column which is not assigned to a primary key.

v0.8.0
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: February 28, 2015*

* [New] Added support for multiple SQL execution widget instances for the same browsed database in SQL tool.
* [New] Added truncate table actions on DatabaseExplorerWidget.
* [Change] Minor adjustments on ModelValidationHelper.
* [Change] Minor adjustments on CustomSQLWidget.
* [Change] Included the delete cascade action to Edit menu in MainWindow.
* [Change] Minor widget adjustments on ModelDatabaseDiffForm.
* [Change] Minor improvements when saving temp models. Now the saving thread will not run if the diff/export/import dialogs are focused avoiding (rare) race conditions.
* [Change] Improved the update notifier to display a recover link and purchase link on the "Get binary package" button menu.
* [Change] Minor adjustment on output icons at ModelExportForm.
* [Change] Improvement on DatabaseModel::getObjectReferences to retrieve indexes as references to columns. This solve the bug related to import and diff processes that was causing detached columns to be dropped even if there were indexes referencing those columns.
* [Change] Added a more friendly error message when the user try to undo/redo an invalid operation at operations history.
* [Change] Minor improvement on ConnectionsConfigWidget adding the ability to make the configured initial database to be auto browsed when using the connection to manage objects on Manage view.
* [Fix] Fixed the output of SQL commands on diff, import and export. The commands now does not comes without original line breaks.
* [Fix] Fixed unexpected dialog blockings and form resetting on diff and export dialogs when minimizing and restoring the application.
* [Fix] Fix a crash when converting a serial column to sequence in which the first is not assigned to a primary key.
* [Fix] Minor fix on crash handler startup. Now exceptions occurred during the process are printed to stdout.
* [Fix] Fix a crash when pasting objects right after closing the source model (from where the objects were copied/cut).
* [Fix] Minor fix on ModelWidget::showObjectForm to correctly show permissions details.
* [Fix] Minor fix on the import process. Now the majority of problems related to objects that are created before their dependencies are solved.
* [Fix] Fixed some bug related to object duplication error treatment on ModelExportHelper.
* [Fix] Fixed a crash on connections config dialog when user removed a single connection and close the application, causing segmentation fault.
* [Fix] Minor fix on SQLToolWidget to avoid the disabling of SQL command input and controls when a database is dropped and there is at least one database being browsed.
* [Fix] Minor fix on session saving process.
* [Fix] Fixed a bug that was causing relationship points to be moved twice when multiple objects were selected on the canvas area.
* [Fix] Fixed some syntax errors on snippets.conf file.
* [Fix] Fixed a bug that was preventing global settings for relationships to be persisted.
* [Fix] Fixed a bug when importing permissions related to functions.
* [Fix] Minor fix on signal/slot connection order in NewObjectOverlayWidget.
* [Fix] Minor improvements on swap objects ids dialog.

v0.8.0-beta2
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: February 07, 2015*

* [New] Added the method Connection::getServerInfo that returns some informations about the connected server.
* [New] Added an attribute to DatabaseExplorerWidget to show the estimated rows amount for selected table.
* [New] Added the ability to cascade delete objects from database model.
* [New] Created missing getters and setters for Operation class.
* [New] Added the ability to set owner, schema and tag for several objects at once through the quick actions menu.
* [New] Added an option to diff process to reuse sequences if the source model has serial columns in which the generated sequence name matches a sequence's name on the imported model.
* [New] Added the support for per-user configuration. Now each user on the system will have his separated configuration folder.
* [New] Added a bug report form on main window to give user the chance to report a bug without use crash handler.
* [New] Added action to enable/disable an object's sql from quick actions menu at ModelWidget.
* [New] Created a new namespace PgModelerUiNS to store shared constants and function in libpgmodeler_ui subproject.
* [New] Added the ability to execute the DROP statements attached to object's SQL when exporting model to DBMS.
* [Change] The method PgModelerNS::formatString was moved to PgModelerUiNS::formatMessage.
* [Change] Simplified the layout of DataManipulationForm making the Advanced tab (filter) be moved to the same tab of result set facilitating the access to filtering features.
* [Change] Restored previous behavior of ModelWidget::cancelObjectAddition and ModelWidget::enableModelActions methods.
* [Change] Removed empty destructors from classes TableView and GraphicalView.
* [Change] Minor changes on destructors of classes that represents graphical objects on libobjrenderer to correctly undo the link between the graphical object and the source object.
* [Change] Improvements on ModelExportHelper adding the ability to ignore certain error triggered by PostgreSQL referencing their codes.
* [Change] Improvements on crash handler to reuse the code from bug report form.
* [Change] Changed the default PREFIX on pgmodeler.pri to /opt/pgmodeler when building on Linux.
* [Change] Several adjustments on deployments scripts to use the new build variable settings.
* [Change] Minor adjustments on main.pro, pgmodeler.pro and pgmodeler.pri files.
* [Change] Additional improvements on start-pgmodeler.sh and startapp.
* [Change] Crash/bug report files now have extensions .bug instead of .crash
* [Change] Removed the unused class Utf8String. This class was used in earlier versions of pgModeler and Qt 5x to fix some issues related to UTF8 string handling. After removed any reference of the class from code the issue seems fixed. In any moment this class can return if we have regressions.
* [Change] All literals throughout the code were replaced by QString() construction.
* [Change] Adjustments done in .pro file in order to correctly compile under Windows.
* [Change] Modifications done on .pro files that will permit custom output paths when building pgModeler from source, enabling it to be package to several linux distros.
* [Change] Moved the method disableReferencesSQL from BaseObjectWidget to PgModelerUiNS.
* [Change] Modifications done on DatabaseImportForm in order to be in the same standard as ModelExportForm and ModelDatabaseDiffForm.
* [Change] Minor improvements on ModelExportHelper in order to show the correct actions (commands) being executed.
* [Change] Improvements on ModelExportForm by including an output tab in order to display all actions taken during the export process.
* [Change] Adjustments on PgModelerCLI, ModelExportForm and ModelExportHelper to accept the "drop objects" option.
* [Change] Minor adjustment on ModelDatabaseDiffForm in order to lower the chances to crash the app if user try to repeatedly cancel and start over the diff process.
* [Change] Minor change on the generation of DROP statements attached to object's SQL.
* [Fix] Minor fixes on html formatted messages.
* [Fix] Fix on GeneralConfigWidget that was not saving code completion enabling status.
* [Fix] Fixed some bugs on libobjrenderer classes that was causing crashes in some models arrangements. Now graphical objects are effectively deallocated only when the whole scene is destroyed.
* [Fix] Minor improvement on OperationList::removeOperations to avoid crashes if a pool object is destroyed outside the operation history (e.g. relationship invalidation).
* [Fix] Several fixes on OperationList to minimize the crashes when undoing/redoing operations.
* [Fix] Minor fix on validation process that was failing sometimes to use temporary names feature.
* [Fix] Minor fix on ModelsDiffHelper to correctly recreate foreign keys that references recreated primary keys.
* [Fix] Minor fix on Table::removeObject to change not-null state of columns only when the removed object is a primary key.
* [Fix] Fixed a bug when converting many-to-many self-relationship and trying to undo the operation.
* [Fix] Fixed the query to retrieve the last_value field of sequences on DatabaseExplorerWidget.
* [Fix] Minor fix on CLI that was wrongly considering <dbmodel> tag attributes default-* as xml code for database objects causing errors on fix process.
* [Fix] Minor fix on diff process that was ignoring column's data type changes.
* [Fix] Minor fix on column SQL generation that was removing quotation on sequences names when using nextval() function call as default value.
* [Fix] Minor fix on ModelFixForm to correctly find the startapp script on MacOSX.
* [Fix] Fixed bug when import/diff user defined types that contains $ in the middle of their names. Now the names are quoted to avoid errors when referencing those types.
* [Fix] Minor fix when saving/restoring sessions. Now, empty sessions (without loaded files) are correctly saved.
* [Fix] Fixed a crash when importing a database in a second time after some error has occurred previously.
* [Fix] First version of a fix to solve problems related to inheritance diff/import. Now pgModeler does not try to drop inherited columns on diff process.
* [Fix] Fixed a crash when destroying constraints stored on operation history.
* [Fix] Fix on the import process to correctly identify and create inherited columns that aren't linked to constraints or other objects. For those referenced by other objects they will be created as detached columns.
* [Fix] Fixed a bug on rule.sch catalog query.
* [Fix] Minor fix on generation of DROP commands.
* [Fix] Fixed some catalog query schema files and the Catalog class itself in order to correctly select/discard objects linked directly or not to extensions.
* [Fix] Minor fix on generation of commands related to extensions. According to the rule, extensions names can't be schema qualified.

v0.8.0-beta1
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: January 10, 2015*

* [New] Created the widget SnippetConfigWidget in order to handle SQL snippets on SQLTool and DatabaseExplorerWidget.
* [New] Introduced a new expression %unset on SchemaParser to clear the named attributes.
* [New] Added support for permission on objects Type and Domain.
* [New] Added the method SchemaParser::extractAttributes that is capable to extract the attributes used in a loaded buffer.
* [New] Added methods to handle snippet selection in DatabaseExplorerWidget and SQLToolWidget.
* [New] Created the hint text for code completion option on GeneralConfigWidget.
* [New] Added an option to enable/disable code completion widget on sql code input fields (linked to CodeCompletionWidget instances).
* [New] Created a method Connection::getConnectionId that returns a string to identify that connection.
* [New] Created the constant GlobalAttributes::SNIPPET_CONF to store the default name for code snippet config file.
* [New] Added some basic code comments on DatabaseExplorerWidget.
* [New] Created the global constant SQL_HIGHLIGHT_CONF_PATH and XML_HIGHLIGHT_CONF_PATH on GlobalAttributes in order to avoid code repetition when loading syntax highlight conf files.
* [New] Added a method to parse rule commands on Catalog.
* [New] Added the method Catalog::getObjectAttributes to retrieve object attributes by its oid.
* [New] Introduced a new step on model validation: relationship configuration checking.
* [New] Added "properties" action on context menu at DatabaseExplorerWidget.
* [New] Added a panel related to object's properties on database explorer widget.
* [Change] The operation history is erased whenever a fix is applied to the model by the validator.
* [Change] Minor change on ColorPickerWidget in order to change color of pickers using stylesheet instead of QPallete.
* [Change] Minor adjustments on AppearanceConfigWidget.
* [Change] Minor copyright update on source files.
* [Change] Renamed the instruction %define to %set at SchemaParser.
* [Change] Removed unused methods from Messagebox.
* [Change] Minor improvements on Messagebox class by adding a simple show() method that receives only trhee parameters.
* [Change] Minor improvements on SchemaParser::translateMetaCharacter()
* [Change] Minor fix on DatabaseExplorerWidget::handleSelectedSnippet()
* [Change] Removed duplicated code to handle snippets on SQLToolWidget.
* [Change] Methods setIgnoreEmptyAttributes and setIgnoreUnkownAttributes at SchemaParser were renamed to ignoreEmptyAttributes and ignoreUnkownAttributes.
* [Change] Minor improvements on ModelNavigationWidget to show the filename associated to a hovered item on combobox.
* [Change] Simplified the syntax for attributes on .sch files from @{attrb} to {attrib}.
* [Change] Updated all .sch files with the new attribute syntax.
* [Change] Minor improvements on configuration widgets in order to detect if the settings was changed, this will avoid the unecessary load/save of the configuration files in ConfigurationForm.
* [Change] Replaced ternary ifs like "1" : "" by ParsersAttributes::_TRUE_ : ""
* [Change] Extensive changes in BaseConfigWidget and its subclasses in order to remove the use of extern keyword on other libs when there is the need to get the current config params. Now, user must call * [class]::getConfigurationParams() where class must be one of the derivates of BaseConfigWidget.
* [Change] The extern pointer configuration_form on mainwindow.cpp was moved into MainWindow class as a subwindow of it.
* [Change] Improvement on BaseConfigWidget and its children classes in order to use polymorphism in ConfigurationForm when needing to save or load confs.
* [Change] Moved the constants PGSQL_VERSION_xx from SchemaParser to a namespace called PgSQLVersions in order to avoid the "static initialization order" problem.
* [Change] Additional constants were created on PgSQLVersions namespace: ALL_VERSIONS and DEFAULT_VERSION.
* [Change] Improvement on XMLParser::hasElement method in order to permit user to check if the current element has a child of the specified XML node type.
* [Change] Minor improvements on BaseConfigWidget.
* [Change] Moved the method DatabaseModel::getObjectType(QString) to BaseObject as a static one.
* [Change] Minor updates on sql-highlight.conf
* [Change] Updated the version info on platform specific installer files.
* [Change] Removed unused constructor on Connection class.
* [Change] Minor improvement on DatabaseImportForm::listObjects, now it's possible to include a database root item.
* [Change] Minor improvements on class DatabaseImportHelper.
* [Fix] Fixed a bug that was crashing application when using special pks in relationshps.
* [Fix] Minor fix on windeploy.sh to copy the correct libs.
* [Fix] Minor fix on macdeploy.sh in order to use Qt 5.4.
* [Fix] Fixes done in order to build pgModeler on Windows using Qt 5.4.
* [Fix] Fixed a bug on SchemaParser that was wrongly reading %define instructions inside if's and creating attributes when it was not need to create them.
* [Fix] Modifications done on attributes initialization at RelationshipWidget in order to fix some crashes. Now pgModeler runs without crashing when compiled in release mode using g++.
* [Fix] Minor fix on aggregates listing on Catalog class. Now the handled types are attached to aggregates' names.
* [Fix] Minor fix on config pages indexes on ConfigurationForm.
* [Fix] Minor fix on MainWindow when there are errors during configuration files loading.
* [Fix] Minor fix on object name formatting at DatabaseExplorerWidget.
* [Fix] Minor fix on SQLToolWidget to set the current database explorer instance properly.
* [Fix] Minor fix on rule catalog query.
* [Fix] Minor fix on table.sch schema file.
* [Fix] Fixed the object filter on database explorer.
* [Fix] Typo correction on ModelExportHelper.
* [Fix] Additional fix to remove g++ warnings.

v0.8.0-beta
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: November 29, 2014*

* [New] Introduced the first version of model database diff feature.
* [New] Created the method Constraint::isCodeDiffersFrom in order to correctly generate the xml code and compare it with another object's code.
* [New] Created the method PgModelerNS::formatString in order to avoid repeatedly replace special chars by html tags.
* [New] Added the method PgSQLType::isEquivalentTo in order to check if a type is equivalent to another. This method helps to avoids unnecessary recreation of columns or type attributes on diff process.
* [New] Created the class DatabaseExplorerWidget in order to permit user manage several database instances on the sql tool.
* [New] Enabled the usage of cached catalog queries on Catalog class in order to increase the performance of the whole import process.
* [New] Added the method PgSQLType::isRegistered that indicates if a type is already registered or not.
* [New] Added the method BaseObject::acceptsAlterCommand to indicate if an object type can be changed through ALTER commands.
* [New] Added the method BaseObject::acceptsDropCommand to be able to identify which kind of object accepts DROP commands.
* [New] Added the method Column::getAlterDefinition in order to get the ALTER definition based upon the differences of two columns.
* [New] Added the ability to schema parser to create attributes from within the parsed schema files or buffer.
* [New] Added the method Table::getTruncateDefinition to return the proper TRUNCATE command.
* [New] Added method Sequence::getAlterDefinition.
* [New] Added method Role::getAlterDefinition.
* [New] Added method Index::getAlterDefinition.
* [New] Created the getAlterDefinition method for Extension and Function.
* [New] Created the custom implementation for getSignature() for Aggregate, Cast, Constraint, Index, Operator Class, Operator Family, Rule, Trigger.
* [New] Created the own versions of getDropDefinition for Function and Index.
* [New] Added to schema parser the ability to evaluate simple comparison expressions in the form (@{attribute} [operator] "value").
* [New] Added a special attribute pgsql-ver when generating code definition to store the currently used pgsql version.
* [New] Introduced a new method BaseObject::getSignature() that returns by default the object's name (formatted or not) and with the schema name prepended (when available).
* [New] Introduced a new name pattern PK_COL_PATTERN which is used to generate the single pk column name for many-to-many relationships.
* [New] Added method getAlterDefinition for Aggregate, Collation and EventTrigger.
* [New] Created a styled text box object.
* [New] Created the new class RoundedRectItem in order to generate rounded corner rectangle items on scene.
* [New] Added a sql disabled info item for relationships.
* [New] SQL disabled status are now displayed as a textbox on top of tables and schemas. Other objects will continue to have their names striked out to denote the deactivation of SQL code.
* [New] Added the ability to copy text from validation widget output.
* [Change] Minor change on Connection class. Added the method Connection::setSilenceConnError in order to silence connection errors in certain cases.
* [Change] Major changes on sql tool. Now it's possible to manage several database instances on the same server connection.
* [Change] Moved the code that was handling database objects tree from SQLToolWidget to DatabaseExplorerWidget
* [Change] Improvements on DBMS export process. Now the process detects when a object is being created, changed or dropped returning the correct message to the user.
* [Change] Minor fix on Role::getAlterDefinition.
* [Change] Removed the generation of inheritance command from Table::getAlterDefinition and moved to Relationship::getInheritDefinition.
* [Change] Minor update on sql highlight configuration file.
* [Change] Minor adjustments on schema files for CREATE commands.
* [Change] Minor update on sql-highlight.conf.
* [Change] Several changes on schema files related to comment command formatting.
* [Change] Removed the attribute ParsersAttributes::DIF_SQL due its deprecation.
* [Change] Improvements on all schema files that makes use of @{pgsql9x} attribute. Those attributes were deprecated and replaced by comparison expressions.
* [Change] The method SchemaParser::storePgSQLVersion was removed due to the deprecation of attribute @{pgsql9x}.
* [Change] Minor change on the Connection class to return the full version or only the major one of the server.
* [Change] Code blocks where there was a special treatment to get object's name filtering by object's type (specially function and operator) were replaced by a single call to BaseObject::getSignature().
* [Change] Improved the way many-to-many relationships can be configured. Now the generated table can have a single column as primary key or a multi-valued one.
* [Change] The ModelWidget::convertRelationshipNN method was improved in order to correctly convert many-to-many relationships with single or multi-valued primary key.
* [Change] Removed the restriction from Catalog and DatabaseImportForm to hide the "postgres" database.
* [Change] Minor change on how drop command is generated for collations.
* [Change] Minor fix on fields which accepts expressions to show scroll bars as needed.
* [Change] Changed the order of actions on left control bar at main window.
* [Change] Minor improvement on export to png process. The output image is generated with a margin.
* [Change] Minor change on sample models and asset images.
* [Change] Minor changes on TextboxView.
* [Change] Changed the style of graphical resentation for schemas, views, tables. Now they are drawn with rounded borders.
* [Change] Minor improvements on how sql disabled info item is generated and managed.
* [Change] Change the relationship cardinality pattern from (1,n) to 1:n.
* [Change] Minor label adjustment for many-to-many relatinships.
* [Change] Minor adjustments on textbox resizing.
* [Change] Minor adjustments on relationships custom points descriptors in order to give better selection and movement.
* [Change] Minor improve when moving a schema object. Relationship points will be moved together.
* [Change] Minor editing forms size adjustments.
* [Change] Changed the layout of modeldatabasediffform.ui and added hint text widgets.
* [Change] Changed the hint texts on general config, relationship config and model restoration form.
* [Change] Minor change on relationship editing form, by default random line colors are enabled.
* [Change] Minor fix on temporary models saving process.
* [Change] Explicit hint texts were moved to the instances of the new class HintTextWidget.
* [Change] Minor improvement on database editing form. LC_COLLATE and LC_CTYPE can be freely modified.
* [Change] Minor improvement on primary key constraints. Columns added to them will be marked as not-null by default. This is done to avoid false-positive changes on the model db diff process.
* [Change] Improvements on main window in order to give more visibility to SQL tool and central widget. Now there are three different views (welcome, design and manage).
* [Fix] Minor fixes throught the code in order to remove g++ warnings.
* [Fix] Minor fix on table's SQL schema file.
* [Fix] Minor fixes on demo version code. Remove the execution time limit and increased the maximum objects count.
* [Fix] Fixed a bug when generating XML code for materialized views that was causing these objects to break DTD rules.
* [Fix] Minor fix on code generation of permissions.
* [Fix] Minor fix on permissions editing form.
* [Fix] Fixed a bug on database import process that was crashing the application whenever importing a composite type.
* [Fix] Minor fix on reverse engineering process when importing columns which reference user defined type that are in pg_catalog.
* [Fix] Minor bug fix on database import process that was causing some crashes.
* [Fix] Minor improvements on "move to schema" feature on model widget. The references to moved object are now correctly updated.
* [Fix] Minor fix on SyntaxHighlighter that was not correctly applying the default font to the parent object.
* [Fix] Minor fix on index class and editing form to accept FILLFACTOR no matter the indexing method used.
* [Fix] Minor fix when removing table children objects and restoring them from operations list.
* [Fix] Fixed a crash when reverse engineering a model. The crash was due to trying to handle a not existent graphical object for pg_catalog schema.
* [Fix] Fixed the resizing of schema objects.
* [Fix] Minor fixes on schema editing form.
* [Fix] Additional fixes for export model to png. Now the correct bounding rect is calculated.
* [Fix] Fixed a bug when exporting model to png that was exporting unnecessary blank areas.
* [Fix] Fixed the click on the object selector input field. When the control is disabled the object selection dialog will not be opened.
* [Fix] Minor fix on domain class and editing form that was not resetting the constraint name attribute.
* [Fix] Minor fix on model validation helper to avoid include the database as a referer of other objects.
* [Fix] Fixed a bug when renaming objects and invalidating their references.
* [Fix] Fixed a bug on schema parser when converting chars to xml entities.

v0.8.0-alpha2
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: September 30, 2014*

* [New] Added an option on general settings to disable ask to validate model before save, export and diff.
* [New] If the user try to save, export or diff a model and it is invalidated, pgModeler will first validate and then proceed with the pending operation.
* [New] Added an entry on general settings to control how graphical objects are created.
* [New] Added an option to disable render smoothness to improve performance on large models.
* [New] Added the method Catalog::getObjectsOIDs in order to retrieve all database objects oid and store them in maps. This method is used as an auxiliary for model-database diff process.
* [New] Added the method ConnectionsConfigWidget::fillConnectionsComboBox() in order to reuse the code that fills up a combobox with configured connections.
* [New] Added the ability to create special primary key on many-to-many relationships.
* [New] Added the ability to save the dock widgets configuration on the main configuration file.
* [New] Added methods to retrieve objects by their names on DatabaseModel class.
* [Change] The configuration form was reestrucured decreasing the size occupied on the screen.
* [Change] Minor improvement on model fix dialog.
* [Change] Minor improvement on constraint import. Fillfactor attribute now is correcly retrieved.
* [Change] Removed extern directives referencing configurations from SyntaxHighlighter and ModelWidget converting the used attributes to static ones.
* [Change] Changed the way threads are created, as well the import and diff helpers instances to avoid race conditions and crashes.
* [Change] Schemas by default will be created to show the bounding rectangle.
* [Change] Minor improvement on BaseObject::getTypes() in order to exclude some types from the resulting vector.
* [Change] Minor widgets adjustment on modeldatabasediffform.ui, modelexportform.ui and databaseimportform.ui.
* [Change] Updates on several resource images.
* [Change] Minor change on example.dbm file.
* [Change] Minor change on DatabaseImportHelper to accept a DatabaseModel instance instead of ModelWidget.
* [Change] Minor adjustments on deployment scripts.
* [Fix] Fixed issue with Inno Setup invalid bitmap error (Windows).
* [Fix] Fixed a bug that was preventing encrypted password to be configured for roles.
* [Fix] Fixed a crash whenever user quit the application on Mac OSX.
* [Fix] Minor fixes in order to compile using Qt 5.3.2.
* [Fix] Fixed a bug when starting the creation process of relationships on model widget.
* [Fix] Fixed a bug related to individual permission exclusion and object removal error when there are permissions attached to it.
* [Fix] Fixed typos on demo version warning messages.
* [Fix] Fixed a bug that was not generating FILLFACTOR attribute for constraints.
* [Fix] Minor fix on object naming rules. Now the dollar sign ($) is accepted in the middle of object's name.
* [Fix] Fixed a bug when import domains which have the same name as some tables.
* [Fix] Fixed a bug on trigger class and editing form that was preventing the "FOR EACH ROW" attribute to be saved.
* [Fix] Fixed a infinite loop on operation list class when calculating the chain size.
* [Fix] Fixed a crash when creating relationships. Apparently this crash was caused by a faulty access on some threads right after close the relationship dialog.
* [Fix] Fixed the bug that was preventing a sequence to be assinged to a column.
* [Fix] Fixed a bug on relationship validation process that was causing errors mainly related to generalization relationships.
* [Fix] Fixed a bug on xml parser when reading DTD files from paths that contains accentuated chars.
* [Fix] Minor fix on Messagebox class when replacing <br> tags on message text.
* [Fix] Minor fix on conditional compilation macros entries for demo version.

v0.8.0-alpha1
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: August 15, 2014*

* [New] pgModeler now is capable to handle table's data through the new data manipulation form.
* [New] The SQL tool received a basic find/replace widget.
* [New] Created a new environment variable for samples directory, PGMODELER_SAMPLES_DIR.
* [New] Source code dialog now optionally appends the SQL of permissions for the current object.
* [New] Added an option to the source code privew dialog to include table's children objects' SQL.
* [New] Added a field on all object's editing form to expose the object's internal id.
* [New] Added an option on the database import process to generate random colors for relationships.
* [New] The model objects widget gained a filtering field that is capable to list objects by their name or internal id.
* [New] Added support for custom SQL for rules, indexes and triggers.
* [New] Added two new sample models.
* [Change] Changed the shortcut for "About pgModeler" to F4 key.
* [Change] Minor update on shortcuts and tooltips of buttons on bottom control bar at main window.
* [Change] Minor improvement on model validation. Now the DDL executed for any object is shown on the output field.
* [Change] Minor change on object's id interval generation.
* [Change] Minor adjust on model export process to force the code cache invalidation when using the unique name generation option.
* [Change] Message box objects are now used on demand and no more as class attributes.
* [Change] The software startup was hugely improved by removing singleton dialogs from main window's constructor, now they are used on demand.
* [Change] Minor update on sql-highlight.conf file.
* [Change] Sample models can be now loaded direclty from the main windows's central widget.
* [Change] Enhancement on the messages of model validation output.
* [Change] Minor fix on object's schema files to correctly generate the appended/prepended SQL code.
* [Change] Major improvement on all core classes (resposible to represent database objects) in order to keep their cache syncronized with the current configuration.
* [Change] Object finder now highlights the parent table if the user selects a child object of the first.
* [Change] Minor improvement on CLI. Added a routine to change a table structure, moving the tags rule, index and trigger to outside of it.
* [Change] Changed the way on how the relationship connection points are determined. Now they depends on how the tables' center points are distant from each other.
* [Change] More improvements done on database model objects classes in order to give more speed on the code generation.
* [Change] Update for French (fr_FR) translation.
* [Chage] Minor improvements on swap objects ids dialog.
* [Change] Huge improvement on validation process mainly for reverse engineered models. Now pgModeler honors the imported structure and in a few cases there will be inconsistencies.
* [Change] The objects rule, index and trigger will have the SQL/XML code generated outside of table's definition due to validation process that sometimes needs to swap id's of those objects.
* [Change] The object selector will trigger the object selection if user click the input field.
* [Change] Changed the 'line color' attribute of relationships to 'custom color'.
* [Fix] Fixed a crash provoked by the constraint editing form when switching the constraint type on the second time the form is opened.
* [Fix] Minor fix on database import process that was wrongly checking all tables as unlogged.
* [Fix] Minor fix on sample models pagila.dbm and usda.dbm to remove the unlogged attribute.
* [Fix] Minor fix on class CodeCompletionWidget to make it persistent as well to remove duplicate items from listing.
* [Fix] Minor fix on message box instances to resize according to the height of the current message.
* [Fix] Fixed a crash whenever the user cancelled the changes on settings and tried to connect to a server using the SQL tool.
* [Fix] Minor fix on SQLToolWidget to disable controls in some situations.
* [Fix] Minor fix on BaseRelationship and Relationship classes to correctly invalidate envolved object's when connecting or disconnecting.
* [Fix] Fixed a crash when closing the last model and the object finder was visible with result list filled.
* [Fix] Object finder now doesn't show duplicated items.
* [Fix] Minor fix on model objects widget when assigning a null model.
* [Fix] Fixed the problem with function parameter name not being generated due to cached code.
* [Fix] Minor fix on index editing form layout.
* [Fix] Fixed the import of rule object on DatabaseImportHelper class.
* [Fix] Fixed a bug when displaying the cardinalities for fk relationships.
* [Fix] Fixed a bug on the reverse engineering process that was wrongly migrating 'timestamp with time zone' data type as 'timestamp' only.
* [Fix] Fixed a bug on the reverse engineering process that was preventing the migration of operator classes / indexes that contains duplicated elements.

v0.8.0-alpha
------
*Codename: <strong>Faithful Elephant</strong>*
*Release date: July 21, 2014*

* [New] Added support for using global settings for relationships on the editing form of those objects.
* [New] A new section was created on settings dialog to manage global configurations for relationships.
* [New] Enabled the movement of schema objects without the need to select their children. This operation does not applies to protected or system schemas like public, pg_catalog.
* [New] Created a more elaborated central widget with some basic operations like create, load, load recent models and restore previous session.
* [New] Reletionships 1:1, 1:n and fk now supports a new style of link between tables. The foreign key columns will be directly linked to primary key columns.
* [New] Created a class called ColorPickerWidget to handle operations where the user need to configure colors for objects.
* [New] Relationship objects now supports custom line colors.
* [New] Added the static methods setPgSQLVersion and getPgSQLVersion on BaseObject class in order to override the PostgreSQL version used to generate code for all instances of that class.
* [New] Introduced a method DatabaseModel::getXMLParser() in order to return the xmlparser of the model to permit user create new objects from xml code within the current database model.
* [Change] Minor adjustment on central widget buttons style on MacOS X.
* [Change] Disabled the collapsed main menu on MacOS X due to particularities on GUI of this system.
* [Change] Minor adjustment on main menu construction on Linux/Windows.
* [Change] Major change on main UI style in order to make a cleaner interface and gain more space on canvas to handle models.
* [Change] Minor model objects style refreshment.
* [Change] Minor optimization on class AppearanceConfigWidget by adding color pickers in order to select color of the elements.
* [Change] A significant code optimization was made on TagWidget by changing the tool buttons that was handling color configuration by color pickers.
* [Change] Added a color picker on RelationshipWidget to permit the configuration of custom line color.
* [Change] Minor layout adjustment on update notifier widget.
* [Change] Important change on SchemaParser class which is not a singleton anymore in order to minimize crashes due to race conditions on running threads.
* [Change] Important change on XMLParser class which is not a singleton anymore in order to minimize crashes due to race conditions on running threads.
* [Fix] Fixed a bug related to partial command execution on SQL tool.
* [Fix] Minor fix on model navigation widget when closing a model.
* [Fix] Fixed a bug that was crashing pgModeler when switching models with the overview widget opened.
* [Fix] Fixed a bug that was eventually making pgModeler enter on a endless looping when calculating the chained operations length.
* [Fix] Fixed a regression related to special objects recreation after rename a primary key column. The problem was that the relationship was wrongly storing the old name of the columns at connection time.

v0.7.2
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: June 27, 2014*

* [New] Added the missing attribute BUFFERING for Gist indexes.
* [New] Added an experimental installer for Linux based upon the Qt Installer Framework.
* [New] Added an file called RELEASENOTES.md. This file will be read by the update notifier on server in order to return to the GUI the change log and additional release info.
* [Change] Minor change on MacOS X deployment script to generate the dmg file with a custom icon.
* [Change] Restored the start-pgmodeler.sh file due to difficulties that some users are having when running pgModeler on Linux systems.
* [Change] Created the folder "installer" to store all files related to installers of all platforms
* [Fix] Fixed a regression related to sequences not being included on database model XML code.
* [Fix] Fixed deployment on Linux. Now start-pgmodeler.sh is correclty copied.
* [Fix] Fixed the display of relationship actions on NewObjectOverlayWidget

v0.7.2-beta1
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: June 21, 2014*

* [New] Added support for event trigger objects. The export and import processes were adjusted to handle this kind of object.
* [New] Added support for UNLOGGED tables.
* [New] Enabled PostgreSQL 9.4 export and import processes.
* [New] Enabled the jsonb datatype for PostgreSQL 9.4.
* [Change] Rolled back to Qt 5.2.1 the version used by the macdeploy.sh script due to crash reported in issue #494 and #482.
* [Change] SQL tool improvement. Added a "drop cascade" action and fixed a bug when refreshing a specific item on database tree.
* [Fix] Minor fix on ColumnWidget, disabled the line breaking in the field "Expression".
* [Fix] Fixed the model fix dialog on MacOS X and Windows platforms.
* [Fix] Schemas names are now correctly striked out on the rectangle when their SQL code is disabled. The system schemas public and pg_catalog will continue not to have their names striked out.
* [Fix] Fixed a bug that was preventing default values of domains to be loaded from file.

v0.7.2-beta
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: June 06, 2014*

* [New] Added an option to convert integer type to serial ones on columns which default value is a nextval(seq::regclass) call.
* [Change] Changed the MacOS X deployment script to use Qt 5.3.
* [Change] Minor size adjustment on the new object overlay.
* [Change] The model objects widget now clears the selection when it lost the focus.
* [Change] Minor change on new object overlay behavior. The overlay will be hidden when any tool button on it is pressed or the ESC key is pressed.
* [Change] The partial database import was improved. Now pgModeler is capable to resolve dependencies of foreign key constraints, creating the needed tables automatically. Additionally, domains are automatically imported when referenced by columns.
* [Fix] Fixed compilation process on Windows system.
* [Fix] Fixed the crash handler call inside the main program.
* [Fix] Minor fix on "object's deps. & refs." dialog that was triggering an error when trying to open a table child object from there.
* [Fix] Added a entry on DatabaseModel::getObjectDependecies() to get the sequence that a column depends on.
* [Fix] Fixed a bug column editing dialog that was wrongly removing the sequence assigned to a column.
* [Fix] Fixed sample plugins compilation when using a LIBDIR value other than the default.
* [Fix] Fixed the build number generation at compilation time.
* [Fix] Minor fix on TableTitleView class to correctly strikeout the schema name only when the schema rectangle is not visible.
* [Fix] Minor adjustment on databaseimportform.ui size.
* [Fix] Fixed the custom page size bug due to patches provided by Qt 5.3. A conditional compilation statement was added on portions where the custom page size is used. This way the code can be build on any Qt version but the fix will available only on 5.3.
* [Fix] Minor fix on the Linux deployment script.


v0.7.2-alpha1
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: May 30, 2014*

* [New] Introduced a "new object" overlay widget which gives user a quick access to actions that create objects.
* [New] Added a step to fix indexes with old <condition> tag on command line interface.
* [New] Added support for item interaction on "object's dependencies and references" dialog.
* [New] Added support for generate temporary names for database, roles and tablespaces when running validation process. This will avoid errors if the original database and the other objects already exists on the server.
* [New] Updated the CLI to include the option to generate temporary object's names.
* [New] Added suppport to save and restore the last position and zoom on the canvas. This behavior can be deactivated on general settings.
* [New] Added support for prepend SQL commands on object's definition.
* [New] Added zoom info popup that appears whenever the user changes the current zoom factor.
* [Change] Renamed the methods setCheckExpression and getCheckExpression methods to setExpression and getExpression because the expression is used either for check and exclude constraints.
* [Change] Renamed the attribute "condition" to "predicate" on index class.
* [Change] Improved the way SQL code is disabled/enabled on editing forms. User now is asked to apply the same disabling/enabling status to object's refereces.
* [Change] Minor size adjustment on SQL append dialog.
* [Change] Changed the order of the tabs on user-defined type editing form.
* [Change] Minor appearance improvement on textbox editing form.
* [Fix] Made visible the field related to predicate expression on exclude constraint editing form.
* [Fix] Added reference checking between an exclude constraint and operators.
* [Fix] Minor fix on copy operation that was not resetting the copied objects list if the user clicked several times the "copy" command without paste.
* [Fix] Minor fix on the model object widget when disabling the controls.
* [Fix] Minor fix on ModelWidget::createSequenceForColumn() operation. The generated sequence is now correctly assigned to the column.
* [Fix] Minor fix on pgModeler command line interface menu text.
* [Fix] Minor fix on permission.dtd file. Attribute "index" was incorreclty marked as required when its optinal.
* [Fix] Minor fix on semantics of one-to-one relationship. When one of the tables are of mandatory participation the foreign key columns must not accept null values.
* [Fix] Minor fix on Windows and Linux deployment scripts.


v0.7.2-alpha
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: May 06, 2014*

* [New] Added a basic routine to check if there is some new version available on pgModeler site.
* [Change] Custom  indexes for columns and constraints added by relationships are now stored on tables. In previous version the relationship was the responsible for that but this approach was provoking the bug related on issue 449.
* [Change] Remove unused parser attributes and commented old code.
* [Change] Removed attributes and methods from relationship which were responsible to control columns, attributes and constraints indexes.
* [Fix] Fixed a bug that was changing the columns positions whenever a relationship was edited on the model.
* [Fix] Minor fix when generate the string that denotes the assigned constraints to a column.
* [Fix] Minor fix on function editing form when handling the result table columns. The variadic field is disabled on the parameter form.
* [Fix] Fixed a crash when removing a view linked to tables.
* [Fix] Minor fix on view reference code generation.
* [Fix] Minor editing form size adjustments.
* [Fix] Minor compilation fix on Windows system.


v0.7.1
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: April 15, 2014*

* [New] Added option to invert panning mode and range selection triggers.
* [New] Added support for use relationship attributes as special primary keys.
* [Change] Improvement on unique name generation for columns and constraints when connecting relatioships.
* [Change] Improvement on copy / paste operations.
* [Change] Minor workaround in order to try to fix the crash due to thread conflict mainly on Windows system.
* [Fix] Minor fix on custom columns positioning.
* [Fix] Input and output files are now correctly escaped on the model fix form and the process works fine.

v0.7.1-beta1
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: April 8, 2014*

* [Change] Minor change on project's description text on about dialog.
* [Fix] Workaround for the slow editing of function's definition. Disabled the automatic syntax highlighting.
* [Fix] Minor fix on reverse engineering process. In some cases the process was aborted due to duplication of relationships caused by an incorreclty name generation for this kind of object.
* [Fix] Minor fix on model objects widget when changing the visible object types.
* [Fix] Fixed the conflict with panning mode and graphical object addition operation.
* [Fix] Fixed a regression introduced by 0.7.1-beta on model fix process.

v0.7.1-beta
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: April 6, 2014*

* [New] Created a small interface to pgmodeler-cli that enables the user to fix a broken model inside pgModeler GUI.
* [New] Added support for assign a sequence as default value of a column. The sequence will be converted to "nextval('seqname'::regclass) and the validation process will check if the sequence is correctly referenced by the table that owns the column.
* [Change] Changed the default behavior of left click on blank areas of canvas. Instead of create a range selection the user will move the viewport (panning mode). To enable range selection user must press SHIFT and click/move to draw the selection rectangle.
* [Fix] Minor fix on connection class in order to accept empty passwords as well passwords that contains spaces.
* [Fix] Fixed the column listing on constraint editing form after remove one or more columns.
* [Fix] Fix a crash when canceling the model saving dialog on Windows.
* [Fix] Minor patch on the model fix process.
* [Fix] Fixed wrong default values for relationship added columns.
* [Fix] Fixed a bug related to file paths with ampersand on config files.
* [Fix] Fix the broken sql for inheritance when a child table don't have columns but only constraints.

v0.7.1-alpha1
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: March 29, 2014*

* [Change] Major model validation improvement. Now special objects are correctly validated.
* [Fix] Fixed a crash when closing a model that contains a view that references columns added by relationships.
* [Fix] Fixed wrong working directory handling on CLI.
* [Fix] Fixed a bug on file loading process that could left behind some objects depending on size and arrange of the loaded model.
* [Fix] Fixed the undesired behavior when moving a table to another schema.

v0.7.1-alpha
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: March 21, 2014*

* [Fix] Fixed connection config. Empty passwords are now accepted.
* [Fix] Fixed schema object code generation.
* [Fix] Fixed the usage of PGMODELER_SCHEMAS_DIR environment variable on import process.
* [Fix] Fixed "ALTER ... SET OWNER" DDL for materialized views.
* [Fix] Fixed duplicated semicolon at end of permissions defintion.

v0.7.0
------
*Codename: <strong>Brave Mastodon</strong>*
*Release date: February 25, 2014*

* [New] Addded a  catalog attribute "hide-postgres-db" in order to avoid listing "postgres" maintainance DB on import operations.
* [New] Added options to hide system/extension objects on SQL tool improving the object listing performance.
* [New] Added support for custom compilation output directory through qmake variables BINDIR, LIBDIR and RESDIR.
* [New] Added support for deferrable unique, exclude and primary key constraints.
* [New] Added support for custom colors on tables and views through tag objects.
* [New] Added support for export models to png image page by page.
* [New] Canvas can now be moved using Control + Arrow keys. If the shift is pressed the movement factor is increased.
* [New] Introduced the SQL tool that permits the execution of arbitrary SQL commands direclty on a server.
* [New] Added methods getType, getTypeId to BaseType and getSQLTypeName to PgSQLType as an alternative to call operators ~, ! and *.
* [New] Added a commented DROP command at start of each object definition (CREATE or ALTER TABLE ADD)
* [New] Added a "Code Preview" tab on permissions dialog.
* [New] Enabled SQL code visualization for FK relationships.
* [New] Added a build number on about dialog. This number is the compilation date in format yyyymmdd.
* [New] Added support for materialized and recursive views (PostgreSQL 9.3 feature).
* [New] Added pgModeler version information on generated sql scripts as well .dbm files for debugging purpose.
* [New] Added support for custom delete/update actions for relationship generated foreign keys.
* [New] Added support for move the canvas by positioning the mouse over corners.
* [New] Added a configuration parameter to control font style for any source code highlight field.
* [New] Added additional PostGiS types: geomval, addbandarg, rastbandarg, raster, reclassarg, unionarg, TopoGeometry, getfaceedges_returntype, validatetopology_returntype.
* [Change] Added support for on-demand updates on sql tool object's tree.
* [Change] Improved the tab navigation experience on editing forms.
* [Change] Minor change on SQL tool to ommit binary data values.
* [Change] Dropped the navigation through object using Alt + <left|right> due to the difficulty to understand the order in which objects are highlighted.
* [Change] Minor change when generate .stacktrace file for crash handler to include pgModeler build number.
* [Change] Minor adjustments on DatabaseImportForm's import execution progress.
* [Change] Minor enhancements on operation list when removing last operations.
* [Change] Minor enhancements on table and relationship dialogs on error control flow.
* [Change] Changed Z-value for relationship labels in order to avoid that name labels don't overlaps the cardinality labels.
* [Change] Removed the translation installing from within plugin loading method at PluginsConfigWidget.
* [Change] The Application class constructor now loads at once all translation files available for the current language including language file for plugins.
* [Change] Minor changes on deploy scripts on all platforms. The parameter '-with-build-num' was introduced in order to generate a package with build number.
* [Change] Relationship dialog enhanced. Now participant tables are described in what role they make part.
* [Change] Minor improvement on model export process.
* [Change] Minor improvement on model validation widget.
* [Change] Minor improvement on crash handler report generation message. Full path to crash file is now shown.
* [Change] Improved the message displayed when user try to save an invalidated model.
* [Change] Minor adjustment on model export dialog size.
* [Change] Minor improvement on model overview widget.
* [Change] Minor adjustments on window title buttons for model export and database import forms.
* [Change] Improvement on connection config form. pgModeler now ask to save/update unsaved connection if the user forgot to.
* [Change] Minor update sql syntax highlighting configuration file.
* [Fix] Fixed bug that was permitting paste already formatted text (html) on source code input fields.
* [Fix] Fix broken range type generation.
* [Fix] The DELETE privilege is now correclty saved on model.
* [Fix] Fixed drop object command on SQL tool.
* [Fix] Fixed bug that was crashing pgModeler when a error was raised on view edit form.
* [Fix] Fixed a bug that was crashing the application when deleting relationship attributes or constraints.
* [Fix] Fixed bug related to the range selection weird behavior when finishing creating a object.
* [Fix] Minor fix on OperationList undo/redo methods to update types names on tables that references a modified type.
* [Fix] Minor fix on View assignment operator to correctly rename the type associated with "this" object.
* [Fix] Minor fix on DatabaseModel to correctly return the references to a view type.
* [Fix] Fixed bug that was causing indexes/triggers that references columns added by relationship have the sql code generated twice.
* [Fix] Minor fix on ResultSet class to identify bytea columns.
* [Fix] Minor fix on CLI menu to add new export modes.
* [Fix] Fixed a crash dealing with duplicated columns on a table.
* [Fix]  Fixed bug when deleting tables and fk relationships together.
* [Fix] Fix bug related to geometry type.
* [Fix] Minor fix on logical expressions evaluation on SchemaParser.
* [Fix] Minor fix on model export when showing the name of objects being exported.
* [Fix] Minor fix on list/view advanced objects of a relationship.
* [Fix] Minor fix on form resizing when showing the protected object alert.
* [Fix] Fixed a minor bug that was crashing pgModeler when visualizing many-to-many relationships.
* [Fix] Fixed some warnings triggered by clang compiler.
* [Fix] Fixed a crash when loading plugins on MacOSX.
* [Fix] Fixed the issue related to import roles from database. pgModeler will not query pg_shadow anymore since this view is a very restricted object. Now role passwords will be imported as ***** (according to docs).
* [Fix] Fixed the object name validation. pgModeler now accepts spaces within names.
* [Fix] Fixed the function editing form resizing.
* [Fix] Fixed a bug that was not loading "sql disabled" state for relationships.
* [Fix] Fixed incorrect behavior of "Zoom In" action on MacOSX.
* [Fix] Trying to fix the infinite loop of the Validation confirm dialog on Windows (more tests needed).

v0.6.2
------
*Codename: <strong>Daring Mammoth</strong>*
*Release date: December 20, 2013*

* [Change] Update Qt version to 5.2.0 on build scripts (Windows only).
* [Change] Linux binaries are now bundled with all needed Qt libs.
* [Change] Important change on the way that special primary keys are created for generalization/copy relationships. Now there is the need to create the relationship first, close the dialog and open it again in order to generate the columns that will be used on the primary key.
* [Fix] Fixed a bug on model loading and relationship validation that was causing pgModeler to ignore some special objects (constraints referencing columns generated by relationships).
* [Fix] Workaround done on the sql append widget when handle a lot of code avoiding slowdowns on the syntax highlighting.
* [Fix] Fixed the incorrect creation of foreign keys on many-to-many relationships.
* [Fix] Fixed the conversion of self many-to-many relationships.
* [Fix] Fixed a bug that was causing some constraints to be destroyed when the relationship was connected to the table that owned the constraint.
* [Fix] Comments that contains apostrophes now are correctly escaped in order to avoid SQL related errors.
* [Fix] Fixed the incorrect generation of SQL code of check constraint associated to many-to-many relationshps.
* [Fix] Minor fix on crash handler when trying to read an stack trace file that doesn't exists.
* [Fix] Minor typos fixes on CLI menu.
* [Fix] Minor fix on the about form positioning.

v0.6.2-beta
------
*Codename: <strong>Daring Mammoth</strong>*
*Release date: November 29, 2013*

* [New] Added an option to drop database before a export process.
* [New] Disabling SQL code now disables the code of all referrer and child objects (experimental!).
* [New] Added support for columns to reference "table types".
* [Change] Object names are trimmed on editing forms to avoid unnecessary error triggering.
* [Change] Minor improvement on crash handler form.
* [Change] Minor change on SQL validation message.
* [Change] Minor improvement on operation list.
* [Fix] Fixed a crash when adding foreign keys to a new table.
* [Fix] Fixed the import of foreign key constraints from PostgreSQL 9.3.
* [Fix] Fixed the creation of fk relationships when the involved tables has too long names. pgModeler no more complains about "already existent object".
* [Fix] Minor fix when showing self fk relationships.
* [Fix] Minor fix on pgmodeler-cli: working directory is now set correctly.
* [Fix] Minor fix when retrieving advanced (generated) objects from relationships.
* [Fix] Fixed a bug that was not properly removing table objects when the user was canceling the table's editing.

v0.6.1
------
*Codename: <strong>Daring Mammoth</strong>*
*Release date: November 3, 2013*

* [New] PostgreSQL version 9.3 activated on code base. Now import and export operations works with this new version.
* [Change] Changed the way inheritance is created. Now the INHERIT command is appended in the table's definition.
* [Change] Update on model validation. Generalization and copy relationships have the participant tables' id's validated in order to check reference breaking.
* [Change] Version info upgraded on MacOSX app bundle configuration file (Info.plist).
* [Change] Minor change on "pgmodeler.vars". Included environment variables for custom Qt installation.
* [Fix] Fixed a bug related to INSTEAD OF/ON UPDATE triggers on views.
* [Fix] Fixed a bug related to incorrectly error raised when setting a owner table in the same schema as the sequece.
* [Fix] Fixed a bug related to importing sequences which name has uppercase characters.
* [Fix] Fixed misspelled "Connetion" word on configuration form.
* [Fix] Typos correction on model validation message box.
* [Fix] Fixed incorrect objects removal after cancel the edition.
* [Fix] Minor fix on disconnection of generalization relationships.
* [Fix] Minor fix on updating table's graphical representation when importing primary keys.
* [Fix] Minor change when displaying the columns' types on table/relationship editing form.
* [Fix] Fixed the compilation process on MacOSX 10.9 (Mavericks).
* [Fix] Minor change on macdeploy.sh to use Qt5.2-beta by default.

v0.6.0
------
*Codename: <strong>Daring Mammoth</strong>*
*Release date: September 30, 2013*

* [New] Added a validation when removing protected FK relationships.
* [New] Added a progress info (at bottom widgets bar) for temporary model saving.
* [New] User can now restore the last session via File > Restore Session. Sessions will not be restored at startup anymore.
* [New] Added a "zoom" option when exporting to PNG image.
* [Change] Disabled the model loading via command line on MacOSX due to bundle particularities.
* [Change] Remove option "Save session" from general config widget.
* [Change] Improved the way schema's children objects are selected/unselected.
* [Change] Improved the printing operation. Now custom paper size has a separated field to assign it's coordinates.
* [Change] The import errors now are written on a log file when "ignore import errors" is checked.
* [Fix] Fixed an inconsistence when removing a table before the fk relationship linked to it. From now on (to avoid crashes) user must remove the relationship first and then remove the table.
* [Fix] Fixed a minor bug on column's graphical representation that was incorrectly configuring the column descriptor for self-relationship fk's.
* [Fix] Minor fix on model overview widget when showing large models.
* [Fix] Fixed bug on pgmodeler-cli that was generating errors when running it outside the executable's directory.
* [Fix] Fixed the calculation of pages to be printed.
* [Fix] Fixed the type enumeration validation to accept space on the names.
* [Fix] Minor fix for GiS types. Spatial auxiliary type name can null.
* [Fix] Minor pgmodeler-cli typos corrections.
* [Fix] Fixed a bug related to XMLParser and threads that was crashing pgModeler on Windows.
* [Fix] Fixes on DatabaseImportHelper to correctly handle extension created types.
* [Fix] Minor fix on PgSQLType::parseString() when creating datatypes from strings.

v0.6.0-beta
------
*Codename: <strong>Daring Mammoth</strong>*
*Release date: September 16, 2013*

* [New] Added experimental reverse engineering support.
* [New] Added an experimental option --fix-model to pgmodeler-cli to permit the user to fix the structure of an older model (generated in pgModeler < 0.6.0) or a corrupted file.
* [New] Added an option to debug the import process printing any generated code to stdout.
* [New] Added support for bidirectinal FK relationships.
* [New] Added a statement "SET search_path [schemas]" on database model SQL code.
* [New] Added missing PostgreSQL built-in types.
* [New] Configured connections now can be duplicated in order to reuse it's attributes.
* [Change] Minor change on main compilation script. The subproject "tests" are included only when compiling in debug mode.
* [Change] Major change on validation widget. Fixes are now applied using a thread and can be aborted any time user want.
* [Change] Change on model saving process. pgModeler will not deny to save invalidated anymore. Now it will ask the user to validate the model or save an invalidate one anyway.
* [Change] Changed the behavior of the operation list. In the first exception the entire list are emptied.
* [Change] Changed the way foreign keys are generated. They always will be generated at end of database definition to avoid reference breaking.
* [Change] Minor improvements on model code generation and copy operations.
* [Change] Removed deprecated "rtree" indexing type.
* [Fix] Minor fixes on PgSQLType class. User types aren`t removed instead they are deactivated to avoid reference breaking.
* [Fix] Minor fix on selecting the children objects of a schema.
* [Fix] Minor fixes on scene and relationship avoiding crashes when destroying the whole graphical scene.
* [Fix] Fixed bug on deleting self relationships.
* [Fix] Minor fix on model export process. The last line of the SQL code now is correctly extracted and executed.
* [Fix] Minor fix on sequence class to accept owner.
* [Fix] Minor fixes on the splash screen control code.
* [Fix] Fixed a bug that was crashing pgModeler at startup.
* [Fix] Fixed a bug that was causing pgModeler to crash when loading operators which name contained '&' char.
* [Fix] Fixed bug related to sequence values assignment.
* [Fix] Fixed "Operation with not allocated object" error on applying validation fixes.
* [Fix] Fixed relationship label position saving.
* [Fix] Minor fix on main window. pgModeler now is not closed while the validation is running.

v0.6.0-alpha1
------
*Codename: <strong>Daring Mammoth</strong>*
*Release date: August 05, 2013*

* [New] Added catalog query for triggers.
* [New] Added catalog query for rules.
* [New] Added indexing method to exclude constraint.
* [New] Added catalog query for constraints.
* [New] Added catalog queries for tables and columns
* [New] Added catalog queries for view, domain, sequence and user defined types.
* [New] Added catalog query for collation.
* [New] Added catalog query for operator family.
* [New] Added catalog query for operator class.
* [New] Object dependencies form now can list indirect dependencies.
* [New] Added an environment variable to set a different location for crash handler executable.
* [New] Objects that has SQL disabled now is shown with name striked out.
* [Change] Minor change on MainWindow::closeEvent()
* [Change] Moved app_style variable to GlobalAttributes::DEFAULT_QT_STYLE.
* [Change] Minor improvement on Exception::getExceptionsText method.
* [Change] Improvements on copy/paste operations.
* [Change] Removed unused linker parameters.
* [Change] Crash handler executable renamed to "pgmodeler-ch".
* [Fix] Fixed possible leak when destroying a ModelWidget instance. Objects from  scene were not being deleted correclty. Fix tests in progress.
* [Fix] Fixed the "Save current session" option on GeneralConfigWidget that wasn't doing it's job correctly.
* [Fix] Fixed a bug that was crashing pgModeler at startup when restoring previous sessions or temporary models.
* [Fix] Minor fix on trigger code generation.
* [Fix] Fixed incorrect loading of multiple triggers/rules on views.
* [Fix] Minor fix on model validation. Operator classes are now checked during the validation process.
* [Fix] Fixed generation of constraints in form of ALTER command. In some cases the constraint code wasn't appended to table's definition.
* [Fix] Minor fixes on cast object.
* [Fix] Minor fixes on databasemodel on retrieving dependencies/references for objects.
* [Fix] Fixed crash handler path variable on MacOSX.

v0.6.0-alpha
------
*Codename: <strong>Daring Mammoth</strong>*
*Release date: July 19, 2013*

* [New] Added the widget to swap objects IDs on model validation form.
* [New] Added indexing type "spgist" to IndexingType class.
* [New] Model validation and export now works with threads.
* [New] Functions now have a "leak proof" attribute.
* [New] VARIADIC key attribute added to function parameters.
* [New] User now can completely disabled UI stylesheets by calling executable with param "-no-stylesheet".
* [New] Added the class Catalog to handle the basis of reverse engineering by reading the pgcatalog/information_schema.
* [New] System tablespaces pg_global and pg_default are now automatically created as new models are added.
* [Change] Minor change on schema area selection. The entire schema area and children can be selected (and moved) by using "Select children" action or SHIFT + left-click.
* [Change] Improved the transition between opened models. The problem was solved putting the temporary models saving in a separated thread.
* [Change] Minor improvement on model restoration operation. Models that fails to be restored can be kept so the user can try to fix them manually (until pgModeler is closed).
* [Change] Minor adjustments on model loading progress.
* [Change] Minor chages on model export form. Export to DBMS is the default option.
* [Change] Minor improvements on pgmodeler-cli. Added "--simulate" option to export dbms.
* [Change] Disabled notice output to console for Connection class. User can re-enable it by calling Connection::setNoticeEnabled() [recompile source needed].
* [Change] Minor changes on PgSQL base types. "[NO] LEAKPROOF" removed from FunctionType class.
* [Change] Minor improvements on SchemaParser, now its possible to make parser ignore empty attributes.
* [Change] libdbconnect renamed to libpgconnector for semantics reasons.
* [Change] Minor improvements on CrashHandler. Added buttons to load report and save embedded model when in analysis mode.
* [Fix] Minor fix on linuxdeploy.sh script related to grep command execution.
* [Fix] Fixed some leaks when destroying objects that are registered as PgSQLType (table, sequence, extension, domain, view, type). Now these type are correctly remove from user type listing.
* [Fix] Fixed bad sql code generation when disabling sql of columns/constraints.
* [Fix] Fixed dependency retrieving for operator classes.
* [Fix] Fixed incorrect reference to "replicate" option on Roles.
* [Fix] Fixed the "ignore duplicity" bug for columns when exporting model.
* [Fix] Fixed library build order now libpgmodeler is built before libpgconnector.
* [Fix] Minor fix on UI stylesheet related tooltips when using Fusion theme.
* [Fix] Fixed the assignment of LC_COLLATE, LC_CTYPE and template db to database instance on DatabaseWidget.


v0.5.2
------
*Codename: <strong>Lovely Duda</strong>*
*Release date: June 27, 2013*

* [New] User now can append free SQL commands to database objects via "Append SQL" command.
* [New] Introduced an experimental code completion on fields that permits code input.
* [New] User can create default sequences from serial columns. This feature does not apply to columns generated by relationships.
* [New] Introduced a feature to break relationship lines in straight angles and to remove all user added points.
* [New] Added support for change font size on textboxes.
* [Change] Removed the code "OIDs=FALSE" from table's SQL.
* [Fix] Minor fix on Mac OSX deployment script.

v0.5.1_r1
---------
*Codename: <strong>Lovely Duda</strong>*
*Release date: June 13, 2013*

* [New] Added deployment scripts on all platforms to compile and pack pgModeler. Note: On Windows the script must run on GNU environment port like Cygwin or MingW.
* [New] Added an special field to columns to easily identify which relationship generated it. (only for columns added by relationship)
* [Change] Model overview widget is now shown always stay on top.
* [Change] Minor improvements on syntax highlighter.
* [Change] Improvements on model validation widget. Output panel now shows the currently validated object (SQL validation).
* [Fix] Removed from user defined type DTD the mandatory use of collation object.
* [Fix] Collation's SQL generation corrected. Encoding is appended to LOCALE keyword.
* [Fix] Corrected the wrongly used COLLATION keyword on user defined type.
* [Fix] Corrected a bug that was crashing pgModeler when selecting relationship created objects on object finder result list.
* [Fix] Extension object naming corrected.
* [Fix] Extension object removal corrected.
* [Fix] Corrected a bug on CLI that was not finding dependencies paths correctly.
* [Fix] Minor fix on task progress widget on MacOSX.
* [Fix] Splash screen corrected on MacOSX
* [Fix] Corrected a bug on relationships that was crashing pgModeler when specifying column name pattern.

v0.5.1
------
*Codename: <strong>Lovely Duda</strong>*
*Release date: June 1, 2013*

* [New] Code base ported to C++11 and Qt5.
* [New] MacOSX compilation now generates an application bundle: pgmodeler.app
* [New] pgModeler is now capable of associate dbm files to its executable being possible opening a model from file manager by clicking it (except for MacOSX, see MacOSX notes).
* [New] Added support for loading models by calling pgModeler gui executable from terminal (e.g. pgmodeler model1.dbm model2.dbm)
* [New] pgModeler logo redesign.
* [New] Added special primary keys support for one-to-one and one-to-many relationships.
* [New] Relationships now supports patterns to define generated objects names. The manual suffix and auto-suffix generation are deprecated.
* [New] Columns/constraints generated by relationship can have position changed on parent table.
* [New] Added smallserial built-in datatype.
* [Change] Improvements on model validation tool. pgModeler will not save the model while isn't completely validated.
* [Fix] minor fix on point insertion on relationships.
* [Fix] Corrected the wrongly displayed interval fields.
* [Fix] Corrected self relationship validation.
* [Fix] Corrected a bug on editing numeric data types.
* [Fix] Minor fixes on build scripts (All platforms).
* [Fix] Added the missing directive CONFIG+=console to main-cli.pro
* [Fix] Minor fixes on task progress exhibition on quick tasks.
* [Fix] pgmodeler-cli output fixed on Windows system.
* [Fix] Table inheritance now copies columns with NOT NULL attribute correclty configured.
* [Fix] Plugin build scripts fixed. Now generated libraries are correctly copied to build directory.
* [Fix] Editing a column with 'numeric' datatype does not generate errors anymore.

v0.5.0
------

*Release date: May 17, 2013*

* [New] Complete main window restyling.
* [New] Added a model validation tool to prevent reference break and name conflicts.
* [New] Added an object navigation using keyboard on model widget. Pressing Alt + [left|right] keys will switch between graphical objects.
* [New] Introduced the pgmodeler-cli. A command line tool to handle model export without loading the graphical interface.
* [New] Added an option to list available configured connections on pgmodeler-cli.
* [New] pgModeler now alerts the user when he try to save an invalidated model.
* [New] pgModeler now aborts app closing when the user wants to do a last saving on modified models.
* [New] Added support for hide relationship labels and table extended attributes on configuration dialog.
* [New] Added "Recent Models" menu.
* [New] Introduced the Xml2Object plugin to help on develpment testings.
* [New] Added partial support for PostgreSQL Extensions objects.
* [New] Added JSON datatype.
* [New] Added support for rules and trigger on views.
* [New] Added support for user defined range types.
* [New] Added support for collations on composite types (user defined).
* [New] Added built-in range types.
* [New] Added support for INCLUDING/EXCLUDING options when dealing with copy relationships.
* [New] Added support for EXCLUDE constraint support
* [New] Added NO INHERIT option to check constraints.
* [New] Added REPLICATION option to roles.
* [New] Added FOR ORDER BY option and removed Recheck from OperatorClassElement.
* [New] Added collation support for index elements.
* [New] Added [NOT] LEAKPROOF key word to functions.
* [New] Added collation attribute to domains.
* [New] Required fields are now highlighted on editing forms.
* [New] pgModeler creates system objects (e.g, public schema and SQL, C, plpgsql languages) when adding a new model.
* [Change] Minor improvements on when showing Views.
* [Change] Relationship points are moved when the parent relationship is being moved together with other objects.
* [Change] Simplified the model loading operation. pgModeler will not try to recreate objects with unsatisfied dependencies instead errors will be raised.
* [Change] Minor changes on FK relationship creation.
* [Change] User-added foreign-keys had code generation changed.
* [Change] Minor improvements on PgModelerPlugin structure.
* [Change] DummyPlugin renamed to Dummy.
* [Change] Improvements on building process for all supported systems.
* [Change] Removed "Save widget positions" from configuration form.
* [Change] Removed fullscreen mode from main window.
* [Change] Removed unused/deprecated error messages.
* [Change] Removed deprecated files COMPILING.md and PLUGINS.md.
* [Change] Subproject libutil was renamed to libutils due to some conflicts on Linux systems.
* [Change] Startup scripts removed. Since the environment variables are set by the installer on Windows and for Unix the variables are set using the new pgmodeler.sh script.
* [Change] "Disable SQL code" option added for all types of objects. Except for textboxes and base relationships (view-table relatioships and fk relationships).
* [Change] Fixed permissions for views.
* [Change] PostgreSQL 8.x support completely removed.
* [Change] Schema files (for SQL code) aren't organized in folders anymore. All code (for different PostgreSQL versions) will be in the same .sch file for each object.
* [Change] Spatial types had SRID digit count upgraded to 5.
* [Change] One-to-one relationships now generates unique names for UNIQUE constraints.
* [Change] Several class improvements, performance tunings and forms readjustments.
* [Fix] Minor fixes on connection configuration form.
* [Fix] Corrected a bug that was crashing pgModeler when adding new schemas.
* [Fix] Corrected a bug that was crashing pgModeler when validation model.
* [Fix] Corrected a bug that was preventing the popup menu to be configured correctly on model widget.
* [Fix] Menu bar style correctly applied on Windows system.
* [Fix] Now relationship labels' position are restored when loading the model file.
* [Fix] Minor fixes on database model code generation.
* [Fix] Corrected the glicthy wheel scroll/zoom on model widget.
* [Fix] Corrected the visual update of schema's rectangle when adding a column on a child table.
* [Fix] Corrected a bug that was preventing a new model to be saved correctly.
* [Fix] Minor fixes on model widget copy/paste operations.
* [Fix] Models now are correclty auto saved when modified.
* [Fix] Corrected operator's signature generation.
* [Fix] Corrected a bug on textbox with unicode texts.
* [Fix] Index and Rule editing forms now handles correctly unicode expressions.
* [Fix] Corrected a bug that was avoiding the name "remembering" during relationship loading.

v0.4.1_r1
---------

*Release date: March 19, 2013*

* [Change] user can now prepend a CTE (commom table expression, a.k.a "with queries") on view's definition.
* [Change] user can now create a single reference containing a expression that defines the entire view.
* [Change] improvements on permissions, user now can control GRANTs and REVOKEs via permission editing form.
* [Fix] fixed invalid UTF-8 chars on function definition.
* [Fix] fixed unavailable "nocreatedb" role option.

v0.4.1
------

*Release date: March 16, 2013 *

* [New] introduced the  "Disable SQL code" option for roles/tablespaces.
* [New] user now can add objects by right-clicking group items on "Model Objects" dockwidget tree.
* [New] added the abbreviation for time and timesptamp data types both with timezone: timetz and timestamptz.
* [New] introduced a object highlight action on "model objects" dockwidget.
* [Change] major changes on SQL code generation/export. Introduced a token to help export process to identify the end of each DDL command.
* [Change] minor improvements on role editing form.
* [Change] when generationg XML code empty tags that stores pure texts are now created with a <![CDATA[]]> tag in order to avoid malformed xml code.
* [Change] index FASTUPDATE and FILLFACTOR params is now activated according the indexing type.
* [Change] index fill factor now is optional.
* [Change] chinese, portuguese and french translations update.
* [Fix] pgModeler no longer crash when in error state (showing an exception) and try to auto save the models.
* [Fix] minor size adjustments on forms.
* [Fix] corrected a bug related to one-to-many relationship validation (endless looping) when changing to automatic suffix generation.
* [Fix] corrected the "apply button disabled" bug on constraint edit form.
* [Fix] IN/OUT keywords now appears on functions signature.
* [Fix] corrected translation bypassing on index edit form.
* [Fix] pgModeler no longer crash when triggering the print action.
* [Fix] triggers no longer complains about assigning a function without parameters.
* [Fix] corrected the loading process for indexes.
* [Fix] corrected some bugs related to GiST and index sorting.
* [Fix] minor fix on quick rename action when renaming a column with primary key.
* [Fix] corrected a bug that was causing pgModeler to complain about duplicated elements when loading indexes.
* [Fix] corrected a bug related to main window title when save a model with a different filename.
* [Fix] fixed a bug related reload a model file after editing a foreign key.
* [Fix] corrected a bug related to invalid chars at task progress.

v0.4.0_r1
---------

*Release date: March 04, 2013 *

* [New] introducing the "pgModeler Wiki" as the main project's support resource.
* [Fix] when main windows is closed the overview widget is closed too.
* [Fix] corrected a bug on operation list widget that was converting an item name to UTF-8 twice.

v0.4.0
------

*Release date: February 27, 2013 *

* [New] introduce a "New object" submenu when activating the schema context menu (right-click)
* [New] tables and view are now graphically separated by colored rectangles representing its schemas.
* [New] compiling pgModeler now works perfectly on Mac OSX system.
* [New] introduced the 'Quick actions' menu that permits: rename, move to another schema, change onwer and edit permissions.
* [New] the relationship editing form gained an "advanced" tab which shows the objects generated and/or represents the relatioship itself.
* [New] the user now can add relationships only creating foreign keys on tables (fk relationships).
* [New] added a french UI translation (provided by [toorpy](https://github.com/toorpy)).
* [Change] all relationships type are now grouped together on "Model objects" widget.
* [Change] chinese UI translation updated (provided by: [Bumanji](https://github.com/Bumanji)).
* [Change] user now can remove fk relationships directly without needing to remove the related foreign keys.
* [Change] field semantics adjustments on relationship editing form.
* [Change] graphical object can be now selected and have the context menu activated only with a single right-click.
* [Change] minor improvements on plugin base class: PgModelerPlugin.
* [Change] widget size adjustments to better showing on Mac OSX system.
* [Change] crashhandler now shows the compiled and running versions of Qt.
* [Change] french UI translation reviewed and updated (provided by [babs](https://github.com/babs)).
* [Change] 'Objects of Model' when used as object picker now expand all the nodes by default.
* [Change] 'Objects of Model' now memorizes the tree state when update an object and / or opening another model.
* [Change] PostGiS 'geometry' type can have a free assigned SRID value.
* [Change] editing forms when shown set the focus on the first field, generally, the object name.
* [Change] 'Objects of Model' widget displays the nodes in alphabetical order.
* [Change] the printing options for the model were moved to the general configuration form.
* [Change] relationship validation method now removes fk relationships when the foreign keys that gerenates is no longer exists.
* [Change] copy/cut/delete commands does not manipulates system objects like schema public and languages C, SQL and plpgsql.
* [Change] pgModeler startup scripts are now path location free meaning that software can be installed where the user desires.
* [Fix] corrected a bug related  constraint name on domain XML code generation.
* [Fix] corrected a bug that was causing crash when click "Apply" on Type editing form with fields not filled.
* [Fix] corrected the "invalid constraint name" error on domain editing form.
* [Fix] corrected the empty DEFAULT clause for columns, types and domains.
* [Fix] corrected a bug related to incorrectly initialized OID attribute when creating tables.
* [Fix] corrected a bug when creating a view with WHERE statement.
* [Fix] corrected a bug related to one-to-many relationships semantics.
* [Fix] corrected some bugs that was causing crash when removing all operations from operation list.
* [Fix] minor bug fixes related to object selection over the model.
* [Fix] corrected a bug on load model dialog filter (chinese UI only).
* [Fix] pgModeler no longer crash when editing objects style.
* [Fix] corrected bug that was deleting two sequeces at once.
* [Fix] pgModeler no longer crash when removing (disconnecting) relationship that has special primary keys.
* [Fix] minor fixes on the startup scripts on all platforms.
* [Fix] corrected an incorrect reference to output stream on Windows system.
* [Fix] shortcuts and popup menu now works correctly when selection an object on 'Objects of Model' tree.
* [Fix] the pgsql base types (represented by tables, sequences, user defined types and domains) are now updated correctly when the related schema is renamed.
* [Fix] corrected some weird SRID value on non spatial types.
* [Fix] corrected bug on objects table when move rows to last / first.
* [Fix] typos corrections on some error messages and dialog titles.
* [Fix] 'referenced columns' combobox on constraint editing form are filled correctly when the dialog is shown in a second time.
* [Fix] pgModeler no longer crash when creating many-to-many relationships.
* [Fix] pgModeler no longer crash when the user activates the print dialog.
* [Fix] corrected bug that was removing fk relationships when pasting objects.
* [Fix] corrected SQL syntax error of 'timestamp with time zone'.
* [Fix] corrected constraint type showing on editing form.
* [Fix] corrected bug on cyrillic typed enums and check constraints expressions.
* [Fix] corrected bug on enumeration type editing form.
* [Fix] corrected bug on 'truncate' table privilege code generation.
* [Fix] corrected column default value code generation.
* [Fix] dummyplugin build process corrected on Windows.
* [Fix] corrected bug on column comment code generation.
* [Fix] corrected bug that was deleting two tables at once.

v0.3.4
------

*Release date: October 17, 2012*

* [New] added chinese UI translation (provided by [gjunming](https://github.com/gjunming)).
* [New] added basic support for PostGiS 2.0 only data types: box2d, box3d, geometry and geography (suggested by [george-silva](https://github.com/george-silva) on [issue#28](https://github.com/pgmodeler/pgmodeler/issues/28))(EXPERIMENTAL). Note: when using these data types make sure that PostGiS extension is installed on database cluster since pgModeler WILL NOT install it automatically or generate the command to do it!
* [New] added a model restoration feature to reopen models after unexpected quit (crash).
* [New] added a crash handler to pgModeler. Now signal SIGSEGV is trapped (in most cases) and the crash handler pops up permiting the user to generate an error report. (EXPERIMENTAL)
* [New] to facilitate the error reporting exceptions stack now can be showed in text format. Users can post the complete error stack when creating an issue.
* [New] icon added to pgModeler executable (Windows only)
* [Change] update on pt_BR translation file.
* [Change] removed "pgmodeler" prefix from translation files.
* [Change] added the field "Underline" on textbox editing form.
* [Fix] corrected the "AlwayOnTop" bug on model overview widget. ([issue#30](https://github.com/pgmodeler/pgmodeler/issues/30))
* [Fix] little fix on startup scripts. Corrected de PGMODELER_ROOT on both Linux and Windows systems. ([issue#29](https://github.com/pgmodeler/pgmodeler/issues/29))
* [Fix] corrected the referece to environment variables PGMODELER_*. Now pgModeler search for necessary paths on current directory if some of these variables are not set.
* [Fix] corrected the validation of UTF-8 names that have 3 bytes length.
* [Fix] corrected the sources path reference on project (.pro) files. Now lupdate command do not generates empty TS files.
* [Fix] corrected a bug that was causing crash where user try to edit protected objects.
* [Fix] corrected the exhibition of UTF-8 messages on ```throw``` statements.

v0.3.3
------

*Release date: October 09, 2012*

* [Change] pgModeler license were update to GPLv3.
* [Change] Error massages and entire UI were translated to en_US. Now people can contribute more easily with translation files. [(issue#8)](https://github.com/pgmodeler/pgmodeler/issues/8)
* [Change] The left side image were removed form all forms giving more space to show widgets.
* [Change] pgModeler now shows a messagebox at startup if any critical error is raised instead to show them on stdin.
* [Fix] Translation files now are correctly loaded depending on system language. [(issue#23)](https://github.com/pgmodeler/pgmodeler/issues/23)
* [Fix] Compilation process and execution is working correctly on Windows system. [(issue#11)](https://github.com/pgmodeler/pgmodeler/issues/11)
* [Fix] No more crashes when dealing with relationships that have special triggers/indexes/columns. [(issue#8)](https://github.com/pgmodeler/pgmodeler/issues/8) [(issue#24)](https://github.com/pgmodeler/pgmodeler/issues/24)

v0.3.2
------

*Release date: September 27, 2012*

* [Change] The default extension for the models now stands for ".dbm" [(issue#9)](https://github.com/pgmodeler/pgmodeler/issues/9)
* [Change] Tables and sequences now can be used as function return type as well parameter type. This is valid for other objects that make use of base types (except for table columns).
* [Change] The relationship conversion command now need to be confirmed by the user.
* [Fix] Compilation process now works correctly on Windows system.
* [Fix] Adjusted the size of some forms to show their fields properly.
* [Fix] The "make distclean" command now make the correct cleanup on build/ directory.
* [Fix] Startup scripts "start-pgmodeler.(sh|bat)" where adjusted. To prevent errors pgModeler need to be started through these scripts.
* [Fix] Corrected the reference to the plugins directory. [(issue#7)](https://github.com/pgmodeler/pgmodeler/issues/7)
* [Fix] The action "New Object -> Tablespace" now is displayed properly.

v0.3.1
------

*Release date: September 18, 2012*

* [New] Relationships generates column suffixes automaticaly. This behavior can be changed on the relationship editing form.
* [New] Added two samples to pgModeler.
* [Change] Tables are now created with "With OIDs" attribute by default.
* [Change] The graphical update method on overview widget has improved preventing unecessary processing.
* [Fix] Class CenaObjetos now doesn't delete objects twice.
* [Fix] Eliminated bug that caused crashing on pgModeler when closing a model.

v0.3.0
------

*Release date: September 12, 2012*

* [New] Added a model overview widget.
* [New] Added export feature that generates PNG image of the models.
* [Fix] Corrected the naming of columns generated by many-to-many relationships.
* [Fix] Corrected generation of XML/SQL code by the model.

v0.2.0
------

*Release date: August 31, 2012*

* [New] Added an interface to implement third party plugins. Check [PLUGINS.md] (https://github.com/pgmodeler/pgmodeler/blob/master/PLUGINS.md) for details.
* [New] Added a short cut to easily control the zoom on the model. Use Crtl + Mouse wheel up (zoom up) or Crtl + Mouse wheel down (zoom down).
* [Change] Due to the plugin interface the compilation method changed back to the form of shared libraries + executable.
* [Fix] No more crashes when removing an primary-key of a table which has relationship with other tables. [(issue#2)](https://github.com/pgmodeler/pgmodeler/issues/2)
* [Fix] Adjusted the semantics of one-to-one relationships.

v0.1.2
------

*Release date: August 24, 2012*

* [New] Added a functionality to save modified models before closing the software.
* [Change] Updated the en_US dictionary with the texts of the above functionality.
* [Fix] Dockwidgets no longer disappear unexpectedly when the main window is minimized.
* [Fix] Operations performed before creating a table object (column, constraint, trigger, index, rule) are no longer removed when any exception is thrown in the creation of these object.
* [Fix] Fixed bug that caused user-defined types had wrong SQL/XML code generated by the model.
* [Fix] Functions and Types received an own range of id in order to create these objects in a correct way.
* [Fix] Eliminated segmentation faults caused by the destruction of relationships which possessed attributes/constraints.
* [Fix] Adjusted the translation to SQL code of one-to-one relationships.
* [Fix] Eliminated segmentation fault when editing relationships and/or undoing an operation involving a relationship.
* [Fix] Identifiers relationships now correctly display the thick line beside the weak entity.

v0.1.1
------

*Release date: August 14, 2012*

* [Fix] Correction of the actions for inserting graphic objects (table, text box, vision and relationship) in Windows environment.
* [Fix] Correction on the display of the maximize button in the window decoration in Windows environment.
* [Fix] Adjust on the position and spacing of widgets in editing forms.
* [Fix] The XML parser can now correctly read DTD files in Windows environment.
* [Fix] The compilation method is no longer in the form of shared libraries + executable and passed to be as standalone executable only.

v0.1.0
------

*Release date: August 9, 2012*

* First pgModeler release.
