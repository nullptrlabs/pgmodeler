qt_add_executable(fileselectortest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    fileselectortest.cpp
)

# target_include_directories(fileselectortest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(fileselectortest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
