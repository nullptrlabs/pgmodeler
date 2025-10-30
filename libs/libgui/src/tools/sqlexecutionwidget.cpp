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

#include "sqlexecutionwidget.h"
#include "settings/snippetsconfigwidget.h"
#include "guiutilsns.h"
#include "utils/plaintextitemdelegate.h"
#include "utilsns.h"
#include "csvdocument.h"
#include "messagebox.h"
#include "pgmodelerguiplugin.h"
#include <QClipboard>
#include <QButtonGroup>
#include <qnamespace.h>

std::map<QString, QString> SQLExecutionWidget::cmd_history;
int SQLExecutionWidget::cmd_history_max_len {1000};
const QString SQLExecutionWidget::ColumnNullValue {"␀"};

SQLExecutionWidget::SQLExecutionWidget(QWidget * parent) : QWidget(parent)
{
	setupUi(this);

	output_wgt->setVisible(false);
	plugins_wgts_stw->setVisible(false);
	sql_cmd_splitter->setSizes({800, 200});

	sql_cmd_txt=GuiUtilsNs::createNumberedTextEditor(sql_cmd_wgt);
	sql_cmd_txt->setObjectName("sql_cmd_txt");

	cmd_history_txt=GuiUtilsNs::createNumberedTextEditor(cmd_history_parent);
	cmd_history_txt->setCustomContextMenuEnabled(false);

	cmd_history_txt->setTabStopDistance(sql_cmd_txt->getTabDistance());
	cmd_history_txt->setContextMenuPolicy(Qt::CustomContextMenu);
	cmd_history_txt->setReadOnly(true);
	cmd_history_txt->installEventFilter(this);

	output_tbw->widget(2)->installEventFilter(this);

	search_history_wgt = new SearchReplaceWidget(cmd_history_txt, search_history_parent);
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(search_history_wgt);
	search_history_parent->setLayout(layout);
	search_history_parent->setVisible(false);

	sql_cmd_hl=new SyntaxHighlighter(sql_cmd_txt, false);
	sql_cmd_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	cmd_history_hl=new SyntaxHighlighter(cmd_history_txt, false);
	cmd_history_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	results_parent->setVisible(false);
	output_tbw->setTabEnabled(0, false);
	code_compl_wgt=new CodeCompletionWidget(sql_cmd_txt, true);

	find_replace_wgt=new SearchReplaceWidget(sql_cmd_txt, search_wgt_parent);
	QHBoxLayout *hbox=new QHBoxLayout(search_wgt_parent);
	hbox->setContentsMargins(0,0,0,0);
	hbox->addWidget(find_replace_wgt);
	search_wgt_parent->setVisible(false);

	run_sql_tb->setToolTip(run_sql_tb->toolTip() + QString(" (%1)").arg(run_sql_tb->shortcut().toString()));
	stop_tb->setToolTip(stop_tb->toolTip() + QString(" (%1)").arg(stop_tb->shortcut().toString()));
	file_tb->setToolTip(file_tb->toolTip() + QString(" (%1)").arg(file_tb->shortcut().toString()));
	output_tb->setToolTip(output_tb->toolTip() + QString(" (%1)").arg(output_tb->shortcut().toString()));
	clear_all_tb->setToolTip(clear_all_tb->toolTip() + QString(" (%1)").arg(clear_all_tb->shortcut().toString()));

	for(auto &btn : top_btns_wgt->findChildren<QToolButton *>())
		GuiUtilsNs::configureWidgetFont(btn, GuiUtilsNs::MediumFontFactor);

	results_tbw->setItemDelegate(new PlainTextItemDelegate(this, true));

	action_load = file_menu.addAction(QIcon(GuiUtilsNs::getIconPath("open")), tr("Load"), QKeySequence("Ctrl+L"));
	action_save = file_menu.addAction(QIcon(GuiUtilsNs::getIconPath("save")), tr("Save"), QKeySequence("Ctrl+S"));
	action_save_as = file_menu.addAction(QIcon(GuiUtilsNs::getIconPath("saveas")), tr("Save as"), QKeySequence("Ctrl+Shift+S"));
	file_tb->setMenu(&file_menu);

	action_search = code_menu.addAction(QIcon(GuiUtilsNs::getIconPath("findtext")), tr("Search"), QKeySequence("Ctrl+F"));
	action_search->setCheckable(true);
	action_wrap = code_menu.addAction(QIcon(GuiUtilsNs::getIconPath("wordwrap")), tr("Word wrap"), QKeySequence("Ctrl+W"));
	action_wrap->setCheckable(true);

	QAction *act = snippets_menu.menuAction();
	act->setText(tr("Snippets"));
	act->setIcon(QIcon(GuiUtilsNs::getIconPath("codesnippet")));
	code_menu.addSeparator();
	code_menu.addAction(act);
	code_tb->setMenu(&code_menu);

	action_filter = result_menu.addAction(QIcon(GuiUtilsNs::getIconPath("filter")), tr("Filter"), QKeySequence("Ctrl+T"));
	action_filter->setCheckable(true);

	act = export_menu.menuAction();
	act->setText(tr("Export"));
	act->setIcon(QIcon(GuiUtilsNs::getIconPath("exportdata")));
	result_menu.addAction(act);

	results_tb->setMenu(&result_menu);

	act = export_menu.addAction(tr("Text file"));
	act->setIcon(QIcon(GuiUtilsNs::getIconPath("txtfile")));
	act->setShortcut(QKeySequence("Ctrl+Shift+T"));

	connect(act, &QAction::triggered, this, [this](){
		SQLExecutionWidget::exportResults(results_tbw, false);
	});

	act = export_menu.addAction(tr("CSV file"));
	act->setIcon(QIcon(GuiUtilsNs::getIconPath("csvfile")));
	act->setShortcut(QKeySequence("Ctrl+Shift+C"));

	filter_wgt->setVisible(false);

	connect(act, &QAction::triggered, this, [this](){
		SQLExecutionWidget::exportResults(results_tbw, true);
	});

	connect(columns_cmb, &QComboBox::currentIndexChanged, this, &SQLExecutionWidget::filterResults);
	connect(filter_edt, &QLineEdit::textChanged, this, &SQLExecutionWidget::filterResults);
	connect(hide_tb, &QToolButton::clicked, action_filter, &QAction::trigger);

	connect(action_filter, &QAction::toggled, this, [this](bool checked){
		filter_wgt->setVisible(checked);

		if(checked)
			filter_edt->setFocus();
		else
			sql_cmd_txt->setFocus();
	});


	connect(all_words_tb, &QCheckBox::toggled, this, &SQLExecutionWidget::filterResults);
	connect(regexp_tb, &QCheckBox::toggled, this, &SQLExecutionWidget::filterResults);
	connect(case_sensitive_tb, &QCheckBox::toggled, this, &SQLExecutionWidget::filterResults);

	connect(action_load, &QAction::triggered, this, &SQLExecutionWidget::loadCommands);
	connect(action_save, &QAction::triggered, this, &SQLExecutionWidget::saveCommands);
	connect(action_save_as, &QAction::triggered, this, &SQLExecutionWidget::saveCommands);

	connect(clear_all_tb, &QToolButton::clicked, this, &SQLExecutionWidget::clearAll);

	connect(sql_cmd_txt, &NumberedTextEditor::textChanged, this, [this](){
		run_sql_tb->setEnabled(!sql_cmd_txt->document()->isEmpty());
	});

	connect(run_sql_tb, &QToolButton::clicked, this, qOverload<>(&SQLExecutionWidget::runSQLCommand));
	connect(output_tb, &QToolButton::toggled, this, &SQLExecutionWidget::toggleOutputPane);
	connect(action_wrap,  &QAction::toggled, sql_cmd_txt, &NumberedTextEditor::setWordWrap);

	connect(action_search, &QAction::toggled, search_wgt_parent, &QWidget::setVisible);
	connect(find_replace_wgt, &SearchReplaceWidget::s_hideRequested, action_search, &QAction::toggle);
	connect(search_history_wgt, &SearchReplaceWidget::s_hideRequested, search_history_parent, &QWidget::hide);

	connect(results_tbw, &QTableView::doubleClicked, this, [](const QModelIndex &index){
		if(PlainTextItemDelegate::getMaxDisplayLength() > 0 &&
			 !PlainTextItemDelegate::isTextEditorEnabled() &&
			 index.data().toString().length() > PlainTextItemDelegate::getMaxDisplayLength())
		{
			GuiUtilsNs::openColumnDataForm(index);
		}
	});

	connect(results_tbw, &QTableView::pressed, this, [this](){
		SQLExecutionWidget::copySelection(results_tbw);
	});

	connect(close_file_tb, &QToolButton::clicked, this, [this](){
			if(clearAll() == QDialog::Accepted)
			{
				filename_edt->clear();
				filename_wgt->setVisible(false);
			}
	});

	connect(&snippets_menu, &QMenu::triggered, this, &SQLExecutionWidget::selectSnippet);
	connect(cmd_history_txt, &NumberedTextEditor::customContextMenuRequested, this, &SQLExecutionWidget::showHistoryContextMenu);

	configureSnippets();
	toggleOutputPane(false);
	filename_wgt->setVisible(false);
	v_splitter->handle(1)->installEventFilter(this);

	stop_tb->setVisible(false);
	sql_exec_hlp.moveToThread(&sql_exec_thread);

	connect(&sql_exec_thread, &QThread::started, &sql_exec_hlp, &SQLExecutionHelper::executeCommand);
	connect(&sql_exec_hlp, &SQLExecutionHelper::s_executionFinished, this, &SQLExecutionWidget::finishExecution);
	connect(&sql_exec_hlp, &SQLExecutionHelper::s_executionAborted, &sql_exec_thread, &QThread::quit);
	connect(&sql_exec_hlp, &SQLExecutionHelper::s_executionAborted, this, &SQLExecutionWidget::handleExecutionAborted);
	connect(stop_tb, &QToolButton::clicked, &sql_exec_hlp, &SQLExecutionHelper::cancelCommand, Qt::DirectConnection);

	// After completing the SQLExecutionWidget construction we install the plugins features
	for(auto &p_wgt : PgModelerGuiPlugin::getPluginsWidgets(this))
		installPluginWidgets(p_wgt.button, p_wgt.widget);
}

