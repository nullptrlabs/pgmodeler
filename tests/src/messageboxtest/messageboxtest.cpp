/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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

#include <QtTest/QtTest>
#include <QTimer>
#include <QScreen>
#include "messagebox.h"
#include "exception.h"
#include "pgmodelerunittest.h"

/*! \brief The Messagebox must size itself so the message text and the buttons always fit,
whatever the length of the text and the labels of the buttons (long translations, for instance).
Each test shows the dialog, waits for it to be laid out and then checks its geometry. */
class MessageboxTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		MessageboxTest() : PgModelerUnitTest(SCHEMASDIR){}

	private:
		//! \brief The visible buttons of the dialog in the order they are displayed
		static QList<QPushButton *> visibleButtons(Messagebox &msgbox);

		/*! \brief Verifies the geometry of the dialog (already visible and laid out), returning
		 * a description of the first problem found or an empty string if everything fits. */
		static QString checkGeometry(Messagebox &msgbox);

		/*! \brief Runs the checks over the dialog while it is displayed, closing it afterwards.
		 * The checks are made twice: right after it is shown and after the user resizes it to the smallest allowed size */
		static QString showAndCheck(Messagebox &msgbox, const std::function<int()> &show_func);

	private slots:
		void shortMessageWithDefaultButtons_data() {}
		void shortMessageWithDefaultButtons();
		void longTranslatedMessageWithLongButtonLabels();
		void veryLongMessageWithLineBreaks();
		void longButtonLabelsWithTheStackTraceButton();
		void stackTracePageKeepsEverythingVisible();
		void smallestAllowedSizeStillFitsTextAndButtons();
};

QList<QPushButton *> MessageboxTest::visibleButtons(Messagebox &msgbox)
{
	QList<QPushButton *> btns;

	for(auto btn : { msgbox.show_errors_btn, msgbox.yes_ok_btn, msgbox.no_btn, msgbox.cancel_btn })
	{
		if(btn->isVisibleTo(&msgbox))
			btns.append(btn);
	}

	return btns;
}

QString MessageboxTest::checkGeometry(Messagebox &msgbox)
{
	QRect dlg_rect = msgbox.rect();
	QList<QPushButton *> btns = visibleButtons(msgbox);

	for(auto btn : btns)
	{
		QRect btn_rect(btn->mapTo(&msgbox, QPoint(0, 0)), btn->size());

		if(btn->width() < btn->sizeHint().width() || btn->height() < btn->sizeHint().height())
			return QString("button `%1' is squeezed: %2x%3 but needs %4x%5")
					.arg(btn->text()).arg(btn->width()).arg(btn->height())
					.arg(btn->sizeHint().width()).arg(btn->sizeHint().height());

		if(!dlg_rect.contains(btn_rect))
			return QString("button `%1' (%2,%3 %4x%5) is outside the dialog (%6x%7)")
					.arg(btn->text()).arg(btn_rect.x()).arg(btn_rect.y()).arg(btn_rect.width()).arg(btn_rect.height())
					.arg(dlg_rect.width()).arg(dlg_rect.height());
	}

	for(int i = 0; i < btns.size(); i++)
	{
		QRect rect_i(btns[i]->mapTo(&msgbox, QPoint(0, 0)), btns[i]->size());

		for(int j = i + 1; j < btns.size(); j++)
		{
			QRect rect_j(btns[j]->mapTo(&msgbox, QPoint(0, 0)), btns[j]->size());

			if(rect_i.intersects(rect_j))
				return QString("buttons `%1' and `%2' overlap").arg(btns[i]->text(), btns[j]->text());
		}
	}

	//The logo that lies in the same row of the buttons must not be covered by them as well
	QRect logo_rect(msgbox.pgmodeler_name_lbl->mapTo(&msgbox, QPoint(0, 0)), msgbox.pgmodeler_name_lbl->size());

	for(auto btn : btns)
	{
		QRect btn_rect(btn->mapTo(&msgbox, QPoint(0, 0)), btn->size());

		if(logo_rect.intersects(btn_rect))
			return QString("button `%1' overlaps the pgModeler logo").arg(btn->text());
	}

	//The message must be entirely visible (the label can't be shorter than the wrapped text)
	if(msgbox.objs_group_wgt->currentIndex() == 0)
	{
		int needed_h = msgbox.msg_lbl->heightForWidth(msgbox.msg_lbl->width());

		/* When the text needs more than the whole screen height there's nothing more to do:
		 * the dialog is already as tall as the screen allows */
		bool max_height = msgbox.height() >= msgbox.screen()->availableGeometry().height();

		if(msgbox.msg_lbl->height() < needed_h && !max_height)
			return QString("the message is clipped: label has %1px of height but the text needs %2px at %3px of width")
					.arg(msgbox.msg_lbl->height()).arg(needed_h).arg(msgbox.msg_lbl->width());

		if(msgbox.msg_lbl->width() < msgbox.msg_lbl->minimumSizeHint().width())
			return "the message label is narrower than its minimum size hint";
	}

	//The dialog can't be smaller than what its layout requires
	QSize min_sz = msgbox.layout()->totalMinimumSize();

	if(msgbox.width() < min_sz.width() || msgbox.height() < min_sz.height())
		return QString("the dialog (%1x%2) is smaller than its layout minimum (%3x%4)")
				.arg(msgbox.width()).arg(msgbox.height()).arg(min_sz.width()).arg(min_sz.height());

	//Whenever possible the dialog must fit in the screen
	QRect avail = msgbox.screen()->availableGeometry();

	if(min_sz.width() <= avail.width() && min_sz.height() <= avail.height() &&
		 (msgbox.width() > avail.width() || msgbox.height() > avail.height()))
		return QString("the dialog (%1x%2) does not fit in the screen (%3x%4)")
				.arg(msgbox.width()).arg(msgbox.height()).arg(avail.width()).arg(avail.height());

	return "";
}

