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

/**
\ingroup libgui
\class CodeCompletionWidget
\brief Widget that handles the code completion (keywords and model object names) on a field that has
the syntax highlighter installed on it.
*/

#ifndef CODE_COMPLETION_WIDGET_H
#define CODE_COMPLETION_WIDGET_H

#include <QCheckBox>
#include <QListWidget>
#include "utils/syntaxhighlighter.h"
#include "databasemodel.h"
#include "catalog.h"

class __libgui CodeCompletionWidget: public QWidget {
	Q_OBJECT

	private:
		/*! \brief This enums is used to determine DML keywords
		 * positions in the parsed SQL command. This one is used
		 * together with dml_kwords_pos and dml_keywords, and the
		 * order of the items in this enums MUST have the same order
		 * of the elements in dml_keywords string list */
		enum DmlKeywordId: unsigned{
			Select, Insert, Update, Delete,
			Truncate, Alter, Drop,	From,
			Join, Into,	As, Set, Table,
			Only,	Where, Exists, Partition,
			Like, Inherits, On, By,

			Inner, Outer, Left,	Right,
			Full, Union, Intersect,
			Except, Distinct,	Values, All
		};

		ObjectType filter_obj_type;

		int filter_kw_pos;

		/*! \brief Stores the first occurency of the DML keywords in the current typed command.
		 *  This is used to help pgModeler retrieve columns/objects names from the database */
		int dml_kwords_pos[All + 1];

		/*! \brief Stores the extracted table aliases where the key is the alias and the
		 * value the schema-qualified table name */
		attribs_map tab_aliases;

		//! \brief Stores the extracted table names and the position in the command they were found
		std::map<int, QString> tab_names_pos;

		static const QStringList dml_keywords;

		static const QString special_chars;

		//! \brief A timer that controls the completion popup
		QTimer popup_timer;
		
		QWidget *completion_wgt;
		
		//! \brief Marks the completion widget as persistent (available only when there is no a database model assinged)
		QCheckBox *always_on_top_chk;
		
		//! \brief Input field that is associated with the code completion
		QPlainTextEdit *code_field_txt;
		
		//! \brief This widget stores the list of completion items
		QListWidget *name_list;
		
		/*! \brief Stores the keywords retrieved from the syntax highlighter.
		These key words are listed whenever the user call the completion */
		QStringList keywords;
		
		/*! \brief This cursor object is configured to use as a replacer when the user
		selects an item on the completion list */
		QTextCursor new_txt_cur,
		
		//! \brief Stores the current text cursor position on the code field
		prev_txt_cur,
		
		lvl_cur;

		/*! \brief Stores the initial cursor position in the code field when the widget was displayed,
		 *  this is used to determine whether the object names must be retrived again from the database
		 *  or we can reuse the current listing and just filter it (see updateList()) */
		int ini_cur_pos;

		//! \brief Current typed word
		QString word;
		
		/*! \brief Stores the char used to trigger the completion widget. If the completion
		was instantiated using a SyntaxHighlighter object this char is retrieved from this last
		or the default (.) is used */
		QChar completion_trigger;
		
		//! \brief Stores the database model used to search for objects and list them on completion
		DatabaseModel *db_model;
		
		//! \brief Catalog object used to retrieve object names from the database system catalogs
		Catalog catalog;

		/*! \brief This is used to simulate an history of selected object
		whenever the user types the completion trigger char. An example of qualifying is access a column
		of a table by typing the full path to it: public[0].table[1].column[2]. The numbers between brace
		are the qualifying level. */
		int qualifying_level;
		
		//! \brief Indicates if the completion was triggered by typing the completion char
		bool auto_triggered,

		enable_snippets;
		
		//! \brief Store the objects selected for each qualifying level
		std::vector<BaseObject *> sel_objects;
		
		std::map<QString, QPixmap> custom_items;
		
		attribs_map custom_items_tips;

