set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)

set(SOURCES
    src/cliglobal.h
    src/compat/compatns.cpp src/compat/compatns.h
    src/compat/reference.cpp src/compat/reference.h
    src/compat/view.cpp src/compat/view.h
    src/pgmodelercliapp.cpp src/pgmodelercliapp.h
    src/pgmodelercliplugin.cpp src/pgmodelercliplugin.h)

if(BUILD_PRIV_PLUGINS)
    list(APPEND SOURCES
        ${PRIV_PLUGINS_SRC}/privpluginsns.h
        ${PRIV_PLUGINS_SRC}/privpluginsns.cpp)
endif()

pgm_add_library(cli ${SOURCES})

target_include_directories(${PGM_TARGET} PUBLIC
    ${LIBCANVAS_INC}
    ${LIBCONNECTOR_INC}
    ${LIBCORE_INC}
    ${LIBGUI_INC}
    ${LIBPARSERS_INC}
    ${LIBUTILS_INC}
    src/compat
    ${PRIV_PLUGINS_SRC}
    ${LIBGUI_AUTOGEN_INC})

target_link_libraries(${PGM_TARGET} PUBLIC
    canvas
    connector
    core
    gui
    parsers
    utils)

pgm_install_library(${PGM_TARGET})
