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

#include "numberedtexteditor.h"
#include <QTextBlock>
#include <QScrollBar>
#include <QFileDialog>
#include <QTemporaryFile>
#include "customuistyle.h"
#include "guiutilsns.h"
#include <QMenu>
#include <QHBoxLayout>
#include <QApplication>
#include <QClipboard>
#include "messagebox.h"
#include "utilsns.h"

bool NumberedTextEditor::line_nums_visible {true};
bool NumberedTextEditor::highlight_lines {true};

QColor NumberedTextEditor::line_hl_color { Qt::yellow };
QFont NumberedTextEditor::default_font {"Source Code Pro", 12};

double NumberedTextEditor::tab_width {0};

QString NumberedTextEditor::src_editor_app;
QString NumberedTextEditor::src_editor_app_args;

NumberedTextEditor::NumberedTextEditor(QWidget * parent, bool act_btns_enabled, qreal custom_fnt_size) : QPlainTextEdit(parent)
{
	file_filters = { tr("SQL file (*.sql)"),	tr("All files (*.*)") };
	default_ext = "sql";
	action_btns_enabled = act_btns_enabled;
	show_act_btns =	false;
	show_line_nums = line_nums_visible;

	line_numbers_wgt = new LineNumbersWidget(this);
	line_numbers_wgt->setObjectName("line_numbers_wgt");
	line_numbers_wgt->setAutoFillBackground(true);

	top_widget = nullptr;
	search_wgt = nullptr;
	load_file_btn = edit_src_btn = clear_btn = nullptr;
	save_file_btn = word_wrap_btn = search_btn = nullptr;
	this->custom_fnt_size = custom_fnt_size;

	if(act_btns_enabled)
	{
		//QPalette pal = this->palette();
		QFont font = this->font();

		show_act_btns = true;
		font.setPointSizeF(font.pointSizeF() * 0.90);

		QVBoxLayout *top_wgt_lt = new QVBoxLayout;
		top_wgt_lt->setContentsMargins(GuiUtilsNs::LtMargins);
		top_widget = new QWidget(this);
		top_widget->setObjectName("top_widget");
		top_widget->setAutoFillBackground(true);
		top_widget->setLayout(top_wgt_lt);

		//pal.setColor(QPalette::Window, LineNumbersWidget::getBackgroundColor());
		//top_widget->setPalette(pal);
		top_widget->setVisible(act_btns_enabled);
		top_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

		search_wgt = new SearchReplaceWidget(this, this);
		search_wgt->setObjectName("search_wgt");
		search_wgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		search_wgt->setVisible(false);
		search_wgt->layout()->setContentsMargins(GuiUtilsNs::LtMargins);

		QHBoxLayout *buttons_lt = new QHBoxLayout;
		buttons_lt->setContentsMargins(0, 0, 0, 0);

		top_wgt_lt->addLayout(buttons_lt);

		QLabel *ico = new QLabel(top_widget);
		msg_lbl = new QLabel(top_widget);
		msg_lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);

		ico->setPixmap(QPixmap(GuiUtilsNs::getIconPath("alert")));
		ico->setScaledContents(true);

		editor_alert_wgt = new QWidget(top_widget);
		editor_alert_wgt->setFont(font);

		QHBoxLayout *alert_lt = new QHBoxLayout;
		alert_lt->setContentsMargins(0,0,0,0);
		alert_lt->addWidget(ico);
		alert_lt->addWidget(msg_lbl);
		editor_alert_wgt->setVisible(false);
		editor_alert_wgt->setLayout(alert_lt);

		buttons_lt->addWidget(editor_alert_wgt);
		buttons_lt->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

		load_file_btn = new QToolButton(top_widget);
		load_file_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("open")));
		load_file_btn->setAutoRaise(true);
		load_file_btn->setText(tr("Load"));
		load_file_btn->setShortcut(QKeySequence("Ctrl+L"));
		load_file_btn->setToolTip(tr("Load text from an external file (%1)")
															.arg(load_file_btn->shortcut().toString()));
		load_file_btn->setFont(font);
		load_file_btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		buttons_lt->addWidget(load_file_btn);

		save_file_btn = new QToolButton(top_widget);
		save_file_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("save")));
		save_file_btn->setAutoRaise(true);
		save_file_btn->setText(tr("Save"));
		save_file_btn->setShortcut(QKeySequence("Ctrl+S"));
		save_file_btn->setToolTip(tr("Save the text to a file (%1)")
															.arg(load_file_btn->shortcut().toString()));
		save_file_btn->setFont(font);
		save_file_btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		buttons_lt->addWidget(save_file_btn);

		copy_btn = new QToolButton(top_widget);
		copy_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("copy")));
		copy_btn->setAutoRaise(true);
		copy_btn->setDisabled(true);
		copy_btn->setText(tr("Copy"));
		copy_btn->setShortcut(QKeySequence("Ctrl+C"));
		copy_btn->setToolTip(tr("Copy the whole text to the clipboard (%1)")
												 .arg(copy_btn->shortcut().toString()));
		copy_btn->setFont(font);
		copy_btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		buttons_lt->addWidget(copy_btn);

		edit_src_btn = new QToolButton(top_widget);
		edit_src_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("edit")));
		edit_src_btn->setAutoRaise(true);
		edit_src_btn->setText(tr("Edit"));
		edit_src_btn->setShortcut(QKeySequence("Ctrl+E"));
		edit_src_btn->setToolTip(tr("Edit the text in the defined external editor (%1)")
														 .arg(edit_src_btn->shortcut().toString()));
		edit_src_btn->setFont(font);
		edit_src_btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		buttons_lt->addWidget(edit_src_btn);

		search_btn = new QToolButton(top_widget);
		search_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("findtext")));
		search_btn->setAutoRaise(true);
		search_btn->setCheckable(true);
		search_btn->setText(tr("Search"));
		search_btn->setShortcut(QKeySequence("Ctrl+F"));
		search_btn->setToolTip(tr("Search in the text field (%1)")
													 .arg(search_btn->shortcut().toString()));
		search_btn->setFont(font);
		search_btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		buttons_lt->addWidget(search_btn);

		word_wrap_btn = new QToolButton(top_widget);
		word_wrap_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("wordwrap")));
		word_wrap_btn->setAutoRaise(true);
		word_wrap_btn->setCheckable(true);
		word_wrap_btn->setText(tr("Wrap"));
		word_wrap_btn->setShortcut(QKeySequence("Ctrl+W"));
		word_wrap_btn->setToolTip(tr("Toggles the word wrap (%1)")
															.arg(word_wrap_btn->shortcut().toString()));
		word_wrap_btn->setFont(font);
		word_wrap_btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		word_wrap_btn->setDisabled(true);
		buttons_lt->addWidget(word_wrap_btn);

		clear_btn = new QToolButton(top_widget);
		clear_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("cleartext")));
		clear_btn->setAutoRaise(true);
		clear_btn->setText(tr("Clear"));
		clear_btn->setToolTip(tr("Clears the entire text"));
		clear_btn->setFont(font);
		clear_btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		clear_btn->setDisabled(true);

		buttons_lt->addWidget(clear_btn);
		ico->setMaximumSize(edit_src_btn->iconSize());
		top_widget->adjustSize();

		connect(load_file_btn, &QToolButton::clicked, this, &NumberedTextEditor::loadFile);
		connect(save_file_btn, &QToolButton::clicked, this, &NumberedTextEditor::saveFile);
		connect(edit_src_btn,  &QToolButton::clicked, this, __slot(this, NumberedTextEditor::editSource));

		connect(copy_btn,  &QToolButton::clicked, this, [this](){
			qApp->clipboard()->setText(toPlainText());
		});

		connect(search_btn, &QToolButton::toggled, this, [this](bool checked){
			search_wgt->setVisible(checked);
			search_wgt->raise();
			resizeWidgets();
		});

		connect(search_wgt, &SearchReplaceWidget::s_hideRequested, search_btn, &QToolButton::toggle);
		connect(word_wrap_btn,  &QToolButton::toggled, this, &NumberedTextEditor::setWordWrap);

		connect(clear_btn, &QToolButton::clicked, this, [this](){
			this->clear();
			clear_btn->setEnabled(false);
		});

		connect(this, &NumberedTextEditor::textChanged, this, [this](){
			clear_btn->setEnabled(!this->document()->isEmpty() && !this->isReadOnly());
			search_btn->setEnabled(!this->document()->isEmpty());
			copy_btn->setEnabled(!this->document()->isEmpty());
			word_wrap_btn->setEnabled(!document()->isEmpty());
			save_file_btn->setEnabled(!document()->isEmpty());
		});

		connect(&src_editor_proc, &QProcess::started, this, &NumberedTextEditor::handleProcessStart);
		connect(&src_editor_proc, &QProcess::errorOccurred, this, &NumberedTextEditor::handleProcessError);
		connect(&src_editor_proc, &QProcess::finished, this, __slot_n(this, NumberedTextEditor::updateSource));
	}

	line_numbers_wgt->raise();
	this->setFont(default_font);

	setWordWrapMode(QTextOption::NoWrap);

	connect(this, &NumberedTextEditor::cursorPositionChanged, this, &NumberedTextEditor::highlightCurrentLine);
	connect(this, &NumberedTextEditor::updateRequest, this, &NumberedTextEditor::updateLineNumbers);

	setCustomContextMenuEnabled(true);
}

