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

#include "codecompletionwidget.h"
#include "settings/generalconfigwidget.h"
#include "guiutilsns.h"
#include "settings/snippetsconfigwidget.h"
#include "utils/htmlitemdelegate.h"
#include "utils/textblockinfo.h"
#include <QScrollBar>
#include <QToolTip>

const QStringList CodeCompletionWidget::dml_keywords {
	/* ATTENTION: the keywords in this list MUST have a counter part in
	 * DmlKeywordId. Also, the list MUST follow the same item order
	 * in DmlKeywordId.
	 * 		 * Insert here the keywords that need have their position determined
	 * in order to call retriveColumnNames() and retrieveObjectsName().
	 * New keywords here need a new entry in DmlKeywordId enum */
	"select", "insert", "update", "delete",
	"truncate", "alter", "drop", "from",
	"join",	"into", "as", "set", "table",
	"only", "where", "exists", "partition",
	"like", "inherits", "on", "by",

	/* Insert new keywords after this point if their position in the SQL command
	 * is not important but if they are need to do some extra checkings */
	"inner", "outer", "left", "right",
	"full", "union", "intersect",
	"except","distinct", "values",
	"all"
};

const QString CodeCompletionWidget::special_chars {"(),*;=><|:!@^+-/&~#"};

CodeCompletionWidget::CodeCompletionWidget(QPlainTextEdit *code_field_txt, bool enable_snippets) :	QWidget(dynamic_cast<QWidget *>(code_field_txt))
{
	if(!code_field_txt)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	resetKeywordsPos();

	this->enable_snippets = enable_snippets;
	popup_timer.setInterval(300);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	completion_wgt=new QWidget(this);
	completion_wgt->setWindowFlags(Qt::Popup);
	completion_wgt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	completion_wgt->setMaximumHeight(350);
	completion_wgt->setMinimumHeight(50);

	always_on_top_chk=new QCheckBox(completion_wgt);
	always_on_top_chk->setText(tr("&Always on top"));
	always_on_top_chk->setToolTip(tr("<p>The widget will be always displayed while typing. It can be closable only by ESC key or when focus changes to another widget.</p>"));
	always_on_top_chk->setFocusPolicy(Qt::NoFocus);
	always_on_top_chk->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	name_list=new QListWidget(completion_wgt);
	name_list->setSpacing(2);
	name_list->setIconSize(QSize(22, 22));
	name_list->setSortingEnabled(false);
	name_list->setSizeAdjustPolicy(QListWidget::AdjustToContents);
	name_list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	name_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	name_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	name_list->setItemDelegate(new HtmlItemDelegate(name_list, true));

	QVBoxLayout *vbox=new QVBoxLayout(completion_wgt);
	vbox->addWidget(name_list);
	vbox->addWidget(always_on_top_chk);
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);
	vbox->setSpacing(GuiUtilsNs::LtSpacing);
	completion_wgt->setLayout(vbox);

	this->code_field_txt=code_field_txt;
	auto_triggered=false;
	filter_kw_pos = ini_cur_pos = -1;

	db_model=nullptr;
	setQualifyingLevel(nullptr);

	connect(name_list, &QListWidget::itemDoubleClicked, this, &CodeCompletionWidget::selectItem);
	connect(name_list, &QListWidget::currentRowChanged, this, &CodeCompletionWidget::showItemTooltip);
	connect(name_list, &QListWidget::currentRowChanged, this, &CodeCompletionWidget::adjustNameListSize);
	connect(name_list->verticalScrollBar(), &QScrollBar::valueChanged, this, &CodeCompletionWidget::adjustNameListSize);

	connect(&popup_timer, &QTimer::timeout, this, [this](){
		if(qualifying_level < 2)
		{
			auto_triggered=true;
			this->show();
		}
	});

	this->setVisible(false);

	if(enable_snippets)
		connect(this, &CodeCompletionWidget::s_wordSelected, this, &CodeCompletionWidget::handleSelectedWord);
}

void CodeCompletionWidget::handleSelectedWord(QString word)
{
	if(SnippetsConfigWidget::isSnippetExists(word))
	{
		QTextCursor tc=code_field_txt->textCursor();
		tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
		tc.removeSelectedText();
		tc.insertText(SnippetsConfigWidget::getParsedSnippet(word));
	}
}

bool CodeCompletionWidget::eventFilter(QObject *object, QEvent *event)
{
	QKeyEvent *k_event=dynamic_cast<QKeyEvent *>(event);

	if(k_event && k_event->type()==QEvent::KeyPress)
	{
		if(object == code_field_txt)
		{
			TextBlockInfo *blk_info = dynamic_cast<TextBlockInfo *>(code_field_txt->textCursor().block().userData());
			int pos_in_blk = code_field_txt->textCursor().positionInBlock();

			//Filters the trigger char and shows up the code completion only if there is a valid database model in use
			if(k_event->key() == completion_trigger.unicode() && (db_model || catalog.isConnectionValid()) &&
				 (!blk_info || (blk_info && blk_info->isCompletionAllowed(pos_in_blk))))
			{
				/* If the completion widget is not visible start the timer to give the user
				a small delay in order to type another character. If no char is typed the completion is triggered */
				if(!completion_wgt->isVisible() && !popup_timer.isActive())
					popup_timer.start();

				if(name_list->isVisible())
				{
					this->selectItem();
					this->show();
				}
			}
			else
			{
				popup_timer.stop();

				//Filters the Crtl+Space to trigger the code completion
				if(k_event->key() == Qt::Key_Space &&
						(k_event->modifiers() == Qt::ControlModifier ||
						 k_event->modifiers() == Qt::MetaModifier))
				{
					setQualifyingLevel(nullptr);
					this->show();
					return true;
				}

				if(k_event->key() == Qt::Key_Space ||
					 k_event->key() == Qt::Key_Backspace ||
					 k_event->key() == Qt::Key_Delete)
				{
					QTextCursor tc=code_field_txt->textCursor();
					tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

					/* Avoiding deleting text using backspace or delete if the current char is the completion trigger (.).
						 This will block the cursor and cause the list to stay in the current qualifying level */
					if(completion_wgt->isVisible() &&
						 (k_event->key() == Qt::Key_Backspace || k_event->key() == Qt::Key_Delete) &&
						 tc.selectedText().contains(completion_trigger))
					{
						event->ignore();
						return true;
					}

					if(k_event->key() == Qt::Key_Space)
					{
						setQualifyingLevel(nullptr);

						if(!always_on_top_chk->isChecked())
							this->close();
					}

					if(always_on_top_chk->isChecked())
						this->show();
				}
			}
		}
		else if(object == name_list)
		{
			if(k_event->key()==Qt::Key_Escape)
			{
				this->close();
				return true;
			}

			//Filters the ENTER/RETURN press to close the code completion widget select the name
			if(k_event->key() == Qt::Key_Enter || k_event->key() == Qt::Key_Return)
			{
				if(!always_on_top_chk->isChecked())
					this->selectItem();
				else
				{
					//Forcing the line break on the code field when holding Control key and hit return/enter
					if(k_event->modifiers() == Qt::ControlModifier)
					{
						QTextCursor cursor=code_field_txt->textCursor();
						code_field_txt->insertPlainText(QChar(QChar::LineFeed));
						cursor.movePosition(QTextCursor::Down);
						code_field_txt->setTextCursor(cursor);
					}
					else
						this->selectItem();

					this->show();
				}

				return true;
			}

			//Filters other key press and redirects to the code input field
			if(k_event->key() != Qt::Key_Up && k_event->key() != Qt::Key_Down &&
							k_event->key() != Qt::Key_PageUp && k_event->key() != Qt::Key_PageDown &&
							k_event->key() != Qt::Key_Home && k_event->key() != Qt::Key_End &&
							k_event->modifiers() != Qt::AltModifier)
			{
				QCoreApplication::sendEvent(code_field_txt, k_event);
				updateList();
				return true;
			}
		}
	}

	return QWidget::eventFilter(object, event);
}

