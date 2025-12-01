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

#include "indexwidget.h"
#include "guiutilsns.h"

IndexWidget::IndexWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Index)
{
	Ui_IndexWidget::setupUi(this);
	GuiUtilsNs::configureWidgetFont(fill_factor_chk, GuiUtilsNs::SmallFontFactor, true);

	predicate_hl = new SyntaxHighlighter(predicate_txt, false, true, font().pointSizeF());
	predicate_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	elements_tab = GuiUtilsNs::createWidgetInParent<ElementsTableWidget>(GuiUtilsNs::LtMargin, elements_pg);
	incl_cols_picker_wgt = GuiUtilsNs::createWidgetInParent<ColumnPickerWidget>(GuiUtilsNs::LtMargin, include_cols_pg);

	indexing_cmb->addItems(IndexingType::getTypes());

	connect(indexing_cmb, &QComboBox::currentIndexChanged, this, &IndexWidget::selectIndexingType);
	connect(fill_factor_chk, &QCheckBox::toggled, fill_factor_sb, &QSpinBox::setEnabled);

	configureTabbedLayout(attributes_tbw);
	configureTabOrder();
	selectIndexingType();

	setMinimumSize(600, 400);
}

void IndexWidget::selectIndexingType()
{
	fast_update_chk->setEnabled(IndexingType(indexing_cmb->currentText())==IndexingType::Gin);
	buffering_chk->setEnabled(IndexingType(indexing_cmb->currentText())==IndexingType::Gist);
	fill_factor_sb->setEnabled(fill_factor_chk->isChecked() && fill_factor_chk->isEnabled());
}

void IndexWidget::setAttributes(DatabaseModel *model, OperationList *op_list, BaseTable *parent_obj, Index *index)
{
	std::vector<IndexElement> idx_elems;

	if(!parent_obj)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	BaseObjectWidget::setAttributes(model, op_list, index, parent_obj);

	incl_cols_picker_wgt->setParentObject(parent_obj);

	if(index)
	{
		if(parent_obj->getObjectType() == ObjectType::View)
			incl_cols_picker_wgt->setColumns(index->getSimpleColumns());
		else
			incl_cols_picker_wgt->setColumns(index->getColumns());

		idx_elems = index->getIndexElements();
		indexing_cmb->setCurrentIndex(indexing_cmb->findText(~index->getIndexingType()));

		fill_factor_chk->setChecked(index->getFillFactor() >= 10);

		if(fill_factor_chk->isChecked())
			fill_factor_sb->setValue(index->getFillFactor());
		else
			fill_factor_sb->setValue(90);

		concurrent_chk->setChecked(index->getIndexAttribute(Index::Concurrent));
		fast_update_chk->setChecked(index->getIndexAttribute(Index::FastUpdate));
		unique_chk->setChecked(index->getIndexAttribute(Index::Unique));
		buffering_chk->setChecked(index->getIndexAttribute(Index::Buffering));
		nulls_not_distinct_chk->setChecked(index->getIndexAttribute(Index::NullsNotDistinct));
		predicate_txt->setPlainText(index->getPredicate());

		selectIndexingType();
	}

	elements_tab->setAttributes<IndexElement>(model, parent_obj);
	elements_tab->setElements<IndexElement>(idx_elems);
}

void IndexWidget::applyConfiguration()
{
	try
	{
		Index *index=nullptr;
		std::vector<IndexElement> idx_elems;

		startConfiguration<Index>();

		index=dynamic_cast<Index *>(this->object);

		BaseObjectWidget::applyConfiguration();

		index->setIndexAttribute(Index::FastUpdate, fast_update_chk->isChecked());
		index->setIndexAttribute(Index::Concurrent, concurrent_chk->isChecked());
		index->setIndexAttribute(Index::Unique, unique_chk->isChecked());
		index->setIndexAttribute(Index::Buffering, buffering_chk->isChecked());
		index->setIndexAttribute(Index::NullsNotDistinct, nulls_not_distinct_chk->isChecked());
		index->setPredicate(predicate_txt->toPlainText().toUtf8());
		index->setIndexingType(IndexingType(indexing_cmb->currentText()));

		if(fill_factor_chk->isChecked())
			index->setFillFactor(fill_factor_sb->value());
		else
			index->setFillFactor(0);

		elements_tab->getElements<IndexElement>(idx_elems);
		index->addIndexElements(idx_elems);

		if(table && table->getObjectType() == ObjectType::View)
			index->setSimpleColumns(incl_cols_picker_wgt->getSimpleColumns());
		else
			index->setColumns(incl_cols_picker_wgt->getColumns());

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

