/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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

#include <QtTest/QtTest>
#include "libgui/widgets/privcodecompletionwidget.h"
#include "utils/syntaxhighlighter.h"
#include "settings/generalconfigwidget.h"
#include "pgmodelerunittest.h"

/*! \brief Exposes the statement-scoping internals of PrivCodeCompletionWidget (now protected) to the
 *  test, without needing a live database connection or simulated keystrokes to exercise them. */
class TestableCompletion: public PrivCodeCompletionWidget {
	public:
		TestableCompletion(QPlainTextEdit *code_field_txt) : PrivCodeCompletionWidget(code_field_txt) {}

		void computeBounds(int cursor_pos) { computeStatementBounds(cursor_pos); }
		int startPos() { return stmt_start_pos; }
		int endPos() { return stmt_end_pos; }
		int commandId() { return findCommandId(); }
		void extractNames() { extractTableNames(); }
		QStringList aliasesOf(const QString &table_name) { return getTableAliases(table_name); }
		QStringList tableNames() { return getTableNames(stmt_start_pos, stmt_end_pos); }
		bool runUpdateObjectsList() { return updateObjectsList(); }
		ObjectType filterObjType() { return filter_obj_type; }
};

class PrivCodeCompletionTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	private:
		QPlainTextEdit *edit = nullptr;
		SyntaxHighlighter *hl = nullptr;
		TestableCompletion *cc = nullptr;

		/*! \brief Sets the text on the editor, forces a full rehighlight (so TextBlockInfo is populated
		 *  and isCompletionAllowed() reflects comments/strings) and moves the cursor to cursor_pos */
		void setText(const QString &text, int cursor_pos);

		//! \brief Returns the [start, end) statement text at the cursor, without its trailing ';' if any
		QString statementTextAt(int cursor_pos);

	public:
		PrivCodeCompletionTest() : PgModelerUnitTest(SCHEMASDIR) {}

	private slots:
		void initTestCase();
		void init();
		void cleanup();

		void multiStatementBufferScopesEachCommand();
		void cteResolvesToTrailingCommand();
		void cteWithGluedParenthesisResolvesToTrailingCommand();
		void semicolonInsideStringLiteralIsNotASeparator();
		void semicolonInsideLineCommentIsNotASeparator();
		void nextStatementKeywordIsNotMistakenForAnImplicitAlias();
		void commentedOutKeywordDoesNotGovernTheStatement();
		void publicSchemaIsNotMistakenForAKeywordBoundary();
		void trailingDdlStatementDoesNotHijackTheCurrentOne();
};

void PrivCodeCompletionTest::initTestCase()
{
	/* CodeCompletionWidget::configureCompletion() only sets up the completion trigger char and the
	 * keywords list when the "code-completion" general setting is enabled, so the real config must be
	 * loaded first (its shipped default is "true"), otherwise configureCompletion() silently no-ops */
	GeneralConfigWidget general_cfg;
	general_cfg.loadConfiguration();
}

void PrivCodeCompletionTest::init()
{
	edit = new QPlainTextEdit;
	hl = new SyntaxHighlighter(edit, false);
	hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
	cc = new TestableCompletion(edit);
	cc->configureCompletion(nullptr, hl);
}

void PrivCodeCompletionTest::cleanup()
{
	delete cc;
	delete hl;
	delete edit;
	cc = nullptr;
	hl = nullptr;
	edit = nullptr;
}

void PrivCodeCompletionTest::setText(const QString &text, int cursor_pos)
{
	edit->setPlainText(text);
	hl->rehighlight();

	QTextCursor tc = edit->textCursor();
	tc.setPosition(cursor_pos);
	edit->setTextCursor(tc);

	cc->computeBounds(cursor_pos);
}

QString PrivCodeCompletionTest::statementTextAt(int cursor_pos)
{
	Q_UNUSED(cursor_pos)
	return edit->toPlainText().mid(cc->startPos(), cc->endPos() - cc->startPos()).trimmed();
}

void PrivCodeCompletionTest::multiStatementBufferScopesEachCommand()
{
	/* This is the exact scenario reported as broken: completion getting confused by a buffer
	 * with more than one statement, picking up the DML command/table of the wrong one */
	QString text = "SELECT a FROM t1 WHERE id = 1;\n"
								 "UPDATE t2 SET x = 1 WHERE id = 2;\n"
								 "DELETE FROM t3 WHERE id = 3;\n";

	setText(text, text.indexOf("WHERE"));
	QCOMPARE(statementTextAt(0), QString("SELECT a FROM t1 WHERE id = 1"));
	QCOMPARE(cc->commandId(), 0); // Select

	int upd_start = text.indexOf("UPDATE");
	int cursor_pos = text.indexOf("SET", upd_start) + 4; // right after "SET "
	setText(text, cursor_pos);
	QCOMPARE(statementTextAt(0), QString("UPDATE t2 SET x = 1 WHERE id = 2"));
	QCOMPARE(cc->commandId(), 2); // Update

	int del_start = text.indexOf("DELETE");
	setText(text, text.indexOf("WHERE", del_start));
	QCOMPARE(statementTextAt(0), QString("DELETE FROM t3 WHERE id = 3"));
	QCOMPARE(cc->commandId(), 3); // Delete
}

