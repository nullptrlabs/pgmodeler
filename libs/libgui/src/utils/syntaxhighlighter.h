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
\class SyntaxHighlighter
\brief Implements a syntax hightlighter with user-defined markup patterns (xml configuration).
*/

#ifndef SYNTAX_HIGHLIGHTER_H
#define SYNTAX_HIGHLIGHTER_H

#include "guiglobal.h"
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTimer>
#include <QPlainTextEdit>
#include "xmlparser.h"
#include "textblockinfo.h"

class __libgui SyntaxHighlighter: public QSyntaxHighlighter {
	Q_OBJECT

	private:
		/*! \brief This struct stores the configuration of formatting groups
		 *  and their respective foreground/background colors */
		struct GroupConfig {
			QString name;
			QTextCharFormat format;
			bool allow_completion, multiline, persistent;

			GroupConfig()
			{
				allow_completion = multiline = persistent = false;
			}

			GroupConfig(const QString &_name, const QTextCharFormat &_format,
									bool _allow_compl, bool _persistent, bool _multiline)
			{
				name = _name;
				format = _format;
				allow_completion = _allow_compl;
				multiline = _multiline;
				persistent = _persistent;
			}
		};

		/*! \brief This struct stores the configuration of enclosing characters
		 *  and their respective foreground/background colors */
		struct EnclosingCharsCfg {
				QChar open_char, close_char;
				QColor fg_color, bg_color;
		};

		QPlainTextEdit *code_field_txt;

		//! \brief XML parser used to parse configuration files
		XmlParser xmlparser;

		//! \brief Default font configuratoin for all instances os syntax highlighter
		static QFont default_font;

		//! \brief Indicates that the current block has no special meaning
		static constexpr int SimpleBlock = -1,

		/*! \brief Indicates that the current block was last formatted by a persistent group,
		 * Indicating that the highlight was applied to the start position of the group
		 * until the end of the block. */
		PersistentBlock = 0,

		/*! \brief Indicates that the current block has an open (but still to close) expression (e.g. multline comments)
		* When the highlighter finds this const it'll do special operation like highlight next blocks with the same
		* configuration as the current one. Due to the possiblity to have many multiline groups the setCurrentBlockState
		* calls using this as parameter will always use the group index as extra value. For example, say we have a
		* "multi-line-comment" group which index is 1 and was applied to the current block, then the block state will
		* be set as OpenExprBlock + 1 */
		OpenExprBlock = 1;

		//! \brief Stores the order in which the groups must be applied
		QStringList groups_order, multilines_order;

		//! \brief Stores the formatting groups configs
		QMap<QString, GroupConfig> group_confs;

		//! \brief Stores the groups initial expressions
		QMap<QString, QList<QRegularExpression>> initial_exprs,

				//! \brief Stores the groups final expressions (only for multiline groups)
				final_exprs;

		//! \brief Stores the enclosing characters config read from file
		QList<EnclosingCharsCfg> enclosing_chrs;

		//! \brief Indicates if the configuration is loaded or not
		bool conf_loaded,

		/*! \brief This causes the highlighter to ignores any RETURN/ENTER press on QTextEdit causing
		 *  the text to be in a single line. Also, in single line mode, pressing tab on a focused
		 *  input causes the focus to jump to the next widget in the hierarchy */
		single_line_mode;

		//! \brief Stores the custom font size to be used instead of default_font size
		qreal custom_font_size;

		//! \brief Stores the char that triggers the code completion
		QChar	completion_trigger;

		QTimer highlight_timer;

		//! \brief Configures the initial attributes of the highlighter
		void configureAttributes();

		/*! \brief This event filter is used to nullify the line breaks when the highlighter
		 is created in single line edit model */
		bool eventFilter(QObject *object, QEvent *event) override;

		/*! \brief Performas a global match of the expression 'expr' in 'text' starting from 'txt_pos'.
		 * If the expression matches any portion of the text then 'matches' will store
		 * all the match positions in the text, otherwise, it will be empty. */
		bool matchExpression(const QString &text, int txt_pos, const QRegularExpression &expr, QList<MatchInfo> &matches);

		/*! \brief Performs a single match of the expression 'expr' in 'text' starting from 'txt_pos'.
		 * If the expression matches any portion of the text then 'm_info' will store
		 * the first match position in the text, otherwise, it will be invalid */
		bool matchExpression(const QString &text, int txt_pos, const QRegularExpression &expr, MatchInfo &m_info);

		/*! \brief Tries to apply the enclosing char formats based on the current cursor position
		 *  on the parent input. Returns true when the enclose char config was applied */
		bool highlightEnclosingChars(const EnclosingCharsCfg &cfg);

		//! \brief Returns a const ref to the named group configuration
		const GroupConfig *getGroupConfig(const QString &group);

		/*! \brief Matches the expression in 'group_cfg' in 'text' starting from 'txt_pos', storing the
		 * matching position(s) in 'matches'. The 'final_expr' forces only final expressions to be used
		 * in the matching. This method returns true when there's at least one matching */
		template<class Class>
		bool matchGroup(const GroupConfig *group_cfg, const QString &text, int txt_pos, bool final_expr, Class &matches);

		/*! \brief Applies a formatting in position defined by 'm_info', using the setting in 'group_cfg'.
		 *  The 'expr_open' and 'expr_closed' is used to indicate if the current formatting is related to
		 *  an open or closed expression (multiline groups). The 'blk_info' instance is the current block
		 *  info structure which is updated by registering a new FragmentInfo for the current text postion,
		 *  to keep track of multiline groups formatting.
		 *
		 *  This method will not apply the formatting if there is already a formatting applied in the
		 *  provided text position in 'm_info'. */
		bool setFormat(const MatchInfo &m_info, const GroupConfig *group_cfg,
										bool expr_open, bool expr_closed, TextBlockInfo *blk_info);

		bool setFormat(const QList<MatchInfo > *matches, const GroupConfig *group_cfg,
									 bool expr_open, bool expr_closed, TextBlockInfo *blk_info);

	public:
		/*! \brief Install the syntax highlighter in a QPlainTextEdit.
		 * If single_line_mode is true the highlighter prevents the parent text field to process line breaks.
		 * If use_custom_tab_width is true the highlighter will use the same tab size as NumberedTextEdit class.
		 * If custom_fnt_size is greater than 0 the higlighter will apply that font size instead of the one set in default_font */
		SyntaxHighlighter(QPlainTextEdit *parent, bool single_line_mode=false, bool use_custom_tab_width=false, qreal custom_fnt_size = 0);

		//! \brief Loads a highlight configuration from a XML file
		void loadConfiguration(const QString &filename);

		//! \brief Returns if the configuration were successfully loaded
		bool isConfigurationLoaded();

		/*! \brief Returns the regexp vector of the specified group. The 'final_expr' bool parameter indicates
		that the final expressions must be returned instead of initial expression (default) */
		QStringList getExpressions(const QString &group_name);

		//! \brief Returns the current configured code completion trigger char
		QChar getCompletionTrigger();

		/*! \brief Returns the current font size according to the custom_font_size attribute.
		 *  If it is greater than 0 then its value is returned otherwise the default_font size is returned */
		qreal getCurrentFontSize();

		//! \brief Sets the default font for all instances of this class
		static void setDefaultFont(const QFont &fnt);

	private slots:
		//! \brief Highlight a line of the text
		void highlightBlock(const QString &text) override;

		//! \brief Clears the loaded configuration
		void clearConfiguration();
};

#endif