NumberedTextEditor::~NumberedTextEditor()
{
	if(src_editor_proc.state() != QProcess::NotRunning)
	{
		disconnect(&src_editor_proc, nullptr, this, nullptr);
		src_editor_proc.terminate();
		src_editor_proc.waitForFinished();
		QFile(tmp_src_file).remove();
	}
}

void NumberedTextEditor::setCustomContextMenuEnabled(bool enabled)
{
	if(!enabled)
	{
		setContextMenuPolicy(Qt::NoContextMenu);
		disconnect(this, &NumberedTextEditor::customContextMenuRequested, this, &NumberedTextEditor::showContextMenu);
	}
	else
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, &NumberedTextEditor::customContextMenuRequested, this, &NumberedTextEditor::showContextMenu, Qt::UniqueConnection);
	}
}

void NumberedTextEditor::setFilenameFilters(const QStringList &list, const QString &def_ext)
{
	file_filters = list;
	default_ext = def_ext;
}

void NumberedTextEditor::setDefaultFont(const QFont &font)
{
	default_font=font;
}

void NumberedTextEditor::setLineNumbersVisible(bool value)
{
	line_nums_visible=value;
}

void NumberedTextEditor::setHighlightLines(bool value)
{
	highlight_lines=value;
}

bool NumberedTextEditor::isHighlightLines()
{
	return highlight_lines;
}

