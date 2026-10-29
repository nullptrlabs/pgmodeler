qt_add_executable(proceduretest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    proceduretest.cpp
)

# target_include_directories(proceduretest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(proceduretest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
