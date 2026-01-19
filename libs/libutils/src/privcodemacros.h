/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2025 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

/**
\ingroup libutils
\brief Defines placeholder macros for private code symbols.

This header provides empty macro definitions that serve as placeholders
when compiling the open source version. In the PLUS version, these macros
are redefined in privcoreinit.h with actual code.
*/

#ifndef PRIV_CODE_MACROS_H
#define PRIV_CODE_MACROS_H

#ifdef PRIV_CODE_SYMBOLS
	#include "privcoreinit.h"
	#include "privcoreclasses.h"
#endif

// Define empty placeholder macros only if they haven't been defined yet
// (i.e., privcoreinit.h was not included)
#ifndef __pgm_plus_gui_init
	#define __pgm_plus_gui_init
#endif

#ifndef __pgm_plus_mwnd_sw_decl
	#define __pgm_plus_mwnd_sw_decl
#endif

#ifndef __pgm_plus_mwnd_sw_impl
	#define __pgm_plus_mwnd_sw_impl
#endif

#endif
