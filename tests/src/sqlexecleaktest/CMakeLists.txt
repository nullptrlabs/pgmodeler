qt_add_executable(sqlexecleaktest WIN32 MACOSX_BUNDLE
    ../../src/pgmodelerunittest.h
    sqlexecleaktest.cpp
)

# SQLExecutionHelper is part of the private core, so only its include directories are needed (the code is in libgui)
pgm_inc_priv_core_sources(sqlexecleaktest OFF)
