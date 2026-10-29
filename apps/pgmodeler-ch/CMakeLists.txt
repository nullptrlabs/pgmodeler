pgm_add_executable(pgmodeler-ch
    src/crashhandlerform.cpp src/crashhandlerform.h
    src/main.cpp)

pgm_set_default_inc_libs(${PGM_TARGET})
pgm_install_executable(${PGM_TARGET} ON)
