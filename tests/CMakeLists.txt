find_package(Qt6 REQUIRED COMPONENTS Test)

include_directories(
    ${LIBCANVAS_INC}
    ${LIBCONNECTOR_INC}
    ${LIBCORE_INC}
    ${LIBGUI_INC}
    ${LIBPARSERS_INC}
    src/
)

link_libraries(
    Qt::Test
    canvas
    connector
    core
    gui
    parsers
    utils)

add_subdirectory(src/main)
add_subdirectory(src/schemafilessyntaxtest)
add_subdirectory(src/baseobjecttest)
add_subdirectory(src/roletest)
add_subdirectory(src/syntaxhighlightertest)
add_subdirectory(src/databasemodeltest)
add_subdirectory(src/schemaparsertest)
add_subdirectory(src/linenumberstest)
add_subdirectory(src/partrelationshiptest)
add_subdirectory(src/foreigndatawrappertest)
add_subdirectory(src/servertest)
add_subdirectory(src/usermappingtest)
add_subdirectory(src/datadicttest)
add_subdirectory(src/fileselectortest)
add_subdirectory(src/transformtest)
add_subdirectory(src/xmlparsertest)
add_subdirectory(src/proceduretest)
add_subdirectory(src/basefunctiontest)
add_subdirectory(src/csvparsertest)
add_subdirectory(src/modelwidgettest)
add_subdirectory(src/messageboxtest)
add_subdirectory(src/uilanguagetest)
add_subdirectory(src/clitest)

# The SQL execution tool is part of the private core. Its leak checks need the LeakSanitizer runtime (USE_ADDR_SANITIZER)
if(BUILD_PRIV_CODE AND USE_ADDR_SANITIZER)
    add_subdirectory(src/sqlexecleaktest)
endif()
