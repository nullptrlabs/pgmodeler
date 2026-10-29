qt_add_executable(servertest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    servertest.cpp
)

# target_include_directories(servertest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(servertest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
