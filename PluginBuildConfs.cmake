# Importing Qt6 components
# Forcing a version >= 6.4.0. If no suitable version is found the script aborts
find_package(Qt6 6.4.0 REQUIRED COMPONENTS UiTools)

# By default, the AUTOUIC option makes the uic look for the .ui files
# under the same folder as the sources. In this case, we force the uic
# command to look for .ui files under ui/*
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ui)

include_directories(
    ${LIBCANVAS_INC}
    ${LIBCONNECTOR_INC}
    ${LIBCORE_INC}
    ${LIBCLI_INC}
    ${LIBPARSERS_INC}
    ${LIBUTILS_INC}
    ${LIBGUI_INC}
    ${PRIV_PLUGINS_SRC})

link_libraries(
    Qt::Core
    Qt::Gui
    Qt::UiTools
    canvas
    cli
    connector
    core
    gui
    parsers
    utils)

if(BUILD_PRIV_PLUGINS)
    set(PRIV_GUI_SOURCES
        ${PRIV_PLUGINS_SRC}/pluginwidget.h
        ${PRIV_PLUGINS_SRC}/privateguiplugin.cpp
        ${PRIV_PLUGINS_SRC}/privateguiplugin.h
        ${PRIV_PLUGINS_SRC}/privpluginsglobal.h)

    set(PRIV_CLI_SOURCES
        ${PRIV_PLUGINS_SRC}/privatecliplugin.cpp
        ${PRIV_PLUGINS_SRC}/privatecliplugin.h
        ${PRIV_PLUGINS_SRC}/privpluginsglobal.h)
endif()

# This function wraps a call to qt_add_plugin and set a
# variable named PGM_TARGET in the parent scope (where the function
# was called).
function(pgm_add_plugin TARGET IS_PRIV_PLUGIN IS_CLI_PLUGIN)
    qt_add_plugin(${TARGET})
    set(PGM_TARGET ${TARGET} PARENT_SCOPE)

    if(IS_PRIV_PLUGIN)
        if(IS_CLI_PLUGIN)
            set(PRIV_SOURCES ${PRIV_CLI_SOURCES})
        else()
            set(PRIV_SOURCES ${PRIV_GUI_SOURCES})
        endif()

        target_sources(${TARGET} PRIVATE ${PRIV_SOURCES})
    endif()
endfunction()

# This function calls pgm_add_plugin setting the IS_CLI_PLUGIN to OFF
# Like pg_add_plugin, it creates a variable called PGM_TARGET in the
# parent scope that registers the name of the plugin
function(pgm_add_gui_plugin TARGET IS_PRIV_PLUGIN)
    pgm_add_plugin(${TARGET} ${IS_PRIV_PLUGIN} OFF)
    set(PGM_TARGET ${PGM_TARGET} PARENT_SCOPE)
endfunction()

# This function calls pgm_add_plugin setting the IS_CLI_PLUGIN to ON
# Like pg_add_plugin, it creates a variable called PGM_TARGET in the
# parent scope that registers the name of the plugin
function(pgm_add_cli_plugin TARGET IS_PRIV_PLUGIN)
    pgm_add_plugin(${TARGET} ${IS_PRIV_PLUGIN} ON)
    set(PGM_TARGET ${PGM_TARGET} PARENT_SCOPE)
endfunction()

# This function adds the resource file (icons/images)
# in the list of plugin's sources to be compiled
# this will cause the RCC command to be called and
# generate the qrc_*.cpp which is linked with the
# plugin's lib
function(pgm_set_plugin_res TARGET)
    qt_add_resources(PLUGIN_RES "res/${TARGET}.qrc")
    target_sources(${TARGET} PRIVATE ${PLUGIN_RES})
endfunction()

# This function configures the assets deployment
# of the plugin library. The first argument TARGET
# is the name of the plugin.
# Arguments specified after TARGET when calling
# this function is considered to be the files and
# directories to be deployed. The files and directory
# will always be considered in relative path to the
# plugin's root diretory. So, make sure to inform only
# the name of the files/dirs not their absolute paths
function(pgm_install_plugin_files TARGET)
    set(SRC_PATH ${PRIV_PLUGINS_ROOT}/${TARGET})

    foreach(file IN LISTS ARGN)
        # Putting directories and files in separate variables
        if(IS_DIRECTORY ${SRC_PATH}/${file})
            list(APPEND _DIRS ${SRC_PATH}/${file})
        else()
            list(APPEND _FILES ${SRC_PATH}/${file})
        endif()
    endforeach()

    # Deploying files
    install(FILES ${_FILES}
        DESTINATION ${PGM_PLUGINSDIR}/${TARGET})

    # Deploying directories
    install(DIRECTORY ${_DIRS}
        DESTINATION ${PGM_PLUGINSDIR}/${TARGET})
endfunction()

# This function configures the deployment settings
# of the plugin library.
function(pgm_install_plugin TARGET)
    install(TARGETS ${TARGET}
        LIBRARY DESTINATION ${PGM_PLUGINSDIR}/${TARGET}
        FRAMEWORK DESTINATION ${PGM_PLUGINSDIR}/${TARGET}
        RUNTIME DESTINATION ${PGM_PLUGINSDIR}/${TARGET})
endfunction()
