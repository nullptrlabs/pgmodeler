cmake_minimum_required(VERSION 3.16)
project(pgmodeler VERSION 2.0 LANGUAGES C CXX)

# Defining where cmake can find the .cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include(BuildConfs)
include(DefaultPaths)
include(BuildNumDate)
include(PrintSummary)
include(InstallAssets)

# Include clang-tidy configuration BEFORE adding subdirectories
# so that CMAKE_CXX_CLANG_TIDY is set before any targets are created
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    include(ClangTidyConf)
endif()

add_subdirectory(${LIBS_DIR}/${LIBUTILS})
add_subdirectory(${LIBS_DIR}/${LIBPARSERS})
add_subdirectory(${LIBS_DIR}/${LIBCORE})
add_subdirectory(${LIBS_DIR}/${LIBCONNECTOR})
add_subdirectory(${LIBS_DIR}/${LIBCANVAS})
add_subdirectory(${LIBS_DIR}/${LIBGUI})
add_subdirectory(${LIBS_DIR}/${LIBCLI})
add_subdirectory(${APPS_DIR}/${PGMODELER_APP})
add_subdirectory(${APPS_DIR}/${PGMODELERCLI_APP})
add_subdirectory(${APPS_DIR}/${PGMODELERCH_APP})
add_subdirectory(${APPS_DIR}/${PGMODELERSE_APP})

if(CMAKE_BUILD_TYPE STREQUAL Debug AND BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_PLUGINS)
    add_subdirectory(${PLUGINS_DIR})
endif()

if(BUILD_PRIV_PLUGINS)
    add_subdirectory(${PRIV_PLUGINS_DIR})
endif()

print_build_summary()
