qt_add_executable(transformtest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    transformtest.cpp
)

# target_include_directories(transformtest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(transformtest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