void NumberedTextEditor::setLineHighlightColor(const QColor &color)
{
	line_hl_color=color;
}

QColor NumberedTextEditor::getLineHighlightColor()
{
	return line_hl_color;
}

void NumberedTextEditor::setTabDistance(double value)
{
	if(value < 0)
		tab_width=0;
	else
		tab_width=value;
}

double NumberedTextEditor::getTabDistance()
{
	if(static_cast<int>(tab_width) == 0)
		return 80;

	return tab_width * QFontMetrics(default_font).horizontalAdvance(' ');
}

void NumberedTextEditor::setSourceEditorApp(const QString &app)
{
	NumberedTextEditor::src_editor_app = app;
}

void NumberedTextEditor::setSourceEditorAppArgs(const QString &args)
{
	NumberedTextEditor::src_editor_app_args = args;
}

void NumberedTextEditor::showContextMenu()
{
	QMenu *ctx_menu;
	QAction *act=nullptr;

	ctx_menu=createStandardContextMenu();

	if(!isReadOnly())
	{
		ctx_menu->addSeparator();

		act = ctx_menu->addAction(tr("Paste code"), QKeySequence("Ctrl+Shift+V"), this, &NumberedTextEditor::pasteCode);
		act->setEnabled(!qApp->clipboard()->text().isEmpty());

		act = ctx_menu->addAction(tr("Upper case"), QKeySequence("Ctrl+U"), this, &NumberedTextEditor::changeSelectionToUpper);
		act->setEnabled(textCursor().hasSelection());

		act = ctx_menu->addAction(tr("Lower case"), QKeySequence("Ctrl+Shift+U"), this, &NumberedTextEditor::changeSelectionToLower);
		act->setEnabled(textCursor().hasSelection());

		ctx_menu->addSeparator();

		act = ctx_menu->addAction(tr("Ident right"), QKeySequence("Tab"), this, &NumberedTextEditor::identSelectionRight);
		act->setEnabled(textCursor().hasSelection());

		act = ctx_menu->addAction(tr("Ident left"), QKeySequence("Shift+Tab"), this, &NumberedTextEditor::identSelectionLeft);
		act->setEnabled(textCursor().hasSelection());
	}

	ctx_menu->exec(QCursor::pos());
	delete ctx_menu;
}

