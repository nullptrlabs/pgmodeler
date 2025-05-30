include(../apps.pri)

TEMPLATE = app
TARGET = pgmodeler-se

isEqual(PRIVATE_PLUGINS, true) {
  windows:RC_FILE=$$PRIV_RES_FOLDER/$$TARGET/windows_ico.qrc
}

SOURCES += src/main.cpp \
    src/aboutsewidget.cpp \
	    src/schemaeditorform.cpp \
			src/sourceeditorwidget.cpp

FORMS += \
	 ui/aboutsewidget.ui \
	 ui/schemaeditorform.ui \
	 ui/sourceeditorwidget.ui

HEADERS += \
	   src/aboutsewidget.h \
	   src/schemaeditorform.h \
	   src/sourceeditorwidget.h

INCLUDEPATH += ../pgmodeler/src

DEPENDPATH += ../pgmodeler \
../pgmodeler/src

# Forcing the linker to use the objects files generated
# during the compilation of the main executable since
# pgmodeler-se uses the same application class
windows: OBJECTS += ../pgmodeler/obj/pgmodelerapp.obj ../pgmodeler/obj/moc_pgmodelerapp.obj
else: OBJECTS += ../pgmodeler/obj/pgmodelerapp.o ../pgmodeler/obj/moc_pgmodelerapp.o

# Deployment settings
target.path = $$PRIVATEBINDIR
INSTALLS = target

# Print the current build settings (see pgmodeler.pri)
printBuildDetails()
