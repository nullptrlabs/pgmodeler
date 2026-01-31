include(../../pgmodeler.pri)

TEMPLATE = lib
TARGET = cli
windows: DESTDIR = $$PWD

# Enables shared library symbols exporting
DEFINES += CLI_SYMBOLS

# Include priv-core sources directly when building with private plugins
isEqual(PRIVATE_PLUGINS, true) {
	include(../../priv-core/priv-core.pri)
}

SOURCES += src/compat/compatns.cpp \
src/pgmodelercliapp.cpp \
src/compat/view.cpp \
src/compat/reference.cpp \
src/pgmodelercliplugin.cpp

HEADERS += src/cliglobal.h \
src/pgmodelercliapp.h \
src/compat/compatns.h \
src/compat/view.h \
src/compat/reference.h \
src/pgmodelercliplugin.h

isEqual(PRIVATE_PLUGINS, true) {
  HEADERS+=$$PLUGINS_SRC_ROOT/src/privpluginsns.h
  SOURCES+=$$PLUGINS_SRC_ROOT/src/privpluginsns.cpp
}

unix|windows: LIBS += $$LIBCANVAS_LIB \
			  $$LIBCONNECTOR_LIB \
			  $$LIBCORE_LIB \
			  $$LIBPARSERS_LIB \
			  $$LIBUTILS_LIB \
		  $$LIBGUI_LIB

INCLUDEPATH += $$LIBCANVAS_INC \
			 $$LIBCONNECTOR_INC \
			 $$LIBCORE_INC \
			 $$LIBPARSERS_INC \
			 $$LIBUTILS_INC \
			 $$LIBGUI_INC \
			 $$PWD/src/compat

DEPENDPATH += $$LIBCANVAS_ROOT \
			$$LIBCONNECTOR_ROOT \
			$$LIBCORE_ROOT \
			$$LIBPARSERS_ROOT \
			$$LIBUTILS_ROOT \
			$$LIBGUI_ROOT
