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

#include "syntaxhighlighter.h"
#include "widgets/numberedtexteditor.h"
#include "attributes.h"
#include "exception.h"
#include "globalattributes.h"
#include <QClipboard>
#include <QMimeData>

QFont SyntaxHighlighter::default_font {"Source Code Pro", 12};

SyntaxHighlighter::SyntaxHighlighter(QPlainTextEdit *parent, bool single_line_mode, bool use_custom_tab_width, qreal custom_fnt_size) : QSyntaxHighlighter(parent)
{
	if(!parent)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	code_field_txt = parent;

	this->setDocument(parent->document());
	this->single_line_mode = single_line_mode;
	custom_font_size = custom_fnt_size;
	configureAttributes();
	parent->installEventFilter(this);

	if(use_custom_tab_width)
		parent->setTabStopDistance(NumberedTextEditor::getTabDistance());

	//Adjusting the size of the parent input according to the current font size
	if(single_line_mode)
	{
		QFont fnt(default_font.family(), getCurrentFontSize());
		QFontMetrics fm = QFontMetrics(fnt);
		int height = fm.height() + (fm.lineSpacing() / 2);

		parent->setMinimumHeight(height);
		parent->setMaximumHeight(height);
		parent->setSizePolicy(parent->sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
		parent->adjustSize();
		parent->setTabChangesFocus(true);
		parent->setLineWrapMode(QPlainTextEdit::NoWrap);
		parent->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		connect(parent, &QPlainTextEdit::textChanged, this, [parent](){
			// Avoiding pasting code with line breaks when the single line mode is activated
			QString txt = parent->toPlainText();
			if(!txt.isEmpty() && txt.contains(QChar::LineFeed))
			{
				txt.remove(QChar::LineFeed);
				parent->blockSignals(true);
				parent->setPlainText(txt);
				parent->blockSignals(false);
			}
		});
	}

	highlight_timer.setInterval(200);
	connect(parent, &QPlainTextEdit::cursorPositionChanged, &highlight_timer, qOverload<>(&QTimer::start));

	connect(&highlight_timer, &QTimer::timeout, this, [this](){
		highlight_timer.stop();

		if(this->single_line_mode)
			return;

		for(auto &cfg : enclosing_chrs)
		{
			if(highlightEnclosingChars(cfg))
				break;
		}
	});
}

bool SyntaxHighlighter::eventFilter(QObject *object, QEvent *event)
{
	//Filters the ENTER/RETURN avoiding line breaks
	if(this->single_line_mode && event->type() == QEvent::KeyPress)
	{
		QKeyEvent *k_event = dynamic_cast<QKeyEvent *>(event);

		if(k_event->key() == Qt::Key_Return || k_event->key() == Qt::Key_Enter)
		{
			event->ignore();
			return true;
		}
	}

	/* If the user is about press Control to paste contents or Right mouse button in
	 order to call the context menu to paste text */
	if(event->type()==QEvent::MouseButtonPress || event->type() == QEvent::KeyPress)
	{
		QKeyEvent *k_event=dynamic_cast<QKeyEvent *>(event);
		QMouseEvent *m_event=dynamic_cast<QMouseEvent *>(event);

		//Remove the formatting from the clipboard buffer to paste only pure text
		if(qApp->clipboard() && qApp->clipboard()->mimeData()->hasHtml() &&
				((m_event && m_event->button()==Qt::RightButton) ||
				 (k_event && k_event->modifiers()==Qt::ControlModifier)))
			qApp->clipboard()->setText(qApp->clipboard()->mimeData()->text());
	}

	return QSyntaxHighlighter::eventFilter(object, event);
}

void SyntaxHighlighter::configureAttributes()
{
	conf_loaded = false;
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
	QString open_group;
	TextBlockInfo *blk_info = nullptr,
			*prev_blk_info = dynamic_cast<TextBlockInfo *>(currentBlock().previous().userData());
	int prev_blk_state = currentBlock().previous().userState();
	bool match_final_exp = false;

	/* Creating a text block info so we can register
	 * each position where there is a text formatting */
	if(!currentBlockUserData())
	{
		blk_info = new TextBlockInfo;
		setCurrentBlockUserData(blk_info);
		setCurrentBlockState(prev_blk_state == PersistentBlock ? SimpleBlock : prev_blk_state);
	}
	else
	{
		//Reset the block's info to permit the rehighlighting
		blk_info = dynamic_cast<TextBlockInfo *>(currentBlockUserData());
		blk_info->reset();
		setCurrentBlockState(SimpleBlock);
	}

	if(prev_blk_info && prev_blk_state >= OpenExprBlock)
	{
		open_group = prev_blk_info->getOpenGroup();
		setCurrentBlockState(prev_blk_state);
		match_final_exp = true;

		/* Forcing an empty block to hold the name of the
		 * previous open block, so a block right after it
		 * can be highlighted with the correct group format */
		if(text.isEmpty())
			blk_info->setOpenGroup(open_group);
	}

	if(!text.isEmpty())
	{
		const GroupConfig *group_cfg = nullptr;
		bool expr_open = false, expr_closed = false;
		QList<MatchInfo> matches;
		MatchInfo m_info;
		const FragmentInfo *f_info = nullptr;
		int pos = 0, open_expr_idx = -1, grp_idx = multilines_order.indexOf(open_group);
		auto ml_itr = multilines_order.begin(),
				ml_itr_end = multilines_order.end();

		/* If we located the open group in the list of multiline groups
		 * we'll start our search from it */
		if(grp_idx >= 0)
			ml_itr += grp_idx;
		else
			grp_idx = 0;

		// Applying multiline expressions format
		while(ml_itr != ml_itr_end)
		{
			group_cfg = &group_confs[*ml_itr];

			// Searching for the opening expression of the current group
			if(!match_final_exp && matchGroup(group_cfg, text, pos, false, m_info))
			{
				f_info = blk_info->getFragmentInfo(m_info.start, m_info.end);
				pos = m_info.end + 1;

				/* If we don't have a text fragment in the starting position
				 * of the current match info or there's a fragment info but
				 * it is not related to a multiline expression (it's not
				 * open and closed) */
				if(!f_info || (!f_info->isOpen() && !f_info->isClosed()))
				{
					/* Flagging the block state as OpenExprBlock, to force the highliting until its end
					 * if an closing expression isn't found */
					match_final_exp = true;
					setCurrentBlockState(OpenExprBlock + grp_idx);
					setFormat(m_info, group_cfg, true, false, blk_info);
					open_group = group_cfg->name;

					/* Flagging where the open expression was found
					 * so we can determine if non-multline group need to be
					 * matched */
					open_expr_idx = m_info.start;
				}
			}

			// If we find the opening expression, we'll search for the closing expression
			if(match_final_exp)
			{
				match_final_exp = false;
				matchGroup(group_cfg, text, pos, true, m_info);
				m_info.start = pos;

				/* The final expression of the group wasn't found in the block
				 * so we consider it a open multiline expression (to be closed
				 * in the next blocks. In that case, we apply the group format until
				 * the end of this block.*/
				if(!m_info.isValid())
				{
					expr_open = true;
					expr_closed = false;
					pos = m_info.end = text.length();

					// Flagging the block state as OpenExprBlock, highliting it until its end
					setCurrentBlockState(OpenExprBlock + grp_idx);
				}
				/* If the closing expression is found we need to flag the expression as
				 * closed and the block state as SimpleBlock so the highlighter can keep
				 * searching other groups in the same block and formatting accordingly */
				else
				{
					pos = m_info.end + 1;
					expr_open = false;
					expr_closed = true;
					open_expr_idx =  -1;
					setCurrentBlockState(SimpleBlock);
				}

				setFormat(m_info, group_cfg, expr_open, expr_closed, blk_info);

				/* If the current block state is OpenExprBlock at this point
				 * it means that we have reached the end of the block and no
				 * closing expression was found, so the next block will
				 * inconditionally inherit the formatting of portion of this
				 * block that was not closed. */
				if(currentBlockState() >= OpenExprBlock)
					break;
			}

			if(pos >= text.length())
			{
				/* If we have reached the end of the block we restarting the highliting
				 * in that block but now using the next multiline group */
				pos = 0;
				ml_itr++;
				grp_idx++;
			}
			else
				pos++;
		}

		/* We abort further groups matching if we have a open expression block
		 * in which the token was found at the beggining of the text, or the
		 * block inherited the OpenExprBlock state from a previous block and
		 * don't have the opening and closing tokens */
		if(open_expr_idx <= 0 && currentBlockState() >= OpenExprBlock)
			return;

		for(auto &grp : groups_order)
		{
			group_cfg = &group_confs[grp];

			if(matchGroup(group_cfg, text, 0, false, matches))
			{
				/* The the current group is a persistent one and its formatting settings were
				 * applied, then we mark the block state as PersistentBlock this force the rehighlight
				 * of next block if they are part of a open multiline expression. */
				if(setFormat(&matches, group_cfg, false, false, blk_info) && group_cfg->persistent)
					setCurrentBlockState(PersistentBlock);
			}
		}
	}
}

bool SyntaxHighlighter::setFormat(const MatchInfo &m_info, const GroupConfig *group_cfg, bool expr_open,
																	bool expr_closed, TextBlockInfo *blk_info)
{
	if(!m_info.isValid() || !group_cfg || !blk_info)
		return false;

	QTextCharFormat fmt = group_cfg ? group_cfg->format : QTextCharFormat();
	int end = m_info.end, len = m_info.getLength();

	/* No formatting will be applied if we found a formatted
	 * text fragment in the current postion */
	if(blk_info->getFragmentInfo(m_info.start, m_info.end))
		return false;

	/* Forcing the highlighting from the starting position of the matching info to the
	 * end of the block when the group is a persistent one */
	if(group_cfg->persistent)
	{
		end = currentBlock().length() - 1;
		len = end - m_info.start + 1;
	}

	fmt.setFontFamilies({ default_font.family() });
	fmt.setFontPointSize(getCurrentFontSize());
	QSyntaxHighlighter::setFormat(m_info.start, len, fmt);

	/* If we are highlighting an open expression we need
	 * to register the name of the group that the confs
	 * are being applied to the block so it can be applied
	 * to the next block */
	if(expr_open && currentBlockState() >= OpenExprBlock)
		blk_info->setOpenGroup(group_cfg->name);

	/* If the block state is SimpleBlock meaning that there's no
	 * open expression. Or, if the current state is an open expression
	 * but the expression was closed we clear the name of the group
	 * so the next block don't inherit formatting */
	else if(currentBlockState() == SimpleBlock ||
					 (currentBlockState() >= OpenExprBlock && expr_closed))
		blk_info->setOpenGroup("");

	blk_info->addFragmentInfo(FragmentInfo(group_cfg->name, m_info.start, end,
																				 expr_open, expr_closed,
																				 group_cfg->allow_completion));

	return true;
}


bool SyntaxHighlighter::setFormat(const QList<MatchInfo > *matches, const GroupConfig *group_cfg, bool expr_open,
																	bool expr_closed, TextBlockInfo *blk_info)
{
	if(!matches || !group_cfg || !blk_info)
		return false;

	bool fmt_applied = false;

	for(auto &m_info : *matches)
	{
		if(setFormat(m_info, group_cfg, expr_open, expr_closed, blk_info))
			fmt_applied = true;
	}

	return fmt_applied;
}

template<class Class>
bool SyntaxHighlighter::matchGroup(const GroupConfig *group_cfg, const QString &text, int txt_pos, bool final_expr, Class &match)
{
	QMap<QString, QList<QRegularExpression>> *exprs_map = final_expr ? &final_exprs : &initial_exprs;

	match.clear();

	if(!group_cfg || !exprs_map->contains(group_cfg->name))
		return false;

	for(auto &expr : (*exprs_map)[group_cfg->name])
		matchExpression(text, txt_pos, expr, match);

	return !match.isEmpty();
}

bool SyntaxHighlighter::matchExpression(const QString &text, int txt_pos, const QRegularExpression &expr, QList<MatchInfo> &matches)
{
	static QRegularExpressionMatchIterator mt_itr;
	static QRegularExpressionMatch match;
	MatchInfo m_info;

	mt_itr = expr.globalMatch(text, txt_pos);

	while(mt_itr.isValid() && mt_itr.hasNext())
	{
		match = mt_itr.next();
		m_info.start = match.capturedStart();
		m_info.end = match.capturedEnd() - 1;

		if(match.isValid() && m_info.isValid() &&
				match.capturedLength() > 0 && !matches.contains(m_info))
		{
			matches.append(m_info);
		}
	}

	return !matches.isEmpty();
}

bool SyntaxHighlighter::matchExpression(const QString &text, int txt_pos, const QRegularExpression &expr, MatchInfo &m_info)
{
	static QRegularExpressionMatch match;
	int mt_start = -1, mt_end = -1;

	match = expr.match(text, txt_pos);
	mt_start = match.capturedStart();
	mt_end = match.capturedEnd() - 1;

	if(mt_start >= 0 && mt_end >= 0 && match.capturedLength() > 0)
	{
		m_info.start = mt_start;
		m_info.end = mt_end;
	}

	return !m_info.isEmpty();
}

const SyntaxHighlighter::GroupConfig *SyntaxHighlighter::getGroupConfig(const QString &group)
{
	if(!group_confs.contains(group))
		return nullptr;

	return &group_confs[group];
}

bool SyntaxHighlighter::highlightEnclosingChars(const EnclosingCharsCfg &cfg)
{
	QString curr_chr;
	QPlainTextEdit *code_txt = qobject_cast<QPlainTextEdit *>(parent());
	QTextCursor tc = code_txt->textCursor();
	int orig_pos = tc.position();
	bool is_numbered_editor = (qobject_cast<NumberedTextEditor *>(code_txt) != nullptr);

	tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
	curr_chr = tc.selectedText();
	tc.setPosition(orig_pos);

	/* If the next character from the current cursor position
	 * is not an enclosing char, we just stop here. */
	if(curr_chr != cfg.open_char && curr_chr != cfg.close_char)
	{
		/* If the code_txt is not an instance of NumberedTextEditor
		 * we need to clear the extra selections to undo the highlighting
		 * of a previously highlighted enclosing chars. */
		if(!is_numbered_editor)
			code_txt->setExtraSelections({});

		return false;
	}

	QChar inc_chr, dec_chr;
	QString code = code_txt->toPlainText();
	int chr_balance_cnt = 0,
			pos = 0, ini_pos = 0,

			/* If the current text is an open char we
			 * search for enclosing char ahead of the
			 * current position (1) otherwise we search
			 * for a close char behind of the current
			 * position (-1) */
			inc = (curr_chr == cfg.open_char ? 1 : -1);

	pos = ini_pos = tc.position();
	inc_chr = (curr_chr == cfg.open_char ? cfg.open_char : cfg.close_char);
	dec_chr = (curr_chr == cfg.open_char ? cfg.close_char : cfg.open_char);

	while(pos >= 0 && pos < code.size())
	{
		if(code[pos] == inc_chr)
			chr_balance_cnt++;
		else if(code[pos] == dec_chr)
			chr_balance_cnt--;

		if(chr_balance_cnt == 0)
			break;

		pos += inc;
	}

	if(ini_pos >= 0)
	{
		QTextCharFormat fmt;
		QList<QTextEdit::ExtraSelection> selections;
		QTextEdit::ExtraSelection sel;

		/* If the current code_txt instance is a NumberedTextEditor
		 * we need to keep the line highlighted prior to any enclosing
		 * char highlighting if the line highlight option is set */
		if(is_numbered_editor && NumberedTextEditor::isHighlightLines() && !single_line_mode)
		{
			sel.format.setBackground(NumberedTextEditor::getLineHighlightColor());
			sel.format.setProperty(QTextFormat::FullWidthSelection, true);
			sel.cursor = tc;
			sel.cursor.clearSelection();
			selections.append(sel);
		}

		fmt = tc.charFormat();

		if(pos >= 0 && pos < code.size())
		{
			// Color config for balanced enclosing chars
			fmt.setBackground(cfg.bg_color);
			fmt.setForeground(cfg.fg_color);
		}
		else
		{
			// Color config for unbalanced enclosing chars
			fmt.setBackground(QColor(200, 0, 0));
			fmt.setForeground(Qt::white);
		}

		// Highlighting the found enclosing char
		tc.setPosition(ini_pos);
		tc.setPosition(ini_pos + 1, QTextCursor::KeepAnchor);
		sel.cursor = tc;
		sel.format = fmt;
		selections.append(sel);

		// Highlighting the matching closing/opening char if found
		if(pos >= 0 && pos < code.size())
		{
			tc.setPosition(pos);
			tc.setPosition(pos + 1, QTextCursor::KeepAnchor);
			sel.cursor = tc;
			selections.append(sel);
		}

		code_txt->setExtraSelections(selections);
		return true;
	}

	return false;
}

qreal SyntaxHighlighter::getCurrentFontSize()
{
	return custom_font_size > 0 ?
					custom_font_size : default_font.pointSizeF();
}

bool SyntaxHighlighter::isConfigurationLoaded()
{
	return conf_loaded;
}

void SyntaxHighlighter::clearConfiguration()
{
	groups_order.clear();
	group_confs.clear();
	initial_exprs.clear();
	final_exprs.clear();
	configureAttributes();
}

void SyntaxHighlighter::loadConfiguration(const QString &filename)
{
	if(filename.isEmpty())
		return;

	attribs_map attribs;
	QString elem, group, pattern;
	bool bold = false, italic = false, strikeout = false,
			underline = false, initial_expr = false,
			final_expr = false, case_sensitive = false;
	QTextCharFormat format;
	QColor bg_color, fg_color;
	GroupConfig group_cfg;
	QRegularExpression regexp;

	try
	{
		clearConfiguration();
		xmlparser.restartParser();
		xmlparser.setDTDFile(GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ObjectDTDDir,
																																				GlobalAttributes::CodeHighlightConf +
																																				GlobalAttributes::ObjectDTDExt),
												 GlobalAttributes::CodeHighlightConf);

		xmlparser.loadXMLFile(filename);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem = xmlparser.getElementName();

					if(elem == Attributes::CompletionTrigger)
					{
						xmlparser.getElementAttributes(attribs);

						if(attribs[Attributes::Value].size() >= 1)
							completion_trigger = attribs[Attributes::Value].at(0);
					}
					else if(elem == Attributes::EnclosingChars)
					{
						xmlparser.getElementAttributes(attribs);

						EnclosingCharsCfg cfg;
						cfg.open_char = attribs[Attributes::OpenChar].front();
						cfg.close_char = attribs[Attributes::CloseChar].front();
						cfg.fg_color = QColor::fromString(attribs[Attributes::FgColor]);
						cfg.bg_color = QColor::fromString(attribs[Attributes::BgColor]);

						enclosing_chrs.push_back(cfg);
					}

					if(elem == Attributes::Group)
					{
						xmlparser.getElementAttributes(attribs);
						group = attribs[Attributes::Name];

						//Raises an error if the group is being constructed by a second time
						if(group_confs.contains(group))
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::DefDuplicatedGroup).arg(group),
															 ErrorCode::DefDuplicatedGroup, PGM_FUNC, PGM_FILE, PGM_LINE);
						}

						//Raises an error if the group does not have children element
						if(!xmlparser.hasElement(XmlParser::ChildElement))
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::DefEmptyGroup).arg(group),
															ErrorCode::DefEmptyGroup, PGM_FUNC, PGM_FILE, PGM_LINE);
						}

						case_sensitive = attribs[Attributes::CaseSensitive] == Attributes::True;
						italic = attribs[Attributes::Italic] == Attributes::True;
						bold = attribs[Attributes::Bold] == Attributes::True;
						underline = attribs[Attributes::Underline] == Attributes::True;
						strikeout = attribs[Attributes::Stikeout] == Attributes::True;

						/* If the attribute isn't defined the fg color will be the same as the
						 * parent's default text color */
						if(attribs[Attributes::FgColor].isEmpty())
							fg_color = code_field_txt->palette().color(QPalette::WindowText);
						else
							fg_color = QColor::fromString(attribs[Attributes::FgColor]);

						// If the attribute isn't defined the default the bg color will be transparent
						if(attribs[Attributes::BgColor].isEmpty())
							bg_color = Qt::transparent;
						else
							bg_color = QColor::fromString(attribs[Attributes::BgColor]);

						format.setFontFamilies({ default_font.family() });
						format.setFontPointSize(default_font.pointSizeF());
						format.setFontItalic(italic);
						format.setFontUnderline(underline);
						format.setFontStrikeOut(strikeout);

						if(bold)
							format.setFontWeight(QFont::Bold);
						else
							format.setFontWeight(QFont::Normal);

						format.setForeground(fg_color);
						format.setBackground(bg_color);

						group_cfg = GroupConfig(group, format,
																		attribs[Attributes::AllowCompletion] != Attributes::False,
																		attribs[Attributes::Persistent] == Attributes::True,
																		false);

						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);

						do
						{
							if(xmlparser.getElementType()==XML_ELEMENT_NODE)
							{
								xmlparser.getElementAttributes(attribs);

								initial_expr = attribs[Attributes::Initial] == Attributes::True;
								final_expr = attribs[Attributes::Final] == Attributes::True;

								if(!group_cfg.multiline && (initial_expr || final_expr))
									group_cfg.multiline = true;

								regexp.setPattern(pattern);

								// Regular expression matching
								if(attribs[Attributes::Type] == Attributes::RegularExp)
									regexp.setPattern(attribs[Attributes::Value]);
								// Wildcard matching
								else if(attribs[Attributes::Type] == Attributes::Wildcard)
									regexp.setPattern(QRegularExpression::wildcardToRegularExpression(attribs[Attributes::Value]));
								// Word boundary matching regexp
								else if(attribs[Attributes::Type] == Attributes::Word)
								{
									/* For matching words we use the pattern \s\b[word]\s\b which indicate that the word must
									 * have at least on space or tab attached, or is delimited by be a word boundary,
									 * or be the only word in the text block. Additionally, we use lookahead(?=) and lookbehind(?<=)
									 * operators to avoid that the space character/word boundary is captured/computed.
									 * This can match the entire word and not parts of it in the text block */
									regexp.setPattern(QString("(?<=\\s|\\b)%1(?=\\s|\\b)")
																		.arg(QRegularExpression::escape(attribs[Attributes::Value])));
								}
								// Exact (escaped) regular expression
								else
									regexp.setPattern(QRegularExpression::escape(attribs[Attributes::Value]));

								regexp.setPatternOptions(!case_sensitive ?
																					QRegularExpression::CaseInsensitiveOption :
																					QRegularExpression::NoPatternOption |
																					QRegularExpression::DontCaptureOption);

								// We throw an error aborting the loading if the regepx has an invalid pattern
								if(!regexp.isValid())
								{
									throw Exception(Exception::getErrorMessage(ErrorCode::InvGroupRegExpPattern).arg(group, filename, regexp.errorString()),
																	ErrorCode::InvGroupRegExpPattern, PGM_FUNC, PGM_FILE, PGM_LINE, nullptr,
																	tr("Pattern: %1").arg(regexp.pattern()));
								}

								if(group_cfg.persistent && (initial_expr || final_expr))
								{
									throw Exception(Exception::getErrorMessage(ErrorCode::InvExprPersistentGroup).arg(group, filename),
																	 ErrorCode::InvExprPersistentGroup, PGM_FUNC, PGM_FILE, PGM_LINE, nullptr,
																	 tr("Pattern: %1").arg(regexp.pattern()));
								}

								if(initial_expr && final_expr)
								{
									throw Exception(Exception::getErrorMessage(ErrorCode::InvExprMultilineGroup).arg(group, filename),
																	 ErrorCode::InvExprMultilineGroup, PGM_FUNC, PGM_FILE, PGM_LINE, nullptr,
																	 tr("Pattern: %1").arg(regexp.pattern()));
								}

								if(final_expr)
									final_exprs[group].append(regexp);
								else
									initial_exprs[group].append(regexp);
							}
						}
						while(xmlparser.accessElement(XmlParser::NextElement));

						group_confs[group] = group_cfg;

						if(group_cfg.multiline)
							multilines_order.append(group);
						else
							groups_order.append(group);

						xmlparser.restorePosition();
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		conf_loaded = true;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e, filename);
	}
}

QStringList SyntaxHighlighter::getExpressions(const QString &group_name)
{
	QStringList exprs;

	if(initial_exprs.contains(group_name))
	{
		for(auto &expr : initial_exprs[group_name])
			exprs.append(expr.pattern());
	}

	return exprs;
}

QChar SyntaxHighlighter::getCompletionTrigger()
{
	return completion_trigger;
}

void SyntaxHighlighter::setDefaultFont(const QFont &fnt)
{
	SyntaxHighlighter::default_font = fnt;
}
