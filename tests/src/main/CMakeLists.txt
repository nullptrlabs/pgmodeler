add_executable(runtests WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    main.cpp
)

# target_include_directories(runtests PRIVATE
#     ${LIBCANVAS_INC}
#     ${LIBCONNECTOR_INC}
#     ${LIBCORE_INC}
#     ${LIBGUI_INC}
#     ${LIBPARSERS_INC}
#     ../../src/
# )

# target_link_libraries(runtests PRIVATE
#     canvas
#     connector
#     core
#     gui
#     parsers
#     utils)
