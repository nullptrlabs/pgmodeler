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

#include "customsqlwidget.h"
#include "guiutilsns.h"

CustomSQLWidget::CustomSQLWidget(QWidget *parent) : BaseObjectWidget(parent)
{
	QFont font;

	Ui_CustomSQLWidget::setupUi(this);
	configureFormLayout(sqlappend_grid, ObjectType::BaseObject);

	append_sql_txt=GuiUtilsNs::createNumberedTextEditor(append_sql_wgt, true);
	prepend_sql_txt=GuiUtilsNs::createNumberedTextEditor(prepend_sql_wgt, true);

	append_sql_hl=new SyntaxHighlighter(append_sql_txt);
	append_sql_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
	append_sql_cp=new CodeCompletionWidget(append_sql_txt, true);

	prepend_sql_hl=new SyntaxHighlighter(prepend_sql_txt);
	prepend_sql_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
	prepend_sql_cp=new CodeCompletionWidget(prepend_sql_txt, true);

	name_edt->setReadOnly(true);
	comment_edt->setVisible(false);
	comment_lbl->setVisible(false);

	GuiUtilsNs::configureWidgetFont(message_lbl, GuiUtilsNs::MediumFontFactor);

	action_gen_insert=new QAction(tr("Generic INSERT"), this);
	action_gen_insert->setObjectName("action_gen_insert");
	action_inc_serials=new QAction(tr("Include serial columns"), this);
	action_inc_serials->setObjectName("action_inc_serials");
	action_exc_serials=new QAction(tr("Exclude serial columns"), this);
	action_exc_serials->setObjectName("action_exc_serials");
	action_gen_select=new QAction(tr("Generic SELECT"), this);
	action_gen_select->setObjectName("action_gen_select");
	action_tab_select=new QAction(tr("Table SELECT"), this);
	action_tab_select->setObjectName("action_tab_select");
	action_gen_update=new QAction(tr("Generic UPDATE"), this);
	action_gen_update->setObjectName("action_gen_update");
	action_tab_update=new QAction(tr("Table UPDATE"), this);
	action_tab_update->setObjectName("action_tab_update");
	action_gen_delete=new QAction(tr("Generic DELETE"), this);
	action_gen_delete->setObjectName("action_gen_delete");
	action_tab_delete=new QAction(tr("Table DELETE"), this);
	action_tab_delete->setObjectName("action_tab_delete");

	insert_menu.addAction(action_inc_serials);
	insert_menu.addAction(action_exc_serials);
	insert_menu.addAction(action_gen_insert);
	select_menu.addAction(action_tab_select);
	select_menu.addAction(action_gen_select);
	update_menu.addAction(action_tab_update);
	update_menu.addAction(action_gen_update);
	delete_menu.addAction(action_tab_delete);
	delete_menu.addAction(action_gen_delete);

	connect(clear_btn, &QPushButton::clicked, this, &CustomSQLWidget::clearCode);
	connect(insert_btn, &QPushButton::clicked, this, &CustomSQLWidget::addCommand);
	connect(select_btn, &QPushButton::clicked, this, &CustomSQLWidget::addCommand);
	connect(update_btn, &QPushButton::clicked, this, &CustomSQLWidget::addCommand);
	connect(delete_btn, &QPushButton::clicked, this, &CustomSQLWidget::addCommand);
	connect(action_gen_insert, &QAction::triggered, this, &CustomSQLWidget::addCommand);
	connect(action_inc_serials, &QAction::triggered, this, &CustomSQLWidget::addCommand);
	connect(action_exc_serials, &QAction::triggered, this, &CustomSQLWidget::addCommand);
	connect(action_gen_select, &QAction::triggered, this, &CustomSQLWidget::addCommand);
	connect(action_tab_select, &QAction::triggered, this, &CustomSQLWidget::addCommand);
	connect(action_gen_update, &QAction::triggered, this, &CustomSQLWidget::addCommand);
	connect(action_tab_update, &QAction::triggered, this, &CustomSQLWidget::addCommand);
	connect(action_gen_delete, &QAction::triggered, this, &CustomSQLWidget::addCommand);
	connect(action_tab_delete, &QAction::triggered, this, &CustomSQLWidget::addCommand);

	setMinimumSize(640, 480);
}

void CustomSQLWidget::configureMenus()
{
	ObjectType obj_type = this->object->getObjectType();
	QPushButton *btns[] { insert_btn, select_btn, delete_btn, update_btn };

	for(auto &btn : btns)
		btn->setMenu(nullptr);

	if(BaseTable::isBaseTable(obj_type))
	{
		if(PhysicalTable::isPhysicalTable(obj_type))
		{
			insert_btn->setMenu(&insert_menu);
			delete_btn->setMenu(&delete_menu);
			update_btn->setMenu(&update_menu);
		}

		select_btn->setMenu(&select_menu);
	}
}