void CodeCompletionWidget::configureCompletion(DatabaseModel *db_model, SyntaxHighlighter *syntax_hl, const QString &keywords_grp)
{
	name_list->clear();
	word.clear();
	setQualifyingLevel(nullptr);
	resetKeywordsPos();
	auto_triggered=false;
	this->db_model=db_model;

	if(GeneralConfigWidget::getConfigurationParam(Attributes::Configuration,
																								Attributes::CodeCompletion) == Attributes::True)
	{
		code_field_txt->installEventFilter(this);
		name_list->installEventFilter(this);

		if(syntax_hl && keywords.isEmpty())
		{
			//Get the keywords from the highlighter
			QStringList exprs = syntax_hl->getExpressions(keywords_grp);
			QRegularExpression regexp("\\(\\?\\=.*");

			for(auto &expr : exprs)
			{
				/* Since keywords are exact match patterns (see SyntaxHighlighter::loadConfiguration)
				 * we need to remove from the pattern the regexp operators in order to extract only the
				 * work itself. */
				keywords.append(expr.remove("(?<=\\s|\\b)").remove(regexp));
			}

			completion_trigger = syntax_hl->getCompletionTrigger();
		}
		else
			completion_trigger = QChar('.');

		if(enable_snippets)
		{
			clearCustomItems();
			insertCustomItems(SnippetsConfigWidget::getAllSnippetsAttribute(Attributes::Id),
												SnippetsConfigWidget::getAllSnippetsAttribute(Attributes::Label),
												QPixmap(GuiUtilsNs::getIconPath("codesnippet")));
		}
	}
	else
	{
		code_field_txt->removeEventFilter(this);
		name_list->removeEventFilter(this);
	}
}

void CodeCompletionWidget::insertCustomItem(const QString &name, const QString &tooltip, const QPixmap &icon)
{
	if(!name.isEmpty())
	{
		QString item_name=name.simplified();
		custom_items[item_name]=icon;
		custom_items_tips[item_name]=tooltip;
	}
}

void CodeCompletionWidget::insertCustomItems(const QStringList &names, const QStringList &tooltips, const QPixmap &icon)
{
	for(int i=0; i < names.size(); i++)
	{
		insertCustomItem(names[i], (i < tooltips.size() ? tooltips[i] : ""), icon);
	}
}

void CodeCompletionWidget::insertCustomItems(const QStringList &names, const QString &tooltip, ObjectType obj_type)
{
	for(auto &name : names)
		insertCustomItem(name, tooltip, QPixmap(GuiUtilsNs::getIconPath(obj_type)));
}

void CodeCompletionWidget::clearCustomItems()
{
	custom_items.clear();
}

void CodeCompletionWidget::setConnection(Connection conn)
{
	// When setting a connection, we disable the lookup in the database model
	db_model = nullptr;
	catalog.closeConnection();
	catalog.setConnection(conn);
}

void CodeCompletionWidget::populateNameList(std::vector<BaseObject *> &objects, QString filter)
{
	QListWidgetItem *item=nullptr;
	QString obj_name;
	ObjectType obj_type;
	QRegularExpression regexp(QRegularExpression::wildcardToRegularExpression(filter.remove('"') + "*"),
														QRegularExpression::CaseInsensitiveOption);

	name_list->clear();

	for(auto &obj : objects)
	{
		obj_type = obj->getObjectType();
		obj_name.clear();

		//Formatting the object name according to the object type
		if(BaseFunction::isBaseFunction(obj_type))
		{
			dynamic_cast<BaseFunction *>(obj)->createSignature(false);
			obj_name = dynamic_cast<BaseFunction *>(obj)->getSignature();
		}
		else if(obj_type == ObjectType::Operator)
			obj_name = dynamic_cast<Operator *>(obj)->getSignature(false);
		else
			obj_name += obj->getName(false, false);

		//The object will be inserted if its name matches the filter or there is no filter set
		if(filter.isEmpty() || regexp.match(obj_name).hasMatch())
		{
			item= new QListWidgetItem(QPixmap(GuiUtilsNs::getIconPath(obj->getSchemaName())), obj_name);
			item->setToolTip(QString("%1 (%2)").arg(obj->getName(true)).arg(obj->getTypeName()));
			item->setData(Qt::UserRole, QVariant::fromValue<void *>(obj));
			item->setToolTip(BaseObject::getTypeName(obj_type));
			name_list->addItem(item);
		}
	}

	name_list->sortItems();
}

void CodeCompletionWidget::show()
{
	prev_txt_cur = code_field_txt->textCursor();
	ini_cur_pos = prev_txt_cur.position();
	updateList();

	popup_timer.stop();

	if(name_list->count() == 0)
		return;

	completion_wgt->show();

	QTimer::singleShot(500, this, [this](){
		showItemTooltip();
	});
}

void CodeCompletionWidget::setQualifyingLevel(BaseObject *obj)
{
	if(!obj)
		qualifying_level=-1;
	else if(obj->getObjectType()==ObjectType::Schema)
		qualifying_level=0;
	else if(BaseTable::isBaseTable(obj->getObjectType()))
		qualifying_level=1;
	else
		qualifying_level=2;

	if(qualifying_level < 0)
	{
		sel_objects={ nullptr, nullptr, nullptr };
	}
	else
	{
		sel_objects[qualifying_level]=obj;
		lvl_cur=code_field_txt->textCursor();
	}
}

