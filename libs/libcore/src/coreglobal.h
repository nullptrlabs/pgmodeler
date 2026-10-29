#ifndef CORE_GLOBAL_H
#define CORE_GLOBAL_H

#include <QtCore/QtGlobal>

#ifdef CORE_SYMBOLS
	#define __libcore Q_DECL_EXPORT
#else
	#define __libcore Q_DECL_IMPORT
#endif

#endif
