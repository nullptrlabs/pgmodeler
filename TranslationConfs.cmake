# Compilation and deployment of the UI translations (.ts -> .qm)
# The LinguistTools component (lrelease) is optional: without it the build
# goes on and the translations are simply not compiled/deployed
find_package(Qt6 6.4.0 QUIET COMPONENTS LinguistTools)

if(NOT TARGET Qt6::lrelease)
    message(WARNING "Qt6 LinguistTools not found: the translations (.ts) will not be compiled nor deployed.")
endif()

# This function compiles the .ts files (the remaining arguments) in .qm files, using lrelease, and
# configures their deployment to INSTALL_DIR. The .qm files take the name of the .ts files
# (e.g. pt_BR.ts -> pt_BR.qm) and are generated under <current binary dir>/lang. The compilation
# is done by the target TARGET, which is part of the default build.
function(pgm_add_translations TARGET INSTALL_DIR)
    if(NOT TARGET Qt6::lrelease)
        return()
    endif()

    set(_QM_DIR ${CMAKE_CURRENT_BINARY_DIR}/lang)

    foreach(_TS ${ARGN})
        get_filename_component(_NAME ${_TS} NAME_WLE)
        set(_QM ${_QM_DIR}/${_NAME}.qm)

        add_custom_command(
            OUTPUT ${_QM}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${_QM_DIR}
            COMMAND Qt6::lrelease ${_TS} -qm ${_QM}
            DEPENDS ${_TS}
            COMMENT "Compiling translation ${_NAME}.qm"
            VERBATIM)

        list(APPEND _QM_FILES ${_QM})
    endforeach()

    add_custom_target(${TARGET} ALL DEPENDS ${_QM_FILES})

    install(FILES ${_QM_FILES}
            DESTINATION ${INSTALL_DIR})
endfunction()