void CodeCompletionWidget::resetKeywordsPos()
{
	for(unsigned id = Select; id <= All; id++)
		dml_kwords_pos[id] = -1;
}

bool CodeCompletionWidget::retrieveColumnNames()
{
	QTextCursor tc = code_field_txt->textCursor(),
			orig_tc = tc;
	int cur_pos = tc.position();
	QStringList tab_names;
	QString curr_word;
	bool found_alias = false, allow_tab_alias = true;

	// If a table alias is being referenced we use the name of the table aliased
	if(word == completion_trigger)
	{
		tc = code_field_txt->textCursor();
		tc.setPosition(tc.position() - 1);
		tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
		curr_word = tc.selectedText();
		curr_word.remove(completion_trigger);
		curr_word = curr_word.trimmed();

		curr_word.removeIf([](const QChar &chr){
			return special_chars.contains(chr);
		});

		if(curr_word.isEmpty() || dml_keywords.contains(curr_word, Qt::CaseInsensitive))
			return false;

		/* If the word is an registered table alias then we will use the related table
		 * to retrieve the columns */
		if(tab_aliases.count(curr_word))
			tab_names.append(tab_aliases[curr_word]);

		found_alias = true;
		curr_word.clear();
	}
	else
	{
		curr_word = word;
		tc = code_field_txt->textCursor();
		tc.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
		tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

		/* In case of the current word is possibly a portion of the name of a column
		 * related to an alias we try to retrieve the previous word which may be
		 * the alias. */
		if(tc.selectedText() == completion_trigger)
		{
			QString alias;
			tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
			alias = tc.selectedText().remove(completion_trigger);

			if(tab_aliases.count(alias))
				tab_names.append(tab_aliases[alias]);
		}

		curr_word.removeIf([](const QChar &chr){
			return special_chars.contains(chr);
		});
	}

	// If no table name was retrieved from aliases names
	if(tab_names.isEmpty() && word != completion_trigger)
	{
		// Retrieving the table name between FROM ... JOIN/WHERE/BY
		if(((dml_kwords_pos[Select] >= 0 && dml_kwords_pos[From] >= 0 &&
				 cur_pos > dml_kwords_pos[Select] && cur_pos < dml_kwords_pos[From]) ||

				(dml_kwords_pos[Select] >= 0 &&
				((dml_kwords_pos[Where] >= 0 && cur_pos > dml_kwords_pos[Where]) ||
				 (dml_kwords_pos[By] >= 0 && cur_pos > dml_kwords_pos[By])))))
		{
			/* We get the table names until the next SELECT to avoid including table names that is
			 * out of the context of the current SELECT/FROM */
			int next_select = code_field_txt->toPlainText().indexOf("select", cur_pos, Qt::CaseInsensitive);
			tab_names = getTableNames(dml_kwords_pos[From], next_select);
		}
		// Retrieving the table name after DELETE FROM ...
		else if((dml_kwords_pos[Delete] >= 0 && dml_kwords_pos[From] >= 0 &&
						 cur_pos > dml_kwords_pos[From]))
		{
			tab_names = getTableNames(dml_kwords_pos[From], dml_kwords_pos[Where]);
		}
		// Retrieving the table name between UPDATE ... SET
		else if((dml_kwords_pos[Update] >= 0 && dml_kwords_pos[Set] >= 0 &&
						 cur_pos > dml_kwords_pos[Set]))
		{
			tab_names = getTableNames(dml_kwords_pos[Update], dml_kwords_pos[Set]);
		}
		// Retrieving the table name between INSERT INTO ...
		else if(dml_kwords_pos[Insert] >= 0 &&	dml_kwords_pos[Into] >= 0)
		{
			// First case: the cursor is between () VALUES
			if(dml_kwords_pos[Values] >= 0 && cur_pos < dml_kwords_pos[Values])
			{
				tab_names = getTableNames(dml_kwords_pos[Into], dml_kwords_pos[Values]);
				allow_tab_alias = false;
			}
			else if(dml_kwords_pos[Values] < 0)
			{
				// Second case: the cursor is after ( but the keyword VALUES is absent
				int open_par = -1, close_par = -1;
				QTextCursor tc = orig_tc;

				tc.setPosition(dml_kwords_pos[Into]);
				code_field_txt->setTextCursor(tc);

				/* If we find the open parenthesis may indicate that the cursor is
				 * in the columns list section of the command */
				if(code_field_txt->find("("))
				{
					QString curr_word;

					open_par = code_field_txt->textCursor().position();
					tc = code_field_txt->textCursor();
					tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);

					/* Now, we have to navigate through the next words and try to find the closing parenthesis
					 * that indicates the end of columns list in the insert command */
					while(!tc.atEnd())
					{
						tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
						curr_word = tc.selectedText();
						tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);

						/* If we find the closing parathesis, or if the columns list is not closed yet
						 * and another opening parenthesis is found or even a DML keyword, we stop to
						 * navigate through the words and assume that the column list end position is
						 * the current cursor position */
						if(curr_word.contains(")") || curr_word.contains("(") || dml_keywords.contains(curr_word))
						{
							close_par = tc.position();
							break;
						}
					}

					code_field_txt->setTextCursor(orig_tc);
				}

				/* If the cursor is betwen the () on the INSERT INTO ... () command
				 * we get the table name that is between the INTO ... ) */
				if(open_par >=0 && close_par >=0 &&
						dml_kwords_pos[Into] < close_par &&
						cur_pos >= open_par && cur_pos <= close_par)
				{
					tab_names = getTableNames(dml_kwords_pos[Into], close_par);

					/* This flag indicates that the table alias must not be prepended to the column names.
					 * This because the columns list in INSERT INTO ... (col_list) does't accept the use
					 * of table aliases */
					allow_tab_alias = false;
				}
			}
		}
	}

	QStringList aux_names, aliases;
	QList<QStringList> split_tab_names;
	QListWidgetItem *item = nullptr;
	attribs_map filter, attribs;
	QString sch_name, tab_name, key, orig_name;
	bool cols_added = false;
	int tab_pos = -1;
	std::map<QString, QListWidgetItem *> items;

	for(auto &name : tab_names)
	{
		aux_names = name.split(completion_trigger);

		/* If the table name is empty or
		 * have extra qualifications (dots) we discard it */
		if(aux_names.isEmpty() || aux_names.size() > 2)
			continue;

		/* If the table name has only the table name without a schema name attached
		 * we create two elements containing { pg_catalog | public, table name, original table name (typed by the user) }
		 * This is used further to retrieve the column names. */
		if(aux_names.size() == 1)
		{
			split_tab_names.append({ "pg_catalog", aux_names[0].trimmed(), name });
			split_tab_names.append({ "public", aux_names[0].trimmed(), name });
		}
		else
			// Otherwise we just create an element in form { schema, table, original table name (typed by the user) }
			split_tab_names.append({ aux_names[0].trimmed(), aux_names[1].trimmed(), name });
	}

	for(auto &names : split_tab_names)
	{
		/* The third element of the split name is the original table name typed by the
		 * user we use it to retrive the table alias if needed */
		orig_name = names[2];

		if(!found_alias && curr_word.isEmpty())
			aliases = getTableAliases(orig_name);

		tab_pos = getTablePosition(orig_name);
		sch_name = names[0];
		tab_name = names[1];

		catalog.setQueryFilter(Catalog::ListAllObjects);

		if(!tab_name.isEmpty())
			filter[Attributes::NameFilter] = QString("^(%1)").arg(curr_word);

		attribs = catalog.getObjectsNames(ObjectType::Column, sch_name, tab_name, filter);

		for(auto &attr : attribs)
		{
			cols_added = true;

			if(aliases.isEmpty())
				aliases.append("");

			for(auto &alias : aliases)
			{
				item = new QListWidgetItem(QIcon(GuiUtilsNs::getIconPath(ObjectType::Column)),
																	 alias.isEmpty() || !allow_tab_alias ?
																	 attr.second : QString("<strong><em>%1</em>.</strong>%2").arg(alias, attr.second));

				item->setData(Qt::UserRole,
											alias.isEmpty() || !allow_tab_alias ?
											BaseObject::formatName(attr.second) :
											QString("%1.%2").arg(BaseObject::formatName(alias),
																					 BaseObject::formatName(attr.second)));

				item->setToolTip(tr("Object: <em>%1</em><br/>Table: %2")
												 .arg(BaseObject::getTypeName(ObjectType::Column),
															QString("<strong>%1</strong>.%2").arg(sch_name, tab_name)));

				key = QString("%1_%2.%3").arg(QString::number(tab_pos).rightJustified(4, '0'),
																			tab_name,	attr.second);
				items[key] = item;
			}
		}
	}

	for(auto &itr : items)
		name_list->addItem(itr.second);

	return cols_added;
}

