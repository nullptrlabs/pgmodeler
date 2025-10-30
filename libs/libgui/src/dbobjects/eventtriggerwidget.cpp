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

#include "eventtriggerwidget.h"

EventTriggerWidget::EventTriggerWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::EventTrigger)
{
	std::map<QString, std::vector<QWidget *> > fields_map;
	std::map<QWidget *, std::vector<QString> > values_map;

	Ui_EventTriggerWidget::setupUi(this);

	function_sel=new ObjectSelectorWidget(ObjectType::Function, this);
	filter_tab=new CustomTableWidget(CustomTableWidget::AddButton |
									 CustomTableWidget::EditButton |
									 CustomTableWidget::UpdateButton |
									 CustomTableWidget::RemoveButton |
									 CustomTableWidget::RemoveAllButton |
									 CustomTableWidget::MoveButtons, false, this);
	filter_tab->setColumnCount(1);
	filter_tab->setHeaderLabel(tr("Tag command"), 0);

	eventtrigger_grid->addWidget(function_sel, 1, 1);
	filter_layout->addWidget(filter_tab);

	configureFormLayout(eventtrigger_grid, ObjectType::EventTrigger);
	setRequiredField(function_lbl);

	configureTabOrder({ event_cmb, function_sel, tag_edt, filter_tab });

	event_cmb->addItems(EventTriggerType::getTypes());

	connect(filter_tab, &CustomTableWidget::s_rowAdded, this, &EventTriggerWidget::handleTagValue);
	connect(filter_tab, &CustomTableWidget::s_rowUpdated, this, &EventTriggerWidget::handleTagValue);

	connect(filter_tab, &CustomTableWidget::s_rowsRemoved,	this,[this](){
		filter_tab->setButtonsEnabled(CustomTableWidget::AddButton, false);
	});

	connect(filter_tab, &CustomTableWidget::s_rowEdited,	this, [this](int row){
		tag_edt->setText(filter_tab->getCellText(row, 0));
	});

	connect(tag_edt, &QLineEdit::textChanged, this, [this](){
		filter_tab->setButtonsEnabled(CustomTableWidget::AddButton, !tag_edt->text().isEmpty());
		filter_tab->setButtonsEnabled(CustomTableWidget::UpdateButton, !tag_edt->text().isEmpty());
	});

	setMinimumSize(500, 440);
}

void EventTriggerWidget::setAttributes(DatabaseModel *model, OperationList *op_list, EventTrigger *event_trig)
{
	BaseObjectWidget::setAttributes(model, op_list, event_trig);
	function_sel->setModel(model);

	if(event_trig)
	{
		event_cmb->setCurrentText(~event_trig->getEvent());
		function_sel->setSelectedObject(event_trig->getFunction());

		QStringList filter=event_trig->getFilter(Attributes::Tag.toUpper());

		if(filter.isEmpty())
			filter=event_trig->getFilter(Attributes::Tag);

		filter_tab->blockSignals(true);

		for(auto &flt : filter)
		{
			filter_tab->addRow();
			filter_tab->setCellText(flt, filter_tab->getRowCount()-1, 0);
		}

		filter_tab->blockSignals(false);
		filter_tab->clearSelection();
	}

	filter_tab->setButtonsEnabled(CustomTableWidget::AddButton, false);
}

void EventTriggerWidget::applyConfiguration()
{
	try
	{
		EventTrigger *event_trig=nullptr;

		startConfiguration<EventTrigger>();
		event_trig=dynamic_cast<EventTrigger *>(this->object);
		BaseObjectWidget::applyConfiguration();

		event_trig->setEvent(EventTriggerType(event_cmb->currentText()));
		event_trig->setFunction(dynamic_cast<Function *>(function_sel->getSelectedObject()));

		event_trig->clearFilter();
		for(unsigned row=0; row < filter_tab->getRowCount(); row++)
			event_trig->setFilter(Attributes::Tag.toUpper(), filter_tab->getCellText(row, 0));

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void EventTriggerWidget::handleTagValue(int row)
{
	if(!tag_edt->text().isEmpty())
	{
		filter_tab->setCellText(tag_edt->text().simplified(), row, 0);
		tag_edt->clear();
		filter_tab->clearSelection();
		filter_tab->setButtonsEnabled(CustomTableWidget::AddButton, false);
	}
	else if(filter_tab->getCellText(row, 0).isEmpty())
		filter_tab->removeRow(row);
}
