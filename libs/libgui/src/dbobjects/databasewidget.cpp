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

#include "databasewidget.h"

DatabaseWidget::DatabaseWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Database)
{
	QStringList loc_list, encodings;
	QFrame *frame=nullptr;
	QGridLayout *grid=nullptr;

	Ui_DatabaseWidget::setupUi(this);
	configureFormLayout(database_grid, ObjectType::Database);

	def_schema_sel=new ObjectSelectorWidget(ObjectType::Schema, this);
	def_collation_sel=new ObjectSelectorWidget(ObjectType::Collation, this);
	def_owner_sel=new ObjectSelectorWidget(ObjectType::Role, this);
	def_tablespace_sel=new ObjectSelectorWidget(ObjectType::Tablespace, this);

	frame=generateInformationFrame(tr("The fields <strong>LC_COLLATE</strong> and <strong>LC_CTYPE</strong> have pre-configured values based upon the running system. You can freely modify those values if you intend to export the model to another host."));
	grid=dynamic_cast<QGridLayout *>(attributes_twg->widget(0)->layout());
	grid->addItem(new QSpacerItem(10,1,QSizePolicy::Fixed,QSizePolicy::Expanding), grid->count()+1, 0);
	grid->addWidget(frame, grid->count()+1, 0, 1, 0);

	frame=generateInformationFrame(tr("Use the above fields to specify the default attributes assigned to new objects created on the database model. Leaving a field empty will cause PostgreSQL to use the default values when exporting the model."));
	grid=dynamic_cast<QGridLayout *>(attributes_twg->widget(1)->layout());

	grid->addWidget(def_collation_sel, 0, 1);
	grid->addWidget(def_schema_sel, 1, 1);
	grid->addWidget(def_owner_sel, 2, 1);
	grid->addWidget(def_tablespace_sel, 3, 1);
	grid->addItem(new QSpacerItem(10,1,QSizePolicy::Fixed,QSizePolicy::Expanding), grid->count()+1, 0);
	grid->addWidget(frame, grid->count()+1, 0, 1, 0);
	frame->setParent(attributes_twg->widget(1));

	//Configures the encoding combobox
	encodings = EncodingType::getTypes();
	encodings.push_front(tr("Default"));
	encoding_cmb->addItems(encodings);

	//Configures the localizations combobox
	for(int i=QLocale::C; i <= QLocale::Chewa; i++)
	{
		for(int i1=QLocale::Afghanistan; i1 <= QLocale::Zimbabwe; i1++)
			loc_list.append(QLocale(static_cast<QLocale::Language>(i),static_cast<QLocale::Country>(i1)).name());
	}

	loc_list.removeDuplicates();
	loc_list.sort();
	loc_list.push_front(tr("Default"));

	lccollate_cmb->addItems(loc_list);
	lcctype_cmb->addItems(loc_list);

	setMinimumSize(560, 380);
}

void DatabaseWidget::setAttributes(DatabaseModel *model)
{
	if(!model)
		return;

	int idx = 0;

	connlim_sb->setValue(model->getConnectionLimit());
	templatedb_edt->setText(model->getTemplateDB());
	author_edt->setText(model->getAuthor());

	idx=encoding_cmb->findText(~model->getEncoding());
	if(idx < 0) idx=0;
	encoding_cmb->setCurrentIndex(idx);

	if(!model->getLocalization(Collation::LcCollate).isEmpty())
		lccollate_cmb->setCurrentText(model->getLocalization(Collation::LcCollate));

	if(!model->getLocalization(Collation::LcCtype).isEmpty())
		lcctype_cmb->setCurrentText(model->getLocalization(Collation::LcCtype));

	def_schema_sel->setModel(model);
	def_schema_sel->setSelectedObject(model->getDefaultObject(ObjectType::Schema));

	def_collation_sel->setModel(model);
	def_collation_sel->setSelectedObject(model->getDefaultObject(ObjectType::Collation));

	def_owner_sel->setModel(model);
	def_owner_sel->setSelectedObject(model->getDefaultObject(ObjectType::Role));

	def_tablespace_sel->setModel(model);
	def_tablespace_sel->setSelectedObject(model->getDefaultObject(ObjectType::Tablespace));

	allow_conn_chk->setChecked(model->isAllowConnections());
	is_template_chk->setChecked(model->isTemplate());

	gen_sql_dis_objs_chk->setChecked(model->isGenDisabledObjsCode());

	BaseObjectWidget::setAttributes(model, model, nullptr);
}

void DatabaseWidget::applyConfiguration()
{
	try
	{
		//Apply the basic configurations
		BaseObjectWidget::applyConfiguration();

		model->setAuthor(author_edt->text().toUtf8());
		model->setTemplateDB(templatedb_edt->text());
		model->setConnectionLimit(connlim_sb->value());

		if(encoding_cmb->currentIndex() > 0)
			model->setEncoding(EncodingType(encoding_cmb->currentText()));
		else
			model->setEncoding(EncodingType(EncodingType::Null));

		if(lccollate_cmb->currentText()!=tr("Default"))
			model->setLocalization(Collation::LcCollate, lccollate_cmb->currentText());
		else
			model->setLocalization(Collation::LcCollate, "");

		if(lcctype_cmb->currentText()!=tr("Default"))
			model->setLocalization(Collation::LcCtype, lcctype_cmb->currentText());
		else
			model->setLocalization(Collation::LcCtype, "");

		model->setDefaultObject(def_schema_sel->getSelectedObject(), ObjectType::Schema);
		model->setDefaultObject(def_owner_sel->getSelectedObject(), ObjectType::Role);
		model->setDefaultObject(def_collation_sel->getSelectedObject(), ObjectType::Collation);
		model->setDefaultObject(def_tablespace_sel->getSelectedObject(), ObjectType::Tablespace);
		model->setIsTemplate(is_template_chk->isChecked());
		model->setAllowConnections(allow_conn_chk->isChecked());
		model->setGenDisabledObjsCode(gen_sql_dis_objs_chk->isChecked());

		finishConfiguration();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
