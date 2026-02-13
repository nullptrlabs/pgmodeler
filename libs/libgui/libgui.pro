include(../../pgmodeler.pri)

TEMPLATE = lib
TARGET = gui

# Including the private assets when building private plugins
isEqual(PRIVATE_PLUGINS, true) {
	RESOURCES += $$PRIV_RES_FOLDER/logoicons.qrc
}

# Including the logo related resources
isEqual(PRIVATE_PLUGINS, false) {
  RESOURCES += res/logoicons.qrc
}

RESOURCES += res/resources.qrc

windows:RCC_DIR += src
windows:DESTDIR = $$PWD

# Enables shared library symbols exporting
DEFINES += GUI_SYMBOLS

# Include priv-core sources directly when building with private plugins
isEqual(PRIVATE_PLUGINS, true) {
	include(../../priv-core/priv-core.pri)
}

SOURCES += src/baseform.cpp \
	src/dbobjects/referenceswidget.cpp \
	src/dbobjects/simplecolumnswidget.cpp \
src/guiutilsns.cpp \
src/mainwindow.cpp \
src/messagebox.cpp \
src/pgmodelerguiplugin.cpp \
src/dbobjects/aggregatewidget.cpp \
src/dbobjects/domainwidget.cpp \
src/dbobjects/languagewidget.cpp \
src/dbobjects/textboxwidget.cpp \
src/dbobjects/basefunctionwidget.cpp \
src/dbobjects/elementwidget.cpp \
src/dbobjects/operatorclasswidget.cpp \
src/dbobjects/relationshipwidget.cpp \
src/dbobjects/transformwidget.cpp \
src/dbobjects/baseobjectwidget.cpp \
src/dbobjects/eventtriggerwidget.cpp \
src/dbobjects/operatorfamilywidget.cpp \
src/dbobjects/rolewidget.cpp \
src/dbobjects/triggerwidget.cpp \
src/dbobjects/castwidget.cpp \
src/dbobjects/extensionwidget.cpp \
src/dbobjects/operatorwidget.cpp \
src/dbobjects/rulewidget.cpp \
src/dbobjects/typewidget.cpp \
src/dbobjects/collationwidget.cpp \
src/dbobjects/foreigndatawrapperwidget.cpp \
src/dbobjects/parameterwidget.cpp \
src/dbobjects/schemawidget.cpp \
src/dbobjects/usermappingwidget.cpp \
src/dbobjects/columnwidget.cpp \
src/dbobjects/foreignserverwidget.cpp \
src/dbobjects/permissionwidget.cpp \
src/dbobjects/sequencewidget.cpp \
src/dbobjects/viewwidget.cpp \
src/dbobjects/constraintwidget.cpp \
src/dbobjects/functionwidget.cpp \
src/dbobjects/pgsqltypewidget.cpp \
src/dbobjects/tablespacewidget.cpp \
src/dbobjects/conversionwidget.cpp \
src/dbobjects/genericsqlwidget.cpp \
src/dbobjects/policywidget.cpp \
src/dbobjects/tablewidget.cpp \
src/dbobjects/databasewidget.cpp \
src/dbobjects/indexwidget.cpp \
src/dbobjects/procedurewidget.cpp \
src/dbobjects/tagwidget.cpp \
src/dbobjects/elementstablewidget.cpp \
src/dbobjects/columnpickerwidget.cpp \
src/settings/appearanceconfigwidget.cpp \
src/settings/configurationform.cpp \
src/settings/generalconfigwidget.cpp \
src/settings/relationshipconfigwidget.cpp \
src/settings/baseconfigwidget.cpp \
src/settings/connectionsconfigwidget.cpp \
src/settings/pluginsconfigwidget.cpp \
src/settings/snippetsconfigwidget.cpp \
src/tools/bugreportform.cpp \
src/tools/databaseimporthelper.cpp \
	src/tools/datagridwidget.cpp \
	src/tools/datahandlingform.cpp \
src/tools/modeldatabasediffform.cpp \
src/tools/modelfixform.cpp \
src/tools/objectsdiffinfo.cpp \
src/tools/sqltoolwidget.cpp \
src/tools/databaseexplorerwidget.cpp \
src/tools/modelexportform.cpp \
src/tools/modelrestorationform.cpp \
src/tools/sqlexecutionhelper.cpp \
src/tools/databaseimportform.cpp \
src/tools/metadatahandlingform.cpp \
src/tools/modelexporthelper.cpp \
src/tools/modelsdiffhelper.cpp \
src/tools/sqlexecutionwidget.cpp \
src/tools/validationinfo.cpp \
src/tools/modelvalidationhelper.cpp \
src/tools/swapobjectsidswidget.cpp \
src/tools/modelvalidationwidget.cpp \
	src/utils/customsortproxymodel.cpp \
