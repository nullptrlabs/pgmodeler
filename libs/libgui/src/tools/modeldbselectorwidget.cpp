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

#include "modeldbselectorwidget.h"
#include "customuistyle.h"
#include "settings/connectionsconfigwidget.h"
#include "databaseimporthelper.h"
#include "databaseimportwidget.h"

ModelDbSelectorWidget::ModelDbSelectorWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	setSelectMode(SelectModel);
	
	alert_frm->setVisible(false);
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);

	connect(connections_cmb, &QComboBox::activated, this, &ModelDbSelectorWidget::listDatabases);
	connect(database_cmb, &QComboBox::activated, this, &ModelDbSelectorWidget::s_selectionChanged);
	connect(model_cmb, &QComboBox::activated, this, &ModelDbSelectorWidget::s_selectionChanged);
	connect(model_cmb, &QComboBox::activated, this, &ModelDbSelectorWidget::updateModelFilename);
}

void ModelDbSelectorWidget::setSelectMode(SelectMode sel_mode)
{
	model_ctrl_wgt->setVisible(sel_mode == SelectModel);
	db_ctrl_wgt->setVisible(sel_mode == SelectDatabase);

	if(sel_mode == SelectDatabase)
	{
		model_cmb->setCurrentIndex(0);
		model_file_edt->clear();
	}
	else
	{
		connections_cmb->setCurrentIndex(0);
		database_cmb->clear();
	}
}

void ModelDbSelectorWidget::setSelectedConnection(const QString &conn_id)
{
	connections_cmb->blockSignals(true);
	connections_cmb->setCurrentText(conn_id);

	if(connections_cmb->currentIndex() < 0 &&
		 connections_cmb->count() > 1)
		connections_cmb->setCurrentIndex(0);

	listDatabases();
	connections_cmb->blockSignals(false);
}

void ModelDbSelectorWidget::setSelectedDatabase(const QString &db_name)
{
	database_cmb->blockSignals(true);
	database_cmb->setCurrentText(db_name);

	if(database_cmb->currentIndex() < 0 &&
		 database_cmb->count() > 1)
		database_cmb->setCurrentIndex(0);

	database_cmb->blockSignals(false);

	emit s_selectionChanged();
}

Connection ModelDbSelectorWidget::getSelectedConnection()
{
	if(database_cmb->currentIndex() <= 0)
		return {};

	return *(reinterpret_cast<Connection *>(connections_cmb->currentData().value<void *>()));
}

QString ModelDbSelectorWidget::getSelectedDatabase()
{
	if(database_cmb->currentIndex() <= 0)
		return "";

	return database_cmb->currentText();
}

unsigned int ModelDbSelectorWidget::getSelectedDatabaseOid()
{
	if(database_cmb->currentIndex() <= 0)
		return 0;

	return database_cmb->currentData().value<unsigned>();
}

ModelWidget *ModelDbSelectorWidget::getSelectedModel()
{
	if(model_cmb->currentIndex() <= 0)
		return nullptr;

	return reinterpret_cast<ModelWidget *>(model_cmb->currentData().value<void *>());
}

bool ModelDbSelectorWidget::isDatabaseSelected()
{
	return database_cmb->currentIndex() >= 1;
}

bool ModelDbSelectorWidget::isModelSelected()
{
	return model_cmb->currentIndex() >= 1;
}

void ModelDbSelectorWidget::clearSelection()
{
	connections_cmb->blockSignals(true);
	database_cmb->blockSignals(true);
	model_cmb->blockSignals(true);

	connections_cmb->setCurrentIndex(0);
	database_cmb->clear();
	model_cmb->setCurrentIndex(0);
	model_file_edt->clear();

	connections_cmb->blockSignals(false);
	database_cmb->blockSignals(false);
	model_cmb->blockSignals(false);

	emit s_selectionChanged();
}

bool ModelDbSelectorWidget::hasSelection()
{
	return isDatabaseSelected() || isModelSelected();
}

void ModelDbSelectorWidget::updateConnections(Connection::ConnOperation def_conn_op)
{
	ConnectionsConfigWidget::fillConnectionsComboBox(connections_cmb, true, def_conn_op);
	connections_cmb->setEnabled(connections_cmb->count() > 0);
	connection_lbl->setEnabled(connections_cmb->isEnabled());

	database_cmb->clear();
	database_cmb->setEnabled(false);
	database_lbl->setEnabled(false);
}

void ModelDbSelectorWidget::updateModels(const QList<ModelWidget *> &models)
{
	QVariant data = model_cmb->currentData();

	model_cmb->blockSignals(true);
	model_cmb->clear();

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
}

void ModelDbSelectorWidget::updateModelFilename()
{
	model_file_edt->clear();

	if(model_cmb->currentIndex() <= 0)
		return;

	ModelWidget *model = reinterpret_cast<ModelWidget *>(model_cmb->currentData().value<void *>());
	model_file_edt->setText(model->getFilename().isEmpty() ? tr("(model not yet saved)") : model->getFilename());
	model_file_edt->setCursorPosition(0);
}

void ModelDbSelectorWidget::listDatabases()
{
	try
	{
		if(connections_cmb->currentIndex() == connections_cmb->count()-1)
		{
			if(ConnectionsConfigWidget::openConnectionsConfiguration(connections_cmb, true))
			{
				updateConnections();
				emit s_connectionsUpdateRequested();
			}
		}

		Connection *conn = reinterpret_cast<Connection *>(connections_cmb->currentData().value<void *>());
		bool is_srv_supported = true;
		QString srv_version;

		if(conn)
		{
			DatabaseImportHelper imp_helper;

			imp_helper.setConnection(*conn);
			DatabaseImportWidget::listDatabases(imp_helper, database_cmb);
			is_srv_supported = imp_helper.getCatalog().isServerSupported();
			srv_version = imp_helper.getCatalog().getServerVersion();

			if(conn->isAutoBrowseDB())
				database_cmb->setCurrentText(conn->getConnectionParam(Connection::ParamDbName));
		}
		else
			database_cmb->clear();

		database_cmb->setEnabled(database_cmb->count() > 0);
		database_lbl->setEnabled(database_cmb->isEnabled());

		if(Connection::isDbVersionIgnored() && !is_srv_supported)
		{
			alert_frm->setVisible(true);
			inv_version_warn_lbl->setText(tr("Unsupported server version <strong>%1</strong> detected! The database comparison may not work properly.").arg(srv_version));
		}
		else
			alert_frm->setVisible(false);
	}
	catch(Exception &e)
	{
		database_cmb->clear();
		database_cmb->setEnabled(false);
		database_lbl->setEnabled(false);
		alert_frm->setVisible(false);
		Messagebox::error(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}

	emit s_selectionChanged();
}

