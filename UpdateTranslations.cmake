# Creates or updates the translation sources (.ts) of one or more languages by running lupdate over
# the source directories. Texts already translated in an existing .ts are preserved, only new texts
# are added (and texts not found anymore are kept as obsolete).
#
# It runs as a script through the target update_ts (see TranslationConfs.cmake), which provides the
# variables below. The names of the source folders are passed by the target, from the variables the build
# uses, because this script can't include the files that define them (they use commands that are not
# available in script mode). The language can be overridden by the environment variable PGM_TS_LANG.
#
#   PGM_SOURCE_DIR                   source root
#   LUPDATE                          path to lupdate
#   PGM_APPS_DIR, PGM_LIBS_DIR       folders of the applications and libraries, relative to the source root
#   PGM_PRIV_CORE_DIR                folder of the private core code (optional in the source tree)
#   PGM_PRIV_PLUGINS_DIR             folder of the private plugins (optional in the source tree)
#   PGM_TS_LANG                      list of language codes in the form xx or xx_XX (e.g. pt_BR, fr_FR);
#                                    the code is also the base name of the .ts files:
#   - the main dictionary is assets/lang/<lang>.ts and gathers the applications, the libraries and the
#     private core code (if it exists);
#   - each plugin in the private plugins folder (if it exists) has its own dictionary in
#     <plugin>/lang/<lang>.ts and gathers the plugin's folder plus the src folder of the private plugins,
#     which is compiled into every plugin.

cmake_minimum_required(VERSION 3.16)

# The language set in the environment takes precedence. Without it the default language, the one
# configured in PGM_TS_LANG, is updated and a warning is shown at the end
set(_LANG_FROM_ENV OFF)

if(DEFINED ENV{PGM_TS_LANG} AND NOT "$ENV{PGM_TS_LANG}" STREQUAL "")
    set(PGM_TS_LANG "$ENV{PGM_TS_LANG}")
    set(_LANG_FROM_ENV ON)
endif()

foreach(_VAR PGM_SOURCE_DIR LUPDATE PGM_APPS_DIR PGM_LIBS_DIR PGM_PRIV_CORE_DIR PGM_PRIV_PLUGINS_DIR PGM_TS_LANG)
    if(NOT DEFINED ${_VAR} OR "${${_VAR}}" STREQUAL "")
        message(FATAL_ERROR "UpdateTranslations: the variable ${_VAR} must be provided.")
    endif()
endforeach()

# A relative source root (e.g. -DPGM_SOURCE_DIR=.) is taken from the current directory. It must be
# absolute because file(GLOB ... RELATIVE) doesn't work with relative paths
get_filename_component(PGM_SOURCE_DIR "${PGM_SOURCE_DIR}" ABSOLUTE)

# The texts marked with PGM_TR2 are recognized as an alias of QT_TRANSLATE_NOOP
set(_ALIAS "QT_TRANSLATE_NOOP+=PGM_TR2")

# Plugins that are not translated: the plugin template and any folder that is not a loadable plugin
# (the ones that don't call pgm_install_plugin, e.g. the pgmodeler-pi executable, or have no CMakeLists.txt)
set(_EXCLUDED_PLUGINS templateplugin)

function(pgm_run_lupdate TS_FILE LOCATIONS LANG)
    # The directory of the .ts must exist, lupdate creates the file but not the folder
    get_filename_component(_TS_DIR ${TS_FILE} DIRECTORY)
    file(MAKE_DIRECTORY ${_TS_DIR})

    execute_process(
        COMMAND ${LUPDATE} ${ARGN} -recursive -locations ${LOCATIONS}
                -tr-function-alias ${_ALIAS}
                -source-language en_US -target-language ${LANG}
                -ts ${TS_FILE}
        WORKING_DIRECTORY ${PGM_SOURCE_DIR}
        RESULT_VARIABLE _RES)

    if(NOT _RES EQUAL 0)
        message(FATAL_ERROR "UpdateTranslations: lupdate failed for ${TS_FILE} (exit code ${_RES}).")
    endif()
endfunction()

foreach(_LANG ${PGM_TS_LANG})
    # xx or xx_XX (also accepting a script, e.g. zh_Hans or zh_Hans_CN)
    if(NOT _LANG MATCHES "^[a-z][a-z][a-z]?(_[A-Z][A-Za-z][A-Za-z]?[A-Za-z]?)*$")
        message(FATAL_ERROR "UpdateTranslations: invalid language code `${_LANG}'. Use xx or xx_XX, e.g. pt_BR or fr_FR.")
    endif()

    message(STATUS "Updating the ${_LANG} dictionaries...")

    # Main dictionary
    set(_MAIN_DIRS ${PGM_APPS_DIR} ${PGM_LIBS_DIR})

    if(EXISTS ${PGM_SOURCE_DIR}/${PGM_PRIV_CORE_DIR})
        list(APPEND _MAIN_DIRS ${PGM_PRIV_CORE_DIR})
    endif()

    # The file locations are omitted in the main dictionary because it lives in the public repository
    # and gathers texts of private code too
    message(STATUS "assets/lang/${_LANG}.ts (${_MAIN_DIRS})")
    pgm_run_lupdate(${PGM_SOURCE_DIR}/assets/lang/${_LANG}.ts none ${_LANG} ${_MAIN_DIRS})

    # Private plugins dictionaries
    if(EXISTS ${PGM_SOURCE_DIR}/${PGM_PRIV_PLUGINS_DIR})
        file(GLOB _PLUGIN_DIRS LIST_DIRECTORIES true RELATIVE ${PGM_SOURCE_DIR}/${PGM_PRIV_PLUGINS_DIR}
             ${PGM_SOURCE_DIR}/${PGM_PRIV_PLUGINS_DIR}/*)
        list(SORT _PLUGIN_DIRS)

        if(NOT _PLUGIN_DIRS)
            message(WARNING "UpdateTranslations: no folder found in ${PGM_SOURCE_DIR}/${PGM_PRIV_PLUGINS_DIR}.")
        endif()

        foreach(_PLUGIN ${_PLUGIN_DIRS})
            set(_CMAKELISTS ${PGM_SOURCE_DIR}/${PGM_PRIV_PLUGINS_DIR}/${_PLUGIN}/CMakeLists.txt)

            if(_PLUGIN IN_LIST _EXCLUDED_PLUGINS OR NOT EXISTS ${_CMAKELISTS})
                continue()
            endif()

            file(READ ${_CMAKELISTS} _CONTENTS)

            if(NOT _CONTENTS MATCHES "pgm_install_plugin\\(")
                continue()
            endif()

            message(STATUS "${PGM_PRIV_PLUGINS_DIR}/${_PLUGIN}/lang/${_LANG}.ts")
            pgm_run_lupdate(${PGM_SOURCE_DIR}/${PGM_PRIV_PLUGINS_DIR}/${_PLUGIN}/lang/${_LANG}.ts relative ${_LANG}
                            ${PGM_PRIV_PLUGINS_DIR}/${_PLUGIN} ${PGM_PRIV_PLUGINS_DIR}/src)
        endforeach()
    endif()
endforeach()

if(NOT _LANG_FROM_ENV)
    message(WARNING "The environment variable PGM_TS_LANG was not provided: the default translation (${PGM_TS_LANG}) was updated instead. "
                    "To update another language run, e.g.: PGM_TS_LANG=fr_FR cmake --build <build dir> --target update_ts")
endif()
