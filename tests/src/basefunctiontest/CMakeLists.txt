qt_add_executable(basefunctiontest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    basefunctiontest.cpp
)

# target_include_directories(basefunctiontest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(basefunctiontest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
