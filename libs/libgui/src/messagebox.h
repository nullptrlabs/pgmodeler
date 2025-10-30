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
\ingroup libgui
\class MessageBox
\brief Implements the message box to show errors / alerts / infos to the user.
*/

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "guiglobal.h"
#include "ui_messagebox.h"
#include "exception.h"
#include "widgets/numberedtexteditor.h"

class __libgui Messagebox: public QDialog, public Ui::Messagebox {
	Q_OBJECT

	private:
		NumberedTextEditor *raw_stack_txt;

		int exec() override;

	protected:
		void resizeEvent(QResizeEvent *) override;

	public:
		static constexpr int Canceled = 2;

		//! \brief Constants used to define the message icon
		enum MessageType: unsigned {
			None,
			Error,
			Info,
			Alert,
			Confirm,
			Success
		};

		//! \brief Constants used to configure the visible buttons
		enum ButtonsId {
			YesNoButtons,
			OkCancelButtons,
			OkButton,
			CloseButton,
			AllButtons
		};

		Messagebox(QWidget * parent = nullptr, Qt::WindowFlags f = Qt::Widget);

		/*! \brief Shows the message box defining the icons and available buttons.
		User can specify custom button labels as well custom icons. The icons can be a path to a local file
		or a Qt  resource icon ':/path/icon' and will be enabled only specifing custom labels for the respective button. */
		int show(const QString &title, const QString &msg, MessageType icon_type=None, ButtonsId buttons=OkButton,
							const QString &yes_lbl="", const QString &no_lbl="", const QString &cancel_lbl="",
							const QString &yes_ico="", const QString &no_ico="", const QString &cancel_ico="");

		//! \brief Shows the message box using an excpetion as message
		int show(Exception e, const QString &msg="", MessageType icon_type=Error, ButtonsId buttons=OkButton,
							const QString &yes_lbl="", const QString &no_lbl="", const QString &cancel_lbl="",
							const QString &yes_ico="", const QString &no_ico="", const QString &cancel_ico="");

		//! \brief Shows a simple message box with the title automatically defined by the icon type
		int show(const QString &msg, MessageType icon_type=None, ButtonsId buttons=OkButton);

		/*! \brief Shows an error message box in which can an error code and exact local of the code is specified.
		 *  Additionally, a reference to a captured exception can be specified and will be used to fill up the stack trace.
		 * Also, it uses an error icon and only OK button */
		static void error(const QString &msg, ErrorCode error_code, const QString &method, const QString &file, int line, Exception *e = nullptr);

		/*! \brief Shows an error message box in which can an error code and exact local of the code is specified.
		 *  Additionally, a list of errors captured previously can be specified and will be used to fill up the stack trace.
		 * Also, it uses an error icon and only OK button */
		static void error(const QString &msg, ErrorCode error_code, const QString &method, const QString &file, int line, std::vector<Exception> &exceptions);

		/*! \brief Shows an error message box in which can an error code and exact local of the code is specified.
		 *  Additionally, a reference to a captured exception can be specified and will be used to fill up the stack trace.
		 * Also, it uses an error icon and only OK button */
		static void error(ErrorCode error_code, const QString &method, const QString &file, int line, Exception *e = nullptr);

		/*! \brief Shows an error message box in which massage and stacktrace is retrived from the provided exception.
		 * Also, it uses an error icon and only OK button */
		static void error(Exception &e, const QString &method, const QString &file, int line);

		//! \brief Shows an error message box with the provided message, an error icon and only OK button
		static void error(const QString &msg);

		/*! \brief Shows an alert message box with the provided message, an alert icon and only OK button
		 *  If an exception is provided, the stacktrace will also be enabled */
		static void alert(const QString &msg, Exception *ex = nullptr);

		//! \brief Shows an info message box with the provided message, an info icon and only OK button
		static void info(const QString &msg);

		//! \brief Shows an success message box with the provided message, an success icon and only OK button
		static void success(const QString &msg);

		//! \brief Shows an confirmation message box with the provided message, a confirmation icon and the defined buttons id
		static int confirm(const QString &msg, ButtonsId btns_id = YesNoButtons,
											 const QString &yes_lbl="", const QString &no_lbl="", const QString &cancel_lbl="",
											 const QString &yes_ico="", const QString &no_ico="", const QString &cancel_ico="");

		//! \brief Shows an confirmation message box with the provided title, message, a confirmation icon and the defined buttons id
		static int confirm(const QString &title, const QString &msg, ButtonsId btns_id = YesNoButtons,
											 const QString &yes_lbl="", const QString &no_lbl="", const QString &cancel_lbl="",
											 const QString &yes_ico="", const QString &no_ico="", const QString &cancel_ico="");

		static bool isAccepted(int res);
		static bool isRejected(int res);
		static bool isCanceled(int res);

		bool isAccepted();
		bool isRejected();
		bool isCanceled();

		void setCustomOptionText(const QString &text);
		void setCustomOptionTooltip(const QString &tooltip);
		bool isCustomOptionChecked();

	private slots:
		void handleYesOkClick();
		void handleNoCancelClick();
};

#endif