bool CodeCompletionWidget::retrieveObjectNames()
{
	attribs_map attribs, filter;
	QListWidgetItem *item = nullptr;
	QString curr_word = word, obj_name;
	QTextCursor tc = code_field_txt->textCursor();
	bool retrieved = false;
	ObjectType child_type;

	while(!curr_word.isEmpty())
	{
		tc.movePosition(QTextCursor::EndOfWord, QTextCursor::MoveAnchor);
		tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
		curr_word = tc.selectedText();

		/* We break the name extraction when:
		 * 1) Finding a comma indicating a separation of keywords/identifiers
		 * 2) The current word is a DML keyword
		 * 3) The current word is a keyword registered in the syntax highlighter config file */
		if(curr_word == "," ||
			 dml_keywords.contains(curr_word, Qt::CaseInsensitive) ||
			 (curr_word != "public" && keywords.contains(curr_word, Qt::CaseInsensitive)))
			break;

		curr_word.removeIf([](const QChar &chr){
			return special_chars.contains(chr);
		});

		obj_name.prepend(curr_word);

		/* If we reached the start of the code, we just break to avoid
		 * repeatedly extracting the first word */
		if(tc.atStart())
			break;

		tc.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
	}

	if(obj_name == completion_trigger)
		return false;

	QStringList names = obj_name.split(completion_trigger),
			tab_names;
	QList<ObjectType> obj_types;
	QString sch_name, tab_name, disp_name, fmt_name;

	if(!word.isEmpty())
		tc.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
	else
		tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);

	curr_word = tc.selectedText().trimmed();
	child_type = BaseObject::getObjectType(curr_word, true);
	tc = code_field_txt->textCursor();

	/* If the completion was triggered after a ALTER TABLE ... (ALTER/DROP/...) [COLUMN/CONSTRAINT/TRIGGER/RULE]
	 * We list the children object of the type right before the text cursor */
	if((BaseTable::isBaseTable(filter_obj_type) &&
			(child_type == ObjectType::Column || child_type == ObjectType::Constraint ||
			 child_type == ObjectType::Trigger || child_type == ObjectType::Rule)) ||

		 /* If the completion was triggered after a ALTER/DROP TRIGGER/RULE/POLICY ... ON table
			* We list the children object of these types right before the text cursor but retrieving
			* the table name after the ON keyword */
		 (dml_kwords_pos[On] >= 0 && tc.position() < dml_kwords_pos[On] &&
			(filter_obj_type == ObjectType::Trigger ||
			 filter_obj_type == ObjectType::Rule ||
			 filter_obj_type == ObjectType::Policy)))
	{
		if(BaseTable::isBaseTable(filter_obj_type))
			tab_names = getTableNames(filter_kw_pos, tc.position());
		else
			tab_names = getTableNames(dml_kwords_pos[On], dml_kwords_pos[On] + 1);

		if(!tab_names.isEmpty())
		{
			names = tab_names[0].split(completion_trigger);
			sch_name = !names.isEmpty() ? names[0] : "";
			tab_name = names.size() > 1 ? names[1] : "";
			obj_types.append(child_type);
		}
	}
	// If the name is not schema-qualified
	else if(names.size() == 1)
	{
		/* If no filter object type was identified or the filter type is a
		 * schema's child we prepare to list schema names first */
		if(filter_obj_type == ObjectType::BaseObject ||
			 BaseObject::isChildObjectType(ObjectType::Schema, filter_obj_type))
			obj_types.append(ObjectType::Schema);

		/* If filter type is a database's child we prepare to list all database child objects
		 * of the filter type */
		else if(BaseObject::isChildObjectType(ObjectType::Database, filter_obj_type))
			obj_types.append(filter_obj_type);
	}
	// For schema-qualified names we only list objects are not database's children
	else if(names.size() == 2 &&
					!BaseObject::isChildObjectType(ObjectType::Database, filter_obj_type))
	{
		/* If we have no filter object type, we list by default only the table-like
		 * and function-like objects */
		if(filter_obj_type == ObjectType::BaseObject ||
			 filter_obj_type == ObjectType::Trigger ||
			 filter_obj_type == ObjectType::Rule)
		{
			obj_types.append({ ObjectType::Table,
												 ObjectType::ForeignTable,
												 ObjectType::View,
												 ObjectType::Aggregate,
												 ObjectType::Function,
												 ObjectType::Procedure,
												 ObjectType::Sequence });
		}
		else
			obj_types.append(filter_obj_type);

		sch_name = names[0];
		obj_name = names[1];
	}

	for(auto &obj_type : obj_types)
	{
		catalog.setQueryFilter(Catalog::ListAllObjects);

		if(!obj_name.isEmpty() && obj_name != completion_trigger)
			filter[Attributes::NameFilter] = QString("^(%1)").arg(obj_name);

		attribs = catalog.getObjectsNames(obj_type, sch_name, tab_name, filter);

		for(auto &attr : attribs)
		{
			disp_name = attr.second;

			// Removing parameter names from functions/procedures/aggregates
			if(obj_type == ObjectType::Function ||
				 obj_type == ObjectType::Procedure ||
				 obj_type == ObjectType::Aggregate)
			{
				disp_name.remove(QRegularExpression("(\\()(.*)(\\))"));
				fmt_name = BaseObject::formatName(disp_name) +
									 attr.second.remove(disp_name);
			}
			// Converting user mapping names from role@server to "FOR role SERVER server"
			else if(obj_type == ObjectType::UserMapping)
			{
				names = disp_name.split("@");
				fmt_name = " FOR " + BaseObject::formatName(names[0]) +
									 " SERVER " + BaseObject::formatName(names[1]);
			}
			else
				fmt_name = BaseObject::formatName(attr.second);

			name_list->addItem(disp_name);
			item = name_list->item(name_list->count() - 1);
			item->setIcon(QIcon(GuiUtilsNs::getIconPath(obj_type)));
			item->setData(Qt::UserRole, fmt_name);

			if(obj_type != ObjectType::Schema)
			{
				item->setToolTip(tr("Object: <em>%1</em><br/>Signature: %2")
												 .arg(BaseObject::getTypeName(obj_type),
															QString("<strong>%1</strong>.%2").arg(sch_name, fmt_name)));
			}
			else
				item->setToolTip(tr("Object: <em>%1</em>").arg(BaseObject::getTypeName(obj_type)));

			retrieved = true;
		}
	}

	name_list->sortItems();
	return retrieved;
}

