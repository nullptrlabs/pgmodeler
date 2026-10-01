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

#include "taskprogresswidget.h"
#include "baseobject.h"
#include "guiutilsns.h"
#include "utilsns.h"
#include <QTimer>
#include <QApplication>

TaskProgressWidget::TaskProgressWidget(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	std::vector<ObjectType> obj_types=BaseObject::getObjectTypes(true);

	setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_TranslucentBackground, true);

	canceled = false;
	connect(cancel_btn, &QPushButton::clicked, this, &TaskProgressWidget::cancelTask);

	//Enforced here instead of relying on the .ui file's default so it survives Designer edits
	setCancelable(false);

	for(auto &obj_tp : obj_types)
		addIcon(enum_t(obj_tp), GuiUtilsNs::getIcon(obj_tp));
}

void TaskProgressWidget::setCancelable(bool value)
{
	cancelable = value;
	cancel_btn->setVisible(value);
	cancel_btn->setEnabled(value);
}

bool TaskProgressWidget::wasCanceled()
{
	return canceled;
}

void TaskProgressWidget::cancelTask()
{
	canceled = true;

	//Prevents piling up further clicks while the caller is still unwinding
	cancel_btn->setEnabled(false);
	text_lbl->setText(tr("Cancelling..."));
}

void TaskProgressWidget::addIcon(unsigned id, const QIcon &ico)
{
	icons[id]=ico;
}

void TaskProgressWidget::setNoProgressState(bool value)
{
	if(value)
		progress_pb->setRange(0, 0);
	else
		progress_pb->setRange(0, 100);
}

void TaskProgressWidget::show()
{
	/* Using a event loop as a workaround to give a little time to task progress
	 to be shown before start the progress update. In tasks too quick, if the event loop above
	 isn't used the task is not shown properly and sometimes stay only on taskbar not poping up
	 to the user. */
	QEventLoop event_loop;

	canceled = false;
	cancel_btn->setEnabled(cancelable);

	GuiUtilsNs::resizeWidget(this);
	QDialog::show();

	//Gives 100ms to the task to be shown and update its contents
	QTimer::singleShot(100, &event_loop, &QEventLoop::quit);
	text_lbl->setText(tr("Waiting task to start..."));
	event_loop.exec(QEventLoop::AllEvents);
}

void TaskProgressWidget::updateProgress(int progress, unsigned icon_id)
{
	updateProgress(progress, "", icon_id);
}

void TaskProgressWidget::updateProgress(int progress, QString text, unsigned icon_id)
{
	if(progress > progress_pb->maximum())
		progress=progress_pb->maximum();

	progress_pb->setValue(progress);

	if(!text.isEmpty())
		text_lbl->setText(UtilsNs::formatMessage(text));

	if(icons.count(icon_id))
		icon_lbl->setPixmap(icons[icon_id].pixmap(icon_lbl->minimumSize()));
	else
		icon_lbl->clear();

	this->repaint();

	/* Pumps the event queue so a pending click on cancel_btn (see setCancelable()) actually gets
	 * delivered -- the caller is expected to be deep inside a long, synchronous operation at this
	 * point, with no event loop of its own running, so this is the only chance that click has to
	 * be processed before wasCanceled() is checked back. Bounded to 1ms so this stays cheap even
	 * when called once per object on a large load. Safe to call unconditionally (not just when
	 * cancelable): this dialog is application-modal (see the .ui file), so nothing other than it
	 * can receive input while it's up, regardless of whether cancel_btn itself is visible */
	qApp->processEvents(QEventLoop::AllEvents, 1);

	/* MacOSX workaround: The event loop below is needed because on this system
		 the task progress is not correctly updated. The event loop causes a little
		 delay (1ms) and it`s sufficient to update the entire widget */
#ifdef Q_OS_MACOS
	QEventLoop event_loop;
	//Gives 5ms to the task to be shown and update its contents
	QTimer::singleShot(5, &event_loop, &QEventLoop::quit);
	event_loop.exec(QEventLoop::AllEvents);
#endif
}

void TaskProgressWidget::close()
{
	QDialog::close();
	progress_pb->setValue(0);
	text_lbl->clear();
	icon_lbl->clear();
}
