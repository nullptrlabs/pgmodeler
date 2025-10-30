set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)

pgm_add_library(connector
    src/catalog.cpp src/catalog.h
    src/connection.cpp src/connection.h
    src/connectorglobal.h
    src/resultset.cpp src/resultset.h)

target_include_directories(${PGM_TARGET} PUBLIC
    ${LIBCORE_INC}
    ${LIBPARSERS_INC}
    ${LIBUTILS_INC}
    ${PGSQL_INC})

target_link_libraries(${PGM_TARGET} PUBLIC
    ${PGSQL_LIBS}
    core
    parsers
    utils)

pgm_install_library(${PGM_TARGET})
