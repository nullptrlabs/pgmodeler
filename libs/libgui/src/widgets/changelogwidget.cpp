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

#include "changelogwidget.h"
#include "customuistyle.h"
#include "settings/generalconfigwidget.h"
#include "guiutilsns.h"
#include "baseform.h"
#include "csvparser.h"

ChangelogWidget::ChangelogWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	model = nullptr;
	setModel(nullptr);

	CustomUiStyle::setStyleHint(CustomUiStyle::DefaultFrmHint, summary_frm);
	CustomUiStyle::setStyleHint(CustomUiStyle::DefaultFrmHint, separator_ln);

	GuiUtilsNs::createDropShadow(this, 5, 5, 30);

	GuiUtilsNs::configureWidgetsFont({ added_cnt_lbl, removed_cnt_lbl,
																		 updated_cnt_lbl, total_cnt_lbl },
																	 GuiUtilsNs::HugeFontFactor);

	connect(inspect_btn, &QPushButton::clicked, this, &ChangelogWidget::inspectChangelog);
	connect(hide_tb, &QToolButton::clicked, this, &ChangelogWidget::s_visibilityChanged);
	connect(clear_btn, &QPushButton::clicked, this, &ChangelogWidget::clearChangelog);
	connect(persisted_chk, &QCheckBox::toggled, this, [this](bool checked){
		model->getDatabaseModel()->setPersistedChangelog(checked);
		model->setModified(true);
	});
}

void ChangelogWidget::setVisible(bool value)
{
	QWidget::setVisible(value);
	adjustSize();
	emit s_visibilityChanged(value);
}

void ChangelogWidget::updateChangelogInfo()
{
	QString last_change_text = tr("Last change: <strong>%1</strong>"),
			first_change_text = tr("First change: <strong>%1</strong>");
	unsigned total_len = !model ? 0 : model->getDatabaseModel()->getChangelogLength();

	if(total_len == 0)
	{
		last_change_lbl->setText(last_change_text.arg('-'));
		first_change_lbl->setText(first_change_text.arg('-'));
		added_cnt_lbl->setText("-");
		removed_cnt_lbl->setText("-");
		updated_cnt_lbl->setText("-");
		total_cnt_lbl->setText("-");
	}
	else
	{
		QString ui_lang = GeneralConfigWidget::getConfigurationParam(Attributes::Configuration, Attributes::UiLanguage),
				dt_format;
		QLocale locale(ui_lang);
		DatabaseModel *db_model = model->getDatabaseModel();

		dt_format = locale.dateTimeFormat();
		dt_format.remove('t'); // Removing timezone info
		dt_format.remove("dddd,"); //Removing day of week name info

		last_change_lbl->setText(last_change_text.arg(locale.toString(db_model->getLastChangelogDate(), dt_format)));
		first_change_lbl->setText(first_change_text.arg(locale.toString(db_model->getFirstChangelogDate(), dt_format)));

		added_cnt_lbl->setText(QString::number(db_model->getChangelogLength(Operation::ObjCreated)));
		removed_cnt_lbl->setText(QString::number(db_model->getChangelogLength(Operation::ObjRemoved)));
		updated_cnt_lbl->setText(QString::number(db_model->getChangelogLength(Operation::ObjModified)));
		total_cnt_lbl->setText(QString::number(total_len));
	}

	inspect_btn->setEnabled(total_len > 0);
	clear_btn->setEnabled(total_len > 0);
	adjustSize();
}

void ChangelogWidget::clearChangelog()
{
	Messagebox msgbox;

	msgbox.show("",
							tr("<strong>ATTENTION:</strong> All the changelog records made until today will be lost and the filtering by \
date of modification in partial diff will be unavailable! Do you want to proceed?"),
								 Messagebox::Alert, Messagebox::YesNoButtons);

	if(msgbox.isAccepted())
	{
		model->getDatabaseModel()->clearChangelog();
		model->setModified(true);
		updateChangelogInfo();
	}
}