src/utils/deletableitemdelegate.cpp \
	src/utils/fragmentinfo.cpp \
src/utils/htmlitemdelegate.cpp \
	src/utils/matchinfo.cpp \
	src/utils/objectslistmodel.cpp \
src/utils/plaintextitemdelegate.cpp \
src/utils/resultsetmodel.cpp \
src/utils/syntaxhighlighter.cpp \
src/utils/textblockinfo.cpp \
src/widgets/aboutwidget.cpp \
    src/widgets/columndatawidget.cpp \
	src/widgets/customtablewidget.cpp \
	src/widgets/debugoutputwidget.cpp \
src/widgets/donatewidget.cpp \
src/widgets/modelnavigationwidget.cpp \
src/widgets/numberedtexteditor.cpp \
	src/widgets/objectsearchwidget.cpp \
	src/widgets/objecttypeslistwidget.cpp \
src/widgets/operationlistwidget.cpp \
src/widgets/modelobjectswidget.cpp \
src/widgets/objectdepsrefswidget.cpp \
src/widgets/sceneinfowidget.cpp \
	src/widgets/searchreplacewidget.cpp \
src/widgets/welcomewidget.cpp \
src/widgets/changelogwidget.cpp \
src/widgets/fileselectorwidget.cpp \
src/widgets/modeloverviewwidget.cpp \
src/widgets/sourcecodewidget.cpp \
src/widgets/codecompletionwidget.cpp \
src/widgets/objectrenamewidget.cpp \
src/widgets/colorpickerwidget.cpp \
src/widgets/layersconfigwidget.cpp \
src/widgets/objectselectorwidget.cpp \
src/widgets/tabledatawidget.cpp \
src/widgets/csvloadwidget.cpp \
src/widgets/layerswidget.cpp \
src/widgets/modelwidget.cpp \
src/widgets/objectsfilterwidget.cpp \
src/widgets/taskprogresswidget.cpp \
src/widgets/customsqlwidget.cpp \
src/widgets/linenumberswidget.cpp \
src/widgets/newobjectoverlaywidget.cpp \
src/widgets/updatenotifierwidget.cpp

HEADERS += src/guiglobal.h \
src/baseform.h \
	src/dbobjects/referenceswidget.h \
	src/dbobjects/simplecolumnswidget.h \
src/guiutilsns.h \
src/mainwindow.h \
src/messagebox.h \
src/pgmodelerguiplugin.h \
src/dbobjects/aggregatewidget.h \
src/dbobjects/domainwidget.h \
src/dbobjects/languagewidget.h \
src/dbobjects/textboxwidget.h \
src/dbobjects/basefunctionwidget.h \
src/dbobjects/elementwidget.h \
src/dbobjects/operatorclasswidget.h \
src/dbobjects/relationshipwidget.h \
src/dbobjects/transformwidget.h \
src/dbobjects/baseobjectwidget.h \
src/dbobjects/eventtriggerwidget.h \
src/dbobjects/operatorfamilywidget.h \
src/dbobjects/rolewidget.h \
src/dbobjects/triggerwidget.h \
src/dbobjects/castwidget.h \
src/dbobjects/extensionwidget.h \
src/dbobjects/operatorwidget.h \
src/dbobjects/rulewidget.h \
src/dbobjects/typewidget.h \
src/dbobjects/collationwidget.h \
src/dbobjects/foreigndatawrapperwidget.h \
src/dbobjects/parameterwidget.h \
src/dbobjects/schemawidget.h \
src/dbobjects/usermappingwidget.h \
src/dbobjects/columnwidget.h \
src/dbobjects/foreignserverwidget.h \
src/dbobjects/permissionwidget.h \
src/dbobjects/sequencewidget.h \
src/dbobjects/viewwidget.h \
src/dbobjects/constraintwidget.h \
src/dbobjects/functionwidget.h \
src/dbobjects/pgsqltypewidget.h \
src/dbobjects/tablespacewidget.h \
src/dbobjects/conversionwidget.h \
src/dbobjects/genericsqlwidget.h \
src/dbobjects/policywidget.h \
src/dbobjects/tablewidget.h \
src/dbobjects/databasewidget.h \
src/dbobjects/indexwidget.h \
src/dbobjects/procedurewidget.h \
src/dbobjects/tagwidget.h \
src/dbobjects/elementstablewidget.h \
src/dbobjects/columnpickerwidget.h \
	src/qtconnectmacros.h \
