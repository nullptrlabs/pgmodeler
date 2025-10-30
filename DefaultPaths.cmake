# Defining default values of the path-related variables
if(LINUX)
  include(LinuxPaths)
elseif(WIN32)
  include(WindowsPaths)
elseif(APPLE)
	include(MacOsPaths)
endif()

# Creting C defines with the custom paths provided
# in the cmake command call
add_compile_definitions(
    BINDIR="${PGM_BINDIR}"
    CONFDIR="${PGM_CONFDIR}"
    DOCDIR="${PGM_DOCDIR}"
    LANGDIR="${PGM_LANGDIR}"
    PLUGINSDIR="${PGM_PLUGINSDIR}"
    PRIVATEBINDIR="${PGM_PRIVATEBINDIR}"
    SAMPLESDIR="${PGM_SAMPLESDIR}"
    SCHEMASDIR="${PGM_SCHEMASDIR}")

if(APPLE)
	set(PGSQL_INC /Library/PostgreSQL/17/include)
	set(PGSQL_LIBS /Library/PostgreSQL/17/lib/libpq.dylib)

	set(XML2_INC /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/libxml2)
	set(XML2_LIBS /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib/libxml2.tbd)
else()
	# Searching for PostgreSQL headers/libraries
	# This command attempts to find the library, REQUIRED argument is optional
	find_package(PostgreSQL REQUIRED)
	set(PGSQL_INC ${PostgreSQL_INCLUDE_DIRS})
	set(PGSQL_LIBS ${PostgreSQL_LIBRARIES})

	# Searching for libxml2 headers/libraries
	# This command attempts to find the library, REQUIRED argument is optional
	find_package(LibXml2 REQUIRED)
	set(XML2_INC ${LIBXML2_INCLUDE_DIR})
	set(XML2_LIBS ${LIBXML2_LIBRARY})
endif()

# Subproject variables
# Store the absolute paths to library subprojects to be referenced in other CMakeLists.txt files
# *_ROOT -> the path to the root folder of the subproject
# *_INC -> the path to the source code folder (src), used by the flag -I (INCLUDEPATH on qmake) passed to the compiler
set(CWD ${CMAKE_CURRENT_SOURCE_DIR})

# Root directory where library subprojects are stored
set(LIBS_DIR libs)

# Root directory where application subprojects are stored
set(APPS_DIR apps)

# Application subproject names
set(PGMODELER_APP pgmodeler)
set(PGMODELERCH_APP pgmodeler-ch)
set(PGMODELERCLI_APP pgmodeler-cli)
set(PGMODELERSE_APP pgmodeler-se)

# Library subproject names and source paths
set(LIBCANVAS libcanvas)
set(LIBCANVAS_ROOT ${CWD}/${LIBS_DIR}/${LIBCANVAS})
set(LIBCANVAS_INC ${LIBCANVAS_ROOT}/src)

set(LIBCONNECTOR libconnector)
set(LIBCONNECTOR_ROOT ${CWD}/${LIBS_DIR}/${LIBCONNECTOR})
set(LIBCONNECTOR_INC ${LIBCONNECTOR_ROOT}/src)

set(LIBCORE libcore)
set(LIBCORE_ROOT ${CWD}/${LIBS_DIR}/${LIBCORE})
set(LIBCORE_INC ${LIBCORE_ROOT}/src)

set(LIBPARSERS libparsers)
set(LIBPARSERS_ROOT ${CWD}/${LIBS_DIR}/${LIBPARSERS})
set(LIBPARSERS_INC ${LIBPARSERS_ROOT}/src)

set(LIBGUI libgui)
set(LIBGUI_ROOT ${CWD}/${LIBS_DIR}/${LIBGUI})
set(LIBGUI_INC ${LIBGUI_ROOT}/src)

set(LIBUTILS libutils)
set(LIBUTILS_ROOT ${CWD}/${LIBS_DIR}/${LIBUTILS})
set(LIBUTILS_INC ${LIBUTILS_ROOT}/src)

set(LIBCLI libcli)
set(LIBCLI_ROOT ${CWD}/${LIBS_DIR}/${LIBCLI})
set(LIBCLI_INC ${LIBCLI_ROOT}/src)

# Configuring the relative paths so the binaries can find the libraries correclty
# First we determine the relative path between BINDIR e PRIVATELIBDIR
file(RELATIVE_PATH RELATIVE_PRIVATELIBDIR ${PGM_BINDIR} ${PGM_PRIVATELIBDIR})

# Setting the RPATH to including $ORIGIN and $ORIGIN/relative_path
set(CMAKE_INSTALL_RPATH "\$ORIGIN;\$ORIGIN/${RELATIVE_PRIVATELIBDIR}")

# Making sure that RPATH is used during install
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
