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

#include "columnwidget.h"
#include "sequencewidget.h"
#include "baseform.h"
#include "settings/generalconfigwidget.h"
#include "coreutilsns.h"
#include "pgsqlversions.h"

ColumnWidget::ColumnWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Column)
{
	QSpacerItem *spacer=new QSpacerItem(10,10,QSizePolicy::Fixed,QSizePolicy::Expanding);
	QStringList list;

	Ui_ColumnWidget::setupUi(this);
	edit_seq_btn->setVisible(false);

	identity_type_cmb->addItems(IdentityType::getTypes());

	data_type=nullptr;
	data_type=new PgSQLTypeWidget(this);

	hl_default_value=nullptr;
	hl_default_value=new SyntaxHighlighter(def_value_txt, true, false, font().pointSizeF());
	hl_default_value->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	sequence_sel=new ObjectSelectorWidget(ObjectType::Sequence, this);
	sequence_sel->setEnabled(false);

	column_grid->addWidget(data_type,0,0,1,0);
	column_grid->addWidget(default_value_grp,1,0,1,1);

	column_grid->addItem(spacer,column_grid->count(),0);
	dynamic_cast<QGridLayout *>(default_value_grp->layout())->addWidget(sequence_sel, 1, 1, 1, 6);

	configureFormLayout(column_grid, ObjectType::Column);
	configureTabOrder({ data_type });

	std::map<QString, std::vector<QWidget *> > fields_map;
	fields_map[generateVersionsInterval(AfterVersion, PgSqlVersions::PgSqlVersion120)].push_back(generated_chk);
	highlightVersionSpecificFields(fields_map);

	connect(expression_rb, &QRadioButton::toggled, this, &ColumnWidget::enableDefaultValueFields);
	connect(sequence_rb, &QRadioButton::toggled, this, &ColumnWidget::enableDefaultValueFields);
	connect(identity_rb, &QRadioButton::toggled, this, &ColumnWidget::enableDefaultValueFields);

	connect(generated_chk, &QCheckBox::toggled, this, [this](bool value){
		notnull_chk->setDisabled(value);
		notnull_chk->setChecked(false);
	});

	connect(edit_seq_btn, &QPushButton::clicked, this, __slot(this, ColumnWidget::editSequenceAttributes));

	setMinimumSize(540, 480);
}

void ColumnWidget::enableDefaultValueFields()
{
	bool is_expr = sender() == expression_rb,
			is_seq = sender() == sequence_rb,
			is_ident = sender() == identity_rb;

	sequence_sel->setEnabled(is_seq);
	def_value_txt->setEnabled(is_expr);
	identity_type_cmb->setEnabled(is_ident);
	notnull_chk->setEnabled(is_expr || is_seq);
	edit_seq_btn->setVisible(is_ident);
	generated_chk->setEnabled(is_expr);
	generated_chk->setChecked(false);
}

void ColumnWidget::setAttributes(DatabaseModel *model, OperationList *op_list, BaseObject *parent_obj, Column *column)
{
	PgSqlType type;

	if(!parent_obj)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	BaseObjectWidget::setAttributes(model, op_list, column, parent_obj);
	sequence_sel->setModel(model);

	ident_col_seq.setValues("", "", "", "", "");
	ident_col_seq.setCycle(false);

	if(column)
	{
		type=column->getType();
		notnull_chk->setChecked(column->isNotNull());
		generated_chk->setChecked(column->isGenerated());
		def_value_txt->setPlainText(column->getDefaultValue());

		if(column->getSequence())
		{
			sequence_rb->click();
			sequence_sel->setEnabled(true);
			sequence_sel->setSelectedObject(column->getSequence());
		}
		else if(column->getIdentityType() != IdentityType::Null)
		{
			identity_rb->click();
			identity_type_cmb->setEnabled(true);
			identity_type_cmb->setCurrentText(~column->getIdentityType());
			notnull_chk->setEnabled(false);
		}
	}

	data_type->setAttributes(type, model, true,
													 UserTypeConfig::BaseType | UserTypeConfig::TableType |
													 UserTypeConfig::ViewType | UserTypeConfig::ForeignTableType |
													 UserTypeConfig::DomainType, true, false);
}

