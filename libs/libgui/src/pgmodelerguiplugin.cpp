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

#include "pgmodelerguiplugin.h"
#include "guiutilsns.h"

QList<PgModelerGuiPlugin *> PgModelerGuiPlugin::reg_plugins;

PgModelerGuiPlugin::PgModelerGuiPlugin()
{
	main_window = nullptr;
	plugin_info_frm = new BaseForm;

	QWidget *widget = new QWidget;
	widget->setWindowTitle(QT_TRANSLATE_NOOP("PgModelerGuiPlugin", "Plugin information"));

	QGroupBox *info_grp = GuiUtilsNs::createWidgetInParent<QGroupBox>(GuiUtilsNs::LtMargin, widget);
	info_grp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	info_grp->setTitle(QT_TRANSLATE_NOOP("PgModelerGuiPlugin", "Version info"));

	title_lbl = GuiUtilsNs::createWidgetInParent<QLabel>(GuiUtilsNs::LtMargin, info_grp);
	author_lbl = GuiUtilsNs::createWidgetInParent<QLabel>(GuiUtilsNs::LtMargin, info_grp);
	version_lbl = GuiUtilsNs::createWidgetInParent<QLabel>(GuiUtilsNs::LtMargin, info_grp);

	GuiUtilsNs::configureWidgetFont(title_lbl, GuiUtilsNs::BigFontFactor, true);
	GuiUtilsNs::configureWidgetsFont({ author_lbl, version_lbl}, GuiUtilsNs::BigFontFactor);

	QHBoxLayout *hbox = GuiUtilsNs::createHBoxLayout(0);

	icon_lbl = new QLabel(widget);
	icon_lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	icon_lbl->setMinimumSize(QSize(64, 64));
	icon_lbl->setMaximumSize(QSize(64, 64));
	icon_lbl->setScaledContents(true);

	hbox->addWidget(info_grp);
	hbox->addWidget(icon_lbl);
	hbox->setAlignment(icon_lbl, Qt::AlignVCenter | Qt::AlignHCenter);

	widget->layout()->addItem(hbox);

	QGroupBox *description_grp = GuiUtilsNs::createWidgetInParent<QGroupBox>(GuiUtilsNs::LtMargin, widget);
	description_grp->setTitle(QT_TRANSLATE_NOOP("PgModelerGuiPlugin", "Description"));
	description_grp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	description_lbl = GuiUtilsNs::createWidgetInParent<QLabel>(GuiUtilsNs::LtMargin, description_grp);
	description_lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	description_lbl->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
	description_lbl->setWordWrap(true);
	description_lbl->setTextInteractionFlags(Qt::TextBrowserInteraction);

	widget->setMinimumSize(500, 250);
	plugin_info_frm->setMainWidget(widget);
}

PgModelerGuiPlugin::~PgModelerGuiPlugin()
{
	delete plugin_info_frm;
}

bool PgModelerGuiPlugin::registerPlugin(PgModelerGuiPlugin *plugin)
{
	if(!plugin || reg_plugins.contains(plugin))
		return false;

	reg_plugins.append(plugin);

	return true;
}

QList<QAction *> PgModelerGuiPlugin::getPluginsActions(ActionId act_id)
{
	QAction *act {};
	QList<QAction *> actions;

	for(auto &plug : reg_plugins)
	{
		act = plug->getAction(act_id);

		if(!act)
			continue;

		/* The action's data receives a const reference to the parent plugin,
		 * so it can be used for specific needs, e.g., in ModelWidget::configurePluginsActionMenu() */
		act->setData(QVariant::fromValue(dynamic_cast<PgModelerGuiPlugin *>(plug)));
		actions.append(act);
	}

	return actions;
}

QList<QToolButton *> PgModelerGuiPlugin::getPluginsToolButtons()
{
	QToolButton *btn {};
	QList<QToolButton *> buttons;

	for(auto &plug : reg_plugins)
	{
		btn = plug->getToolButton();

		if(!btn)
			continue;

		buttons.append(btn);
	}

	return buttons;
}

QList<PgModelerGuiPlugin::PluginWidgets> PgModelerGuiPlugin::getPluginsWidgets(QWidget *parent)
{
	QList<PluginWidgets> widgets;
	PluginWidgets p_wgt;

	for(auto &plug : reg_plugins)
	{
		p_wgt = plug->createWidgets(parent);

		if(!p_wgt.button && !p_wgt.widget)
			continue;

		widgets.append(p_wgt);
	}

	return widgets;
}

void PgModelerGuiPlugin::initPlugin(MainWindow *main_window)
{
	this->main_window = main_window;

	configurePluginInfo(getPluginTitle(),
						getPluginVersion(),
						getPluginAuthor(),
						getPluginDescription());
}

void PgModelerGuiPlugin::postInitPlugin()
{
	if(!main_window)
	{
		throw Exception(QT_TRANSLATE_NOOP("PgModelerGuiPlugin", "Attempting to post-initialize a plugin without initializing the application's main window!"),
										ErrorCode::Custom, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void PgModelerGuiPlugin::configurePluginInfo(const QString &title, const QString &version, const QString &author, const QString &description)
{
	title_lbl->setText(title);
	version_lbl->setText(QString(QT_TRANSLATE_NOOP("PgModelerPlugin", "<strong>Version</strong> %1")).arg(version));
	author_lbl->setText(QString(QT_TRANSLATE_NOOP("PgModelerPlugin","<strong>Author</strong> %1")).arg(author));
	description_lbl->setText(description);
	icon_lbl->setPixmap(getPluginPixmap(getPluginName()));
}

void PgModelerGuiPlugin::showPluginInfo()
{
	plugin_info_frm->show();
}

QString PgModelerGuiPlugin::getPluginIconPath(const QString &icon_name)
{
	return QString(":/%1/%2.png").arg(getPluginName(), icon_name);
}

QIcon PgModelerGuiPlugin::getPluginIcon(const QString &icon_name)
{
	return { QIcon(getPluginIconPath(icon_name)) };
}

QPixmap PgModelerGuiPlugin::getPluginPixmap(const QString &icon_name)
{
	return { QPixmap(getPluginIconPath(icon_name)) };
}

PgModelerGuiPlugin::PluginWidgets PgModelerGuiPlugin::createWidgets(QWidget *)
{
	return {};
}

bool PgModelerGuiPlugin::isSelectionValid()
{
	return true;
}
