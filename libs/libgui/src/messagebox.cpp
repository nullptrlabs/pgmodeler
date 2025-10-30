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

#include "messagebox.h"
#include "customuistyle.h"
#include "guiutilsns.h"
#include "utilsns.h"
#include <qobject.h>

Messagebox::Messagebox(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);

	setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	show_errors_btn->setVisible(false);
	custom_option_chk->setVisible(false);
	raw_stack_txt = nullptr;

	connect(yes_ok_btn, &QPushButton::clicked, this, &Messagebox::handleYesOkClick);
	connect(no_btn, &QPushButton::clicked, this, &Messagebox::handleNoCancelClick);
	connect(cancel_btn, &QPushButton::clicked, this, &Messagebox::handleNoCancelClick);

	connect(show_errors_btn, &QPushButton::toggled, this, [this](bool checked){
		objs_group_wgt->setCurrentIndex(checked ? 1 : 0);
		resize(baseSize().width() * (checked ? 1.25 : 1),
					 baseSize().height() * (checked ? 3 : 1));
	});
}

void Messagebox::handleYesOkClick()
{
	exceptions_trw->clear();
	accept();
}

void Messagebox::handleNoCancelClick()
{
	exceptions_trw->clear();

	if((sender()==no_btn && !cancel_btn->isVisible()) ||
			(sender()==cancel_btn && !no_btn->isVisible()))
		reject();
	else if(sender()==no_btn && cancel_btn->isVisible())
		reject();
	else if(sender()==cancel_btn && no_btn->isVisible())
	{
		done(Canceled);
	}
}

bool Messagebox::isAccepted()
{
	return isAccepted(result());
}

bool Messagebox::isRejected()
{
	return isRejected(result());
}

bool Messagebox::isCanceled()
{
	return isCanceled(result());
}

bool Messagebox::isAccepted(int res)
{
	return res == Accepted;
}

bool Messagebox::isRejected(int res)
{
	return res == Rejected;
}

bool Messagebox::isCanceled(int res)
{
	return res == Canceled;
}

void Messagebox::setCustomOptionText(const QString &text)
{
	custom_option_chk->setVisible(!text.isEmpty());
	custom_option_chk->setText(text);
}

void Messagebox::setCustomOptionTooltip(const QString &tooltip)
{
	custom_option_chk->setToolTip(tooltip);
}

bool Messagebox::isCustomOptionChecked()
{
	return custom_option_chk->isChecked();
}

int Messagebox::show(Exception e, const QString &msg, MessageType icon_type, ButtonsId buttons, const QString &yes_lbl, const QString &no_lbl, const QString &cancel_lbl,
											const QString &yes_ico, const QString &no_ico, const QString &cancel_ico)
{
	QString fmt_msg, title;

	if(!raw_stack_txt)
	{
		raw_stack_txt = GuiUtilsNs::createNumberedTextEditor(stacktrace_tbw->widget(2), true);
		raw_stack_txt->setReadOnly(true);
		raw_stack_txt->setLineNumbersVisible(false);
		raw_stack_txt->setWordWrap(true);
		GuiUtilsNs::configureTextEditFont(raw_stack_txt, font().pointSizeF());
		stacktrace_tbw->widget(2)->layout()->setContentsMargins(GuiUtilsNs::LtMargins);
	}

	raw_stack_txt->setPlainText(e.getExceptionsText());
	extra_info_txt->setPlainText(e.getExceptiosExtraInfo());
	stacktrace_tbw->setTabVisible(1, !e.getExceptiosExtraInfo().isEmpty());

	GuiUtilsNs::createExceptionsTree(exceptions_trw, e, nullptr);
	exceptions_trw->expandAll();
	exceptions_trw->scrollToTop();

	if(msg.isEmpty())
		fmt_msg = UtilsNs::formatMessage(e.getErrorMessage());
	else
		fmt_msg = UtilsNs::formatMessage(msg);

	return show(title, fmt_msg, icon_type, buttons, yes_lbl, no_lbl, cancel_lbl, yes_ico, no_ico, cancel_ico);
}

int Messagebox::show(const QString &msg, MessageType icon_type, ButtonsId buttons)
{
	return show("", msg,  icon_type, buttons);
}

void Messagebox::error(const QString &msg)
{
	Messagebox msgbox;
	msgbox.show(msg, Error);
}

void Messagebox::error(const QString &msg, ErrorCode error_code, const QString &method, const QString &file, int line, Exception *e)
{
	Messagebox msgbox;
	msgbox.show(Exception(msg, error_code, method, file, line, e));
}

void Messagebox::error(const QString &msg, ErrorCode error_code, const QString &method, const QString &file, int line, std::vector<Exception> &exceptions)
{
	Messagebox msgbox;
	msgbox.show(Exception(msg, error_code, method, file, line, exceptions));
}

void Messagebox::error(ErrorCode error_code, const QString &method, const QString &file, int line, Exception *e)
{
	error(Exception::getErrorMessage(error_code), error_code, method, file, line, e);
}

void Messagebox::error(Exception &e, const QString &method, const QString &file, int line)
{
	error(e.getErrorMessage(), e.getErrorCode(), method, file, line, &e);
}

void Messagebox::alert(const QString &msg, Exception *ex)
{
	Messagebox msgbox;

	if(ex)
		msgbox.show(*ex, msg, Alert);
	else
		msgbox.show(msg, Alert);
}

void Messagebox::info(const QString &msg)
{
	Messagebox msgbox;
	msgbox.show(msg, Info);
}

