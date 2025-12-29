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

#include "configurationwidget.h"
#include "guiutilsns.h"
#include <QButtonGroup>

ConfigurationWidget::ConfigurationWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	general_conf = new GeneralConfigWidget;
	appearance_conf = new AppearanceConfigWidget;
	connections_conf = new ConnectionsConfigWidget;
	relationships_conf = new RelationshipConfigWidget;
	snippets_conf = new SnippetsConfigWidget;
	plugins_conf = new PluginsConfigWidget;

	GuiUtilsNs::configureWidgetsFont({ apply_btn, revert_btn, defaults_btn },
																	 GuiUtilsNs::BigFontFactor);

	QList<BaseConfigWidget *> wgt_list { general_conf, appearance_conf, relationships_conf,
																			 connections_conf, snippets_conf, plugins_conf };

	for(auto &wgt : wgt_list)
	{
		confs_stw->addWidget(wgt);

		connect(wgt, &BaseConfigWidget::s_configurationChanged, this, [this](bool changed) {
			apply_btn->setEnabled(changed);
			revert_btn->setEnabled(changed);
		});
	}

	h_splitter->setSizes({700, 800});

	connect(revert_btn, &QPushButton::clicked, this, &ConfigurationWidget::__discardConfiguration);
	connect(apply_btn,  &QPushButton::clicked, this, __slot(this, ConfigurationWidget::applyConfiguration));
	connect(defaults_btn,  &QPushButton::clicked, this, __slot(this, ConfigurationWidget::restoreDefaults));

	QFont fnt;
	int view_idx = GeneralConfWgt;

	QButtonGroup *btn_group { new QButtonGroup(this) };
	btn_group->setExclusive(true);

	for(auto &btn : { general_tb, appearance_tb, relationships_tb,
										connections_tb, snippets_tb, plugins_tb  })
	{
		btn_group->addButton(btn);

		fnt = btn->font();
		fnt.setWeight(QFont::Medium);
		btn->setFont(fnt);
		btn->setProperty(Attributes::ObjectId.toStdString().c_str(), view_idx++);
		GuiUtilsNs::configureWidgetFont(btn, GuiUtilsNs::MediumFontFactor);
		//GuiUtilsNs::createDropShadow(btn, 1, 1, 5);
	}

	connect(btn_group, &QButtonGroup::buttonToggled, this, [this](QAbstractButton *btn){
		confs_stw->setCurrentIndex(btn->property(Attributes::ObjectId.toStdString().c_str()).toInt());
	});
}

ConfigurationWidget::~ConfigurationWidget()
{
	connections_conf->destroyConnections();
}

void ConfigurationWidget::hideEvent(QHideEvent *)
{
	general_tb->setChecked(true);
}

void ConfigurationWidget::showEvent(QShowEvent *)
{
	snippets_conf->snippet_txt->updateLineNumbers();
}

void ConfigurationWidget::__discardConfiguration()
{
	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);

		for(auto &conf_wgt : confs_stw->findChildren<BaseConfigWidget *>())
		{
			if(conf_wgt->isConfigurationChanged())
				conf_wgt->loadConfiguration();
		}

		emit s_configurationReverted();
		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

int ConfigurationWidget::checkChangedConfiguration()
{
	for(auto &conf_wgt : confs_stw->findChildren<BaseConfigWidget *>())
	{
		if(conf_wgt->isConfigurationChanged())
		{
			int res = Messagebox::confirm(tr("Some configuration parameters were changed! How do you wish to proceed?"),
																		Messagebox::AllButtons, tr("Apply"), tr("Discard"), tr("Keep editing"),
																		"", GuiUtilsNs::getIconPath("delete"), GuiUtilsNs::getIconPath("edit"));

			if(res == Messagebox::Accepted)
				applyConfiguration();
			else if(res == Messagebox::Rejected)
				__discardConfiguration();

			return res;
		}
	}

	return Messagebox::Accepted;
}

void ConfigurationWidget::applyConfiguration()
{
	qApp->setOverrideCursor(Qt::WaitCursor);

	for(auto &conf_wgt : confs_stw->findChildren<BaseConfigWidget *>())
	{
		if(conf_wgt->isConfigurationChanged())
			conf_wgt->saveConfiguration();
	}

	general_conf->applyConfiguration();
	relationships_conf->applyConfiguration();

	emit s_configurationChanged();

	qApp->restoreOverrideCursor();
}

void ConfigurationWidget::loadConfiguration()
{
	for(auto &conf_wgt : confs_stw->findChildren<BaseConfigWidget *>())
	{
		try
		{
			conf_wgt->loadConfiguration();
		}
		catch(Exception &e)
		{
			if(e.getErrorCode() == ErrorCode::PluginsNotLoaded)
			{
				Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
			}
			else
			{
				Messagebox msg_box;				
				Exception ex = Exception(Exception::getErrorMessage(ErrorCode::ConfigurationNotLoaded).arg(e.getExtraInfo()),PGM_FUNC,PGM_FILE,PGM_LINE, &e);

				msg_box.show(ex, QString("%1 %2").arg(ex.getErrorMessage(), tr("In some cases restore the default settings related to it may solve the problem. Would like to do that?")),
										 Messagebox::Alert, Messagebox::YesNoButtons, tr("Restore"), "", "", GuiUtilsNs::getIconPath("refresh"));

				if(msg_box.isAccepted())
					conf_wgt->restoreDefaults();
			}
		}
	}
}

void ConfigurationWidget::restoreDefaults()
{
	int res = Messagebox::confirm(tr("Any modification made until now in the current section will be lost! Do you really want to restore default settings?"));

	if(Messagebox::isAccepted(res))
		qobject_cast<BaseConfigWidget *>(confs_stw->currentWidget())->restoreDefaults();
}
