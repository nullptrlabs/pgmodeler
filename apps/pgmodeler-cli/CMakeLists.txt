pgm_add_executable(pgmodeler-cli src/main.cpp)
pgm_set_default_inc_libs(${PGM_TARGET})
pgm_inc_priv_core_sources(${PGM_TARGET} OFF)
pgm_install_executable(${PGM_TARGET} OFF)