void PrivCodeCompletionTest::cteResolvesToTrailingCommand()
{
	// The command that governs the statement is the trailing SELECT, not the one inside the CTE body
	QString text = "WITH cte AS (SELECT * FROM inner_tab) SELECT * FROM cte WHERE x = 1;\n";
	setText(text, text.indexOf("WHERE"));
	QCOMPARE(cc->commandId(), 0); // Select
}

void PrivCodeCompletionTest::cteWithGluedParenthesisResolvesToTrailingCommand()
{
	// Same as above, but with no spaces around the parenthesis (AS(select...)select...)
	QString text = "WITH cte AS(SELECT * FROM inner_tab)SELECT * FROM cte WHERE x = 1;\n";
	setText(text, text.indexOf("WHERE"));
	QCOMPARE(cc->commandId(), 0); // Select
}

void PrivCodeCompletionTest::semicolonInsideStringLiteralIsNotASeparator()
{
	QString text = "SELECT 'a;b' AS col FROM t1 WHERE id = 1;\n"
								 "UPDATE t2 SET y = 2;\n";

	setText(text, text.indexOf("WHERE"));
	QCOMPARE(statementTextAt(0), QString("SELECT 'a;b' AS col FROM t1 WHERE id = 1"));
	QCOMPARE(cc->commandId(), 0); // Select
}

void PrivCodeCompletionTest::semicolonInsideLineCommentIsNotASeparator()
{
	QString text = "SELECT a FROM t1 -- comment; with a fake separator\nWHERE id = 1;\n"
								 "UPDATE t2 SET y = 2;\n";

	setText(text, text.indexOf("WHERE"));
	QCOMPARE(statementTextAt(0),
					 QString("SELECT a FROM t1 -- comment; with a fake separator\nWHERE id = 1"));
}

void PrivCodeCompletionTest::nextStatementKeywordIsNotMistakenForAnImplicitAlias()
{
	/* Reported bug: a buffer with no ';' between a SELECT and the next command. The blank lines
	 * before "CREATE" made extractTableNames() walk past the table name into the next, unrelated
	 * statement and register "CREATE" as an implicit alias of pds.afastamento */
	QString text = "-- DROP DATABASE IF EXISTS alto;\n"
								 "select  from pds.afastamento\n"
								 "\n"
								 "CREATE DATABASE alto\n";

	setText(text, text.indexOf("from") + 5);
	cc->extractNames();
	QVERIFY(cc->aliasesOf("pds.afastamento").isEmpty());
}

void PrivCodeCompletionTest::commentedOutKeywordDoesNotGovernTheStatement()
{
	/* The leading comment contains "DROP", which is itself a recognized command: findCommandId()
	 * must skip it (it isn't real code) and resolve to the actual SELECT that follows */
	QString text = "-- DROP DATABASE IF EXISTS alto;\n"
								 "select  from pds.afastamento\n";

	setText(text, text.indexOf("from") + 5);
	QCOMPARE(cc->commandId(), 0); // Select
}

void PrivCodeCompletionTest::publicSchemaIsNotMistakenForAKeywordBoundary()
{
	/* Regression: "public" is itself a recognized SQL keyword (it's in the broad keywords list used
	 * by the syntax highlighter), so a naive "is this word a keyword" guard against that list wrongly
	 * aborted the table name extraction right on the schema qualifier of the most common schema there is */
	QString text = "select  from public.arquivo";

	setText(text, text.indexOf("from") + 5);
	cc->extractNames();
	QVERIFY(cc->tableNames().contains("public.arquivo"));
	QVERIFY(cc->aliasesOf("public.arquivo").isEmpty());
}

void PrivCodeCompletionTest::trailingDdlStatementDoesNotHijackTheCurrentOne()
{
	/* Reported bug: a dangling, ';'-less "select * from apd." (the user just typed the trigger dot,
	 * about to complete the schema's table names) followed by a complete, unrelated ALTER SCHEMA
	 * statement. Since nothing closed the SELECT, the ';' based forward search stretched stmt_end_pos
	 * across the ALTER, whose identifyObjectType() then hijacked filter_obj_type to Schema, which in
	 * turn made retrieveObjectNames() refuse to list the schema's tables at all (Schema is a database
	 * child type, so the "list tables" branch is skipped whenever filter_obj_type is already a
	 * database child) */
	QString text = "CREATE SCHEMA reserva;\n"
								 "\n"
								 "select * from apd.\n"
								 "\n"
								 "-- ddl-end --\n"
								 "ALTER SCHEMA reserva OWNER TO postgres;\n";

	setText(text, text.indexOf("apd.") + 4);

	// May throw due to the lack of a live connection in retrieveColumnNames()/retrieveObjectNames();
	// what matters here is that filter_obj_type is never hijacked by the trailing ALTER
	try { cc->runUpdateObjectsList(); } catch(...) {}

	QCOMPARE(cc->filterObjType(), ObjectType::BaseObject);
}

QTEST_MAIN(PrivCodeCompletionTest)
#include "privcodecompletiontest.moc"