void CustomSQLWidget::setAttributes(DatabaseModel *model, BaseObject *object)
{
	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!BaseObject::acceptsCustomSQL(object->getObjectType()))
		throw Exception(ErrorCode::OprObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		BaseObjectWidget::setAttributes(model, object, nullptr);

		name_edt->setText(QString("%1 (%2)").arg(object->getSignature()).arg(object->getTypeName()));

		if(object->getObjectType()==ObjectType::Database)
		{
			end_of_model_chk->setChecked(dynamic_cast<DatabaseModel *>(object)->isAppendAtEOD());
			begin_of_model_chk->setChecked(dynamic_cast<DatabaseModel *>(object)->isPrependedAtBOD());
		}

		append_sql_txt->setFocus();
		append_sql_txt->setPlainText(object->getAppendedSQL());
		append_sql_cp->configureCompletion(model, append_sql_hl);
		append_sql_txt->moveCursor(QTextCursor::End);

		append_sql_txt->setFocus();
		prepend_sql_txt->setPlainText(object->getPrependedSQL());
		prepend_sql_cp->configureCompletion(model, prepend_sql_hl);
		prepend_sql_txt->moveCursor(QTextCursor::End);

		end_of_model_chk->setVisible(object->getObjectType()==ObjectType::Database);
		begin_of_model_chk->setVisible(object->getObjectType()==ObjectType::Database);

		obj_id_lbl->setVisible(false);
		obj_icon_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath(object->getObjectType())));

		configureMenus();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}

}

void CustomSQLWidget::applyConfiguration()
{
	if(this->object->getObjectType()==ObjectType::Database)
	{
		dynamic_cast<DatabaseModel *>(this->object)->setAppendAtEOD(end_of_model_chk->isChecked());
		dynamic_cast<DatabaseModel *>(this->object)->setPrependAtBOD(begin_of_model_chk->isChecked());
	}

	this->object->setAppendedSQL(append_sql_txt->toPlainText());
	this->object->setPrependedSQL(prepend_sql_txt->toPlainText());
	this->sqlcodes_twg->setCurrentIndex(0);

	emit s_closeRequested();
}

void CustomSQLWidget::addCommand()
{
	PhysicalTable *table=dynamic_cast<PhysicalTable *>(this->object);
	BaseTable *base_table=dynamic_cast<BaseTable *>(this->object);
	QString cmd,
			ins_cmd=QString("INSERT INTO %1 (%2) VALUES (%3);"),
			sel_cmd=QString("SELECT * FROM %1;"),
			del_cmd=QString("DELETE * FROM %1;"),
			upd_cmd=QString("UPDATE %1 SET ;");
	QPlainTextEdit *sqlcode_txt=(sqlcodes_twg->currentIndex()==0 ? append_sql_txt : prepend_sql_txt);

	if(sender()->objectName().contains("insert") ||
			sender()->objectName().contains("serial"))
	{
		if(!table || sender()==action_gen_insert)
			cmd=ins_cmd.arg("table").arg("cols").arg("values");
		else if(table)
		{
			bool inc_serials=(sender()==action_inc_serials);
			QString cols, vals;
			unsigned val_id=1;

			for(unsigned i=0; i < table->getColumnCount(); i++)
			{
				if(inc_serials ||
						(!inc_serials && !table->getColumn(i)->getType().isSerialType()))
				{
					cols+=table->getColumn(i)->getName(true) + ',';
					vals+=QString("val%1,").arg(val_id++);
				}
			}

			cols.remove(cols.size()-1, 1);
			vals.remove(vals.size()-1, 1);
			cmd=ins_cmd.arg(table->getName(true)).arg(cols).arg(vals);
		}
	}
	else if(sender()->objectName().contains("select"))
	{
		if(!base_table || sender()==action_gen_select)
			cmd=sel_cmd.arg("object");
		else if(base_table)
			cmd=sel_cmd.arg(base_table->getName(true));
	}
	else if(sender()->objectName().contains("delete"))
	{
		if(!table || sender()==action_gen_delete)
			cmd=del_cmd.arg("object");
		else if(table)
			cmd=del_cmd.arg(table->getName(true));
	}
	else
	{
		if(!table || sender()==action_gen_update)
			cmd=upd_cmd.arg("object");
		else if(table)
			cmd=upd_cmd.arg(table->getName(true));
	}

	if(!sqlcode_txt->toPlainText().isEmpty())
		sqlcode_txt->insertPlainText("\n");

	sqlcode_txt->insertPlainText(cmd);
}

void CustomSQLWidget::clearCode()
{
	QPlainTextEdit *sqlcode_txt=(sqlcodes_twg->currentIndex()==0 ? append_sql_txt : prepend_sql_txt);
	QTextCursor tc=sqlcode_txt->textCursor();
	tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	tc.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
	tc.removeSelectedText();
}
