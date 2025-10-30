qt_add_executable(roletest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    roletest.cpp
)

# target_include_directories(roletest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(roletest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
