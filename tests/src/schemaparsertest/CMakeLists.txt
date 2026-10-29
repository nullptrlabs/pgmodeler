qt_add_executable(schemaparsertest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    schemaparsertest.cpp
)

# target_include_directories(schemaparsertest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(schemaparsertest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)

