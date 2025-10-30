pgm_add_executable(pgmodeler
    src/main.cpp
    src/pgmodelerapp.cpp src/pgmodelerapp.h)

pgm_set_default_inc_libs(${PGM_TARGET})
pgm_install_executable(${PGM_TARGET} OFF)

# Additional files deployed only on MacOS
if(APPLE)
    install(FILES res/Info.plist res/PkgInfo
            DESTINATION ${PGM_PREFIX})

    set(BUNDLE_RES_DIR ${PRIV_PLUGINS_RES}/${PGM_TARGET}/Resources)

    if(PLUS_VERSION AND EXISTS ${BUNDLE_RES_DIR})
        install(DIRECTORY ${BUNDLE_RES_DIR}
        DESTINATION ${PGM_PREFIX})
    else()
        install(DIRECTORY res/Resources
        DESTINATION ${PGM_PREFIX})
    endif()
endif()
