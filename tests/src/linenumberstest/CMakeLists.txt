qt_add_executable(linenumberstest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    linenumberstest.cpp
)

# target_include_directories(linenumberstest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(linenumberstest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
