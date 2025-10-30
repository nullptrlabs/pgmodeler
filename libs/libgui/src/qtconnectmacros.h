/*
# Projeto: Modelador de Banco de Dados PostgreSQL (pgsqlDBM)
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
\ingroup libgui
\brief Implements the macros that replaces the Qt's signal/slot connect method for cases
when a slot throws exceptions that need to be treated in the limits of the slot execution
preventing the exception to be redirected to the outside world which may lead to
unexpected behavior and crashes.
*/

#ifndef QT_CONNECT_MACROS_H
#define QT_CONNECT_MACROS_H

#include <QObject>

//! \brief This macro expands to a block of code inside a try/catch blockz
#define __trycatch(body) \
try \
{ \
	body \
} \
catch(Exception &e) \
{ \
	Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE); \
}

/*! \brief This macro expands to a slot method call to be used with QObject::connect method.
 * The macro takes as arguments the receiver object, and the method to be executed that is
 * owned by the receiver object (slot). Only slots that TAKE NO ARGUMENTS are accepted.
 *
 * The expanded macro encloses the slot call in a try/catch block inside a
 * lambda function in order to self-contain all exceptions that eventually go
 * outside the slot scope, avoiding undefined behavior and crashes.
 *
 * NOTES:
 *
 * 1) Differently from the original Qt's signal/slot connection, the slot passed
 * to the macro MUST NOT be the reference (address) of the receiver's method.
 *
 * 2) The macro uses the receiver object as the lambda captured object/variable, so
 * it's not possible to specify other lambda capture modes. If this is needed then
 * you have to create a commom connect call using a lambda as slot and specify
 * the needed captures.
 *
 * Usage:
 *
 * > Qt's original
 *		connect(sender, &SenderClass::s_someSignal, receiver, &ReceiverClass::someMethod)
 *
 * > Macro (note the absence of & in slot specification)
 *		connect(sender, &SenderClass::s_someSignal, context, __slot(receiver, ReceiverClass::someMethod))
 *
 * > The expanded result of the macro usage will be:
 *		connect(sender, &SenderClass::s_someSignal, context, [receiver](){
 *			try {
 *				receiver->ReceiverClass::someMethod();
 *			}
 *			catch(Exception &e) {
 *				Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
 *			}
 *		});
 */
#define __slot(receiver, method) \
[receiver]() { \
	__trycatch( (receiver)->method(); ) \
}

/*! \brief This macro expands to a slot method call to be used with QObject::connect method.
 * The macro takes as arguments the receiver object, and the method to be executed that is
 * owned by the receiver object (slot). Only slots that TAKE ONE OR MORE ARGUMENTS are accepted.
 *
 * The expanded macro encloses the slot call in a try/catch block inside a
 * lambda function in order to self-contain all exceptions that eventually go
 * outside the slot scope, avoiding undefined behavior and crashes.
 *
 * NOTES:
 *
 * 1) Differently from the original Qt's signal/slot connection, the slot passed
 * to the macro MUST NOT be the reference (address) of the receiver's method.
 *
 * 2) The macro uses the receiver object as the lambda captured object/variable, so
 * it's not possible to specify other lambda capture modes. If this is needed then
 * you have to create a commom connect call using a lambda as slot and specify
 * the needed captures.
 *
 * Usage:
 *
 * > Qt's original:
 *		connect(sender, &SenderClass::s_someSignal, receiver, &ReceiverClass::someMethod)
 *
 * > Macro (note the absence of & in slot specification):
 *		connect(sender, &SenderClass::s_someSignal, context, __slot_n(receiver, ReceiverClass::someMethod))
 *
 * > The expanded result of the macro usage will be:
 *		connect(sender, &SenderClass::s_someSignal, context, [receiver](auto... args){
 *			try {
 *				receiver->ReceiverClass::someMethod(args...);
 *			}
 *			catch(Exception &e) {
 *				Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
 *			}
 *		});
 */
#define __slot_n(receiver, method) \
[receiver](auto... args) { \
	__trycatch( (receiver)->method(args...); ) \
}

/*! \brief This macro is just a placeholder for the original QObject::connect call.
 * It acts just to flag where a original QObject connect is happening.
 * This macro is for TEMPORARY USE and will be replaced by the original call
 * once the signal/slots connections that need to use try/catch versions are refactored.
 */
/* #define q_connect(sender, signal, receiver, slot, ...) \
connect(sender, signal, receiver, slot, ##__VA_ARGS__); */

#endif
