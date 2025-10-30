set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)

pgm_add_library(parsers
    src/attributes.h
    src/csvdocument.cpp src/csvdocument.h
    src/csvparser.cpp src/csvparser.h
    src/parsersglobal.h
    src/schemaparser.cpp src/schemaparser.h
    src/xmlparser.cpp src/xmlparser.h)

target_include_directories(${PGM_TARGET} PUBLIC
    ${LIBUTILS_INC}
    ${XML2_INC})

target_link_libraries(${PGM_TARGET} PUBLIC
    ${XML2_LIBS}
    utils)

pgm_install_library(${PGM_TARGET})
