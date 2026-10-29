# Below, the user can specify where all generated file can be placed
# through a set of variables, being them:
#
# PGM_PREFIX        -> the root directory where the files will be placed
# PGM_BINDIR        -> where executables accessible by the user resides
# PGM_PRIVATEBINDIR -> where executables not directly accessible by the user resides
# PGM_PRIVATELIBDIR -> where libraries not directly shared through the system resides
# PGM_PLUGINSDIR    -> where third party plugins are installed
# PGM_SHAREDIR      -> where shared files and resources should be placed
# PGM_CONFDIR       -> where the pgModeler's configuration folder (conf) resides
# PGM_DOCDIR        -> where documentation related files are placed
# PGM_LANGDIR       -> where the UI translation folder (lang) resides
# PGM_SAMPLESDIR    -> where the sample models folder (samples) resides
# PGM_SCHEMASDIR    -> where the object's schemas folder (schema) resides

# Default configuration for package pgModeler.
# The default prefix is /usr/local
if(NOT DEFINED PGM_PREFIX)
		set(PGM_PREFIX /Applications/pgModeler.app/Contents)
endif()

if(NOT DEFINED PGM_BINDIR)
		set(PGM_BINDIR ${PGM_PREFIX}/MacOS)
endif()

if(NOT DEFINED PGM_PRIVATEBINDIR)
		set(PGM_PRIVATEBINDIR ${PGM_BINDIR})
endif()

if(NOT DEFINED PGM_PRIVATELIBDIR)
		set(PGM_PRIVATELIBDIR ${PGM_PREFIX}/Frameworks)
endif()

if(NOT DEFINED PGM_PLUGINSDIR)
		set(PGM_PLUGINSDIR ${PGM_BINDIR}/plugins)
endif()

if(NOT DEFINED PGM_SHAREDIR)
		set(PGM_SHAREDIR ${PGM_BINDIR})
endif()

if(NOT DEFINED PGM_CONFDIR)
		set(PGM_CONFDIR ${PGM_BINDIR}/conf)
endif()

if(NOT DEFINED PGM_DOCDIR)
		set(PGM_DOCDIR ${PGM_SHAREDIR})
endif()

if(NOT DEFINED PGM_LANGDIR)
		set(PGM_LANGDIR ${PGM_SHAREDIR}/lang)
endif()

if(NOT DEFINED PGM_SAMPLESDIR)
		set(PGM_SAMPLESDIR ${PGM_SHAREDIR}/samples)
endif()

if(NOT DEFINED PGM_SCHEMASDIR)
		set(PGM_SCHEMASDIR ${PGM_BINDIR}/schemas)
endif()
