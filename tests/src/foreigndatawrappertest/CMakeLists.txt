qt_add_executable(foreigndatawrappertest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    foreigndatawrappertest.cpp
)

# target_include_directories(foreigndatawrappertest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(foreigndatawrappertest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