void NumberedTextEditor::pasteCode()
{
	QString code = qApp->clipboard()->text();
	QStringList buffer = code.split(QChar::LineFeed),
		patterns = {
			"^(\\s)*((\\%2)?(\\s)*)*(%1)",
			"(%1)((\\s)*(\\%2)?)*$",
			"(%1)(\\s)*(\\%2)(\\s)*(%1)",
			"(%1)(\\s)*(\\%2)",
			"(\\%2)(\\s)*(%1)"
		},
			opers = { "\"+", "'.", "'+", "\".", "\";", "\';" };
	QChar concat_opr = '+', str_delim = '"';
	QRegularExpression regexp;

	for(auto &line : buffer)
	{
		/* Detecting the combination of string delimiter and concatenation operator
		 * present in the current buffer line */
		for(auto &opr : opers)
		{
			str_delim = opr[0];
			concat_opr = opr[1];

			if(line.contains(QRegularExpression(QString("^(\\s)*(\\%1|\\%2)+(\\s)*").arg(str_delim, concat_opr))) &&
				 line.contains(QRegularExpression(QString("(\\s)*(\\%1|\\%2)+(\\s)*$").arg(str_delim, concat_opr))))
				break;
		}

		/* Cleaning up the line using the patterns configured with the
		 * determined string delimiter and concatation operator */
		for(auto &pattern : patterns)
		{
			regexp.setPattern(pattern.arg(str_delim, concat_opr));
			line.remove(regexp);
		}
	}

	insertPlainText(buffer.join(QChar::LineFeed));
}

void NumberedTextEditor::changeSelectionToLower()
{
	changeSelectionCase(true);
}

void NumberedTextEditor::changeSelectionToUpper()
{
	changeSelectionCase(false);
}

void NumberedTextEditor::changeSelectionCase(bool lower)
{
	QTextCursor cursor=textCursor();

	if(cursor.hasSelection())
	{
		int start=cursor.selectionStart(),
				end=cursor.selectionEnd();

		if(!lower)
			cursor.insertText(cursor.selectedText().toUpper());
		else
			cursor.insertText(cursor.selectedText().toLower());

		cursor.setPosition(start);
		cursor.setPosition(end, QTextCursor::KeepAnchor);
		setTextCursor(cursor);
	}
}

void NumberedTextEditor::identSelectionRight()
{
	identSelection(true);
}

void NumberedTextEditor::identSelectionLeft()
{
	identSelection(false);
}

void NumberedTextEditor::identSelection(bool ident_right)
{
	QTextCursor cursor=textCursor();

	if(cursor.hasSelection())
	{
		QStringList lines;
		int start=-1,	end=-1,
				factor=(ident_right ? 1 : -1),	count=0;
		QString buff = toPlainText();

		/* Forcing the selection of the very beggining of the first line and
		as well the end of the last line to avoid moving chars and break words wrongly */
		start = buff.lastIndexOf(QChar('\n'), cursor.selectionStart());
		end = buff.indexOf(QChar('\n'), cursor.selectionEnd());

		if(start < 0)
			start = 0;

		if(end < 0)
			end = buff.length();

		cursor.setPosition(start, QTextCursor::MoveAnchor);
		cursor.setPosition(end, QTextCursor::KeepAnchor);
		lines = cursor.selectedText().split(QChar(QChar::ParagraphSeparator));

		for(auto &line : lines)
		{
			if(!line.isEmpty())
			{
				if(ident_right)
				{
					line.prepend(QChar('\t'));
					count++;
				}
				else if(line.at(0) == QChar('\t'))
				{
					line.remove(0,1);
					count++;
				}
			}
		}

		if(count > 0)
		{
			cursor.insertText(lines.join(QChar('\n')));

			//Preserving the selection in the text to permit user perform several identations
			cursor.setPosition(start);
			cursor.setPosition(end + (count * factor), QTextCursor::KeepAnchor);
			setTextCursor(cursor);
		}
	}
}