void ChangelogWidget::inspectChangelog()
{
	BaseForm base_form;
	QTableWidget *data_tbw = new QTableWidget;
	CsvDocument csv_doc;
	CsvParser csv_parser;

	try
	{
		QString csv_buf = model->getDatabaseModel()->getChangelogDefinition(true);

		csv_buf.prepend(QString("\"%1\";\"%2\";\"%3\";\"%4\"\n")
									 .arg(tr("Date"), tr("Object"), tr("Type"), tr("Action")));

		csv_parser.setColumnInFirstRow(true);
		csv_doc = csv_parser.parseBuffer(csv_buf);
		GuiUtilsNs::populateTable(data_tbw, csv_doc);

		QHeaderView *header = data_tbw->horizontalHeader();
		QAbstractItemModel *h_model = header->model();
		QStringList icons = {
			GuiUtilsNs::getIconPath("typedatetime"), GuiUtilsNs::getIconPath("objects"),
			GuiUtilsNs::getIconPath("usertype"), GuiUtilsNs::getIconPath("changelog")
		};

		for(int section = 0; section <= 3; section++)
		{
			h_model->setHeaderData(section, Qt::Horizontal,
														 QVariant::fromValue<Qt::Alignment>(Qt::AlignLeft | Qt::AlignVCenter),
														 Qt::TextAlignmentRole);

			h_model->setHeaderData(section, Qt::Horizontal, QIcon(icons[section]), Qt::DecorationRole);
		}

		ObjectType obj_type;
		QFont fnt;
		QString action;
		QMap<QString, QString> actions_i18n = {
			{	Attributes::Created, tr("Created") },
			{ Attributes::Deleted, tr("Deleted") },
			{ Attributes::Updated, tr("Updated") }
		},

		actions_icons = {
			{	Attributes::Created, "created" },
			{ Attributes::Deleted, "removed" },
			{ Attributes::Updated, "modified" }
		};

		data_tbw->setUpdatesEnabled(false);

		for(int row = 0; row < data_tbw->rowCount(); row++)
		{
			obj_type = BaseObject::getObjectType(data_tbw->item(row, 2)->text());
			action = data_tbw->item(row, 3)->text();

			data_tbw->item(row, 1)->setIcon(QIcon(GuiUtilsNs::getIconPath(obj_type)));
			data_tbw->item(row, 2)->setText(BaseObject::getTypeName(obj_type));
			data_tbw->item(row, 3)->setText(actions_i18n[action]);
			data_tbw->item(row, 3)->setIcon(QIcon(GuiUtilsNs::getIconPath(actions_icons[action])));

			fnt = data_tbw->item(row, 2)->font();
			fnt.setItalic(true);
			data_tbw->item(row, 2)->setFont(fnt);
		}

		header->swapSections(0, 1);
		header->swapSections(1, 2);
		header->setStretchLastSection(true);
		data_tbw->setSortingEnabled(true);
		data_tbw->setEditTriggers(QAbstractItemView::NoEditTriggers);
		data_tbw->setAlternatingRowColors(true);
		data_tbw->setUpdatesEnabled(true);
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	data_tbw->setWindowTitle("Changelog entries");
	base_form.setMainWidget(data_tbw);
	base_form.setButtonConfiguration(Messagebox::OkButton);
	base_form.apply_ok_btn->setShortcut(QKeySequence("Enter"));
	base_form.main_frm->layout()->setContentsMargins(GuiUtilsNs::LtMargins);

	GeneralConfigWidget::restoreWidgetGeometry(&base_form, this->metaObject()->className());
	base_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&base_form, this->metaObject()->className());
}

void ChangelogWidget::setModel(ModelWidget *model)
{	
	if(this->model)
		disconnect(this->model, nullptr, this, nullptr);

	this->model = model;
	updateChangelogInfo();
	frame->setEnabled(model != nullptr);

	if(model)
	{
		persisted_chk->setChecked(model->getDatabaseModel()->isPersistedChangelog());

		connect(this->model, &ModelWidget::s_objectManipulated, this, &ChangelogWidget::updateChangelogInfo);
		connect(this->model, &ModelWidget::s_objectModified, this, &ChangelogWidget::updateChangelogInfo);
		connect(this->model, &ModelWidget::s_objectCreated, this, &ChangelogWidget::updateChangelogInfo);
		connect(this->model, &ModelWidget::s_objectRemoved, this, &ChangelogWidget::updateChangelogInfo);
	}
}
