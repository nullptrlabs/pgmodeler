set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(SOURCES
    src/baseform.cpp src/baseform.h
    src/dbobjects/aggregatewidget.cpp src/dbobjects/aggregatewidget.h
    src/dbobjects/basefunctionwidget.cpp src/dbobjects/basefunctionwidget.h
    src/dbobjects/baseobjectwidget.cpp src/dbobjects/baseobjectwidget.h
    src/dbobjects/castwidget.cpp src/dbobjects/castwidget.h
    src/dbobjects/collationwidget.cpp src/dbobjects/collationwidget.h
    src/dbobjects/columnpickerwidget.cpp src/dbobjects/columnpickerwidget.h
    src/dbobjects/columnwidget.cpp src/dbobjects/columnwidget.h
    src/dbobjects/constraintwidget.cpp src/dbobjects/constraintwidget.h
    src/dbobjects/conversionwidget.cpp src/dbobjects/conversionwidget.h
    src/dbobjects/databasewidget.cpp src/dbobjects/databasewidget.h
    src/dbobjects/domainwidget.cpp src/dbobjects/domainwidget.h
    src/dbobjects/elementstablewidget.cpp src/dbobjects/elementstablewidget.h
    src/dbobjects/elementwidget.cpp src/dbobjects/elementwidget.h
    src/dbobjects/eventtriggerwidget.cpp src/dbobjects/eventtriggerwidget.h
    src/dbobjects/extensionwidget.cpp src/dbobjects/extensionwidget.h
    src/dbobjects/foreigndatawrapperwidget.cpp src/dbobjects/foreigndatawrapperwidget.h
    src/dbobjects/foreignserverwidget.cpp src/dbobjects/foreignserverwidget.h
    src/dbobjects/functionwidget.cpp src/dbobjects/functionwidget.h
    src/dbobjects/genericsqlwidget.cpp src/dbobjects/genericsqlwidget.h
    src/dbobjects/indexwidget.cpp src/dbobjects/indexwidget.h
    src/dbobjects/languagewidget.cpp src/dbobjects/languagewidget.h
    src/dbobjects/operatorclasswidget.cpp src/dbobjects/operatorclasswidget.h
    src/dbobjects/operatorfamilywidget.cpp src/dbobjects/operatorfamilywidget.h
    src/dbobjects/operatorwidget.cpp src/dbobjects/operatorwidget.h
    src/dbobjects/parameterwidget.cpp src/dbobjects/parameterwidget.h
    src/dbobjects/permissionwidget.cpp src/dbobjects/permissionwidget.h
    src/dbobjects/pgsqltypewidget.cpp src/dbobjects/pgsqltypewidget.h
    src/dbobjects/policywidget.cpp src/dbobjects/policywidget.h
    src/dbobjects/procedurewidget.cpp src/dbobjects/procedurewidget.h
    src/dbobjects/referenceswidget.cpp src/dbobjects/referenceswidget.h
    src/dbobjects/relationshipwidget.cpp src/dbobjects/relationshipwidget.h
    src/dbobjects/rolewidget.cpp src/dbobjects/rolewidget.h
    src/dbobjects/rulewidget.cpp src/dbobjects/rulewidget.h
    src/dbobjects/schemawidget.cpp src/dbobjects/schemawidget.h
    src/dbobjects/sequencewidget.cpp src/dbobjects/sequencewidget.h
    src/dbobjects/simplecolumnswidget.cpp src/dbobjects/simplecolumnswidget.h
    src/dbobjects/tablespacewidget.cpp src/dbobjects/tablespacewidget.h
    src/dbobjects/tablewidget.cpp src/dbobjects/tablewidget.h
    src/dbobjects/tagwidget.cpp src/dbobjects/tagwidget.h
    src/dbobjects/textboxwidget.cpp src/dbobjects/textboxwidget.h
    src/dbobjects/transformwidget.cpp src/dbobjects/transformwidget.h
    src/dbobjects/triggerwidget.cpp src/dbobjects/triggerwidget.h
    src/dbobjects/typewidget.cpp src/dbobjects/typewidget.h
    src/dbobjects/usermappingwidget.cpp src/dbobjects/usermappingwidget.h
    src/dbobjects/viewwidget.cpp src/dbobjects/viewwidget.h
    src/guiglobal.h
    src/guiutilsns.cpp src/guiutilsns.h
    src/mainwindow.cpp src/mainwindow.h
    src/messagebox.cpp src/messagebox.h
    src/pgmodelerguiplugin.cpp src/pgmodelerguiplugin.h
    src/qtconnectmacros.h
    src/settings/appearanceconfigwidget.cpp src/settings/appearanceconfigwidget.h
    src/settings/baseconfigwidget.cpp src/settings/baseconfigwidget.h
    src/settings/connectionsconfigwidget.cpp src/settings/connectionsconfigwidget.h
    src/settings/generalconfigwidget.cpp src/settings/generalconfigwidget.h
    src/settings/pluginsconfigwidget.cpp src/settings/pluginsconfigwidget.h
    src/settings/relationshipconfigwidget.cpp src/settings/relationshipconfigwidget.h
    src/settings/snippetsconfigwidget.cpp src/settings/snippetsconfigwidget.h
    src/settings/configurationwidget.h src/settings/configurationwidget.cpp
    src/tools/bugreportform.cpp src/tools/bugreportform.h
    src/tools/databaseexplorerwidget.cpp src/tools/databaseexplorerwidget.h
    src/tools/databaseimportwidget.cpp src/tools/databaseimportwidget.h
    src/tools/databaseimporthelper.cpp src/tools/databaseimporthelper.h
    src/tools/datagridwidget.cpp src/tools/datagridwidget.h
    src/tools/datahandlingform.cpp src/tools/datahandlingform.h
    src/tools/metadatahandlingwidget.cpp src/tools/metadatahandlingwidget.h
    src/tools/modelexportwidget.cpp src/tools/modelexportwidget.h
    src/tools/modelexporthelper.cpp src/tools/modelexporthelper.h
    src/tools/modelfixwidget.cpp src/tools/modelfixwidget.h
    src/tools/modelrestorationform.cpp src/tools/modelrestorationform.h
    src/tools/modelsdiffhelper.cpp src/tools/modelsdiffhelper.h
    src/tools/modelvalidationhelper.cpp src/tools/modelvalidationhelper.h
    src/tools/modelvalidationwidget.cpp src/tools/modelvalidationwidget.h
    src/tools/objectsdiffinfo.cpp src/tools/objectsdiffinfo.h
    src/tools/sqlexecutionhelper.cpp src/tools/sqlexecutionhelper.h
    src/tools/sqlexecutionwidget.cpp src/tools/sqlexecutionwidget.h
    src/tools/sqltoolwidget.cpp src/tools/sqltoolwidget.h
    src/tools/swapobjectsidswidget.cpp src/tools/swapobjectsidswidget.h
    src/tools/validationinfo.cpp src/tools/validationinfo.h
    src/tools/difftoolwidget.cpp src/tools/difftoolwidget.h
    src/tools/modeldbselectorwidget.h src/tools/modeldbselectorwidget.cpp
    src/tools/fixtoolswidget.h src/tools/fixtoolswidget.cpp
    src/utils/customsortproxymodel.cpp src/utils/customsortproxymodel.h
    src/utils/deletableitemdelegate.cpp src/utils/deletableitemdelegate.h
    src/utils/fragmentinfo.cpp src/utils/fragmentinfo.h
    src/utils/htmlitemdelegate.cpp src/utils/htmlitemdelegate.h
    src/utils/matchinfo.cpp src/utils/matchinfo.h
    src/utils/objectslistmodel.cpp src/utils/objectslistmodel.h
    src/utils/plaintextitemdelegate.cpp src/utils/plaintextitemdelegate.h
    src/utils/resultsetmodel.cpp src/utils/resultsetmodel.h
    src/utils/syntaxhighlighter.cpp src/utils/syntaxhighlighter.h
    src/utils/textblockinfo.cpp src/utils/textblockinfo.h
    src/widgets/aboutwidget.cpp src/widgets/aboutwidget.h
    src/widgets/changelogwidget.cpp src/widgets/changelogwidget.h
    src/widgets/codecompletionwidget.cpp src/widgets/codecompletionwidget.h
    src/widgets/colorpickerwidget.cpp src/widgets/colorpickerwidget.h
    src/widgets/columndatawidget.cpp src/widgets/columndatawidget.h
    src/widgets/csvloadwidget.cpp src/widgets/csvloadwidget.h
    src/widgets/customsqlwidget.cpp src/widgets/customsqlwidget.h
    src/widgets/customtablewidget.cpp src/widgets/customtablewidget.h
    src/widgets/debugoutputwidget.cpp src/widgets/debugoutputwidget.h
    src/widgets/donatewidget.cpp src/widgets/donatewidget.h
    src/widgets/fileselectorwidget.cpp src/widgets/fileselectorwidget.h
    src/widgets/layersconfigwidget.cpp src/widgets/layersconfigwidget.h
    src/widgets/layerswidget.cpp src/widgets/layerswidget.h
    src/widgets/linenumberswidget.cpp src/widgets/linenumberswidget.h
    src/widgets/modelnavigationwidget.cpp src/widgets/modelnavigationwidget.h
    src/widgets/modelobjectswidget.cpp src/widgets/modelobjectswidget.h
    src/widgets/modeloverviewwidget.cpp src/widgets/modeloverviewwidget.h
    src/widgets/modelwidget.cpp src/widgets/modelwidget.h
    src/widgets/newobjectoverlaywidget.cpp src/widgets/newobjectoverlaywidget.h
    src/widgets/numberedtexteditor.cpp src/widgets/numberedtexteditor.h
    src/widgets/objectdepsrefswidget.cpp src/widgets/objectdepsrefswidget.h
    src/widgets/objectrenamewidget.cpp src/widgets/objectrenamewidget.h
    src/widgets/objectsearchwidget.cpp src/widgets/objectsearchwidget.h
    src/widgets/objectselectorwidget.cpp src/widgets/objectselectorwidget.h
    src/widgets/objectsfilterwidget.cpp src/widgets/objectsfilterwidget.h
    src/widgets/objecttypeslistwidget.cpp src/widgets/objecttypeslistwidget.h
    src/widgets/operationlistwidget.cpp src/widgets/operationlistwidget.h
    src/widgets/sceneinfowidget.cpp src/widgets/sceneinfowidget.h
    src/widgets/searchreplacewidget.cpp src/widgets/searchreplacewidget.h
    src/widgets/sourcecodewidget.cpp src/widgets/sourcecodewidget.h
    src/widgets/tabledatawidget.cpp src/widgets/tabledatawidget.h
    src/widgets/taskprogresswidget.cpp src/widgets/taskprogresswidget.h
    src/widgets/updatenotifierwidget.cpp src/widgets/updatenotifierwidget.h
    src/widgets/welcomewidget.cpp src/widgets/welcomewidget.h)