void NumberedTextEditor::loadFile()
{
	QByteArray buff;
	bool loaded = false;

	try
	{
		loaded = GuiUtilsNs::selectAndLoadFile(buff,	tr("Load file"), QFileDialog::ExistingFile,
																					{ tr("SQL file (*.sql)"),	tr("All files (*.*)") });
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	if(loaded)
	{
		clear();
		setPlainText(buff);
		clear_btn->setEnabled(!document()->isEmpty());
	}
}

void NumberedTextEditor::saveFile()
{
	try
	{
		GuiUtilsNs::selectAndSaveFile(toPlainText().toUtf8(), tr("Save file"),
																	QFileDialog::AnyFile,
																	file_filters,	{}, default_ext);
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void NumberedTextEditor::editSource()
{
	QByteArray buffer;
	QFile input;
	QStringList args;

	if(tmp_src_file.isEmpty())
	{
		QTemporaryFile tmp_file;
		tmp_file.setFileTemplate(GlobalAttributes::getTemporaryFilePath("source_XXXXXX.sql"));
		tmp_file.open();
		tmp_src_file = tmp_file.fileName();
		tmp_file.close();
	}

	input.setFileName(tmp_src_file);

	if(!input.open(QFile::WriteOnly | QFile::Truncate))
		throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotAccessed)
										.arg(tmp_src_file),
										ErrorCode::FileDirectoryNotAccessed ,PGM_FUNC,PGM_FILE,PGM_LINE);

	buffer.append(this->toPlainText().toUtf8());
	input.write(buffer);
	input.close();

	//Starting the source editor application using the temp source file as input
	if(!src_editor_app_args.isEmpty())
		args.push_back(src_editor_app_args);

	args.push_back(tmp_src_file);

	src_editor_proc.setProgram(src_editor_app);
	src_editor_proc.setArguments(args);
	src_editor_proc.setWorkingDirectory(QDir::currentPath());
	src_editor_proc.start();
	src_editor_proc.waitForStarted();
}

void NumberedTextEditor::enableEditor()
{
	editor_alert_wgt->setVisible(false);
	load_file_btn->setEnabled(true);
	edit_src_btn->setEnabled(true);
	clear_btn->setEnabled(!this->document()->isEmpty());
	word_wrap_btn->setEnabled(!this->document()->isEmpty());
	save_file_btn->setEnabled(!this->document()->isEmpty());
	this->setReadOnly(false);
}

void NumberedTextEditor::updateSource(int exit_code, QProcess::ExitStatus)
{
	if(exit_code != 0)
		handleProcessError();
	else
	{
		QFile input(tmp_src_file);

		enableEditor();

		if(!input.open(QFile::ReadOnly))
			throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotAccessed)
											.arg(tmp_src_file),
											ErrorCode::FileDirectoryNotAccessed ,PGM_FUNC,PGM_FILE,PGM_LINE);

		this->setPlainText(input.readAll());
		input.close();
		input.remove();
	}
}

void NumberedTextEditor::handleProcessStart()
{
	if(src_editor_proc.state() == QProcess::Running)
	{
		msg_lbl->setText(tr("The editor <strong>%1</strong> (pid: %2) is running.")
												.arg(QFileInfo(src_editor_proc.program()).fileName())
												.arg(src_editor_proc.processId()));

		editor_alert_wgt->setVisible(true);
		load_file_btn->setEnabled(false);
		edit_src_btn->setEnabled(false);
		clear_btn->setEnabled(false);
		this->setReadOnly(true);
	}
}

