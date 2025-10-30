qt_add_executable(csvparsertest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    csvparsertest.cpp
)

# target_include_directories(csvparsertest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(csvparsertest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
