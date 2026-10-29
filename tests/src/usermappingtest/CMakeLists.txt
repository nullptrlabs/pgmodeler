qt_add_executable(usermappingtest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    usermappingtest.cpp
)

# target_include_directories(usermappingtest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(usermappingtest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)

