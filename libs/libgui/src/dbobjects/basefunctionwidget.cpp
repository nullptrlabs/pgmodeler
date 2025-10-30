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

#include "basefunctionwidget.h"
#include "baseform.h"
#include "defaultlanguages.h"
#include "guiutilsns.h"
#include "dbobjects/parameterwidget.h"

BaseFunctionWidget::BaseFunctionWidget(QWidget *parent, ObjectType obj_type) : BaseObjectWidget(parent, obj_type)
{
	QGridLayout *grid = nullptr;
	Ui_BaseFunctionWidget::setupUi(this);

	source_code_txt = new NumberedTextEditor(this, true);
	dynamic_cast<QGridLayout *>(source_code_frm->layout())->addWidget(source_code_txt, 1, 0, 1, 2);

	source_code_hl = new SyntaxHighlighter(source_code_txt);
	source_code_cp = new CodeCompletionWidget(source_code_txt, true);

	parameters_tab = new CustomTableWidget(CustomTableWidget::AllButtons ^
																					CustomTableWidget::UpdateButton, true, this);
	parameters_tab->setColumnCount(4);
	parameters_tab->setHeaderLabel(tr("Name"), 0);
	parameters_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("parameter")), 0);
	parameters_tab->setHeaderLabel(tr("Type"), 1);
	parameters_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")), 1);
	parameters_tab->setHeaderLabel(tr("Mode"), 2);
	parameters_tab->setHeaderLabel(tr("Default Value"), 3);

	grid = new QGridLayout;
	grid->addWidget(parameters_tab, 0, 0, 1, 1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	func_config_twg->widget(1)->setLayout(grid);
	security_cmb->addItems(SecurityType::getTypes());


	transform_type_wgt = new PgSQLTypeWidget(this);
	transform_types_tab = new CustomTableWidget(CustomTableWidget::AllButtons ^
																							 (CustomTableWidget::UpdateButton |
																								CustomTableWidget::EditButton |
																								CustomTableWidget::ResizeColsButton), true, this);
	transform_types_tab->setColumnCount(1);
	transform_types_tab->setHeaderLabel(tr("Type"), 0);
	transform_types_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")), 0);

	grid = new QGridLayout;
	grid->addWidget(transform_type_wgt, 0, 0, 1, 1);
	grid->addWidget(transform_types_tab, 1, 0, 1, 1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	func_config_twg->widget(2)->setLayout(grid);


	config_params_tab = new CustomTableWidget(CustomTableWidget::AllButtons ^
																							 (CustomTableWidget::UpdateButton |
																								CustomTableWidget::EditButton), true, this);
	config_params_tab->setColumnCount(2);
	config_params_tab->setHeaderLabel(tr("Parameter"), 0);
	config_params_tab->setHeaderLabel(tr("Value"), 1);
	config_params_tab->setCellsEditable(true);

	grid = new QGridLayout;
	grid->addWidget(config_params_tab, 0, 0, 1, 1);
	grid->addWidget(hint_frm, 1, 0, 1, 1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	func_config_twg->widget(4)->setLayout(grid);

	connect(language_cmb, &QComboBox::currentIndexChanged, this, __slot(this, BaseFunctionWidget::selectLanguage));

	connect(transform_types_tab, &CustomTableWidget::s_rowAdded, this, [this](int row){
		__trycatch( transform_types_tab->setCellText(~transform_type_wgt->getPgSQLType(), row, 0); )
	});

	setRequiredField(language_lbl);
	setRequiredField(symbol_lbl);
	setRequiredField(library_lbl);
	setRequiredField(sourc_code_lbl);
}

void BaseFunctionWidget::handleParameter(CustomTableWidget *params_tab, Parameter param, int result, bool handle_param_modes)
{
	if(!params_tab) return;

	int row_cnt = 0, row = 0;
	row_cnt = params_tab->getRowCount();

	//Case the user applied the configuration on the parameter editing form
	if(result == QDialog::Accepted)
	{
		row = params_tab->getSelectedRow();

		/* If the row index is negative indicates the row in question is empty, e.g.,
		the user is not editing an existing row, but adding a new one, so the line to be
		considered in the table will always be the last recently included */
		if(row < 0) row = row_cnt-1;

		showParameterData(params_tab, param, row, handle_param_modes);
	}
	else if(result == QDialog::Rejected)
	{
		//Removes the last line from table
		if(row_cnt > 0 && params_tab->getCellText(row_cnt - 1, 0).isEmpty())
			params_tab->removeRow(row_cnt - 1);
	}
}

void BaseFunctionWidget::duplicateParameter(CustomTableWidget *params_tab, int curr_row, int new_row, bool dup_param_modes)
{
	Parameter new_param;
	new_param = getParameter(params_tab, curr_row, dup_param_modes);
	new_param.setName(new_param.getName() + "_cp");
	showParameterData(params_tab, new_param, new_row, dup_param_modes);
}

void BaseFunctionWidget::showParameterForm(CustomTableWidget *params_tab, bool enable_param_modes)
{
	if(!params_tab) return;

	try
	{
		Parameter aux_param;
		int row_idx;
		ParameterWidget *parameter_wgt = new ParameterWidget;
		BaseForm parent_form;

		parameter_wgt->param_in_chk->setEnabled(enable_param_modes);
		parameter_wgt->param_out_chk->setEnabled(enable_param_modes);
		parameter_wgt->param_variadic_chk->setEnabled(enable_param_modes);
		parameter_wgt->default_value_edt->setEnabled(enable_param_modes);

		row_idx=params_tab->getSelectedRow();

		if(row_idx >= 0 && !params_tab->getCellText(row_idx, 0).isEmpty())
			aux_param = getParameter(params_tab, row_idx, enable_param_modes);

		parameter_wgt->setAttributes(aux_param, model);
		parent_form.setMainWidget(parameter_wgt);
		parent_form.exec();

		aux_param = parameter_wgt->getParameter();
		handleParameter(params_tab, aux_param, parent_form.result(), enable_param_modes);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

Parameter BaseFunctionWidget::getParameter(CustomTableWidget *params_tab, unsigned row, bool set_param_modes)
{
	Parameter param;

	if(params_tab)
	{
		try
		{
			QString param_modes;
			param.setName(params_tab->getCellText(row,0));
			param.setType(params_tab->getRowData(row).value<PgSqlType>());

			if(set_param_modes)
			{
				param_modes = params_tab->getCellText(row, 2);
				param.setIn(param_modes.contains("IN"));
				param.setOut(param_modes.contains("OUT"));
				param.setVariadic(param_modes == "VARIADIC");
				param.setDefaultValue(params_tab->getCellText(row, 3));
			}
		}
		catch(Exception &e)
		{
			throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}
	}

	return param;
}

void BaseFunctionWidget::showParameterData(CustomTableWidget *params_tab, Parameter param, unsigned row, bool show_param_modes)
{
	if(!params_tab) return;

	params_tab->setCellText(param.getName(), row, 0);
	params_tab->setCellText(*param.getType(), row, 1);
	params_tab->setRowData(QVariant::fromValue<PgSqlType>(param.getType()), row);

	if(show_param_modes)
	{
		params_tab->setCellText(param.getModeString(), row, 2);
		params_tab->setCellText(param.getDefaultValue(), row, 3);
	}
}

void BaseFunctionWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, BaseFunction *func)
{
	std::vector<BaseObject *> languages;
	Language *lang = nullptr;
	QStringList list;
	unsigned count = 0, i = 0;
	Parameter param;
	PgSqlType aux_type;

	BaseObjectWidget::setAttributes(model, op_list, func, schema);
	languages = model->getObjects(ObjectType::Language);

	while(!languages.empty())
	{
		lang = dynamic_cast<Language *>(languages.back());
		languages.pop_back();
		list.append(lang->getName());
	}

	list.sort();
	language_cmb->addItems(list);
	language_cmb->setCurrentText(DefaultLanguages::Sql);

	transform_type_wgt->setAttributes(PgSqlType(), model, false);
	transform_types_tab->removeRows();

	if(func)
	{
		language_cmb->setCurrentIndex(language_cmb->findText(func->getLanguage()->getName()));
		security_cmb->setCurrentIndex(security_cmb->findText(~func->getSecurityType()));

		count = func->getParameterCount();
		parameters_tab->blockSignals(true);

		for(i=0; i < count; i++)
		{
			parameters_tab->addRow();
			param = func->getParameter(i);
			showParameterData(parameters_tab, param, i, true);
		}

		parameters_tab->clearSelection();
		parameters_tab->blockSignals(false);

		if(!func->getLibrary().isEmpty())
		{
			symbol_edt->setText(func->getSymbol());
			library_edt->setText(func->getLibrary());
		}
		else
			source_code_txt->setPlainText(func->getFunctionSource());

		for(auto &type : func->getTransformTypes())
		{
			transform_types_tab->addRow();
			transform_types_tab->setCellText(~type, transform_types_tab->getRowCount() - 1, 0);
		}

		for(auto &cfg_param : func->getConfigurationParams())
		{
			config_params_tab->addRow();
			config_params_tab->setCellText(cfg_param.first, config_params_tab->getRowCount() - 1, 0);
			config_params_tab->setCellText(cfg_param.second, config_params_tab->getRowCount() - 1, 1);
		}

		transform_types_tab->clearSelection();
		config_params_tab->clearSelection();
	}
}

void BaseFunctionWidget::selectLanguage()
{
	bool c_lang = (language_cmb->currentText() == DefaultLanguages::C);

	source_code_frm->setVisible(!c_lang);
	library_frm->setVisible(c_lang);

	if(!c_lang)
	{
		try
		{
			source_code_hl->loadConfiguration(GlobalAttributes::getConfigurationFilePath(
																					language_cmb->currentText() +
																					GlobalAttributes::HighlightFileSuffix));
		}
		catch(Exception &)
		{
			source_code_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
		}

		source_code_hl->rehighlight();
		source_code_cp->configureCompletion(this->model, source_code_hl);
	}
}

void BaseFunctionWidget::applyBasicConfiguration(BaseFunction *func)
{
	try
	{
		unsigned count = 0, row = 0;
		Parameter param;
		QString param_modes;

		func->setLanguage(model->getObject(language_cmb->currentText(), ObjectType::Language));
		func->setSecurityType(security_cmb->currentText());
		func->removeParameters();

		count=parameters_tab->getRowCount();

		for(row=0; row < count; row++)
		{
			param.setName(parameters_tab->getCellText(row,0));
			param.setType(parameters_tab->getRowData(row).value<PgSqlType>());

			param_modes = parameters_tab->getCellText(row,2);
			param.setIn(param_modes.indexOf("IN") >= 0);
			param.setOut(param_modes.indexOf("OUT") >= 0);
			param.setVariadic(param_modes.indexOf("VARIADIC") >= 0);
			param.setDefaultValue(parameters_tab->getCellText(row, 3));

			func->addParameter(param);
		}

		func->removeTransformTypes();
		count = transform_types_tab->getRowCount();

		for(row = 0; row < count; row++)
			func->addTransformType(PgSqlType(transform_types_tab->getCellText(row, 0)));

		func->removeConfigurationParams();
		count = config_params_tab->getRowCount();

		for(row = 0; row < count; row++)
			func->setConfigurationParam(config_params_tab->getCellText(row, 0), config_params_tab->getCellText(row, 1));

		if(language_cmb->currentText() == DefaultLanguages::C)
		{
			func->setLibrary(library_edt->text());
			func->setSymbol(symbol_edt->text());
		}
		else
			func->setFunctionSource(source_code_txt->toPlainText().toUtf8());
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