set(FORMS
    ui/baseform.ui
    ui/dbobjects/columnpickerwidget.ui
    ui/dbobjects/referenceswidget.ui
    ui/dbobjects/simplecolumnswidget.ui
    ui/mainwindow.ui
    ui/messagebox.ui
    ui/dbobjects/aggregatewidget.ui
    ui/dbobjects/domainwidget.ui
    ui/dbobjects/languagewidget.ui
    ui/dbobjects/relationshipwidget.ui
    ui/dbobjects/transformwidget.ui
    ui/dbobjects/basefunctionwidget.ui
    ui/dbobjects/elementwidget.ui
    ui/dbobjects/operatorclasswidget.ui
    ui/dbobjects/rolewidget.ui
    ui/dbobjects/triggerwidget.ui
    ui/dbobjects/baseobjectwidget.ui
    ui/dbobjects/eventtriggerwidget.ui
    ui/dbobjects/operatorfamilywidget.ui
    ui/dbobjects/rulewidget.ui
    ui/dbobjects/typewidget.ui
    ui/dbobjects/castwidget.ui
    ui/dbobjects/extensionwidget.ui
    ui/dbobjects/operatorwidget.ui
    ui/dbobjects/schemawidget.ui
    ui/dbobjects/usermappingwidget.ui
    ui/dbobjects/collationwidget.ui
    ui/dbobjects/foreigndatawrapperwidget.ui
    ui/dbobjects/parameterwidget.ui
    ui/dbobjects/sequencewidget.ui
    ui/dbobjects/viewwidget.ui
    ui/dbobjects/columnwidget.ui
    ui/dbobjects/foreignserverwidget.ui
    ui/dbobjects/permissionwidget.ui
    ui/dbobjects/tablespacewidget.ui
    ui/dbobjects/constraintwidget.ui
    ui/dbobjects/functionwidget.ui
    ui/dbobjects/pgsqltypewidget.ui
    ui/dbobjects/tablewidget.ui
    ui/dbobjects/conversionwidget.ui
    ui/dbobjects/genericsqlwidget.ui
    ui/dbobjects/policywidget.ui
    ui/dbobjects/tagwidget.ui
    ui/dbobjects/databasewidget.ui
    ui/dbobjects/indexwidget.ui
    ui/dbobjects/textboxwidget.ui
    ui/settings/appearanceconfigwidget.ui
    ui/settings/connectionsconfigwidget.ui
    ui/settings/pluginsconfigwidget.ui
    ui/settings/snippetsconfigwidget.ui
    ui/settings/generalconfigwidget.ui
    ui/settings/relationshipconfigwidget.ui
    ui/settings/configurationwidget.ui
    ui/tools/bugreportform.ui
    ui/tools/datagridwidget.ui
    ui/tools/datahandlingform.ui
    ui/tools/modelexportwidget.ui
    ui/tools/sqlexecutionwidget.ui
    ui/tools/databaseexplorerwidget.ui
    ui/tools/metadatahandlingwidget.ui
    ui/tools/modelfixwidget.ui
    ui/tools/sqltoolwidget.ui
    ui/tools/modelrestorationform.ui
    ui/tools/swapobjectsidswidget.ui
    ui/tools/modelvalidationwidget.ui
    ui/tools/databaseimportwidget.ui
    ui/tools/difftoolwidget.ui
    ui/tools/modeldbselectorwidget.ui
    ui/tools/fixtoolswidget.ui
    ui/widgets/aboutwidget.ui
    ui/widgets/customtablewidget.ui
    ui/widgets/donatewidget.ui
    ui/widgets/modelobjectswidget.ui
    ui/widgets/objectrenamewidget.ui
    ui/widgets/objectsearchwidget.ui
    ui/widgets/objecttypeslistwidget.ui
    ui/widgets/searchreplacewidget.ui
    ui/widgets/sourcecodewidget.ui
    ui/widgets/fileselectorwidget.ui
    ui/widgets/modeloverviewwidget.ui
    ui/widgets/objectselectorwidget.ui
    ui/widgets/changelogwidget.ui
    ui/widgets/objectsfilterwidget.ui
    ui/widgets/tabledatawidget.ui
    ui/widgets/colorpickerwidget.ui
    ui/widgets/layersconfigwidget.ui
    ui/widgets/newobjectoverlaywidget.ui
    ui/widgets/taskprogresswidget.ui
    ui/widgets/csvloadwidget.ui
    ui/widgets/layerswidget.ui
    ui/widgets/objectdepsrefswidget.ui
    ui/widgets/operationlistwidget.ui
    ui/widgets/updatenotifierwidget.ui
    ui/widgets/customsqlwidget.ui
    ui/widgets/modelnavigationwidget.ui
    ui/widgets/sceneinfowidget.ui
    ui/widgets/welcomewidget.ui)