src/settings/appearanceconfigwidget.h \
src/settings/configurationform.h \
src/settings/generalconfigwidget.h \
src/settings/relationshipconfigwidget.h \
src/settings/baseconfigwidget.h \
src/settings/connectionsconfigwidget.h \
src/settings/pluginsconfigwidget.h \
src/settings/snippetsconfigwidget.h \
src/tools/bugreportform.h \
src/tools/databaseimporthelper.h \
	src/tools/datagridwidget.h \
	src/tools/datahandlingform.h \
src/tools/modeldatabasediffform.h \
src/tools/modelfixform.h \
src/tools/objectsdiffinfo.h \
src/tools/sqltoolwidget.h \
src/tools/databaseexplorerwidget.h \
src/tools/modelexportform.h \
src/tools/modelrestorationform.h \
src/tools/sqlexecutionhelper.h \
src/tools/databaseimportform.h \
src/tools/metadatahandlingform.h \
src/tools/modelexporthelper.h \
src/tools/modelsdiffhelper.h \
src/tools/sqlexecutionwidget.h \
src/tools/validationinfo.h \
src/tools/modelvalidationhelper.h \
src/tools/swapobjectsidswidget.h \
src/tools/modelvalidationwidget.h \
	src/utils/customsortproxymodel.h \
src/utils/deletableitemdelegate.h \
	src/utils/fragmentinfo.h \
src/utils/htmlitemdelegate.h \
	src/utils/matchinfo.h \
	src/utils/objectslistmodel.h \
src/utils/plaintextitemdelegate.h \
src/utils/resultsetmodel.h \
src/utils/syntaxhighlighter.h \
src/utils/textblockinfo.h \
src/widgets/aboutwidget.h \
    src/widgets/columndatawidget.h \
	src/widgets/customtablewidget.h \
	src/widgets/debugoutputwidget.h \
src/widgets/donatewidget.h \
src/widgets/modelnavigationwidget.h \
src/widgets/numberedtexteditor.h \
	src/widgets/objectsearchwidget.h \
	src/widgets/objecttypeslistwidget.h \
src/widgets/operationlistwidget.h \
src/widgets/modelobjectswidget.h \
src/widgets/objectdepsrefswidget.h \
src/widgets/sceneinfowidget.h \
	src/widgets/searchreplacewidget.h \
src/widgets/welcomewidget.h \
src/widgets/changelogwidget.h \
src/widgets/fileselectorwidget.h \
src/widgets/modeloverviewwidget.h \
src/widgets/sourcecodewidget.h \
src/widgets/codecompletionwidget.h \
src/widgets/objectrenamewidget.h \
src/widgets/colorpickerwidget.h \
src/widgets/layersconfigwidget.h \
src/widgets/objectselectorwidget.h \
src/widgets/tabledatawidget.h \
src/widgets/csvloadwidget.h \
src/widgets/layerswidget.h \
src/widgets/modelwidget.h \
src/widgets/objectsfilterwidget.h \
src/widgets/taskprogresswidget.h \
src/widgets/customsqlwidget.h \
src/widgets/linenumberswidget.h \
src/widgets/newobjectoverlaywidget.h \
src/widgets/updatenotifierwidget.h

FORMS += ui/baseform.ui \
ui/dbobjects/columnpickerwidget.ui \
	ui/dbobjects/referenceswidget.ui \
	ui/dbobjects/simplecolumnswidget.ui \