void NumberedTextEditor::handleProcessError()
{
	QStringList errors = { src_editor_proc.errorString(),  src_editor_proc.readAllStandardError() };

	Messagebox::error(UtilsNs::formatMessage(tr("Failed to run the source code editor <strong>%1</strong>! Make to sure that the application path points to a valid executable and the current user has permission to run the application. Error message returned: <strong>%2</strong>")
																						.arg(src_editor_proc.program())
																						.arg(errors.join("\n\n"))));

	enableEditor();
}

void NumberedTextEditor::setReadOnly(bool ro)
{
	if(action_btns_enabled)
	{
		load_file_btn->setEnabled(!ro);
		edit_src_btn->setEnabled(!ro);
		load_file_btn->setVisible(!ro);
		edit_src_btn->setVisible(!ro);
		clear_btn->setEnabled(!ro && !this->document()->isEmpty());
		clear_btn->setVisible(!ro);
	}

	QPlainTextEdit::setReadOnly(ro);
}

void NumberedTextEditor::showLineNumbers(bool show)
{
	show_line_nums = line_nums_visible && show;
	line_numbers_wgt->setVisible(show_line_nums);
	resizeWidgets();
}

void NumberedTextEditor::showActionButtons(bool show)
{
	if(!top_widget)
		return;

	show_act_btns = show;
	top_widget->setVisible(show);
	resizeWidgets();
}

void NumberedTextEditor::setFocus()
{
	QPlainTextEdit::setFocus();
	this->highlightCurrentLine();
}

void NumberedTextEditor::updateLineNumbers()
{
	line_numbers_wgt->setVisible(line_nums_visible && show_line_nums);

	if(!line_nums_visible)
		return;

	QFont fnt = default_font;

	if(custom_fnt_size)
		fnt.setPointSizeF(custom_fnt_size);

	setFont(fnt);
	line_numbers_wgt->setFont(fnt);

	QTextBlock block = firstVisibleBlock();
	int block_number = block.blockNumber(),

			//Calculates the first block postion (in widget coordinates)
			top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top()),

			/* We need to retrieve the minimal height for one line of the block. So, we divide the
			 * total block height by the block line numbers (in case of the block has more than one line, e.g., the
			 * line wrap mode is activated) */
			height = static_cast<int>(blockBoundingRect(block).height()) / block.lineCount(),
			bottom = top + height,
			dy = top;

	unsigned first_line=0, line_count=0;
	double tab_stop_dist = 0;

	// Calculates the visible lines by iterating over the visible/valid text blocks.
	while(block.isValid())
	{
		if(block.isVisible())
		{
			line_count += block.lineCount();

			if(first_line == 0)
				first_line = static_cast<unsigned>(block_number + 1);
		}

		block = block.next();
		top = bottom;
		bottom = top + static_cast<int>(blockBoundingRect(block).height());
		++block_number;

		/* Check if the line count converted to widget coordinates is higher than the widget height.
		 This is done to avoid draw line numbers that are beyond the widget's height */
		if(bottom > this->height())
			break;
	}

	line_numbers_wgt->drawLineNumbers(first_line, line_count, dy, height);
	tab_stop_dist = this->tabStopDistance();

	if(round(tab_stop_dist) != round(NumberedTextEditor::getTabDistance()))
		setTabStopDistance(NumberedTextEditor::getTabDistance());
}

