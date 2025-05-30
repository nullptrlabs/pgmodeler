include(../../pgmodeler.pri)

TEMPLATE = lib
TARGET = connector

# Enables shared library symbols exporting
DEFINES += CONNECTOR_SYMBOLS

HEADERS += src/connectorglobal.h \
	   src/resultset.h \
	   src/connection.h \
	   src/catalog.h

SOURCES += src/resultset.cpp \
	   src/connection.cpp \
	   src/catalog.cpp

unix|windows: LIBS += $$PGSQL_LIB \
		      $$LIBCORE_LIB \
		      $$LIBPARSERS_LIB \
		      $$LIBUTILS_LIB

INCLUDEPATH += $$LIBCORE_INC \
	       $$LIBPARSERS_INC \
	       $$LIBUTILS_INC

DEPENDPATH += $$LIBCORE_ROOT \
	      $$LIBPARSERS_ROOT \
	      $$LIBUTILS_ROOT

# Deployment settings
target.path = $$PRIVATELIBDIR
INSTALLS = target
