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
	track_changes = has_changes = prevent_close = false;
	main_wgt = nullptr;
	tab_order_mng = nullptr;
	setupUi(this);
	setWindowFlags(this->windowFlags() | Qt::WindowMinMaxButtonsHint);
}

void BaseForm::setButtonConfiguration(Messagebox::ButtonsId button_conf)
{
	if(button_conf==Messagebox::OkCancelButtons)
	{
		accept_btn->setText(tr("&Apply"));
		reject_btn->setVisible(true);
	}
	else if(button_conf==Messagebox::OkCloseButtons)
	{
		accept_btn->setText(tr("&Ok"));
		reject_btn->setText(tr("&Close"));
		reject_btn->setIcon(GuiUtilsNs::getIcon("close1"));
		reject_btn->setVisible(true);
	}
	else
	{
		if(button_conf==Messagebox::CloseButton)
		{
			accept_btn->setText(tr("&Close"));
			accept_btn->setIcon(GuiUtilsNs::getIcon("close1"));
		}
		else
		{
			accept_btn->setText(tr("&Ok"));
			accept_btn->setIcon(GuiUtilsNs::getIcon("confirm"));
		}

		reject_btn->setVisible(false);
	}

	accept_btn->setDefault(button_conf != Messagebox::CloseButton);
}

void BaseForm::setPreventClose(bool value)
{
	prevent_close = value;
}

void BaseForm::adjustMinimumSize()
{
	adjustSize();
	setMinimumSize(size());
}

void BaseForm::installTabManager()
{
	if(!main_wgt)
	{
		qDebug() << "BaseForm::installTabManager: no main widget to install tab order manager!";
		return;
	}

	TabOrderManager *tab_mng = main_wgt->findChild<TabOrderManager *>();

	if((tab_mng && tab_mng->parent() == main_wgt) || tab_order_mng)
	{
		qDebug() << "BaseForm::installTabManager: main widget ("
						 << main_wgt->metaObject()->className()
						 << ") already has a tab order manager installed!";
		return;
	}

	tab_order_mng = new TabOrderManager(main_wgt);
	main_wgt->installEventFilter(tab_order_mng);
}

void BaseForm::resizeForm(QWidget *widget)
{
	if(!widget)
		return;

	main_wgt = widget;

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

void BaseForm::reject()
{
	if(prevent_close)
		return;

	if(track_changes && has_changes)
	{
		int res = Messagebox::confirm(tr("Some fields in the form were modified! Do you really want to close and discard changes?"));

		if(res == QDialog::Rejected)
		{
			setResult(-1);
			return;
		}
	}

	QDialog::reject();
}

void BaseForm::closeEvent(QCloseEvent *event)
{
	if(prevent_close)
	{
		event->ignore();
		return;
	}

	this->reject();

	if(result() < 0)
	{
		event->ignore();
		return;
	}
}

void BaseForm::enableTrackChanges(bool enable, QStringList excl_wgts)
{
	QLineEdit *edt = nullptr;
	QPlainTextEdit *txt = nullptr;
	QAbstractButton *btn = nullptr;
	QSpinBox *spb = nullptr;
	CustomTableWidget *ctab = nullptr;
	QComboBox *cmb = nullptr;
	ObjectSelectorWidget *sel = nullptr;

	track_changes = enable;
	has_changes = false;

	for(auto &wgt : findChildren<QWidget *>())
	{
		if(excl_wgts.contains(wgt->objectName()))
			continue;

		edt = qobject_cast<QLineEdit *>(wgt);
		txt = qobject_cast<QPlainTextEdit *>(wgt);
		btn = qobject_cast<QAbstractButton *>(wgt);
		spb = qobject_cast<QSpinBox *>(wgt);
		ctab = qobject_cast<CustomTableWidget *>(wgt);
		cmb = qobject_cast<QComboBox *>(wgt);
		sel = qobject_cast<ObjectSelectorWidget *>(wgt);

		if(edt || txt || btn ||
			 spb || ctab || cmb || sel)
		{
			if(!enable)
				disconnect(spb, nullptr, this, nullptr);
			else
			{
				if(edt && !edt->isReadOnly())
					connect(edt, &QLineEdit::textChanged, this, &BaseForm::setFieldChanged);
				else if(txt && !txt->isReadOnly())
					connect(txt, &QPlainTextEdit::textChanged, this, &BaseForm::setFieldChanged);
				else if(btn && btn->isCheckable())
					connect(btn, &QAbstractButton::toggled, this, &BaseForm::setFieldChanged);
				else if(spb)
					connect(spb, &QSpinBox::valueChanged, this, &BaseForm::setFieldChanged);
				else if(cmb)
					connect(cmb, &QComboBox::currentIndexChanged, this, &BaseForm::setFieldChanged);
				else if(ctab)
				{
					connect(ctab, &CustomTableWidget::s_cellChanged, this, &BaseForm::setFieldChanged);
					connect(ctab, &CustomTableWidget::s_rowCountChanged, this, &BaseForm::setFieldChanged);
					connect(ctab, &CustomTableWidget::s_rowsMoved, this, &BaseForm::setFieldChanged);
				}
				else if(sel)
					connect(sel, &ObjectSelectorWidget::s_selectorChanged, this, &BaseForm::setFieldChanged);
			}
		}
	}
}

void BaseForm::setFieldChanged()
{
	has_changes = true;
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
	connect(reject_btn, &QPushButton::clicked, this, &BaseForm::reject);
	connect(accept_btn, &QPushButton::clicked, this, &BaseForm::accept);
}