ui/mainwindow.ui \
ui/messagebox.ui \
ui/dbobjects/aggregatewidget.ui \
ui/dbobjects/domainwidget.ui \
ui/dbobjects/languagewidget.ui \
ui/dbobjects/relationshipwidget.ui \
ui/dbobjects/transformwidget.ui \
ui/dbobjects/basefunctionwidget.ui \
ui/dbobjects/elementwidget.ui \
ui/dbobjects/operatorclasswidget.ui \
ui/dbobjects/rolewidget.ui \
ui/dbobjects/triggerwidget.ui \
ui/dbobjects/baseobjectwidget.ui \
ui/dbobjects/eventtriggerwidget.ui \
ui/dbobjects/operatorfamilywidget.ui \
ui/dbobjects/rulewidget.ui \
ui/dbobjects/typewidget.ui \
ui/dbobjects/castwidget.ui \
ui/dbobjects/extensionwidget.ui \
ui/dbobjects/operatorwidget.ui \
ui/dbobjects/schemawidget.ui \
ui/dbobjects/usermappingwidget.ui \
ui/dbobjects/collationwidget.ui \
ui/dbobjects/foreigndatawrapperwidget.ui \
ui/dbobjects/parameterwidget.ui \
ui/dbobjects/sequencewidget.ui \
ui/dbobjects/viewwidget.ui \
ui/dbobjects/columnwidget.ui \
ui/dbobjects/foreignserverwidget.ui \
ui/dbobjects/permissionwidget.ui \
ui/dbobjects/tablespacewidget.ui \
ui/dbobjects/constraintwidget.ui \
ui/dbobjects/functionwidget.ui \
ui/dbobjects/pgsqltypewidget.ui \
ui/dbobjects/tablewidget.ui \
ui/dbobjects/conversionwidget.ui \
ui/dbobjects/genericsqlwidget.ui \
ui/dbobjects/policywidget.ui \
ui/dbobjects/tagwidget.ui \
ui/dbobjects/databasewidget.ui \
ui/dbobjects/indexwidget.ui \
ui/dbobjects/textboxwidget.ui \
ui/settings/appearanceconfigwidget.ui \
ui/settings/connectionsconfigwidget.ui \
ui/settings/pluginsconfigwidget.ui \
ui/settings/snippetsconfigwidget.ui \
ui/settings/configurationform.ui \
ui/settings/generalconfigwidget.ui \
ui/settings/relationshipconfigwidget.ui \
ui/tools/bugreportform.ui \
	ui/tools/datagridwidget.ui \
	ui/tools/datahandlingform.ui \
ui/tools/modelexportform.ui \
ui/tools/sqlexecutionwidget.ui \
ui/tools/databaseexplorerwidget.ui \
ui/tools/metadatahandlingform.ui \
ui/tools/modelfixform.ui \
ui/tools/sqltoolwidget.ui \
ui/tools/databaseimportform.ui \
ui/tools/modeldatabasediffform.ui \
ui/tools/modelrestorationform.ui \
ui/tools/swapobjectsidswidget.ui \
ui/tools/modelvalidationwidget.ui \
ui/widgets/aboutwidget.ui \
	ui/widgets/customtablewidget.ui \
ui/widgets/donatewidget.ui \
ui/widgets/modelobjectswidget.ui \
ui/widgets/objectrenamewidget.ui \
	ui/widgets/objectsearchwidget.ui \
	ui/widgets/objecttypeslistwidget.ui \
	ui/widgets/searchreplacewidget.ui \
ui/widgets/sourcecodewidget.ui \
ui/widgets/fileselectorwidget.ui \
ui/widgets/modeloverviewwidget.ui \
ui/widgets/objectselectorwidget.ui \
ui/widgets/changelogwidget.ui \
ui/widgets/objectsfilterwidget.ui \
ui/widgets/tabledatawidget.ui \
ui/widgets/colorpickerwidget.ui \
ui/widgets/layersconfigwidget.ui \
ui/widgets/newobjectoverlaywidget.ui \
ui/widgets/taskprogresswidget.ui \
ui/widgets/csvloadwidget.ui \
ui/widgets/layerswidget.ui \
ui/widgets/objectdepsrefswidget.ui \
ui/widgets/operationlistwidget.ui \
ui/widgets/updatenotifierwidget.ui \
ui/widgets/customsqlwidget.ui \
ui/widgets/modelnavigationwidget.ui \
ui/widgets/sceneinfowidget.ui \
ui/widgets/welcomewidget.ui

unix|windows: LIBS += $$LIBCANVAS_LIB \
		      $$LIBCONNECTOR_LIB \
		      $$LIBCORE_LIB \
		      $$LIBPARSERS_LIB \
		      $$LIBUTILS_LIB

INCLUDEPATH += $$LIBCANVAS_INC \
	       $$LIBCONNECTOR_INC \
	       $$LIBCORE_INC \
	       $$LIBPARSERS_INC \
	       $$LIBUTILS_INC \
	       $$PWD/src/dbobjects \
	       $$PWD/src/settings \
	       $$PWD/src/tools \
	       $$PWD/src/utils \
	       $$PWD/src/widgets

DEPENDPATH += $$LIBCANVAS_ROOT \
	      $$LIBCONNECTOR_ROOT \
	      $$LIBCORE_ROOT \
	      $$LIBPARSERS_ROOT \
	      $$LIBUTILS_ROOT

isEqual(PRIVATE_PLUGINS, true) {
  HEADERS+=$$PLUGINS_SRC_ROOT/src/privpluginsns.h
  SOURCES+=$$PLUGINS_SRC_ROOT/src/privpluginsns.cpp
}

# Deployment settings
target.path = $$PRIVATELIBDIR
INSTALLS = target

