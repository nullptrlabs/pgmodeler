qt_add_executable(xmlparsertest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    xmlparsertest.cpp
)

# target_include_directories(xmlparsertest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(xmlparsertest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)


