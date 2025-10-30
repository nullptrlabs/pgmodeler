qt_add_executable(baseobjecttest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    baseobjecttest.cpp
)

# target_include_directories(baseobjecttest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(baseobjecttest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
