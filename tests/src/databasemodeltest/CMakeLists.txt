qt_add_executable(databasemodeltest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    databasemodeltest.cpp
)

# target_include_directories(databasemodeltest PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ${LIBUTILS_INC}
# )

# target_link_libraries(databasemodeltest PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