void CodeCompletionWidget::extractTableNames()
{
	QTextCursor tc = code_field_txt->textCursor();
	QString curr_word, tab_name, alias;
	bool extract_alias = false, tab_name_extracted = false, is_special_char = false;
	TextBlockInfo *blk_info = nullptr;
	int pos_in_blk = -1;

	tab_aliases.clear();
	tab_names_pos.clear();
	tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);

	while(!tc.atEnd())
	{
		tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
		curr_word = tc.selectedText();
		curr_word.remove('"');
		blk_info = dynamic_cast<TextBlockInfo *>(tc.block().userData());
		pos_in_blk = tc.positionInBlock();

		tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);

		/* If the current block doesn't allow completion (e.g. comment block)
		 * we just skip the table name/alias extraction */
		if(blk_info && !blk_info->isCompletionAllowed(pos_in_blk))
			continue;

		/* Every time we find a new SELECT keyword we reset name/alias
		 * extraction control variables and wait until a new FROM
		 * is found so a new table name/alias can be extracted */
		if(curr_word.compare("select", Qt::CaseInsensitive) == 0)
		{
			curr_word.clear();
			extract_alias = false;
		}

		if(!curr_word.isEmpty() &&
			 (curr_word.compare("from", Qt::CaseInsensitive) == 0 ||
				curr_word.compare("join", Qt::CaseInsensitive) == 0 ||
				curr_word.compare("into", Qt::CaseInsensitive) == 0 ||
				curr_word.compare("on", Qt::CaseInsensitive) == 0 ||
				curr_word.compare("update", Qt::CaseInsensitive) == 0 ||
				curr_word.compare("table", Qt::CaseInsensitive) == 0 ||
				curr_word.compare("view", Qt::CaseInsensitive) == 0 ||
				(extract_alias && !alias.isEmpty() && curr_word == ",")))
		{
			tc.movePosition(QTextCursor::EndOfWord, QTextCursor::MoveAnchor);
			extract_alias = tab_name_extracted = false;
			tab_name.clear();
			alias.clear();

			while(!tc.atEnd())
			{
				tc.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
				curr_word = tc.selectedText().trimmed();
				curr_word.remove('"');
				tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);

				blk_info = dynamic_cast<TextBlockInfo *>(tc.block().userData());
				pos_in_blk = tc.positionInBlock();

				if(curr_word.isEmpty() ||
					 (blk_info && !blk_info->isCompletionAllowed(pos_in_blk)))
					continue;

				// If we found a DML keyword or special char, we abort the table name/alias extraction
				is_special_char = special_chars.contains(curr_word);

				if(is_special_char ||
					 (curr_word.compare("as", Qt::CaseInsensitive) != 0 &&
						dml_keywords.contains(curr_word, Qt::CaseInsensitive)))
				{
					/* If we found a special character and the table name was extracted but not yet
					 * registered, we force the name registration. This happens for example on
					 * INSERT INTO tablename (), where table name is processed but a ( is found,
					 * so to avoid breaking the name extration without registering a valid name
					 * we forcibly register it before break the routine. */
					if(is_special_char &&
						 !tab_name_extracted && !tab_name.isEmpty() && !tab_name.endsWith(completion_trigger))
						tab_names_pos[tc.position() - tab_name.length()] = tab_name;

					break;
				}

				// If we find an AS keyword after extracting the table name we switch to alias extraction
				if(!extract_alias && !curr_word.isEmpty() &&
					 (curr_word.compare("as", Qt::CaseInsensitive) == 0 || tab_name_extracted))
					extract_alias = true;

				if(!extract_alias && !special_chars.contains(curr_word))
				{
					QTextCursor aux_tc = tc;
					QString next_char;

					/* Moving the cursor to the end of the current word so we can retrieve
					 * the next character right after it. If the character is empty means
					 * that the table name was fully retrieved (with or without schema name) */
					aux_tc.movePosition(QTextCursor::EndOfWord, QTextCursor::MoveAnchor);
					aux_tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					next_char = aux_tc.selectedText().trimmed();

					/* If the tab_name ends with the completion trigger char (.) it means that
					 * we have a schema qualified name but it still lacks the real name of the table.
					 * Thus the current word will be used as the table name, being appended to the current
					 * value of tab_name, creating the full, schema-qualified name of the table. */
					if(tab_name.endsWith(completion_trigger) ||
							(next_char.isEmpty() && curr_word != completion_trigger))
						tab_name_extracted = true;

					tab_name.append(curr_word);

					// Register the table name and position
					if(tab_name_extracted)
						tab_names_pos[tc.position() - tab_name.length()] = tab_name;
				}
				else
				{
					// Register the alias only if it does not exist and a table name is fully specified
					if(extract_alias &&
						 !special_chars.contains(curr_word) && !tab_aliases.count(curr_word) &&
							!tab_name.isEmpty() && curr_word.compare("as", Qt::CaseInsensitive) != 0)
					{
						alias.append(curr_word);
						tab_aliases[alias] = tab_name;
					}

					if(!alias.isEmpty())
						break;
				}
			}
		}
	}
}

