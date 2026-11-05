qt_add_executable(privcodecompletiontest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    privcodecompletiontest.cpp
)

# PrivCodeCompletionWidget only exists when priv-core is built into the gui target (see libs/libgui/CMakeLists.txt)
if(BUILD_PRIV_CODE)
    target_include_directories(privcodecompletiontest PRIVATE ${PRIV_CORE_INC})
endif()
