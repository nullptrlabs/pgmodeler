include(../../pgmodeler.pri)

TEMPLATE = lib
TARGET = utils

# Enables shared library symbols exporting
DEFINES += UTILS_SYMBOLS

HEADERS += src/utilsglobal.h \
src/attribsmap.h \
src/customuistyle.h \
src/exception.h \
src/enumtype.h \
src/globalattributes.h \
src/pgsqlversions.h \
src/doublenan.h \
src/application.h \
src/utilsns.h \
src/pgmodelerplugin.h

SOURCES += src/exception.cpp \
src/customuistyle.cpp \
src/globalattributes.cpp \
src/pgsqlversions.cpp \
src/application.cpp \
src/utilsns.cpp \
src/pgmodelerplugin.cpp

# Deployment settings
target.path = $$PRIVATELIBDIR
INSTALLS = target
