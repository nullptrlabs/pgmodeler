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

#include "modelselectorwidget.h"

ModelSelectorWidget::ModelSelectorWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	connect(model_cmb, &QComboBox::activated, this, &ModelSelectorWidget::s_selectionChanged);
	connect(model_cmb, &QComboBox::activated, this, &ModelSelectorWidget::updateModelFilename);
}

ModelWidget *ModelSelectorWidget::getSelectedModel()
{
	if(model_cmb->currentIndex() <= 0)
		return nullptr;

	return reinterpret_cast<ModelWidget *>(model_cmb->currentData().value<void *>());
}

bool ModelSelectorWidget::isModelSelected()
{
	return model_cmb->currentIndex() >= 1;
}

void ModelSelectorWidget::clearSelection()
{
	model_cmb->blockSignals(true);
	model_cmb->setCurrentIndex(0);
	model_file_edt->clear();
	model_cmb->blockSignals(false);

	emit s_selectionChanged();
}

bool ModelSelectorWidget::hasSelection()
{
	return isModelSelected();
}

void ModelSelectorWidget::updateModels(const QList<ModelWidget *> &models)
{
	QVariant data = model_cmb->currentData();

	model_cmb->blockSignals(true);
	model_cmb->clear();
	model_file_edt->clear();

	if(models.isEmpty())
		model_cmb->addItem(tr("No models found"));
	else
		model_cmb->addItem(tr("Found %1 model(s)").arg(models.size()));

	for(auto &model_wgt : models)
		model_cmb->addItem(model_wgt->getDatabaseModel()->getName(), QVariant::fromValue<void *>(model_wgt));

	model_cmb->blockSignals(false);

	int data_idx = model_cmb->findData(data);
	model_cmb->setCurrentIndex(data_idx < 0 ? 0 : data_idx);
	model_cmb->setEnabled(!models.isEmpty());
	model_file_edt->setEnabled(!models.isEmpty());

	emit s_selectionChanged();
}

void ModelSelectorWidget::updateModelFilename()
{
	model_file_edt->clear();

	if(model_cmb->currentIndex() <= 0)
		return;

	ModelWidget *model = reinterpret_cast<ModelWidget *>(model_cmb->currentData().value<void *>());
	model_file_edt->setText(model->getFilename().isEmpty() ? tr("(model not yet saved)") : model->getFilename());
	model_file_edt->setCursorPosition(0);
}
