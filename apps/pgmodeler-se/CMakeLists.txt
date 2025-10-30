# By default, the AUTOUIC option makes the uic look for the .ui files
# under the same folder as the sources. In this case, we force the uic
# command to look for .ui files under ui/*
set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

pgm_add_executable(pgmodeler-se
    src/aboutsewidget.cpp src/aboutsewidget.h
    src/main.cpp
    src/schemaeditorform.cpp src/schemaeditorform.h
    src/sourceeditorwidget.cpp src/sourceeditorwidget.h
    ui/aboutsewidget.ui
    ui/schemaeditorform.ui
    ui/sourceeditorwidget.ui
    ../pgmodeler/src/pgmodelerapp.cpp)

target_include_directories(${PGM_TARGET} PRIVATE
	${PGM_TARGET}_autogen
	../pgmodeler/src)

pgm_set_default_inc_libs(${PGM_TARGET})
pgm_install_executable(${PGM_TARGET} ON)