void ColumnWidget::editSequenceAttributes()
{
	Column *col = dynamic_cast<Column *>(this->object);
	Schema *schema = nullptr;
	BaseForm editing_form(this);
	SequenceWidget *seq_wgt=new SequenceWidget;
	BaseTable *table = col ? col->getParentTable() : nullptr;

	if(table)
		schema = dynamic_cast<Schema *>(table->getSchema());
	else
		schema = this->model->getSchema("public");

	ident_col_seq.setName(QString("%1_%2_seq").arg(table ? table->getName() : "").arg(col ? col->getName() : "new_column"));
	ident_col_seq.setName(CoreUtilsNs::generateUniqueName(&ident_col_seq, *model->getObjectList(ObjectType::Sequence), false));
	ident_col_seq.setSchema(schema);

	if(col)
	{
		ident_col_seq.setDefaultValues(col->getType());
		ident_col_seq.setValues(col->getIdSeqMinValue(), col->getIdSeqMaxValue(), col->getIdSeqIncrement(), col->getIdSeqStart(), col->getIdSeqCache());
		ident_col_seq.setCycle(col->isIdSeqCycle());
	}

	seq_wgt->setAttributesReadonly(this->model, nullptr, nullptr, &ident_col_seq, col);
	editing_form.setMainWidget(seq_wgt);
	GeneralConfigWidget::restoreWidgetGeometry(&editing_form, seq_wgt->metaObject()->className());
	editing_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&editing_form, seq_wgt->metaObject()->className());
}

void ColumnWidget::applyConfiguration()
{
	try
	{
		Column *column=nullptr;
		Constraint *pk = nullptr, *constr = nullptr;
		PhysicalTable *parent_tab = dynamic_cast<PhysicalTable *>(table);
		std::vector<Constraint *> fks;
		BaseRelationship *rel = nullptr;
		startConfiguration<Column>();

		column=dynamic_cast<Column *>(this->object);
		column->setNotNull(notnull_chk->isChecked());
		column->setGenerated(generated_chk->isChecked());
		column->setType(data_type->getPgSQLType());

		if(expression_rb->isChecked())
			column->setDefaultValue(def_value_txt->toPlainText());
		else if(sequence_rb->isChecked())
			column->setSequence(sequence_sel->getSelectedObject());
		else
			column->setIdentityType(IdentityType(identity_type_cmb->currentText()));

		column->setIdSeqAttributes(ident_col_seq.getMinValue(), ident_col_seq.getMaxValue(), ident_col_seq.getIncrement(),
															 ident_col_seq.getStart(), ident_col_seq.getCache(), ident_col_seq.isCycle());

		if(parent_tab)
		{
			pk = parent_tab->getPrimaryKey();

			if(pk && pk->isColumnReferenced(column) && !notnull_chk->isChecked())
				throw Exception(Exception::getErrorMessage(ErrorCode::NullPrimaryKeyColumn)
												.arg(column->getName())
												.arg(pk->getParentTable()->getSignature(true)),
												ErrorCode::NullPrimaryKeyColumn,PGM_FUNC,PGM_FILE,PGM_LINE);

			// Separating fks in which the column is part so the fk relationships can be properly updated
			for(unsigned idx = 0; idx < parent_tab->getConstraintCount(); idx++)
			{
				constr = parent_tab->getConstraint(idx);

				if(constr && constr->getConstraintType() == ConstraintType::ForeignKey && constr->isColumnExists(column, Constraint::SourceCols))
					fks.push_back(constr);
			}
		}

		BaseObjectWidget::applyConfiguration();
		model->updateViewsReferencingTable(parent_tab);

		/* Updating the mandatory state of destination tables on FK relationships
		 * derived from the table's fk in which the column is part of based upon
		 * notnull state of the column */
		for(auto &fk : fks)
		{
			rel = model->getRelationship(fk->getParentTable(), fk->getReferencedTable(), fk);

			if(rel)
			{
				rel->setMandatoryTable(BaseRelationship::DstTable, column->isNotNull());
				rel->setModified(true);
			}
		}

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

