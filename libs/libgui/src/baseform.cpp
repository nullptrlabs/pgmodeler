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

#include "baseform.h"
#include "guiutilsns.h"

BaseForm::BaseForm(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);
	setWindowFlags((this->windowFlags() | Qt::WindowMinMaxButtonsHint) /* ^ Qt::WindowContextHelpButtonHint */);
}

void BaseForm::setButtonConfiguration(Messagebox::ButtonsId button_conf)
{
	if(button_conf==Messagebox::OkCancelButtons)
	{
		apply_ok_btn->setText(tr("&Apply"));
		cancel_btn->setVisible(true);
	}
	else
	{
		if(button_conf==Messagebox::CloseButton)
		{
			apply_ok_btn->setText(tr("&Close"));
			apply_ok_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("close1")));
		}
		else
		{
			apply_ok_btn->setText(tr("&Ok"));
			apply_ok_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("confirm")));
		}

		cancel_btn->setVisible(false);
	}

	apply_ok_btn->setDefault(button_conf != Messagebox::CloseButton);
}

void BaseForm::adjustMinimumSize()
{
	adjustSize();
	setMinimumSize(size());
}

void BaseForm::resizeForm(QWidget *widget)
{
	if(!widget)
		return;

	QVBoxLayout *vbox=new QVBoxLayout;
	QSize min_size=widget->minimumSize();
	int max_h = 0, max_w = 0, curr_w =0, curr_h = 0;
	QScreen *screen = qApp->primaryScreen();
	QSize screen_sz = screen->size();

	max_w = screen_sz.width() * 0.70;
	max_h = screen_sz.height() * 0.70;
	vbox->setContentsMargins(0, 0, 0, 0);

	/* If the widget's minimum size is zero then we need to do
			a size adjustment on the widget prior to insert it into the dialog */
	if(min_size.height() <= 0 || min_size.width() <= 0)
	{
		widget->adjustSize();
		min_size = widget->size();
	}

	//Insert the widget into a scroll area if it's minimum size exceeds the 70% of screen's dimensions
	if(max_w < min_size.width() || max_h < min_size.height())
	{
		QScrollArea *scrollarea=nullptr;
		scrollarea=new QScrollArea(main_frm);
		scrollarea->setFrameShape(QFrame::NoFrame);
		scrollarea->setFrameShadow(QFrame::Plain);
		scrollarea->setWidget(widget);
		scrollarea->setWidgetResizable(true);
		widget->setParent(scrollarea);
		vbox->addWidget(scrollarea);
	}
	else
	{
		vbox->addWidget(widget);
		widget->setParent(main_frm);
	}

	main_frm->setLayout(vbox);
	this->adjustSize();

	curr_h=this->height();
	curr_w=min_size.width();

	// If the current height is greater than the widget's minimum height we will use a medium value
	if(curr_h > min_size.height() && min_size.height() < max_h)
		curr_h = (curr_h + min_size.height())/2.5;
	//Using the maximum height if the widget's minimum height exceeds the maximum allowed
	else if(min_size.height() >= max_h)
		curr_h = max_h;

	curr_w += (vbox->contentsMargins().left() +
						 vbox->contentsMargins().right()) * 6;

	curr_h += pgmodeler_name_lbl->minimumHeight() +
							((buttons_lt->contentsMargins().top() +
								buttons_lt->contentsMargins().bottom()) * 6);

	if(curr_w > screen_sz.width())
		curr_w = screen_sz.width() * 0.80;

	if(curr_h > screen_sz.height())
		curr_h = screen_sz.height() * 0.80;

	this->setMinimumSize(min_size);
	this->resize(curr_w, curr_h);
	this->adjustSize();
}

void BaseForm::closeEvent(QCloseEvent *)
{
	this->reject();
}

void BaseForm::setMainWidget(QWidget *widget)
{
	if(!widget)	return;

	setWindowTitle(widget->windowTitle());

	if(!widget->windowIcon().isNull())
		setWindowIcon(widget->windowIcon());
	else
		setWindowIcon(QIcon(GuiUtilsNs::getIconPath("pgmodeler_logo")));

	resizeForm(widget);
	setButtonConfiguration(Messagebox::OkButton);
	connect(cancel_btn, &QPushButton::clicked, this, &BaseForm::reject);
	connect(apply_ok_btn, &QPushButton::clicked, this, &BaseForm::accept);
}
