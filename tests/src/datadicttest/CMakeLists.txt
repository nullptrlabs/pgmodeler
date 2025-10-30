qt_add_executable(datadicttest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    datadicttest.cpp
)

# target_include_directories(datadicttest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(datadicttest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
