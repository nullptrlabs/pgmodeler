#ifndef UTILS_GLOBAL_H
#define UTILS_GLOBAL_H

#include <QtCore/QtGlobal>

#ifdef UTILS_SYMBOLS
	#define __libutils Q_DECL_EXPORT
#else
	#define __libutils Q_DECL_IMPORT
#endif

#endif
