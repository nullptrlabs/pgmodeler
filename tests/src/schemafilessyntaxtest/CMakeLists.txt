qt_add_executable(schemafilessyntaxtest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    schemafilessyntaxtest.cpp
)

# target_include_directories(schemafilessyntaxtest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(schemafilessyntaxtest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
