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
#include "tabordermanager.h"

BaseForm::BaseForm(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);
	setWindowFlags(this->windowFlags() | Qt::WindowMinMaxButtonsHint);
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
			apply_ok_btn->setIcon(GuiUtilsNs::getIcon("close1"));
		}
		else
		{
			apply_ok_btn->setText(tr("&Ok"));
			apply_ok_btn->setIcon(GuiUtilsNs::getIcon("confirm"));
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

	QSize min_size = widget->minimumSize();
	int max_h = 0, max_w = 0, curr_w =0, curr_h = 0;
	QScreen *screen = qApp->primaryScreen();
	QSize screen_sz = screen->size();

	max_w = screen_sz.width() * 0.70;
	max_h = screen_sz.height() * 0.70;

	QVBoxLayout *vbox = GuiUtilsNs::createVBoxLayout(0, GuiUtilsNs::LtSpacing);
	vbox->setContentsMargins(0, 0, 0, 0);

	/* If the widget's minimum size is zero then we need to do
			a size adjustment on the widget prior to insert it into the dialog */
	if(min_size.height() <= 0 || min_size.width() <= 0)
	{
		widget->adjustSize();
		min_size = widget->size();
	}
	else
		min_size = widget->minimumSize();

	//Insert the widget into a scroll area if it's minimum size exceeds the 70% of screen's dimensions
	if(max_w < min_size.width() || max_h < min_size.height())
	{
		QScrollArea *scrollarea = nullptr;
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

	/* The minimum size must include the base form margins and
	 * the height of the name logo at bottom */
	min_size.setWidth(min_size.width() + (GuiUtilsNs::LtMargin * 4));
	min_size.setHeight(min_size.height() + pgmodeler_name_lbl->height() + (GuiUtilsNs::LtMargin * 4));
	setMinimumSize(min_size);

	/* Making the widget temporarily fixed
	 * so we can determine the optimal size
	 * by calling adjustSize() */
	QSize max_size = widget->maximumSize();
	setMaximumSize(min_size);
	adjustSize();
	setMaximumSize(max_size);

	curr_h = height();
	curr_w = min_size.width();

	bool resize_wgt = false;

	/* If the current size of the widget exceeds
	 * the screen demension, we shrink it */
	if(curr_w > screen_sz.width())
	{
		curr_w = screen_sz.width() * 0.80;
		resize_wgt = true;
	}

	if(curr_h > screen_sz.height())
	{
		curr_h = screen_sz.height() * 0.80;
		resize_wgt = true;
	}

	if(resize_wgt)
		resize(curr_w, curr_h);
}

void BaseForm::closeEvent(QCloseEvent *)
{
	this->reject();
}

void BaseForm::setMainWidget(QWidget *widget)
{
	if(!widget)
		return;

	setWindowTitle(widget->windowTitle());

	if(!widget->windowIcon().isNull())
		setWindowIcon(widget->windowIcon());
	else
		setWindowIcon(GuiUtilsNs::getIcon("pgmodeler_logo"));

	resizeForm(widget);

	setButtonConfiguration(Messagebox::OkButton);
	connect(cancel_btn, &QPushButton::clicked, this, &BaseForm::reject);
	connect(apply_ok_btn, &QPushButton::clicked, this, &BaseForm::accept);
}
