qt_add_executable(partrelationshiptest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    partrelationshiptest.cpp
)

# target_include_directories(partrelationshiptest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(partrelationshiptest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
