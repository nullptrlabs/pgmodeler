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

#include "fixtoolswidget.h"
#include "guiutilsns.h"

FixToolsWidget::FixToolsWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	GuiUtilsNs::configureWidgetsFont({ run_tool_btn, cancel_btn }, GuiUtilsNs::BigFontFactor);

	model_fix_wgt = GuiUtilsNs::createWidgetInParent<ModelFixWidget>(fix_pg);
	metadata_wgt = GuiUtilsNs::createWidgetInParent<MetadataHandlingWidget>(metadata_pg);

	h_splitter->setSizes({800, 700});

	connect(tools_tbw, &QTabWidget::currentChanged, this, &FixToolsWidget::setCurrentTool);
	setCurrentTool();
}

bool FixToolsWidget::isToolRunning()
{
	return model_fix_wgt->isProcessRunning();
}

void FixToolsWidget::updateModels(const QList<ModelWidget *> &models)
{
	metadata_wgt->updateModels(models);
}

void FixToolsWidget::setCurrentTool()
{
	if(tools_tbw->currentIndex() == 0)
	{
		disconnect(metadata_wgt, nullptr, run_tool_btn, nullptr);
		disconnect(run_tool_btn, nullptr, metadata_wgt, nullptr);
		disconnect(cancel_btn, nullptr, metadata_wgt, nullptr);

		connect(model_fix_wgt, &ModelFixWidget::s_modelFixEnabled, run_tool_btn, &QPushButton::setEnabled);
		connect(model_fix_wgt, &ModelFixWidget::s_modelLoadRequested, this, &FixToolsWidget::s_modelLoadRequested);

		connect(run_tool_btn, &QPushButton::clicked, model_fix_wgt, &ModelFixWidget::fixModel);
		connect(cancel_btn, &QPushButton::clicked, model_fix_wgt, &ModelFixWidget::cancelFix);

		connect(model_fix_wgt, &ModelFixWidget::s_modelFixStarted, this, [this](){
			cancel_btn->setEnabled(true);
		});

		connect(model_fix_wgt, &ModelFixWidget::s_modelFixFinished, this, [this](){
			cancel_btn->setEnabled(false);
		});

		run_tool_btn->setEnabled(model_fix_wgt->isFixEnabled());
	}
	else
	{
		disconnect(model_fix_wgt, nullptr, run_tool_btn, nullptr);
		disconnect(run_tool_btn, nullptr, model_fix_wgt, nullptr);
		disconnect(cancel_btn, nullptr, model_fix_wgt, nullptr);

		connect(metadata_wgt, &MetadataHandlingWidget::s_metadataHandlingEnabled, run_tool_btn, &QPushButton::setEnabled);
		connect(run_tool_btn, &QPushButton::clicked, metadata_wgt, &MetadataHandlingWidget::handleObjectsMetada);

		run_tool_btn->setEnabled(metadata_wgt->isMetadataHandlingEnabled());
	}

	cancel_btn->setEnabled(false);
	cancel_btn->setVisible(tools_tbw->currentIndex() == 0);
}
