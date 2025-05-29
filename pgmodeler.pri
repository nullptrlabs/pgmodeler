# This file contains the main variables settings to build pgModeler on all supported platforms.

# General Qt settings
QT += core widgets printsupport network svg
CONFIG += ordered qt stl rtti exceptions warn_on c++17
TEMPLATE = subdirs
MOC_DIR = moc
OBJECTS_DIR = obj
UI_DIR = src

contains(CONFIG, debug):{
  DEFINES+=PGMODELER_DEBUG

  # Enabling ccache (https://ccache.dev) in debug mode to speed up recompilations
  isEqual(USE_CCACHE, true):CONFIG+=ccache
}

# Disables all the APIs deprecated before Qt 6.4.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060400
!defined(NO_CHECK_CURR_VER, var):DEFINES+=CHECK_CURR_VER

# Forcing the compilation using Qt 6.4.x
!versionAtLeast(QT_VERSION, "6.4.0") {
  error("Unsupported Qt version detected: $${QT_VERSION}! pgModeler must be compiled with at least Qt 6.4.0")
}

# Store the absolute paths to library subprojects to be referenced in other .pro files
# *_ROOT -> the path to the root folder of the subproject
# *_LIB -> the library flags (-L -l) (LIBS on qmake) passed to the compiler that points to the library generated from a subproject
# *_INC -> the path to the source code folder (src), used by the flag -I (INCLUDEPATH on qmake) passed to the compiler

LIBCANVAS = libcanvas
LIBCANVAS_ROOT = $$absolute_path($$PWD/libs/$$LIBCANVAS)
LIBCANVAS_INC = $$LIBCANVAS_ROOT/src $$top_builddir/libs/$$LIBCANVAS/src

LIBCONNECTOR = libconnector
LIBCONNECTOR_ROOT = $$absolute_path($$PWD/libs/$$LIBCONNECTOR)
LIBCONNECTOR_INC = $$LIBCONNECTOR_ROOT/src $$top_builddir/libs/$$LIBCONNECTOR/src

LIBCORE = libcore
LIBCORE_ROOT = $$absolute_path($$PWD/libs/$$LIBCORE)
LIBCORE_INC = $$LIBCORE_ROOT/src $$top_builddir/libs/$$LIBCORE/src

LIBPARSERS = libparsers
LIBPARSERS_ROOT = $$absolute_path($$PWD/libs/$$LIBPARSERS)
LIBPARSERS_INC = $$LIBPARSERS_ROOT/src $$top_builddir/libs/$$LIBPARSERS/src

LIBGUI = libgui
LIBGUI_ROOT = $$absolute_path($$PWD/libs/$$LIBGUI)
LIBGUI_INC = $$LIBGUI_ROOT/src $$top_builddir/libs/$$LIBGUI/src

LIBUTILS = libutils
LIBUTILS_ROOT = $$absolute_path($$PWD/libs/$$LIBUTILS)
LIBUTILS_INC = $$LIBUTILS_ROOT/src $$top_builddir/libs/$$LIBUTILS/src

LIBCLI = libcli
LIBCLI_ROOT = $$absolute_path($$PWD/libs/$$LIBCLI)
LIBCLI_INC = $$LIBCLI_ROOT/src $$top_builddir/libs/$$LIBCLI

macx|unix {
	LIBCANVAS_LIB = -L$$LIBCANVAS_ROOT -lcanvas
	LIBCONNECTOR_LIB = -L$$LIBCONNECTOR_ROOT -lconnector
	LIBCORE_LIB = -L$$LIBCORE_ROOT -lcore
	LIBPARSERS_LIB = -L$$LIBPARSERS_ROOT -lparsers
	LIBGUI_LIB = -L$$LIBGUI_ROOT -lgui
	LIBUTILS_LIB = -L$$LIBUTILS_ROOT -lutils
	LIBCLI_LIB = -L$$LIBCLI_ROOT -lcli
}
windows {
	CONFIG(release): c = release
	CONFIG(debug, debug|release): c = debug

	LIBCANVAS_LIB = -L$$top_builddir/libs/libcanvas/$$c -lcanvas
	LIBCONNECTOR_LIB = -L$$top_builddir/libs/libconnector/$$c -lconnector
	LIBCORE_LIB = -L$$top_builddir/libs/libcore/$$c -lcore
	LIBPARSERS_LIB = -L$$top_builddir/libs/libparsers/$$c -lparsers
	LIBGUI_LIB = -L$$top_builddir/libs/libgui/$$c -lgui
	LIBUTILS_LIB = -L$$top_builddir/libs/libutils/$$c -lutils
	LIBCLI_LIB = -L$$top_builddir/libs/libcli/$$c -lcli
}

