qt_add_executable(clitest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    clitest.cpp
)

# The tests run the pgmodeler-cli executable straight from the build tree, so
# it must be built first and the directories of the libraries it was linked
# against are handed to the test in order to be added to the library search path
add_dependencies(clitest pgmodeler-cli)

target_compile_definitions(clitest PRIVATE
    CLI_BIN="$<TARGET_FILE:pgmodeler-cli>"
    CLI_LIBS_PATH="$<TARGET_FILE_DIR:canvas>:$<TARGET_FILE_DIR:cli>:$<TARGET_FILE_DIR:connector>:$<TARGET_FILE_DIR:core>:$<TARGET_FILE_DIR:gui>:$<TARGET_FILE_DIR:parsers>:$<TARGET_FILE_DIR:utils>"
    CLI_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
