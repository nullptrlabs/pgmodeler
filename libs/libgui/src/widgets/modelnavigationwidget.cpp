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

#include "modelnavigationwidget.h"
#include "guiutilsns.h"
#include <QToolTip>

ModelNavigationWidget::ModelNavigationWidget(QWidget *parent): QWidget(parent)
{
	setupUi(this);

	connect(models_cmb, &QComboBox::currentIndexChanged, this, &ModelNavigationWidget::setCurrentModel);

	connect(close_tb, &QToolButton::clicked, this, [this](){
		emit s_modelCloseRequested(models_cmb->currentIndex());
	});

	connect(next_tb, &QToolButton::clicked, this, [this](){
		models_cmb->setCurrentIndex(models_cmb->currentIndex()+1);
	});

	connect(previous_tb, &QToolButton::clicked, this, [this](){
		models_cmb->setCurrentIndex(models_cmb->currentIndex()-1);
	});

	connect(models_cmb, &QComboBox::highlighted, this, &ModelNavigationWidget::showTooltip);

	previous_tb->setToolTip(previous_tb->toolTip() + QString(" (%1)").arg(previous_tb->shortcut().toString()));
	next_tb->setToolTip(next_tb->toolTip() + QString(" (%1)").arg(next_tb->shortcut().toString()));
	close_tb->setToolTip(close_tb->toolTip() + QString(" (%1)").arg(close_tb->shortcut().toString()));

	//GuiUtilsNs::createDropShadow(previous_tb, 1, 1, 5);
	//GuiUtilsNs::createDropShadow(next_tb, 1, 1, 5);
	//GuiUtilsNs::createDropShadow(close_tb, 1, 1, 5);
}

int ModelNavigationWidget::getCurrentIndex()
{
	return models_cmb->currentIndex();
}

QString ModelNavigationWidget::getText(int idx)
{
	if(idx < 0 || idx >= models_cmb->count())
		return "";

	return models_cmb->itemText(idx);
}

QList<ModelWidget *> ModelNavigationWidget::getModelWidgets()
{
	return model_wgts;
}

void ModelNavigationWidget::addModel(ModelWidget *model)
{
	if(model)
	{
		QString tooltip;

		setEnabled(true);
		models_cmb->blockSignals(true);

		tooltip=model->getFilename();

		if(tooltip.isEmpty())
			tooltip=tr("(Model not saved yet)");

		models_cmb->addItem(model->getDatabaseModel()->getName(), tooltip);
		models_cmb->setCurrentIndex(models_cmb->count()-1);
		models_cmb->setToolTip(tooltip);

		models_cmb->blockSignals(false);

		model_wgts.append(model);

		enableNavigationButtons();
	}
}

void ModelNavigationWidget::updateModelText(int idx, const QString &text, const QString &filename)
{
	if(idx >= 0 && idx < models_cmb->count())
	{
		models_cmb->setItemText(idx, QString("%1").arg(text));
		models_cmb->setItemData(idx, filename);

		if(idx==models_cmb->currentIndex())
			models_cmb->setToolTip(filename);
	}
}

void ModelNavigationWidget::setCurrentModelModified(bool modified)
{
	if(models_cmb->count() == 0)
		return;

	models_cmb->setItemIcon(models_cmb->currentIndex(),
													modified ? QIcon(GuiUtilsNs::getIconPath("save")) : QIcon());
}

void ModelNavigationWidget::removeModel(int idx)
{
	models_cmb->blockSignals(true);
	models_cmb->removeItem(idx);
	this->setEnabled(models_cmb->count() >= 1);

	if(models_cmb->count() >= 1)
		models_cmb->setToolTip(models_cmb->currentData().toString());
	models_cmb->blockSignals(false);

	model_wgts.removeAt(idx);

	enableNavigationButtons();
	emit s_modelRemoved(idx);
}

void ModelNavigationWidget::setCurrentModel()
{
	models_cmb->setToolTip(models_cmb->currentData().toString());

	enableNavigationButtons();

	if(models_cmb->currentIndex() >= 0)
		emit s_currentModelChanged(models_cmb->currentIndex());
}

void ModelNavigationWidget::enableNavigationButtons()
{
	previous_tb->setEnabled(models_cmb->currentIndex() > 0 && models_cmb->count() > 1);
	next_tb->setEnabled(models_cmb->currentIndex() >= 0 && models_cmb->currentIndex()!=(models_cmb->count()-1));
}

void ModelNavigationWidget::showTooltip(int idx)
{
	QString tooltip=models_cmb->itemData(idx).toString();
	QToolTip::showText(QCursor::pos(), tooltip);
}