SQLExecutionWidget::~SQLExecutionWidget()
{
	if(sql_exec_thread.isRunning())
	{
		sql_exec_hlp.cancelCommand();
		sql_exec_thread.quit();
		sql_exec_thread.wait();
	}

	destroyResultModel();
}

bool SQLExecutionWidget::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonDblClick && object == v_splitter->handle(1))
	{
		output_tb->setChecked(!v_splitter->handle(1)->isEnabled());
		return true;
	}

	if(event->type()== QEvent::MouseButtonPress &&
		 dynamic_cast<QMouseEvent *>(event)->button()==Qt::MiddleButton &&
		 object == cmd_history_txt &&
		 cmd_history_txt->textCursor().hasSelection())
	{
		sql_cmd_txt->appendPlainText(cmd_history_txt->textCursor().selectedText());
		return true;
	}

	if(event->type() == QEvent::Show && object == output_tbw->widget(2))
	{
		if(cmd_history_txt->toPlainText().count(QChar('\n')) !=
			 cmd_history[sql_cmd_conn.getConnectionId(true,true)].count(QChar('\n')))
		{
			cmd_history_txt->clear();
			cmd_history_txt->appendPlainText(cmd_history[sql_cmd_conn.getConnectionId(true,true)]);
			cmd_history_txt->updateLineNumbers();
		}

		return true;
	}

	return QWidget::eventFilter(object, event);
}