set(RESOURCES res/resources.qrc)

if(BUILD_PRIV_PLUGINS)
    list(APPEND SOURCES
        ${PRIV_PLUGINS_SRC}/privpluginsns.h
        ${PRIV_PLUGINS_SRC}/privpluginsns.cpp)
endif()

if(PLUS_VERSION)
    list(APPEND RESOURCES ${PRIV_PLUGINS_RES}/logoicons.qrc)
else()
    list(APPEND RESOURCES res/logoicons.qrc)
endif()

# By default, the AUTOUIC option makes the uic look for the .ui files
# under the same folder as the sources. In this case, we force the uic
# command to look for .ui files under ui/*
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ui
    ui/dbobjects
    ui/settings
    ui/widgets
    ui/tools)

pgm_add_library(gui
    ${SOURCES}
    ${FORMS}
    ${RESOURCES})

# We create the global variable LIBGUI_AUTOGEN_INC that storse the path to the
# gui_autogen/include folder to be reused by other subprojects
set(LIBGUI_AUTOGEN_INC ${CMAKE_CURRENT_BINARY_DIR}/${PGM_TARGET}_autogen/include
    CACHE INTERNAL "Path to the gui_autogen/include folder")

target_include_directories(${PGM_TARGET} PUBLIC
    ${LIBCANVAS_INC}
    ${LIBCONNECTOR_INC}
    ${LIBCORE_INC}
    ${LIBPARSERS_INC}
    ${LIBUTILS_INC}
    src/
    src/dbobjects
    src/settings
    src/tools
    src/utils
    src/widgets
    ${PRIV_PLUGINS_SRC}
    ${LIBGUI_AUTOGEN_INC})

target_link_libraries(${PGM_TARGET} PUBLIC
    connector
    canvas
    connector
    core
    parsers
    utils)

pgm_install_library(${PGM_TARGET})
