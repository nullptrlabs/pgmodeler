# Compiling the UI translations (assets/lang/*.ts) and copying the .qm files
# to the folder configured in PGM_LANGDIR (LANGDIR)
file(GLOB PGM_UI_TS_FILES CONFIGURE_DEPENDS assets/lang/*.ts)

if(PGM_UI_TS_FILES)
    pgm_add_translations(translations ${PGM_LANGDIR} ${PGM_UI_TS_FILES})
endif()

# Copying the samples/*.dbm to the destination
install(DIRECTORY assets/samples/
        DESTINATION ${PGM_SAMPLESDIR}
        FILES_MATCHING
        PATTERN "*.dbm")

# Copying all folder under schemas/ to the destination
# Without recreating the schemas/ folder in the destination
file(GLOB sch_dirs LIST_DIRECTORIES true "assets/schemas/*")

foreach(sch_dir ${sch_dirs})
    # Extracts the name of the current schema directory
    get_filename_component(dirname ${sch_dir} NAME)

    if(IS_DIRECTORY ${sch_dir})
        install(DIRECTORY ${sch_dir}
                DESTINATION ${PGM_SCHEMASDIR}
                USE_SOURCE_PERMISSIONS)
    endif()
endforeach()

# Copying private schema folders (catalog/) from priv-core — PLUS builds only
if(PLUS_VERSION OR BUILD_PRIV_ASSETS)
    install(DIRECTORY ${PRIV_CORE_ROOT}/assets/schemas/catalog
            DESTINATION ${PGM_SCHEMASDIR}
            USE_SOURCE_PERMISSIONS)
endif()

# Copying all template configs from assets/conf
install(DIRECTORY assets/conf/
        DESTINATION ${PGM_CONFDIR}
        FILES_MATCHING
        PATTERN "*.conf"
        PATTERN "*.dbm"
        PATTERN "*.png")

if(PLUS_VERSION OR BUILD_PRIV_ASSETS)
		install(FILES ${PRIV_CORE_ROOT}/res/icons/pgmodeler_logo.png
            DESTINATION ${PGM_CONFDIR})
endif()

install(DIRECTORY assets/conf/schemas
        DESTINATION ${PGM_CONFDIR})

install(DIRECTORY assets/conf/dtd
        DESTINATION ${PGM_CONFDIR})

# Copying  the highlight configs from dark theme to be the initial one
install(DIRECTORY assets/conf/themes/dark/
        DESTINATION ${PGM_CONFDIR}
        FILES_MATCHING
        PATTERN "*-highlight.conf")

# Creating the "defaults" configs dir containing the files from the initial template config dir
install(DIRECTORY assets/conf/themes/dark/
        DESTINATION ${PGM_CONFDIR}/defaults
        FILES_MATCHING
        PATTERN "*-highlight.conf")

install(DIRECTORY assets/conf/
        DESTINATION ${PGM_CONFDIR}/defaults
        FILES_MATCHING
        PATTERN "*.conf"
        PATTERN "*.dbm"
        PATTERN "schemas" EXCLUDE
        PATTERN "themes" EXCLUDE
        PATTERN "dtd" EXCLUDE)

# Copying private conf files (diff-presets.*) from priv-core — PLUS builds only
if(PLUS_VERSION OR BUILD_PRIV_ASSETS)
    install(FILES ${PRIV_CORE_ROOT}/assets/conf/diff-presets.conf
            DESTINATION ${PGM_CONFDIR})

    install(FILES ${PRIV_CORE_ROOT}/assets/conf/diff-presets.conf
            DESTINATION ${PGM_CONFDIR}/defaults)

    install(FILES ${PRIV_CORE_ROOT}/assets/conf/dtd/diff-presets.dtd
            DESTINATION ${PGM_CONFDIR}/dtd)

    install(FILES ${PRIV_CORE_ROOT}/assets/conf/schemas/diff-presets.sch
            DESTINATION ${PGM_CONFDIR}/schemas)
endif()

# Copying doc files
install(FILES
        README.md
        CHANGELOG.md
        LICENSE
        RELEASENOTES.md
        DESTINATION ${PGM_DOCDIR})