QStringList CodeCompletionWidget::getTableNames(int start_pos, int stop_pos)
{
	if(start_pos < 0)
		return {};

	QStringList names;

	for(auto &itr : tab_names_pos)
	{
		if(stop_pos >= 0 && itr.first > stop_pos)
			break;

		if(itr.first >= start_pos)
			names.append(itr.second);
	}

	names.removeDuplicates();
	return names;
}

int CodeCompletionWidget::getTablePosition(const QString &name)
{
	if(name.isEmpty())
		return -1;

	for(auto &itr : tab_names_pos)
	{
		if(itr.second == name)
			return itr.first;
	}

	return -1;
}

QStringList CodeCompletionWidget::getTableAliases(const QString &name)
{
	QStringList aliases;

	for(auto &itr : tab_aliases)
	{
		if(itr.second == name)
			aliases.append(itr.first);
	}

	return aliases;
}

void CodeCompletionWidget::setCurrentItem(QListWidgetItem *item)
{
	if(!item || (item && item->isHidden()))
		QToolTip::hideText();
	else if(item)
	{
		name_list->setCurrentItem(item);
		item->setSelected(true);
	}
}

ObjectType CodeCompletionWidget::identifyObjectType(QTextCursor tc)
{
	static QStringList lv1_words {"user", "foreign", "materialized", "event", "operator"},
										 lv2_words {"family", "class", "data", "table", "view", "mapping", "trigger"};
	QString word, next_word;

	tc.movePosition(QTextCursor::NextWord);
	tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
	word = tc.selectedText();

	if(lv1_words.contains(word, Qt::CaseInsensitive))
	{
		tc.movePosition(QTextCursor::NextWord);
		tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

		next_word = tc.selectedText();

		if(lv2_words.contains(next_word))
		{
			word += " " + next_word;

			if(next_word.endsWith("data", Qt::CaseInsensitive))
			{
				tc.movePosition(QTextCursor::NextWord);
				tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
				word += " " + tc.selectedText();
			}
			else if(next_word.endsWith("view"))
			{
				word.remove("materialized ");
			}
		}
	}

	return BaseObject::getObjectType(word, true);
}

bool CodeCompletionWidget::updateObjectsList()
{
	QTextCursor orig_tc, tc;
	QStringList dml_cmds;
	unsigned kw_id = Select;
	int found_kw_id = -1, pos_in_blk = -1;
	bool cursor_after_kw = false, kw_found = false;
	TextBlockInfo *blk_info = nullptr;
	QTextDocument::FindFlags find_flags[2] = { (QTextDocument::FindWholeWords |
																							QTextDocument::FindBackward),
																						 QTextDocument::FindWholeWords };

	dml_cmds = dml_keywords.mid(Select, 7);
	orig_tc = tc = code_field_txt->textCursor();
	filter_obj_type = ObjectType::BaseObject;
	resetKeywordsPos();

	for(auto &kw : dml_keywords)
	{
		for(auto &flag : find_flags)
		{
			code_field_txt->setTextCursor(tc);

			if(dml_kwords_pos[kw_id] >= 0)
				break;

			kw_found = code_field_txt->find(kw, flag);
			blk_info = dynamic_cast<TextBlockInfo *>(code_field_txt->textCursor().block().userData());
			pos_in_blk = code_field_txt->textCursor().positionInBlock();

			// Avoiding using the position of a found keyword that is in a commented block
			if(kw_found && blk_info && blk_info->isCompletionAllowed(pos_in_blk))
			{
				dml_kwords_pos[kw_id] = code_field_txt->textCursor().position();

				if(found_kw_id < 0 && dml_cmds.contains(kw))
					found_kw_id = kw_id;

				/* Special case for ALTER/DROP: we try to identify which type
				 * of object is being altered/dropped by getting the two previous
				 * words in order */
				if((kw_id == Alter || kw_id == Drop) && filter_obj_type == ObjectType::BaseObject)
				{
					filter_kw_pos = dml_kwords_pos[kw_id];
					filter_obj_type = identifyObjectType(code_field_txt->textCursor());
				}

				if(!cursor_after_kw && orig_tc.position() >= dml_kwords_pos[kw_id])
					cursor_after_kw = true;
			}
			else
				dml_kwords_pos[kw_id] = -1;
		}

		code_field_txt->setTextCursor(tc);
		kw_id++;
	}

	code_field_txt->setTextCursor(orig_tc);

	// If none of the dml command is found, abort the completion
	if(found_kw_id < 0 || orig_tc.position() == 0 || !cursor_after_kw)
		return false;

	try
	{
		bool cols_retrieved = false, objs_retrieved = false;

		name_list->clear();
		extractTableNames();
		cols_retrieved = retrieveColumnNames();

		if(!cols_retrieved)
			objs_retrieved = retrieveObjectNames();

		return cols_retrieved || objs_retrieved;
	}
	catch(Exception &e)
	{
		QTextStream out(stdout);
		out << e.getExceptionsText() << Qt::endl;
		return false;
	}
}

void CodeCompletionWidget::updateWidgetPosSize()
{
	//Sets the list position right below of text cursor
	QPoint pos = code_field_txt->viewport()->mapToGlobal(code_field_txt->cursorRect().bottomLeft());
	QSize screen_sz = completion_wgt->screen()->size();

	// Adjust the position of the widget if it extrapolates the screen limits
	if((pos.x() + completion_wgt->width()) > screen_sz.width())
		pos.setX(pos.x() - completion_wgt->width());

	completion_wgt->move(pos);
	name_list->scrollToTop();
	name_list->setFocus();
	adjustNameListSize();
}

