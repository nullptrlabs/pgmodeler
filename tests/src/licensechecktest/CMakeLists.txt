qt_add_executable(licensechecktest WIN32 MACOSX_BUNDLE
    licensechecktest.cpp
)

# The license check lives in the private core, built into the gui target (see libs/libgui/CMakeLists.txt)
if(BUILD_PRIV_CODE)
    target_include_directories(licensechecktest PRIVATE ${PRIV_CORE_INC})
    target_link_libraries(licensechecktest PRIVATE OpenSSL::Crypto)
endif()
