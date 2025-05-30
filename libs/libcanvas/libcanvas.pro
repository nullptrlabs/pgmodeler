include(../../pgmodeler.pri)

TEMPLATE = lib
TARGET = canvas

# Enables shared library symbols exporting
DEFINES += CANVAS_SYMBOLS

HEADERS += src/canvasglobal.h \
		src/baseobjectview.h \
		src/layeritem.h \
	    src/textboxview.h \
	    src/tableview.h \
	    src/graphicalview.h \
	    src/relationshipview.h \
	    src/tabletitleview.h \
	    src/tableobjectview.h \
	    src/basetableview.h \
	    src/objectsscene.h \
            src/schemaview.h \
            src/roundedrectitem.h \
            src/styledtextboxview.h \
	    src/beziercurveitem.h \
	    src/textpolygonitem.h \
    src/attributestoggleritem.h

SOURCES +=  src/baseobjectview.cpp \
	src/layeritem.cpp \
	    src/textboxview.cpp \
	    src/tableview.cpp \
	    src/graphicalview.cpp \
	    src/relationshipview.cpp \
	    src/tabletitleview.cpp \
	    src/tableobjectview.cpp \
	    src/basetableview.cpp \
	    src/objectsscene.cpp \
	    src/schemaview.cpp \
            src/roundedrectitem.cpp \
            src/styledtextboxview.cpp \
	    src/beziercurveitem.cpp \
	    src/textpolygonitem.cpp \
    src/attributestoggleritem.cpp

unix|windows: LIBS += $$LIBCORE_LIB \
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
