set(CMAKE_WARN_DEPRECATED OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# By default the compiler used is clang/clang++
if(LINUX)
  set(CMAKE_C_COMPILER /usr/bin/clang)
  set(CMAKE_CXX_COMPILER /usr/bin/clang++)
elseif(WIN32)
  set(CMAKE_C_COMPILER C:/msys64/mingw64/bin/clang)
  set(CMAKE_CXX_COMPILER C:/msys64/mingw64/bin/clang++)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Importing Qt6 components
# Forcing a version >= 6.4.0. If no suitable version is found the script aborts
find_package(Qt6 6.4.0 REQUIRED COMPONENTS Core Gui Network PrintSupport Svg Widgets)

qt_standard_project_setup()

# Disabling deprecated Qt code of version below 6.4
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060400)

link_libraries(
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::PrintSupport
    Qt::Svg
    Qt::Widgets)

# Enable the compilation of sample plugins
# if their sources exists in plugins/*
set(PLUGINS_DIR plugins)
set(PLUGINS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/${PLUGINS_DIR})

if(EXISTS ${PLUGINS_ROOT})
    # Enabling the plugins build
    set(BUILD_PLUGINS ON)
endif()

# Private plugins source detection
# If the folder when the sources are stored we configure a
# set of variables and compiler defs that instruct cmake to
# include private code and resources
set(PRIV_PLUGINS_DIR priv-plugins)
set(PRIV_PLUGINS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/${PRIV_PLUGINS_DIR})
set(PRIV_PLUGINS_RES ${PRIV_PLUGINS_ROOT}/res)

if(NOT DEMO_VERSION AND PLUS_VERSION AND EXISTS ${PRIV_PLUGINS_ROOT})
    # Enabling the private plugins build
    set(BUILD_PRIV_PLUGINS ON)
    set(PRIV_PLUGINS_SRC ${PRIV_PLUGINS_ROOT}/src)
    add_compile_definitions(PRIVATE_PLUGINS_SYMBOLS)
endif()

# Adding custom C defs if some cmake variables are set
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_definitions(PGMODELER_DEBUG)
endif()

if(NOT DEFINED NO_CHECK_CURR_VER)
    add_compile_definitions(CHECK_CURR_VER)
endif()

if(SNAPSHOT_BUILD)
    add_compile_definitions(SNAPSHOT_BUILD)
endif()

if(DEMO_VERSION)
    add_compile_definitions(DEMO_VERSION)
endif()

if(NO_UPDATE_CHECK)
    add_compile_definitions(NO_UPDATE_CHECK)
endif()

if(APPIMAGE_BUILD AND LINUX)
    add_compile_definitions(APPIMAGE_BUILD)
endif()

# This function wraps a call to qt_add_executable and set a
# variable named PGM_TARGET in the parent scope (where the function
# was called). In macOS, the executable is always built as standalone
# not as a application bundle
# The parameter USE_PRIV_EXEC_ICONS when ON indicates that the executable
# icon must be the one available in private resources dir otherwise
# the one in the target's res/ is used instead. This options has
# effect only when building Windows binaries
function(pgm_add_executable TARGET)
  list(APPEND _SOURCES ${ARGN})

  if(WIN32)
    set(PRIV_ICO_RES ${PRIV_PLUGINS_RES}/${TARGET}/windows_ico.rc)

    if((PLUS_VERSION OR BUILD_PRIV_PLUGINS) AND EXISTS ${PRIV_ICO_RES})
      set(EXEC_ICO_RES ${PRIV_ICO_RES})
    else()
      set(EXEC_ICO_RES res/windows_ico.rc)
    endif()

    list(APPEND _SOURCES ${EXEC_ICO_RES})
  endif()

  add_executable(${TARGET} ${_SOURCES})

  if(WIN32)
    set_target_properties(${TARGET} PROPERTIES
                          WIN32_EXECUTABLE TRUE
                          LINK_FLAGS "-mwindows")
  elseif(APPLE)
    set_target_properties(${TARGET} PROPERTIES
                          MACOSX_BUNDLE FALSE
                          BUILD_WITH_INSTALL_RPATH TRUE
                          INSTALL_NAME_DIR "@executable_path/../Frameworks")
  endif()

  set(PGM_TARGET ${TARGET} PARENT_SCOPE)
endfunction()

# This function configure the default includes and link libraries
# as being the libs subprojects sources and generated libs
function(pgm_set_default_inc_libs TARGET)
  target_include_directories(${TARGET} PRIVATE
	  ${LIBCANVAS_INC}
	  ${LIBCLI_INC}
	  ${LIBCONNECTOR_INC}
	  ${LIBCORE_INC}
	  ${LIBGUI_INC}
	  ${LIBPARSERS_INC}
	  ${LIBUTILS_INC})

  target_link_libraries(${TARGET} PRIVATE
	  canvas
	  cli
	  connector
	  core
	  gui
	  parsers
	  utils)
endfunction()

# This function wraps a call to qt_add_library and set a
# variable named PGM_TARGET in the parent scope (where the function
# was called). The library is always built as with option SHARED
function(pgm_add_library TARGET)
  string(TOUPPER  "${TARGET}_SYMBOLS" LIB_SYMBOLS)
  add_compile_definitions(${LIB_SYMBOLS})

  add_library(${TARGET} SHARED ${ARGN})

	if(APPLE)
		set_target_properties(${TARGET} PROPERTIES
				BUILD_WITH_INSTALL_RPATH TRUE
				INSTALL_NAME_DIR "@executable_path/../Frameworks")
	endif()

  set(PGM_TARGET ${TARGET} PARENT_SCOPE)
endfunction()

# This function configures the deployment settings of a library.
function(pgm_install_library TARGET)
    install(TARGETS ${TARGET}
        LIBRARY DESTINATION ${PGM_PRIVATELIBDIR}
        FRAMEWORK DESTINATION ${PGM_PRIVATELIBDIR}
        RUNTIME DESTINATION ${PGM_PRIVATELIBDIR})
endfunction()

# This function configures the deployment settings of an executable.
# The IS_PRIVBIN, when true, indicates that the output folder of the binary
# must be the one configured in PGM_PRIVATEBINDIR otherwise will be PGM_BINDIR
function(pgm_install_executable TARGET IS_PRIVBIN)
    if(IS_PRIVBIN)
        set(DEST ${PGM_PRIVATEBINDIR})
    else()
        set(DEST ${PGM_BINDIR})
    endif()

    install(TARGETS ${TARGET}
				BUNDLE DESTINATION ${DEST}
        RUNTIME DESTINATION ${DEST})
endfunction()