void SQLExecutionWidget::reloadHighlightConfigs()
{
	try
	{
		sql_cmd_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
		sql_cmd_txt->highlightCurrentLine();
		cmd_history_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void SQLExecutionWidget::installPluginWidgets(QToolButton *btn, QWidget *wgt)
{
	if(!btn)
		return;

	plugins_btns_lt->addWidget(btn);

	/* Forcing the button to have the same features of all other buttons in the
	 * top area when they lie */
	btn->setIconSize(run_sql_tb->iconSize());
	btn->setFont(run_sql_tb->font());
	btn->setSizePolicy(run_sql_tb->sizePolicy());
	btn->setToolButtonStyle(run_sql_tb->toolButtonStyle());
	btn->setAutoRaise(run_sql_tb->autoRaise());
	btn->setParent(this);

	connect(btn, &QToolButton::toggled, this, &SQLExecutionWidget::togglePluginButton);

	int idx = -1;

	if(wgt)
		idx = plugins_wgts_stw->addWidget(wgt);

	btn->setProperty(Attributes::Index.toStdString().c_str(), idx);
}

void SQLExecutionWidget::togglePluginButton(bool checked)
{
	QToolButton *p_btn = qobject_cast<QToolButton *>(sender());

	if(!p_btn->isCheckable())
		return;

	for(auto &btn : findChildren<QToolButton *>())
	{
		if(btn != p_btn &&
			 plugins_btns_lt->indexOf(btn) >= 0 && btn->isChecked())
		{
			btn->blockSignals(true);
			btn->setChecked(false);
			btn->blockSignals(false);
		}
	}

	int wgt_idx = p_btn->property(Attributes::Index.toStdString().c_str()).toInt();

	// Disabling updates prevents flickering when hidding the stacked widgets
	plugins_wgts_stw->setUpdatesEnabled(false);
	plugins_wgts_stw->setVisible(checked && wgt_idx >= 0);
	plugins_wgts_stw->setCurrentIndex(wgt_idx);
	plugins_wgts_stw->setUpdatesEnabled(true);
}

void SQLExecutionWidget::setConnection(Connection conn)
{
	sql_exec_hlp.setConnection(conn);
	sql_cmd_conn = conn;
	db_name_lbl->setText(conn.getConnectionId(true, true, true));
	code_compl_wgt->setConnection(conn);
}

void SQLExecutionWidget::setSQLCommand(const QString &sql)
{
	sql_cmd_txt->clear();
	sql_cmd_txt->setPlainText(sql);
}

bool SQLExecutionWidget::hasSQLCommand()
{
	return !sql_cmd_txt->document()->isEmpty();
}

QString SQLExecutionWidget::getSQLCommand(bool selected)
{
	QString cmd;

	if(selected)
	{
		cmd = sql_cmd_txt->textCursor().selectedText();
		cmd.replace(QChar::ParagraphSeparator, '\n');
	}

	return !cmd.isEmpty() ? cmd : sql_cmd_txt->toPlainText();
}

void SQLExecutionWidget::showEvent(QShowEvent *)
{
	sql_cmd_txt->setFocus();
}

void SQLExecutionWidget::resizeEvent(QResizeEvent *event)
{
	Qt::ToolButtonStyle style = Qt::ToolButtonTextBesideIcon;

	if(event->size().width() < this->baseSize().width())
		style = Qt::ToolButtonIconOnly;

	if(run_sql_tb->toolButtonStyle() != style)
	{
		for(auto &btn : findChildren<QToolButton *>(Qt::FindDirectChildrenOnly))
			btn->setToolButtonStyle(style);
	}
}

void SQLExecutionWidget::fillResultsTable(Catalog &catalog, ResultSet &res, QTableWidget *results_tbw, bool store_data)
{
	if(!results_tbw)
		throw Exception(ErrorCode::OprNotAllocatedObject ,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		int col=0, row=0, col_cnt=res.getColumnCount();
		QTableWidgetItem *item=nullptr;
		std::vector<unsigned> type_ids;
		std::vector<unsigned>::iterator end;
		std::vector<attribs_map> types;
		std::map<unsigned, QString> type_names;
		Catalog::QueryFilter orig_filter=catalog.getQueryFilter();

		results_tbw->setRowCount(0);
		results_tbw->setColumnCount(col_cnt);
		results_tbw->verticalHeader()->setVisible(true);
		results_tbw->setSortingEnabled(false);
		results_tbw->blockSignals(true);
		results_tbw->setUpdatesEnabled(false);

		//Configuring the grid columns with the names of retrived table columns
		for(col=0; col < col_cnt; col++)
		{
			type_ids.push_back(res.getColumnTypeId(col));
			item=new QTableWidgetItem(" " + res.getColumnName(col));
			item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			item->setData(Qt::UserRole, res.getColumnName(col));
			results_tbw->setHorizontalHeaderItem(col, item);
		}

		//Retrieving the data type names for each column
		catalog.setQueryFilter(Catalog::ListAllObjects);
		std::sort(type_ids.begin(), type_ids.end());
		end=std::unique(type_ids.begin(), type_ids.end());
		type_ids.erase(end, type_ids.end());

		types=catalog.getObjectsAttributes(ObjectType::Type, "", "", type_ids);

		for(auto &tp : types)
			type_names[tp[Attributes::Oid].toUInt()]=tp[Attributes::Name];

		catalog.setQueryFilter(orig_filter);

		//Assinging the type names as tooltip on header items
		QString type_name;

		for(col=0; col < col_cnt; col++)
		{
			item=results_tbw->horizontalHeaderItem(col);
			item->setToolTip(type_names[res.getColumnTypeId(col)]);
			type_name = type_names[res.getColumnTypeId(col)];
			item->setData(Qt::ToolTipRole, type_name);
			item->setIcon(QIcon(GuiUtilsNs::getIconPath(ResultSetModel::getPgTypeIconName(type_name))));
		}

		if(res.accessTuple(ResultSet::FirstTuple))
		{
			results_tbw->setRowCount(res.getTupleCount());

			do
			{
				//Fills the current row with the values of current tuple
				for(col=0; col < col_cnt; col++)
				{
					item=new QTableWidgetItem;
					item->setText(res.getColumnValue(col));

					/* When storing column values in the QTableWidget items we need distinguish empty from null values
					 * Since it may affect the generation of SQL like delete when the field value is used somehow (see DataManipulationForm::getDMLCommand) */
					if(store_data)
						item->setData(Qt::UserRole, res.isColumnValueNull(col) ? ColumnNullValue : item->text());

					results_tbw->setItem(row, col, item);
				}

				//Configure the vertical header to show the current tuple id
				results_tbw->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
				row++;
			}
			while(res.accessTuple(ResultSet::NextTuple));
		}

		results_tbw->resizeColumnsToContents();
		results_tbw->resizeRowsToContents();
		results_tbw->setUpdatesEnabled(true);
		results_tbw->blockSignals(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void SQLExecutionWidget::handleExecutionAborted(Exception e)
{
	QString time_str=QString("[%1]:").arg(QTime::currentTime().toString("hh:mm:ss.zzz"));

	switchToExecutionMode(false);
	clearOutput();

	GuiUtilsNs::createOutputListItem(msgoutput_lst,
										UtilsNs::formatMessage(QString("%1 %2").arg(time_str, e.getErrorMessage())),
										QPixmap(GuiUtilsNs::getIconPath("error")));

	if(e.getErrorCode()==ErrorCode::ConnectionTimeout ||
		 e.getErrorCode()==ErrorCode::ConnectionBroken)
	{
		GuiUtilsNs::createOutputListItem(msgoutput_lst,
											QString("%1 %2").arg(time_str, tr("No results retrieved or changes done due to the error above! Run the command again.")),
											QPixmap(GuiUtilsNs::getIconPath("alert")), false);
	}

	output_tbw->setTabText(1, tr("Messages (%1)").arg(msgoutput_lst->count()));

	msgoutput_lst->setVisible(true);
	results_parent->setVisible(false);
	results_tb->setEnabled(false);

	addToSQLHistory(sql_exec_hlp.getCommand(), 0, e.getErrorMessage());
	qApp->alert(this);
}

void SQLExecutionWidget::finishExecution(int rows_affected)
{
	if(sql_exec_hlp.isCancelled())
		destroyResultModel();
	else
	{
		bool empty = false;
		ResultSetModel *res_model = sql_exec_hlp.getResultSetModel();

		end_exec=QDateTime::currentDateTime().toMSecsSinceEpoch();
		total_exec = end_exec - start_exec;

		results_tbw->setSortingEnabled(false);
		results_tbw->blockSignals(true);
		results_tbw->setUpdatesEnabled(false);

		destroyResultModel();

		results_tbw->setModel(res_model);
		results_tbw->resizeColumnsToContents();
		results_tbw->resizeRowsToContents();
		results_tbw->setUpdatesEnabled(true);
		results_tbw->blockSignals(false);

		filter_edt->blockSignals(true);
		filter_edt->clear();
		filter_edt->blockSignals(false);

		columns_cmb->blockSignals(true);
		columns_cmb->clear();

		for(int col = 0; res_model && col < res_model->columnCount(QModelIndex()); col++)
			columns_cmb->addItem(res_model->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString());

		columns_cmb->blockSignals(false);

		addToSQLHistory(sql_exec_hlp.getCommand(), rows_affected);

		empty = (!res_model || res_model->rowCount() == 0);
		output_tbw->setTabEnabled(0, !empty);
		results_parent->setVisible(!empty);
		results_tb->setEnabled(!empty);

		if(!empty)
		{
			output_tbw->setTabText(0, tr("Results (%1)").arg(res_model->rowCount()));
			output_tbw->setCurrentIndex(0);
		}
		else
		{
			output_tbw->setTabText(0, tr("Results"));
			output_tbw->setCurrentIndex(1);
		}

		msgoutput_lst->clear();

		for(QString notice : sql_exec_hlp.getNotices())
		{
			GuiUtilsNs::createOutputListItem(msgoutput_lst,
																					QString("[%1]: %2").arg(QTime::currentTime().toString("hh:mm:ss.zzz")).arg(notice.trimmed()),
																					QPixmap(GuiUtilsNs::getIconPath("alert")), false);
		}

		GuiUtilsNs::createOutputListItem(msgoutput_lst,
																				UtilsNs::formatMessage(tr("[%1]: SQL command successfully executed in <em><strong>%2</strong></em>. <em>%3 <strong>%4</strong></em>")
																																		 .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
																																		 .arg(total_exec >= 1000 ? QString("%1 s").arg(total_exec/1000.0) : QString("%1 ms").arg(total_exec))
																																		 .arg(!res_model ? tr("Rows affected") :  tr("Rows retrieved"))
																																		 .arg(rows_affected)),
																				QPixmap(GuiUtilsNs::getIconPath("info")));

		output_tbw->setTabText(1, tr("Messages (%1)").arg(msgoutput_lst->count()));
	}

	switchToExecutionMode(false);
	sql_exec_thread.quit();
	qApp->alert(this);
}

void SQLExecutionWidget::filterResults()
{
	QModelIndexList list;
	int rows_cnt = results_tbw->model()->rowCount();

	// Filtering using regular expressions
	if(regexp_tb->isChecked())
	{
		QRegularExpression pattern { filter_edt->text() };

		if(!case_sensitive_tb->isChecked())
			pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

		if(all_words_tb->isChecked())
			pattern.setPattern(QString("^%1$").arg(pattern.pattern()));

		list = results_tbw->model()->match(results_tbw->model()->index(0, columns_cmb->currentIndex()),
																			 Qt::DisplayRole, pattern, -1, Qt::MatchRegularExpression);
	}
	// Filtering using normal text matching
	else
	{
		Qt::MatchFlags flags = Qt::MatchContains;

		if(all_words_tb->isChecked())
			flags = Qt::MatchExactly;
		
		if(case_sensitive_tb->isChecked())
			flags |= Qt::MatchCaseSensitive;

		list = results_tbw->model()->match(results_tbw->model()->index(0, columns_cmb->currentIndex()),
																			 Qt::DisplayRole, filter_edt->text(), -1, flags);
	}

	results_tbw->blockSignals(true);
	results_tbw->setUpdatesEnabled(false);

	for(int row = 0; row < rows_cnt; row++)
		results_tbw->hideRow(row);

	if(!list.isEmpty())
	{
		for(auto &idx : list)
			results_tbw->showRow(idx.row());
	}

	results_tbw->blockSignals(false);
	results_tbw->setUpdatesEnabled(true);
	results_tbw->update();
}

void SQLExecutionWidget::addToSQLHistory(const QString &cmd, unsigned rows, const QString &error)
{
	if(!cmd.isEmpty())
	{
		QString fmt_cmd;

		if(!cmd_history_txt->toPlainText().isEmpty())
			fmt_cmd += "\n";

		fmt_cmd += QString("-- %1 [%2] -- \n")
							 .arg(tr("Executed at"))
							 .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
		fmt_cmd += cmd;
		fmt_cmd += QChar('\n');

		if(!error.isEmpty())
		{
			fmt_cmd += QString("-- %1 --\n").arg(tr("Command failed"));
			fmt_cmd += QString("/*\n%1\n*/\n").arg(error);
		}
		else
			fmt_cmd += QString("-- %1 %2\n").arg(tr("Rows:")).arg(rows);

		if(!fmt_cmd.trimmed().endsWith(Attributes::DdlEndToken))
			fmt_cmd += Attributes::DdlEndToken + QChar('\n');

		SQLExecutionWidget::validateSQLHistoryLength(sql_cmd_conn.getConnectionId(true,true), fmt_cmd, cmd_history_txt);
	}
}

void SQLExecutionWidget::validateSQLHistoryLength(const QString &conn_id, const QString &fmt_cmd, NumberedTextEditor *cmd_history_txt)
{
	QString cmds;
	int ln_count = 0;

	cmds = cmd_history[conn_id];
	ln_count = cmds.count(QChar('\n'));
	ln_count += fmt_cmd.count(QChar('\n'));

	if(ln_count > cmd_history_max_len)
	{
		QStringList buffer = cmds.split(QChar('\n'));
		cmds = buffer.mid(buffer.size()/2).join(QChar('\n'));
		cmds = cmds.mid(cmds.indexOf(Attributes::DdlEndToken) + Attributes::DdlEndToken.length());
		cmd_history[conn_id] = cmds.trimmed();

		if(cmd_history_txt)
		{
			cmd_history_txt->clear();
			cmd_history_txt->appendPlainText(cmds);
		}
	}

	cmd_history[conn_id].append(fmt_cmd);

	if(cmd_history_txt)
		cmd_history_txt->appendPlainText(fmt_cmd);
}

void SQLExecutionWidget::switchToExecutionMode(bool value)
{
	run_sql_tb->setVisible(!value);
	stop_tb->setVisible(value);
	file_tb->setEnabled(!value);
	code_tb->setEnabled(!value);
	results_tb->setEnabled(!value);
	output_tb->setEnabled(!value);
	sql_cmd_txt->setEnabled(!value);
	cmd_history_parent->setEnabled(!value);
	search_history_parent->setEnabled(!value);

	if(value)
	{
		this->setCursor(Qt::WaitCursor);
		sql_cmd_txt->setCursor(Qt::WaitCursor);
		sql_cmd_txt->clearFocus();
	}
	else
	{
		this->setCursor(Qt::ArrowCursor);
		sql_cmd_txt->setCursor(Qt::ArrowCursor);
		sql_cmd_txt->setFocus();
	}
}

void SQLExecutionWidget::destroyResultModel()
{
	if(results_tbw->model())
	{
		ResultSetModel *result_model = dynamic_cast<ResultSetModel *>(results_tbw->model());
		results_tbw->blockSignals(true);
		results_tbw->setModel(nullptr);
		delete result_model;
		results_tbw->blockSignals(false);
	}
}

void SQLExecutionWidget::runSQLCommand()
{
	runSQLCommand(getSQLCommand(true));
}

void SQLExecutionWidget::runSQLCommand(const QString &cmd)
{
	output_tb->setChecked(true);
	msgoutput_lst->clear();
	sql_exec_hlp.setCommand(cmd);
	start_exec = QDateTime::currentDateTime().toMSecsSinceEpoch();
	sql_exec_thread.start();

	switchToExecutionMode(true);
	clearOutput();

	GuiUtilsNs::createOutputListItem(msgoutput_lst,
																		tr("[%1]: SQL command is running...")
																				.arg(QTime::currentTime().toString("hh:mm:ss.zzz")),
																		QPixmap(GuiUtilsNs::getIconPath("info")), false);
}

void SQLExecutionWidget::saveCommands()
{
	try
	{
		bool browse_file = (sender() == action_save_as || filename_edt->text().isEmpty());
		QString filename = filename_edt->text();

		if(browse_file)
		{
			QStringList sel_files = GuiUtilsNs::selectFiles(
					tr("Save SQL commands"),
					QFileDialog::AnyFile,	QFileDialog::AcceptSave,
					{ tr("SQL file (*.sql)"),
						tr("All files (*.*)") }, {}, "sql");

			if(!sel_files.isEmpty())
				filename = sel_files.at(0);
		}
		else
			filename = filename_edt->text();

		if(!filename.isEmpty())
		{
			UtilsNs::saveFile(filename, sql_cmd_txt->toPlainText().toUtf8());
			filename_edt->setText(filename);
			filename_wgt->setVisible(true);
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SQLExecutionWidget::loadCommands()
{
	try
	{
		QStringList sel_files = GuiUtilsNs::selectFiles(
				tr("Load SQL commands"),
				QFileDialog::ExistingFile,	QFileDialog::AcceptOpen,
				{ tr("SQL file (*.sql)"),
					tr("All files (*.*)") }, {});

		if(!sel_files.isEmpty())
		{
			sql_cmd_txt->clear();
			sql_cmd_txt->setPlainText(UtilsNs::loadFile(sel_files.at(0)));

			filename_edt->setText(sel_files.at(0));
			filename_wgt->setVisible(true);
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SQLExecutionWidget::exportResults(QTableView *results_tbw, bool csv_format)
{
	if(!results_tbw)
		return;

	try
	{
		QStringList sel_files = GuiUtilsNs::selectFiles(
				tr("Save file"),
				QFileDialog::AnyFile,	QFileDialog::AcceptSave,
				{ csv_format ? tr("CSV file (*.csv)") : tr("Text file (*.txt"),
					tr("All files (*.*)") }, {}, csv_format ? "csv" : "txt");

		if(!sel_files.isEmpty())
		{
			qApp->setOverrideCursor(Qt::WaitCursor);
			results_tbw->setUpdatesEnabled(false);
			results_tbw->blockSignals(true);
			results_tbw->selectAll();

			UtilsNs::saveFile(sel_files.at(0),
												 csv_format ?
														 generateCSVBuffer(results_tbw) :
														 generateTextBuffer(results_tbw));

			results_tbw->clearSelection();
			results_tbw->blockSignals(false);
			results_tbw->setUpdatesEnabled(true);
			qApp->restoreOverrideCursor();
		}
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

int SQLExecutionWidget::clearAll()
{
	int res = Messagebox::confirm(tr("The SQL input field and the results grid will be cleared! Want to proceed?"));

	if(Messagebox::isAccepted(res))
	{
		sql_cmd_txt->setPlainText("");
		output_tb->setChecked(false);

		clearOutput();

		results_tb->setEnabled(results_tbw->model() &&
													 results_tbw->model()->rowCount() > 0);
	}

	return res;
}

void SQLExecutionWidget::clearOutput()
{
	msgoutput_lst->clear();
	output_tbw->setTabText(0, tr("Results"));
	output_tbw->setTabText(1, tr("Messages"));
	output_tbw->setCurrentIndex(1);
	output_tbw->setTabEnabled(0, false);
	destroyResultModel();
}

QByteArray SQLExecutionWidget::generateCSVBuffer(QTableView *results_tbw, bool inc_col_names)
{
	return generateBuffer(results_tbw, CsvDocument::Separator, inc_col_names, true);
}

QByteArray SQLExecutionWidget::generateTextBuffer(QTableView *results_tbw, bool inc_col_names)
{
	return generateBuffer(results_tbw, QChar('\t'), inc_col_names, false);
}

QByteArray SQLExecutionWidget::generateBuffer(QTableView *results_tbw, QChar separator, bool incl_col_names, bool csv_format)
{
	if(!results_tbw)
		throw Exception(ErrorCode::OprNotAllocatedObject ,PGM_FUNC,PGM_FILE,PGM_LINE);

	if((results_tbw->model() && results_tbw->model()->rowCount() == 0) ||
		 !results_tbw->selectionModel())
		return {};

	QAbstractItemModel *model = results_tbw->model();
	QModelIndexList sel_indexes;
	QByteArray buf;
	QStringList line;
	QModelIndex index;
	QString str_pattern = csv_format ?
													QString("%1%2%1").arg(CsvDocument::TextDelimiter).arg("%1") :
													QString("%1"),
			value;
	int start_row = -1, start_col = -1,
			row_cnt = 0, col_cnt = 0;

	sel_indexes = results_tbw->selectionModel()->selectedIndexes();
	start_row = sel_indexes.at(0).row();
	start_col = sel_indexes.at(0).column();
	row_cnt = (sel_indexes.last().row() - start_row) + 1;
	col_cnt = (sel_indexes.last().column() - start_col) + 1;

	int col=0, row=0,
			max_col=start_col + col_cnt,
			max_row=start_row + row_cnt;

	if(incl_col_names)
	{
		//Creating the header
		for(col=start_col; col < max_col; col++)
		{
			if(results_tbw->isColumnHidden(col))
				continue;

			value = model->headerData(col, Qt::Horizontal).toString().trimmed();

			if(csv_format)
				value.replace(CsvDocument::TextDelimiter, QString("%1%1").arg(CsvDocument::TextDelimiter));

			line.append(str_pattern.arg(value));
		}

		buf.append(line.join(separator).toUtf8());
		buf.append(CsvDocument::LineBreak.unicode());
		line.clear();
	}

	//Creating the content
	for(row=start_row; row < max_row; row++)
	{
		for(col=start_col; col < max_col; col++)
		{
			if(results_tbw->isColumnHidden(col))
				continue;

			index = model->index(row, col);
			value = index.data().toString();

			/* If the index value is empty but it has a check state value
			 * we assume the value of the columns as true/false to reflect
			 * the check state of the item */
			if(value.isEmpty() && !index.data(Qt::CheckStateRole).isNull())
				value = index.data(Qt::CheckStateRole) == Qt::Checked ? Attributes::True : Attributes::False;

			if(csv_format)
				value.replace(CsvDocument::TextDelimiter, QString("%1%1").arg(CsvDocument::TextDelimiter));

			line.append(str_pattern.arg(value));
		}

		buf.append(line.join(separator).toUtf8());
		line.clear();
		buf.append(CsvDocument::LineBreak.unicode());
	}

	return buf;
}

void SQLExecutionWidget::copySelection(QTableView *results_tbw, bool use_popup, bool csv_is_default, bool incl_col_names)
{
	if(!results_tbw)
		return;

	try
	{
		QItemSelectionModel *selection = results_tbw->selectionModel();

		if(selection && (!use_popup || (use_popup && QApplication::mouseButtons()==Qt::RightButton)))
		{
			QMenu copy_menu, copy_mode_menu, save_menu;
			QAction *act = nullptr, *act_csv = nullptr, *act_txt = nullptr,
							*act_save = nullptr, *act_save_txt = nullptr, *act_save_csv = nullptr;

			if(use_popup)
			{
				act = copy_mode_menu.menuAction();
				act->setText(tr("Selection"));
				act->setIcon(QIcon(GuiUtilsNs::getIconPath("selection")));

				act_txt = copy_mode_menu.addAction(tr("Copy as text"));
				act_txt->setIcon(QIcon(GuiUtilsNs::getIconPath("txtfile")));

				act_csv = copy_mode_menu.addAction(tr("Copy as CSV"));
				act_csv->setIcon(QIcon(GuiUtilsNs::getIconPath("csvfile")));

				act_save = save_menu.menuAction();
				act_save->setText(tr("Save as..."));
				act_save->setIcon(QIcon(GuiUtilsNs::getIconPath("saveas")));
				copy_mode_menu.addAction(act_save);

				act_save_txt = save_menu.addAction(tr("Text file"));
				act_save_txt->setIcon(QIcon(GuiUtilsNs::getIconPath("txtfile")));

				act_save_csv = save_menu.addAction(tr("CSV file"));
				act_save_csv->setIcon(QIcon(GuiUtilsNs::getIconPath("csvfile")));

				copy_menu.addAction(act);
				act = copy_menu.exec(QCursor::pos());
			}

			if(!use_popup || act)
			{
				QByteArray buffer;

				bool is_csv = ((!use_popup && csv_is_default) ||
											 (use_popup && (act == act_csv || act == act_save_csv))),

						is_save = (use_popup && (act == act_save_txt || act == act_save_csv));

				buffer = is_csv ?
									generateCSVBuffer(results_tbw, incl_col_names) :
									generateTextBuffer(results_tbw, incl_col_names);

				if(!is_save)
					qApp->clipboard()->setText(buffer);
				else
				{
					GuiUtilsNs::selectAndSaveFile(buffer,
																				 tr("Save file"),
																				 QFileDialog::AnyFile,
																				 { is_csv ? tr("CSV file (*.csv)") :tr("Text file (*.txt)"),	tr("All files (*.*)") },
																				 {}, is_csv ? "csv" : "txt");
				}
			}
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SQLExecutionWidget::selectSnippet(QAction *act)
{
	QTextCursor cursor=sql_cmd_txt->textCursor();
	cursor.movePosition(QTextCursor::End);

	sql_cmd_txt->appendPlainText(SnippetsConfigWidget::getParsedSnippet(act->text()));
	sql_cmd_txt->setTextCursor(cursor);
}

void SQLExecutionWidget::toggleOutputPane(bool visible)
{
	// Disabling updates prevents flickering when hidding the output panel
	output_wgt->setUpdatesEnabled(false);
	output_wgt->setVisible(visible);
	output_wgt->setUpdatesEnabled(true);

	if(!visible)
		/* Force the splitter size to be the same as the sql_cmd_wgt maximum height
	   in order to force the splitter to the bottom, hiding the output pane */
		v_splitter->setSizes({sql_cmd_wgt->maximumHeight(), 0});
	else
		//Restore the splitter to the default size
		v_splitter->setSizes({700, 300});
}

void SQLExecutionWidget::configureSnippets()
{ 
	SnippetsConfigWidget::configureSnippetsMenu(&snippets_menu);
	code_compl_wgt->configureCompletion(nullptr, sql_cmd_hl);
}

void SQLExecutionWidget::saveSQLHistory()
{
	try
	{
		SchemaParser schparser;
		attribs_map attribs;
		QString commands;

		for(auto hist : cmd_history)
		{
			attribs[Attributes::Connection] = hist.first;
			attribs[Attributes::Commands] = hist.second;
			schparser.ignoreEmptyAttributes(true);
			commands += schparser.getSourceCode(GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir,
																																														 Attributes::Commands +
																																														 GlobalAttributes::SchemaExt), attribs);
		}

		schparser.loadFile(GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir,
																																			GlobalAttributes::SQLHistoryConf +
																																			GlobalAttributes::SchemaExt));

		attribs.clear();
		attribs[Attributes::Commands] = commands;

		UtilsNs::saveFile(GlobalAttributes::getConfigurationFilePath(GlobalAttributes::SQLHistoryConf),
											schparser.getSourceCode(attribs).toUtf8());
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SQLExecutionWidget::loadSQLHistory()
{
	try
	{
		XmlParser xmlparser;
		attribs_map attribs;

		xmlparser.setDTDFile(GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ObjectDTDDir,
																																				GlobalAttributes::SQLHistoryConf +
																																				GlobalAttributes::ObjectDTDExt),
												 GlobalAttributes::SQLHistoryConf);

		xmlparser.loadXMLFile(GlobalAttributes::getConfigurationFilePath(GlobalAttributes::SQLHistoryConf));

		cmd_history.clear();

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementName() == Attributes::Commands)
				{
					xmlparser.getElementAttributes(attribs);
					xmlparser.savePosition();

					if(xmlparser.accessElement(XmlParser::ChildElement))
						cmd_history[attribs[Attributes::Connection]].append(xmlparser.getElementContent());

					xmlparser.restorePosition();
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SQLExecutionWidget::destroySQLHistory()
{
	int res = Messagebox::confirm(tr("This action will wipe out all the SQL commands history for all connections! Do you really want to proceed?"));

	if(Messagebox::isAccepted(res))
	{
		QFile::remove(GlobalAttributes::getConfigurationFilePath(GlobalAttributes::SQLHistoryConf));
		SQLExecutionWidget::cmd_history.clear();
	}
}

void SQLExecutionWidget::setSQLHistoryMaxLength(int len)
{
	if(len < 1000 || len > 20000)
		len = 1000;

	SQLExecutionWidget::cmd_history_max_len = len;
}

int SQLExecutionWidget::getSQLHistoryMaxLength()
{
	return SQLExecutionWidget::cmd_history_max_len;
}

void SQLExecutionWidget::enableSQLExecution(bool enable)
{
	try
	{
		sql_cmd_txt->setEnabled(enable);
		run_sql_tb->setEnabled(enable && !sql_cmd_txt->toPlainText().isEmpty());
		code_tb->setEnabled(enable);
		search_wgt_parent->setEnabled(enable);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void SQLExecutionWidget::showHistoryContextMenu()
{
	QMenu *ctx_menu=cmd_history_txt->createStandardContextMenu();
	QAction *action_clear = new QAction(QPixmap(GuiUtilsNs::getIconPath("cleartext")), tr("Clear history"), ctx_menu),
			*action_save = new QAction(QPixmap(GuiUtilsNs::getIconPath("save")), tr("Save history"), ctx_menu),
			*action_reload = new QAction(QPixmap(GuiUtilsNs::getIconPath("refresh")), tr("Reload history"), ctx_menu),
			*action_toggle_find = nullptr,
			*exec_act = nullptr;

	if(!search_history_parent->isVisible())
		action_toggle_find = new QAction(QPixmap(GuiUtilsNs::getIconPath("findtext")), tr("Search in history"), ctx_menu);
	else
		action_toggle_find = new QAction(tr("Hide search tool"), ctx_menu);

	ctx_menu->addSeparator();
	ctx_menu->addAction(action_toggle_find);
	ctx_menu->addAction(action_reload);
	ctx_menu->addAction(action_save);
	ctx_menu->addSeparator();
	ctx_menu->addAction(action_clear);

	exec_act = ctx_menu->exec(QCursor::pos());

	if(exec_act == action_clear)
	{
		int res = Messagebox::confirm(tr("This action will wipe out all the SQL commands history for the current connection! Do you really want to proceed?"));

		if(Messagebox::isAccepted(res))
		{
			cmd_history_txt->clear();
			cmd_history[sql_cmd_conn.getConnectionId(true,true)].clear();
		}
	}
	else if(exec_act == action_save)
		SQLExecutionWidget::saveSQLHistory();
	else if(exec_act == action_reload)
	{
		SQLExecutionWidget::loadSQLHistory();
		cmd_history_txt->clear();
		cmd_history_txt->appendPlainText(cmd_history[sql_cmd_conn.getConnectionId(true,true)]);
		cmd_history_hl->rehighlight();
	}
	else if(exec_act == action_toggle_find)
		search_history_parent->setVisible(!search_history_parent->isVisible());

	delete ctx_menu;
}
