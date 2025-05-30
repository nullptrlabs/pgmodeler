# This file contains the main variables shared by executables subprojects

include(../pgmodeler.pri)

unix|windows: LIBS += $$LIBGUI_LIB \
		      $$LIBCANVAS_LIB \
		      $$LIBCONNECTOR_LIB \
		      $$LIBCORE_LIB \
		      $$LIBPARSERS_LIB \
					$$LIBUTILS_LIB \
					$$LIBCLI_LIB

INCLUDEPATH += $$LIBGUI_INC \
	       $$LIBCANVAS_INC \
	       $$LIBCONNECTOR_INC \
	       $$LIBCORE_INC \
	       $$LIBPARSERS_INC \
				 $$LIBUTILS_INC \
				 $$LIBCLI_INC

DEPENDPATH += $$LIBGUI_ROOT \
	      $$LIBCANVAS_ROOT \
	      $$LIBCONNECTOR_ROOT \
	      $$LIBCORE_ROOT \
	      $$LIBPARSERS_ROOT \
				$$LIBUTILS_ROOT \
				$$LIBCLI_ROOT


unix:LIBS += $$QMAKE_LIBS_EXECINFO
windows:RCC_DIR=src/

isEqual(PRIVATE_PLUGINS, false) {
  windows:RC_FILE=res/windows_ico.qrc
}
