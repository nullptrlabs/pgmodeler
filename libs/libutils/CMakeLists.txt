set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOMOC ON)

pgm_add_library(utils
    src/application.cpp src/application.h
    src/attribsmap.h
    src/customuistyle.cpp src/customuistyle.h
    src/doublenan.h
    src/enumtype.h
    src/exception.cpp src/exception.h
    src/globalattributes.cpp src/globalattributes.h
    src/pgmodelerplugin.cpp src/pgmodelerplugin.h
    src/pgsqlversions.cpp src/pgsqlversions.h
    src/utilsglobal.h
    src/utilsns.cpp src/utilsns.h)

pgm_install_library(${PGM_TARGET})
