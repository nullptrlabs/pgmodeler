#ifndef CLI_GLOBAL_H
#define CLI_GLOBAL_H

#include <QtCore/QtGlobal>

#ifdef CLI_SYMBOLS
	#define __libcli Q_DECL_EXPORT
#else
	#define __libcli Q_DECL_IMPORT
#endif

#endif