void CodeCompletionWidget::updateList()
{
	QListWidgetItem *item=nullptr;
	QString pattern;
	std::vector<BaseObject *> objects;
	std::vector<ObjectType> types=BaseObject::getObjectTypes(false, 	{ ObjectType::Textbox,
																																			ObjectType::Relationship,
																																			ObjectType::BaseRelationship });
	QTextCursor tc;

	new_txt_cur = tc = code_field_txt->textCursor();

	qApp->setOverrideCursor(Qt::WaitCursor);

	word.clear();

	/* Try to move the cursor to the previous char in order to check if the user is
	calling the completion without an attached word */
	tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

	if(!tc.selectedText().trimmed().isEmpty() && new_txt_cur.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor))
	{
		//Move the cursor right before the trigger char in order to get the complete word
		code_field_txt->setTextCursor(new_txt_cur);
		word=code_field_txt->textCursor().selectedText();
		word.remove('"');

		//Case the completion was triggered using the trigger char
		if(db_model && (auto_triggered || completion_trigger==word))
		{
			/* The completion will try to find a schema, table or view that matches the word,
			if the serach returns one item the completion will start/continue an qualifying level */
			new_txt_cur.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
			code_field_txt->setTextCursor(new_txt_cur);
			word=code_field_txt->textCursor().selectedText();
			word.remove(completion_trigger);
			word.remove('"');

			objects=db_model->findObjects(word, { ObjectType::Schema, ObjectType::Table,
																						ObjectType::ForeignTable, ObjectType::View }, false, false, true);

			if(objects.size()==1)
				setQualifyingLevel(objects[0]);
		}

		code_field_txt->setTextCursor(prev_txt_cur);
	}

	if(!word.isEmpty() && !auto_triggered)
		pattern = "(^" + word.simplified() + ")";
	else if(auto_triggered)
		pattern=word;

	pattern.replace("*", "\\*");

	if(db_model)
	{
		//Negative qualifying level means that user called the completion before a space (empty word)
		if(qualifying_level < 0)
			//The default behavior for this is to search all the objects on the model
			objects=db_model->findObjects(pattern, types, false, !auto_triggered, auto_triggered);
		else
		{
			QString left_word;

			//Searching objects according to qualifying level.
			tc=code_field_txt->textCursor();
			tc.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);

			/* Retrieving the word at the left in order to compare it to the object's name at the current qualifying level,
		 if the word does not matches the object then children objects will not be retrieved */
			if(tc.selectedText().contains('\"'))
			{
				tc.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
				left_word=tc.selectedText();
				left_word.remove('"');
			}
			else
				left_word=tc.selectedText();

			//Level 0 indicates that user selected a schema, so all objects of the schema are retrieved
			if(qualifying_level==0 /*&& left_word==sel_objects[qualifying_level]->getName()*/)
				objects=db_model->getObjects(sel_objects[qualifying_level]);

			/* Level 1 indicates that user selected a table or view, so all child objects are retrieved.
		 If the current level is 1 and the table/view name isn't present then the children will not be listed */
			else if(qualifying_level==1 /*&& left_word==sel_objects[qualifying_level]->getName()*/)
				objects=dynamic_cast<BaseTable *>(sel_objects[qualifying_level])->getObjects();

			/* If the current qualifying level and current word does retrieve any object as a fallback
		 we try to find any object in the model and reset the qualifying level */
			else
			{
				objects=db_model->findObjects(pattern, types, false, !auto_triggered, auto_triggered);
				setQualifyingLevel(nullptr);
			}

			/* If the typed word is equal to the current level object's name clear the order in order
			to avoid listing the same object */
			if(qualifying_level >=0 && word==sel_objects[qualifying_level]->getName())
				word.clear();
		}

		populateNameList(objects, word);
	}

	/* If the current cursor position is after the initial position (when the completion was first shown)
	 * we filter the items already retrieved before, this avoids keep querying the system catalogs
	 * repeatdly to retrieve the same items. */
	if(catalog.isConnectionValid() &&
			ini_cur_pos >= 0 && tc.position() >= ini_cur_pos)
	{
		QList<QListWidgetItem *> list = name_list->findItems(word, Qt::MatchStartsWith);
		QListWidgetItem *first_item = nullptr;

		name_list->setUpdatesEnabled(false);

		for(auto &item : name_list->findItems("*", Qt::MatchWildcard))
			item->setHidden(true);

		for(auto &item : list)
		{
			item->setHidden(false);

			if(!first_item)
				first_item = item;
		}

		name_list->setUpdatesEnabled(true);
		qApp->restoreOverrideCursor();

		setCurrentItem(first_item);
		updateWidgetPosSize();

		return;
	}

	// Retrieving object names from the database if a valid connection is configured
	bool db_objs_retrieved = false;
	name_list->clear();

	if(catalog.isConnectionValid())
		db_objs_retrieved = updateObjectsList();

	/* List the keywords if no object was retrived from databas or
	 * the qualifying level is negative or the completion wasn't triggered
	 * using the special char */
	if(!db_objs_retrieved && qualifying_level < 0 && !auto_triggered)
	{
		QRegularExpression regexp(pattern, QRegularExpression::CaseInsensitiveOption);

		name_list->setUpdatesEnabled(false);

		//If there are custom items, they wiill be placed at the very beggining of the list
		if(!custom_items.empty())
		{
			QStringList list;
			QListWidgetItem *item=nullptr;

			for(auto &itr : custom_items)
			{
				if(itr.first.contains(regexp))
					list.push_back(itr.first);
			}

			list.sort();
			for(auto &item_name : list)
			{
				item=new QListWidgetItem(custom_items[item_name], item_name);
				item->setToolTip(custom_items_tips[item_name]);
				name_list->addItem(item);
			}
		}

		for(auto &kw : keywords.filter(regexp))
		{
			item=new QListWidgetItem(QPixmap(GuiUtilsNs::getIconPath("keyword")), kw);
			item->setToolTip(tr("SQL Keyword"));
			name_list->addItem(item);
		}

		name_list->setUpdatesEnabled(true);
	}

	qApp->restoreOverrideCursor();
	setCurrentItem(name_list->count() == 0 ? nullptr : name_list->item(0));
	updateWidgetPosSize();
}

