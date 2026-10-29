qt_add_executable(syntaxhighlightertest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    syntaxhighlightertest.cpp
)

# target_include_directories(syntaxhighlightertest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(syntaxhighlightertest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
