qt_add_executable(schemaparserdifftest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    legacyschemaparser.cpp
    legacyschemaparser.h
    schemaparserdifftest.cpp
)

target_compile_definitions(schemaparserdifftest PRIVATE GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