# Set the flag passed to compiler to indicate a snapshot build
isEqual(SNAPSHOT_BUILD, true): DEFINES+=SNAPSHOT_BUILD

# Set the flag passed to compiler to build the demo version
isEqual(DEMO_VERSION, true) {
 DEFINES+=DEMO_VERSION
 unset(PRIVATE_PLUGINS)
}

# Set up the flag passed to compiler to disable all code related to update checking
isEqual(NO_UPDATE_CHECK, true): DEFINES+=NO_UPDATE_CHECK

# Set up the plugin folder to be used
PLUGINS_FOLDER=plugins
isEqual(PRIVATE_PLUGINS, true) {
  DEFINES+=PRIVATE_PLUGINS_SYMBOLS
  PLUGINS_FOLDER=priv-plugins
  PRIV_RES_FOLDER=$$PWD/$$PLUGINS_FOLDER/res
}

# Include the plugins subprojects only if exists
PLUGINS_SRC_ROOT=$$PWD/$$PLUGINS_FOLDER
PLUGINS_PRO_FILE=$$PLUGINS_SRC_ROOT/$${PLUGINS_FOLDER}.pro
INCLUDEPATH+=$$PLUGINS_SRC_ROOT/src

# Properly defining build number/date constant
unix {
 BUILDNUM=$$system("$$PWD/getbuildnum.sh")
 BUILDDATE=$$system("date '+%Y%m%d'")
} else {
 BUILDNUM=$$system("$$PWD/getbuildnum.bat")
 BUILDDATE=$$system("$$PWD/getbuildnum.bat")
}

DEFINES+=BUILDNUM=\\\"$${BUILDNUM}\\\"
DEFINES+=BUILDDATE=\\\"$${BUILDDATE}\\\"

# Below, the user can specify where all generated file can be placed
# through a set of variables, being them:
#
# PREFIX        -> the root directory where the files will be placed
# BINDIR        -> where executables accessible by the user resides
# PRIVATEBINDIR -> where executables not directly accessible by the user resides
# PRIVATELIBDIR -> where libraries not directly shared through the system resides
# PLUGINSDIR    -> where third party plugins are installed
# SHAREDIR      -> where shared files and resources should be placed
# CONFDIR       -> where the pgModeler's configuration folder (conf) resides
# DOCDIR        -> where documentation related files are placed
# LANGDIR       -> where the UI translation folder (lang) resides
# SAMPLESDIR    -> where the sample models folder (samples) resides
# SCHEMASDIR    -> where the object's schemas folder (schema) resides
#
# The values of each variable changes between supported platforms and are describe as follow


# Linux custom variables settings
linux {
  CONFIG += x11

  # If the AppImage generation option is set
  isEqual(APPIMAGE_BUILD, true):{

  # Set the flag passed to compiler to indicate a appimage build
  DEFINES+=APPIMAGE_BUILD

  !defined(PREFIX, var): PREFIX = /usr/local/pgmodeler-appimage
  BINDIR = $$PREFIX
  PRIVATEBINDIR = $$PREFIX
  PRIVATELIBDIR = $$PREFIX/lib
  PLUGINSDIR = $$PREFIX/lib/pgmodeler/plugins
  SHAREDIR = $$PREFIX
  CONFDIR = $$SHAREDIR/conf
  DOCDIR = $$SHAREDIR
  LANGDIR = $$SHAREDIR/lang
  SAMPLESDIR = $$SHAREDIR/samples
  SCHEMASDIR = $$SHAREDIR/schemas
  }

  !isEqual(APPIMAGE_BUILD, true):{
  # Default configuration for package pgModeler.
  # The default prefix is /usr/local
  !defined(PREFIX, var):        PREFIX = /usr/local
  !defined(BINDIR, var):        BINDIR = $$PREFIX/bin
  !defined(PRIVATEBINDIR, var): PRIVATEBINDIR = $$PREFIX/bin
  !defined(PRIVATELIBDIR, var): PRIVATELIBDIR = $$PREFIX/lib/pgmodeler
  !defined(PLUGINSDIR, var):    PLUGINSDIR = $$PREFIX/lib/pgmodeler/plugins
  !defined(SHAREDIR, var):      SHAREDIR = $$PREFIX/share/pgmodeler
  !defined(CONFDIR, var):       CONFDIR = $$SHAREDIR/conf
  !defined(DOCDIR, var):        DOCDIR = $$SHAREDIR
  !defined(LANGDIR, var):       LANGDIR = $$SHAREDIR/lang
  !defined(SAMPLESDIR, var):    SAMPLESDIR = $$SHAREDIR/samples
  !defined(SCHEMASDIR, var):    SCHEMASDIR = $$SHAREDIR/schemas
 }

  # Specifies where to find the libraries at runtime
  RELATIVE_PRIVATELIBDIR = $$relative_path($$PRIVATELIBDIR, $$BINDIR)
  QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\' -Wl,-rpath,\'\$$ORIGIN/$$RELATIVE_PRIVATELIBDIR\'"

  # Forcing the display of some warnings
  CONFIG(debug, debug|release): QMAKE_CXXFLAGS += "-Wall -Wextra -Wuninitialized"
}