		//! \brief Puts the selected object name on the current cursor position.
		void insertObjectName(BaseObject *obj);
		
		//! \brief Filters the necessary events to trigger the completion as well to control/select items
		bool eventFilter(QObject *object, QEvent *event) override;

		/*! \brief Insert the objects of the vector into the name listing. The filter parameter is used to
		insert only the object which names matches the filter */
		void populateNameList(std::vector<BaseObject *> &objects, QString filter="");
		
		//! \brief Configures the current qualifying level according to the passed object
		void setQualifyingLevel(BaseObject *obj);

		/*! \brief If a connection is configured, populates the list with the columns of
		 *  tables, tables, schemas and functions listed in FROM/JOIN clauses */
		bool updateObjectsList();

		//! \brief Adjusts the position and size of the widget after listing items
		void updateWidgetPosSize();

		//! \brief Reset the DML keywords positions in the current typed code
		void resetKeywordsPos();

		/*! \brief Retrieve the column names from the database based on the current
		 *  typed DML command (SELECT, UPDATE, DELETE) and the position of the cursor */
		bool retrieveColumnNames();

		/*! \brief Retrive the names of tables, views, foreign tables, functions, procedures and aggregates
		 *  depending o the current position of the cursor in the typed DML command */
		bool retrieveObjectNames();

		//! \brief Parses the entire command in order to extract the table names and aliases
		void extractTableNames();

		/*! \brief Returns a list of extracted table names based upon the start_pos (cursor position).
		 *  The stop_pos forces the method to return the list once the position of any searched table
		 *  exceeds the specified value */
		QStringList getTableNames(int start_pos, int stop_pos);

		int getTablePosition(const QString &name);

		QStringList getTableAliases(const QString &name);

		/*! \brief Returns the ObjectType based on the sequential keywords type in the current text cursor position
		 *  This is used by updateObjectsList() when triggering the completion on ALTER/DROP commandas */
		ObjectType identifyObjectType(QTextCursor tc);

		//! \brief Set the provided item as the one selected in the name list
		void setCurrentItem(QListWidgetItem *item);
		
	public:
		CodeCompletionWidget(QPlainTextEdit *code_field_txt, bool enable_snippets = false);
		
		/*! \brief Configures the completion. If an syntax highlighter is specified, the completion widget will
		retrive the keywords and the trigger char from it. The keyword group name can be also specified in case the
		highlighter uses an different configuration */
		void configureCompletion(DatabaseModel *db_model, SyntaxHighlighter *syntax_hl=nullptr, const QString &keywords_grp="keywords");
		
		//! \brief Inserts a custom named item on the list with a custom icon. Custom item will always appear at the beggining of the list
		void insertCustomItem(const QString &name, const QString &tooltip, const QPixmap &icon);
		
		//! \brief Inserts several custom named item on the list with a custom icon. Custom item will always appear at the beggining of the list
		void insertCustomItems(const QStringList &names, const QStringList &tooltips, const QPixmap &icon);

		//! \brief Inserts several custom named items on the list with an icon related to the obj_type. Custom item will always appear at the beggining of the list
		void insertCustomItems(const QStringList &names, const QString &tooltip, ObjectType obj_type);
		
		//! \brief Clear the custom added items
		void clearCustomItems();

		//! \brief Sets the connection params used to retrive column names
		void setConnection(Connection conn);
		
	public slots:
		//! \brief Updates the completion list based upon the typed word
		void updateList();
		
		//! \brief Shows the configured completion list
		void show();
		
		//! \brief Close without select any item on completion list
		void close();
		
		//! \brief Selects an item and closes the completion list
		void selectItem();
		
		void showItemTooltip();

		void adjustNameListSize();
		
	signals:
		//! \brief This signal is emitted whenever a word is placed into the parent textbox through the completion popup.
		void s_wordSelected(QString);

	private slots:
		void handleSelectedWord(QString word);
};

#endif