void Messagebox::success(const QString &msg)
{
	Messagebox msgbox;
	msgbox.show(msg, Success);
}

int Messagebox::confirm(const QString &msg, ButtonsId btns_id,
												const QString &yes_lbl, const QString &no_lbl, const QString &cancel_lbl,
												const QString &yes_ico, const QString &no_ico, const QString &cancel_ico)
{
	Messagebox msgbox;
	return msgbox.show("", msg, Confirm, btns_id,
										 yes_lbl, no_lbl, cancel_lbl,
										 yes_ico, no_ico, cancel_ico);
}

int Messagebox::confirm(const QString &title, const QString &msg, ButtonsId btns_id, const QString &yes_lbl, const QString &no_lbl, const QString &cancel_lbl, const QString &yes_ico, const QString &no_ico, const QString &cancel_ico)
{
	Messagebox msgbox;
	return msgbox.show(title, msg, Confirm, btns_id,
										 yes_lbl, no_lbl, cancel_lbl,
										 yes_ico, no_ico, cancel_ico);
}

int Messagebox::show(const QString &title, const QString &msg, MessageType icon_type, ButtonsId buttons,
										 const QString &yes_lbl, const QString &no_lbl, const QString &cancel_lbl,
										 const QString &yes_ico, const QString &no_ico, const QString &cancel_ico)
{
	QString icon_name, aux_title;
	QWidgetList btns = { yes_ok_btn, no_btn, cancel_btn, show_errors_btn };
	CustomUiStyle::StyleHint style_hint;

	if(!yes_lbl.isEmpty())
		yes_ok_btn->setText(yes_lbl);
	else
	{
		QString btn_txt;

		if(buttons == CloseButton)
			btn_txt = tr("&Close");
		else if(buttons == OkButton)
			btn_txt = tr("&Ok");
		else
			btn_txt = tr("&Yes");

		yes_ok_btn->setText(btn_txt);
	}

	if(!yes_ico.isEmpty())
		yes_ok_btn->setIcon(QIcon(yes_ico));
	else
		yes_ok_btn->setIcon(buttons != CloseButton ? QIcon(GuiUtilsNs::getIconPath("confirm")) : QIcon(GuiUtilsNs::getIconPath("close1")));

	no_btn->setText(!no_lbl.isEmpty() ? no_lbl : tr("&No"));
	no_btn->setIcon(!no_ico.isEmpty() ? QIcon(no_ico) : QIcon(GuiUtilsNs::getIconPath("close1")));

	cancel_btn->setText(!cancel_lbl.isEmpty() ? cancel_lbl : tr("&Cancel"));
	cancel_btn->setIcon(!cancel_ico.isEmpty() ? QIcon(cancel_ico) : QIcon(GuiUtilsNs::getIconPath("cancel")));

	no_btn->setVisible(buttons==YesNoButtons || buttons==AllButtons);
	cancel_btn->setVisible(buttons==OkCancelButtons || buttons==AllButtons);

	for(auto &btn : btns)
	{
		btn->adjustSize();
		btn->setMinimumSize(btn->size());
	}

	switch(icon_type)
	{
		case Error:
			icon_name = "error";
			aux_title = title.isEmpty() ? tr("Error") : title;
			style_hint = CustomUiStyle::ErrorFrmHint;
		break;

		case Info:
			icon_name = "info";
			aux_title = title.isEmpty() ? tr("Information") : title;
			style_hint = CustomUiStyle::InfoFrmHint;
		break;

		case Alert:
			icon_name = "alert";
			aux_title = title.isEmpty() ? tr("Alert") : title;
			style_hint = CustomUiStyle::AlertFrmHint;
		break;

		case Confirm:
			icon_name = "question";
			aux_title = title.isEmpty() ? tr("Confirmation") : title;
			style_hint = CustomUiStyle::ConfirmFrmHint;
		break;

		case Success:
			icon_name = "success";
			aux_title = title.isEmpty() ? tr("Success") : title;
			style_hint = CustomUiStyle::SuccessFrmHint;
		break;

		default:
			icon_name = "";
			aux_title = title;
		break;
	}

	icon_lbl->setVisible(!icon_name.isEmpty());

	if(!icon_name.isEmpty())
		icon_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath(icon_name)));

	msg_lbl->setText(msg);

	setWindowTitle(aux_title);
	objs_group_wgt->setCurrentIndex(0);
	show_errors_btn->setChecked(false);
	show_errors_btn->setVisible(exceptions_trw->topLevelItemCount() > 0);

	double w_factor = 0.25, h_factor = 0.15;
	QSize sz = screen()->size();

	if(sz.width() * w_factor > minimumWidth())
		setMinimumWidth(sz.width() * w_factor);

	setMinimumHeight(sz.height() * h_factor);

	int ln_cnt = QString(msg).replace(QRegularExpression("(<)(br)(/)?(>)",
																		QRegularExpression::CaseInsensitiveOption),
																		"\n").count('\n');

	if(ln_cnt > 0)
		adjustSize();
	else
		resize(minimumSize());

	setBaseSize(size());
	CustomUiStyle::setStyleHint(style_hint, frame);

	return exec();
}

int Messagebox::exec()
{
	/* Forcing the restoration of the overriden cursor
	 * when opening the message box, this will avoid
	 * the wait cursor to be displayed when the messagebox
	 * is show after a exception is raised */
	qApp->restoreOverrideCursor();

	return QDialog::exec();
}

void Messagebox::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
}