# Windows custom variables settings
windows {
  CONFIG += windows

  # Removing optimization flag -O2 (level 2) and replacing by
  # level 0 (the default according to GCC docs) because at level 2
  # in MingW Clang/GCC some C++17 techniques don't work properly
  # (e.g. inline static class members). Maybe it's a bug in the code is wrong (more likely)
  # or is a bug in the compiler? The fact is that reducing the executable/libs code
  # optmization causes the tool to work as expected.
  gcc:QMAKE_CXXFLAGS_RELEASE -= -O2
  gcc:QMAKE_CXXFLAGS_RELEASE *= -O0

  # The default prefix is ./build
  !defined(PREFIX, var):        PREFIX = $$PWD/build
  !defined(BINDIR, var):        BINDIR = $$PREFIX
  !defined(PRIVATEBINDIR, var): PRIVATEBINDIR = $$PREFIX
  !defined(PRIVATELIBDIR, var): PRIVATELIBDIR = $$PREFIX
  !defined(PLUGINSDIR, var):    PLUGINSDIR = $$PREFIX/plugins
  !defined(SHAREDIR, var):      SHAREDIR = $$PREFIX
  !defined(CONFDIR, var):       CONFDIR = $$PREFIX/conf
  !defined(DOCDIR, var):        DOCDIR = $$PREFIX
  !defined(LANGDIR, var):       LANGDIR = $$PREFIX/lang
  !defined(SAMPLESDIR, var):    SAMPLESDIR = $$PREFIX/samples
  !defined(SCHEMASDIR, var):    SCHEMASDIR = $$PREFIX/schemas
}


# MacOS X custom variables settings
macx {
  CONFIG -= app_bundle

  # The default prefix is ./build/pgmodeler.app/Contents
  !defined(PREFIX, var):        PREFIX = /Applications/pgModeler.app/Contents
  !defined(BINDIR, var):        BINDIR = $$PREFIX/MacOS
  !defined(PRIVATEBINDIR, var): PRIVATEBINDIR = $$BINDIR
  !defined(PRIVATELIBDIR, var): PRIVATELIBDIR = $$PREFIX/Frameworks
  !defined(PLUGINSDIR, var):    PLUGINSDIR = $$BINDIR/plugins
  !defined(SHAREDIR, var):      SHAREDIR = $$BINDIR
  !defined(CONFDIR, var):       CONFDIR = $$BINDIR/conf
  !defined(DOCDIR, var):        DOCDIR = $$BINDIR
  !defined(LANGDIR, var):       LANGDIR = $$BINDIR/lang
  !defined(SAMPLESDIR, var):    SAMPLESDIR = $$BINDIR/samples
  !defined(SCHEMASDIR, var):    SCHEMASDIR = $$BINDIR/schemas

  # Specifies where to find the libraries at runtime
  QMAKE_RPATHDIR += @executable_path/../Frameworks
  QMAKE_LFLAGS_SONAME = -Wl,-install_name,@loader_path/../Frameworks/
}

# Creating constants based upon the custom paths so the GlobalAttributes
# namespace can correctly configure the paths inside the code
DEFINES += BINDIR=\\\"$${BINDIR}\\\" \
           PLUGINSDIR=\\\"$${PLUGINSDIR}\\\" \
           PRIVATEBINDIR=\\\"$${PRIVATEBINDIR}\\\" \
           CONFDIR=\\\"$${CONFDIR}\\\" \
           DOCDIR=\\\"$${DOCDIR}\\\" \
           LANGDIR=\\\"$${LANGDIR}\\\" \
           SAMPLESDIR=\\\"$${SAMPLESDIR}\\\" \
           SCHEMASDIR=\\\"$${SCHEMASDIR}\\\"