QString MessageboxTest::showAndCheck(Messagebox &msgbox, const std::function<int()> &show_func)
{
	QString result;

	QTimer::singleShot(150, &msgbox, [&](){
		QApplication::processEvents();
		result = checkGeometry(msgbox);

		if(result.isEmpty())
		{
			//Shrinks the dialog as much as the user would be able to do and checks again
			msgbox.resize(1, 1);
			QApplication::processEvents();
			result = "(after shrinking the dialog) " + checkGeometry(msgbox);

			if(result == "(after shrinking the dialog) ")
				result.clear();
		}

		msgbox.accept();
	});

	show_func();
	return result;
}

void MessageboxTest::shortMessageWithDefaultButtons()
{
	Messagebox msgbox;
	QString res = showAndCheck(msgbox, [&](){
		return msgbox.show("Hello!", Messagebox::Info);
	});

	QVERIFY2(res.isEmpty(), qPrintable(res));
}

void MessageboxTest::longTranslatedMessageWithLongButtonLabels()
{
	Messagebox msgbox;
	QString res = showAndCheck(msgbox, [&](){
		return msgbox.show("", "<strong>AVISO:</strong> O modelo <strong>demodb</strong> não foi validado desde a última modificação! "
											 "Recomenda-se validá-lo antes de salvar para criar um modelo consistente; caso contrário, o arquivo gerado "
											 "ficará quebrado, exigindo correções manuais para poder ser carregado novamente!",
											 Messagebox::Confirm, Messagebox::AllButtons,
											 "Validar", "Salvar mesmo assim", "Cancelar", "", "", "", true);
	});

	QVERIFY2(res.isEmpty(), qPrintable(res));
}

void MessageboxTest::veryLongMessageWithLineBreaks()
{
	Messagebox msgbox;
	QString msg;

	for(int i = 0; i < 12; i++)
		msg += QString("Linha %1: uma mensagem bastante longa que precisa ser quebrada em várias linhas pelo layout da caixa de mensagem.<br/>").arg(i);

	QString res = showAndCheck(msgbox, [&](){
		return msgbox.show("", msg, Messagebox::Alert, Messagebox::OkCancelButtons,
											 "Continuar assim mesmo e sobrescrever", "", "Cancelar operação", "", "", "", true);
	});

	QVERIFY2(res.isEmpty(), qPrintable(res));
}

void MessageboxTest::longButtonLabelsWithTheStackTraceButton()
{
	Messagebox msgbox;
	QString res = showAndCheck(msgbox, [&](){
		return msgbox.show(Exception("Erro de teste com uma mensagem razoavelmente longa para forçar a quebra de linha na caixa de mensagem.",
																 ErrorCode::Custom, PGM_FUNC, PGM_FILE, PGM_LINE),
											 "", Messagebox::Error, Messagebox::OkCancelButtons,
											 "Tentar novamente agora", "", "Cancelar tudo");
	});

	QVERIFY2(res.isEmpty(), qPrintable(res));
}

void MessageboxTest::stackTracePageKeepsEverythingVisible()
{
	Messagebox msgbox;
	QString result;

	QTimer::singleShot(150, &msgbox, [&](){
		QApplication::processEvents();
		msgbox.show_errors_btn->click();
		QApplication::processEvents();

		if(msgbox.objs_group_wgt->currentIndex() != 1)
			result = "the stack trace page was not displayed";
		else
			result = checkGeometry(msgbox);

		//The tree with the exceptions must have a usable area
		if(result.isEmpty() && (msgbox.exceptions_trw->height() < 60 || msgbox.exceptions_trw->width() < 200))
			result = QString("the exceptions tree is too small: %1x%2").arg(msgbox.exceptions_trw->width()).arg(msgbox.exceptions_trw->height());

		if(result.isEmpty())
		{
			//Going back to the message must restore a size that fits the message again
			msgbox.show_errors_btn->click();
			QApplication::processEvents();
			result = checkGeometry(msgbox);
		}

		msgbox.accept();
	});

	msgbox.show(Exception("Erro de teste", ErrorCode::Custom, PGM_FUNC, PGM_FILE, PGM_LINE),
							"", Messagebox::Error, Messagebox::OkButton);

	QVERIFY2(result.isEmpty(), qPrintable(result));
}

/*! \brief The same instance can be reused to show different messages: nothing from the previous
message (button sizes, dialog size) may remain and force the dialog to be larger (or smaller) than needed. */
void MessageboxTest::smallestAllowedSizeStillFitsTextAndButtons()
{
	Messagebox msgbox;

	QString res = showAndCheck(msgbox, [&](){
		return msgbox.show("", "Uma mensagem com um botão de rótulo muito, muito, muito, muito longo mesmo.",
											 Messagebox::Confirm, Messagebox::YesNoButtons,
											 "Sim, quero fazer isso agora mesmo e não perguntar", "Não, deixa pra depois", "", "", "", "", true);
	});

	QVERIFY2(res.isEmpty(), qPrintable(res));

	QSize first_min = msgbox.layout()->totalMinimumSize();

	res = showAndCheck(msgbox, [&](){
		return msgbox.show("", "Ok.", Messagebox::Info, Messagebox::OkButton, "", "", "", "", "", "", true);
	});

	QVERIFY2(res.isEmpty(), qPrintable(res));
	QVERIFY2(msgbox.layout()->totalMinimumSize().width() < first_min.width(),
					 "the minimum size must shrink back when a smaller message is shown with the same instance");
}

QTEST_MAIN(MessageboxTest)
#include "messageboxtest.moc"