void NumberedTextEditor::resizeWidgets()
{
	QRect rect = contentsRect();
	int py = action_btns_enabled && show_act_btns ?
						top_widget->height()  : 0,

			lt_margin = line_nums_visible && show_line_nums ? rect.left() : 0,

			bt_margin = 0,

			hscroll_h = horizontalScrollBar()->isVisible() ?
									horizontalScrollBar()->height(): 0,

			vscroll_w = verticalScrollBar()->isVisible() ?
									verticalScrollBar()->width() : 0,

			width = rect.width() - vscroll_w;

	if(search_wgt && show_act_btns)
	{
		search_wgt->adjustSize();

		bt_margin = search_wgt && search_btn->isChecked() ?
								search_wgt->height() : 0;

		search_wgt->setGeometry(rect.left(),
														rect.bottom() - (bt_margin +  hscroll_h),
														width, search_wgt->height());
	}

	setViewportMargins(getLineNumbersWidth(), py,
										 vscroll_w * 0.05,
										 bt_margin + (hscroll_h * 0.05));

	if(line_nums_visible && show_line_nums)
	{
		line_numbers_wgt->setGeometry(lt_margin, rect.top() + py,
																 getLineNumbersWidth(),
																 rect.height() - py - (bt_margin + hscroll_h));
	}

	if(top_widget && show_act_btns)
	{
		top_widget->setStyleSheet(QString("QWidget#%1 { background-color: %2; }")
															.arg(top_widget->objectName(), line_numbers_wgt->getBackgroundColor().name()));
		
		top_widget->setGeometry(lt_margin, rect.top(),
														width, top_widget->height());
	}

	QString border_pal = CustomUiStyle::isDarkPalette() ? "dark" : "mid",

			vp_style = QString("QWidget#%1 { \
														background-color: palette(base); \
														%2 \n \
														%3 \n \
														%4 \n \
														}").arg(viewport()->objectName(),
																		 show_act_btns ? "border-top: 1px solid palette(" + border_pal + ");" : "",
																		 line_nums_visible && show_line_nums ? "border-left: 1px solid palette(" + border_pal + ");" : "",
																		 (search_wgt && search_wgt->isVisible()) ? "border-bottom: 1px solid palette(" + border_pal + ");" : "");

	viewport()->setStyleSheet(vp_style);

	setStyleSheet(QString("NumberedTextEditor { background-color: palette(window); border: 1px solid palette(%1); }")
								.arg(CustomUiStyle::isDarkPalette() ? "dark" : "mid"));
}

int NumberedTextEditor::getLineNumbersWidth()
{
	if(!line_nums_visible || !show_line_nums)
		return 0;

	int digits = 1, max = qMax(1, blockCount()),
			chr_width = 0;

	while(max >= 10)
	{
		max /= 10;
		++digits;
	}

	chr_width = this->fontMetrics().horizontalAdvance(QChar('9'));
	return (15 + (chr_width * digits));
}

void NumberedTextEditor::setWordWrapMode(QTextOption::WrapMode policy)
{
	QPlainTextEdit::setWordWrapMode(policy);
}

void NumberedTextEditor::resizeEvent(QResizeEvent *event)
{
	QPlainTextEdit::resizeEvent(event);
	resizeWidgets();
	updateLineNumbers();
}

void NumberedTextEditor::keyPressEvent(QKeyEvent *event)
{
	/* Ignoring the command Shift + Enter to breaking the line
	 * If we don't ignore, the editor will understand that the line
	 * was broken as if it was wrapped, painting a return character ↪
	 * instead of the line number in the line numbers widget */
	if((event->key() == Qt::Key_Enter ||
			event->key() == Qt::Key_Return) && event->modifiers() == Qt::ShiftModifier)
	{
		event->ignore();
		return;
	}

	if(!isReadOnly() && event->key()==Qt::Key_V && event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
	{
		pasteCode();
	}
	else if(!isReadOnly() && textCursor().hasSelection())
	{
		if(event->key()==Qt::Key_U && event->modifiers()!=Qt::NoModifier)
		{
			if(event->modifiers()==Qt::ControlModifier)
				changeSelectionToUpper();
			else if(event->modifiers()==(Qt::ControlModifier | Qt::ShiftModifier))
				changeSelectionToLower();
		}
		else if(event->key()==Qt::Key_Tab || event->key()==Qt::Key_Backtab)
		{
			if(event->key()==Qt::Key_Tab)
				identSelectionRight();
			else if(event->key()==Qt::Key_Backtab)
				identSelectionLeft();
		}
		else
			QPlainTextEdit::keyPressEvent(event);
	}
	else
		QPlainTextEdit::keyPressEvent(event);
}

void NumberedTextEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if(highlight_lines && !isReadOnly())
	{
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(line_hl_color);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

void NumberedTextEditor::setWordWrap(bool value)
{
	if(word_wrap_btn && sender() != word_wrap_btn)
	{
		word_wrap_btn->blockSignals(true);
		word_wrap_btn->setChecked(value);
		word_wrap_btn->blockSignals(false);
	}

	setWordWrapMode(value ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
	setHorizontalScrollBarPolicy(value ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
	resizeWidgets();
	updateLineNumbers();
}