# pgModeler depends on libpq and libxml2 this way to variables
# are define so the compiler can find the libs at link time.
#
# PGSQL_LIB -> Full path to libpq.(so | dll | dylib)
# PGSQL_INC -> Root path where PgSQL includes can be found
#
# XML_LIB   -> Full path to libxml2.(so | dll | dylib)
# XML_INC   -> Root path where XML2 includes can be found

linux: {
  # If all custom variables PGSQL_??? and XML_??? are defined
  # Then we use them instead of discovering the paths via pkg-config
  dep_paths = "$$PGSQL_LIB" "$$XML_LIB" "$$PGSQL_INC" "$$XML_INC"
  if(count(dep_paths, 4)):{
    INCLUDEPATH += "$$PGSQL_INC" "$$XML_INC"
    has_dep_paths = true
  }

  # If not all of the PGSQL_??? and XML_??? vars are defined
  # Then we default to use pkg-config for libpq and libxml-2.0
  !defined(has_dep_paths,var): {
    CONFIG += link_pkgconfig
    PKGCONFIG = libpq libxml-2.0
    PGSQL_LIB = -lpq
    XML_LIB = -lxml2
  }
}

macx {
  !defined(PGSQL_LIB, var): PGSQL_LIB = /Library/PostgreSQL/15/lib/libpq.dylib
  !defined(PGSQL_INC, var): PGSQL_INC = /Library/PostgreSQL/15/include
  !defined(XML_INC, var): XML_INC = /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/libxml2
  !defined(XML_LIB, var): XML_LIB = /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib/libxml2.tbd
  INCLUDEPATH += "$$PGSQL_INC" "$$XML_INC"
}

windows {
  !defined(PGSQL_LIB, var): PGSQL_LIB = C:/msys64/mingw64/bin/libpq.dll
  !defined(PGSQL_INC, var): PGSQL_INC = C:/msys64/mingw64/include
  !defined(XML_INC, var): XML_INC = C:/msys64/mingw64/include/libxml2
  !defined(XML_LIB, var): XML_LIB = C:/msys64/mingw64/bin/libxml2-2.dll
  INCLUDEPATH += "$$PGSQL_INC" "$$XML_INC"
}

linux:defined(has_dep_paths,var) | macx | windows : {
  !exists($$PGSQL_LIB) {
    PKG_ERROR = "PostgreSQL libraries"
    VARIABLE = "PGSQL_LIB"
    VALUE = $$PGSQL_LIB
  }

  !exists($$PGSQL_INC/libpq-fe.h) {
    PKG_ERROR = "PostgreSQL headers"
    VARIABLE = "PGSQL_INC"
    VALUE = $$PGSQL_INC
  }

  !exists($$XML_LIB) {
    PKG_ERROR = "XML2 libraries"
    VARIABLE = "XML_LIB"
    VALUE = $$XML_LIB
  }

  !exists($$XML_INC) {
    PKG_ERROR = "XML2 headers"
    VARIABLE = "XML_INC"
    VALUE = $$XML_INC
  }

  !isEmpty(PKG_ERROR) {
    warning("$$PKG_ERROR were not found at \"$$VALUE\"!")
    warning("Please correct the value of $$VARIABLE and try again!")
    error("pgModeler compilation aborted.")
  }
}

# Define a custom function to print build details
defineTest(printBuildDetails) {
 LB=$$escape_expand(\n)
 log($$LB)
 log("** pgModeler build details ** $$LB $$LB")
 log("  PREFIX        = $$PREFIX $$LB")
 log("  BINDIR        = $$BINDIR $$LB")
 log("  PRIVATEBINDIR = $$PRIVATEBINDIR $$LB")
 log("  PRIVATELIBDIR = $$PRIVATELIBDIR $$LB")
 log("  PLUGINSDIR    = $$PLUGINSDIR $$LB")
 log("  SHAREDIR      = $$SHAREDIR $$LB")
 log("  CONFDIR       = $$CONFDIR $$LB")
 log("  DOCDIR        = $$DOCDIR $$LB")
 log("  LANGDIR       = $$LANGDIR $$LB")
 log("  SAMPLESDIR    = $$SAMPLESDIR $$LB")
 log("  SCHEMASDIR    = $$SCHEMASDIR $$LB $$LB")
 log("* To change a variable value run qmake again by setting the new desired value, e.g.: $$LB")
 log("  > qmake PREFIX+=/usr/local -r pgmodeler.pro $$LB $$LB")
 log("* To proceed with build process by running: $$LB")
 log("  >  make && make install $$LB")
 log($$LB)
 return(true)
}