void CodeCompletionWidget::selectItem()
{
	if(!name_list->selectedItems().isEmpty())
	{
		QListWidgetItem *item=name_list->selectedItems().at(0);

		if(qualifying_level < 0)
			code_field_txt->setTextCursor(new_txt_cur);

		//If the selected item is a object (data not null)
		if(!catalog.isConnectionValid() && !item->data(Qt::UserRole).isNull())
		{
			BaseObject *object=nullptr;
			QTextCursor tc;

			//Try to retrieve the object's reference
			object=reinterpret_cast<BaseObject *>(item->data(Qt::UserRole).value<void *>());

			/* Move the cursor to the start of the word because all the chars will be replaced
			with the object name */
			prev_txt_cur.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);

			tc=prev_txt_cur;
			tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

			/* An small workaround to correctly write the object name in the current
			qualifying level without remove the parent's name. This happens only when
			the completion is marked as persistent */
			if(always_on_top_chk->isChecked())
			{
				if(tc.selectedText().startsWith(completion_trigger))
				{
					prev_txt_cur.movePosition(QTextCursor::EndOfWord, QTextCursor::MoveAnchor);

					if(!tc.selectedText().endsWith(completion_trigger))
						prev_txt_cur.insertText(completion_trigger);
				}
				else if(qualifying_level >= 0 && !tc.selectedText().endsWith(completion_trigger))
				{
					prev_txt_cur.movePosition(QTextCursor::EndOfWord, QTextCursor::MoveAnchor);
					prev_txt_cur.insertText(completion_trigger);
				}
			}
			else if(tc.selectedText().contains('"'))
				prev_txt_cur=tc;

			code_field_txt->setTextCursor(prev_txt_cur);
			insertObjectName(object);
			setQualifyingLevel(object);
		}
		// If we are selecting items from the database catalog
		else if(catalog.isConnectionValid())
		{
			QTextCursor tc = code_field_txt->textCursor();
			QChar last_chr = word.isEmpty() ? QChar::Null : word.at(word.length() - 1);
			QString prefix, item_data = item->data(Qt::UserRole).toString();

			// If the word and doesn't end in a special char is not empty we replace it by the selected item in the list
			if(!word.isEmpty() && word != completion_trigger && !special_chars.contains(last_chr))
				tc.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
			// If it ends with an special char we preserve the cursor position so the selected item text can be insert after
			else if(!word.isEmpty() && special_chars.contains(last_chr))
				tc = prev_txt_cur;
			// If the current word is the completion trigger or a comma we preserve the char
			else if(word == completion_trigger || word == ",")
				prefix = word;

			code_field_txt->setTextCursor(tc);
			code_field_txt->insertPlainText(prefix + (!item_data.isEmpty() ? item_data : item->text()));
		}
		else
		{
			code_field_txt->insertPlainText(item->text() + " ");
			setQualifyingLevel(nullptr);
		}

		emit s_wordSelected(item->text());
	}
	else
		setQualifyingLevel(nullptr);

	name_list->clearSelection();
	auto_triggered=false;

	if(!always_on_top_chk->isChecked())
		this->close();
}

void CodeCompletionWidget::showItemTooltip()
{
	QListWidgetItem *item = name_list->currentItem();

	if(item)
	{
		QToolTip::hideText();
		QPoint pos = name_list->mapToGlobal(QPoint(name_list->width(), name_list->geometry().top()));
		QToolTip::showText(pos, item->toolTip());
	}
}

void CodeCompletionWidget::adjustNameListSize()
{
	int item_cnt = 0;

	/* Determining the number of visible items, this will determine
	 * the maximum completion widget height */
	for(auto &item : name_list->findItems("*", Qt::MatchWildcard))
	{
		if(item->isHidden())
			continue;

		item_cnt++;
	}

	QRect rect = name_list->viewport()->contentsRect(), brect;
	QListWidgetItem *item = nullptr,
			*first_item = name_list->itemAt(rect.topLeft() +
																			QPoint(GuiUtilsNs::LtMargin, GuiUtilsNs::LtMargin)),
			*last_item = name_list->itemAt(rect.bottomLeft() +
																		 QPoint(GuiUtilsNs::LtMargin, -GuiUtilsNs::LtMargin));
	int first_row = name_list->row(first_item),
			last_row = name_list->row(last_item),
			list_w = 0, item_w = 0, vis_item_cnt = 0,
			margin = 2 * GuiUtilsNs::LtMargin;

	QFontMetrics fm(name_list->font());

	if(first_row >= 0 && last_row < 0)
		last_row = name_list->count() - 1;
	// In case the list is empty
	else if(first_row < 0 && last_row < 0)
	{
		name_list->setFixedHeight(completion_wgt->minimumHeight() + margin);
		completion_wgt->adjustSize();
		adjustSize();
		return;
	}

	// Determining the maximum width of the visible items
	for(int row = first_row; row <= last_row; row++)
	{
		item = name_list->item(row);

		if(!item || item->isHidden())
			continue;

		vis_item_cnt++;

		brect = fm.boundingRect(item->text().
														remove(HtmlItemDelegate::TagRegExp));

		item_w = brect.width() +
						 name_list->iconSize().width() + margin +
						 name_list->verticalScrollBar()->width();

		if(item_w > list_w)
			list_w = item_w;
	}

	name_list->setFixedWidth(list_w < always_on_top_chk->width() ?
													 always_on_top_chk->width() : list_w);

	int item_h = 0,
			base_h = name_list->iconSize().height() + GuiUtilsNs::LtMargin;

	item_h = base_h * item_cnt;
	item_h += margin;

	if(item_h < completion_wgt->minimumHeight())
		item_h = completion_wgt->minimumHeight() + margin;
	else if(item_h > completion_wgt->maximumHeight())
	{
		item_h = completion_wgt->maximumHeight() -
						 always_on_top_chk->height() - (2 * margin);
	}

	if(vis_item_cnt <= 10)
		name_list->setFixedHeight(item_h);

	completion_wgt->adjustSize();
	adjustSize();
}

void CodeCompletionWidget::close()
{
	name_list->clearSelection();
	ini_cur_pos = -1;
	completion_wgt->close();
	auto_triggered=false;
	QToolTip::hideText();
}

void CodeCompletionWidget::insertObjectName(BaseObject *obj)
{
	bool sch_qualified=!sel_objects[0],
			modify_name=QApplication::keyboardModifiers()==Qt::AltModifier;
	QString name=obj->getName(true, sch_qualified);
	ObjectType obj_type=obj->getObjectType();
	int move_cnt=0;

	if(modify_name &&
			(PhysicalTable::isPhysicalTable(obj_type) || TableObject::isTableObject(obj_type)))
	{
		if(PhysicalTable::isPhysicalTable(obj_type))
		{
			PhysicalTable *tab=dynamic_cast<PhysicalTable *>(obj);

			name+="(";
			for(unsigned i=0; i < tab->getColumnCount(); i++)
				name+=tab->getColumn(i)->getName(true) + ",";

			name.remove(name.size()-1, 1);
			name+=")";
		}
		else
		{
			if(sel_objects[0])
				move_cnt=2;
			else
				move_cnt=3;

			lvl_cur.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor, move_cnt);
			code_field_txt->setTextCursor(lvl_cur);
		}
	}
	else if(BaseFunction::isBaseFunction(obj_type))
	{
		BaseFunction *func = dynamic_cast<BaseFunction *>(obj);
		func->createSignature(true, sch_qualified);
		name = func->getSignature();
	}
	else if(obj_type==ObjectType::Cast)
	{
		name.replace(',', " AS ");
	}
	else if(obj_type==ObjectType::Aggregate)
	{
		Aggregate *agg;
		agg=dynamic_cast<Aggregate *>(obj);
		name+="(";

		if(agg->getDataTypeCount()==0)
			name+='*';
		else
		{
			for(unsigned i=0; i < agg->getDataTypeCount(); i++)
				name+=~agg->getDataType(i) + ',';
			name.remove(name.size()-1, 1);
		}

		name+=')';
	}

	code_field_txt->insertPlainText(name);
}
