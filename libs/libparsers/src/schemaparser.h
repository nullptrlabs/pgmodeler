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
\ingroup libparsers
\class SchemaParser
\brief Defines the schema micro language that is used to generate code.
\note <strong>Creation date:</strong> 19/06/2008
*/

#ifndef SCHEMA_PARSER_H
#define SCHEMA_PARSER_H

#include "parsersglobal.h"
#include <QDir>
#include <QTextStream>
#include "attribsmap.h"
#include "exception.h"
#include <QRegularExpression>

class __libparsers SchemaParser {
	private:
		struct IncludeInfo {
			QString include_file;
			int start_line {-1},
					end_line {-1},
					stmt_length {0};

			bool isValid()
			{
				return start_line >= 0 &&
							 end_line >= start_line &&
							 stmt_length > 0;
			}

			bool contains(int line)
			{
				return isValid() &&
							 line >= start_line && line <= end_line;
			}
		};

		/*! \brief Stores the information (start/end lines) of included files
		 *  in the buffer. This is used to report syntax/semantic errors at the
		 *  correct location */
		std::vector<IncludeInfo> include_infos;

		/*! \brief Indicates that the parser should ignore unknown
		 attributes avoiding raising exceptions */
		bool ignore_unk_atribs;

		/*! \brief Indicates that the parser should ignore empty
		 attributes avoiding raising exceptions */
		bool ignore_empty_atribs;

		//! \brief RegExp used to validate attribute names
		static const QRegularExpression AttribRegExp;

		//! \brief Get an attribute name from the buffer on the current position
		QString getAttribute(bool &found_conv_to_xml);

		//! \brief Get an conditional instruction from the buffer on the current position
		QString getConditional();

		//! \brief Get a metacharacter or escaped character token from the buffer on the current position
		QString getMetaOrEscapedToken(bool is_escaped);

		//! \brief Get an metacharacter from the buffer on the current position
		QString getMetaCharacter();

		//! \brief Get an escaped character from the buffer on the current position
		QString getEscapedCharacter();

		/*! \brief Returns the result (true|false) of conditional expression evaluation.
		The expression is evaluated from the left to the right and not support Polish Notation, so
		as the parser finds an attribute evaluates it and stores the result, when find another one
		evaluates and compare with previous evaluation, and so on. Example:

		Let's suppose a1 and a2 has values and a3 is empty, the following expression
		results will be:

		%if {a1} %or {a2} %and {a3} %then --> FALSE
		%if {a1} %or {a2} %and %not {a3} %then --> TRUE
		%if {a1} %or %not {a3} %then --> TRUE
		%if {a1} %and {a3} %then --> FALSE
		*/
		bool evaluateExpression();

		/*! \brief Returns the result (true|false) of a comparison expression. A comparison expression
		have the form: ( {attribute} [operator] "value" ), where:

		(           --> Starts the expression
		{attribute} --> Is the attribute to be compared to a value
		[operator]  --> A valid comparison operator:
						== (equal), != (not equal), < (less than), > (greater than)
						<= (less or equal to), >= (greater or equal to)
		)           --> Closes the expression

		The parenthesis are mandatory otherwise the parser will not recognize the expression
		and raise an exception. Multiple expressions combined with logical operators
		%not %and %or in the same () are not supported. */
		bool evaluateComparisonExpr();

		/*! \brief Creates a new attribute when finding:
		1) %set  {attrib-name} [expr]  or
		2) %set @{existing-attrib} [expr]

		Where [expr] can be pure texts, meta chars or other attributes exists overwrite its value.

		In the second form note the @ at the beginnig of 'existing-attribute', this variation creates
		a new attribute which the name is the value of {existing-attribute}. In this case the parser
		may return error if the value used is an invalid name.

		The %set construction must be the only one in the line otherwise the parser will return
		errors if another instruction starting with % is found. */
		void defineAttribute();

		//! \brief Clears the value of attributes when finding the instruction: %unset {attr1} {attr2}...
		void unsetAttribute();

		//! \brief Increments the column counter while blank chars (space and tabs) are found on the line
		void ignoreBlankChars(const QString &line);

		//! \brief Translates the meta char token to the real character
		QString convertMetaCharacter(const QString &meta);

		//! \brief Translates the escaped char token to the equivalent character
		QString convertEscapedCharacter(const QString &escaped);

		QString convertMetaOrEscaped(const QString &token, bool is_escaped);

		/*! \brief Get an word from the buffer on the current position (word is any string that isn't
		 a conditional instruction or comment) */
		QString getWord();

		//! \brief Gets a pure text, ignoring elements of the language
		QString getPlainText();

		/*! \brief Returns whether a character is special i.e. indicators of attributes
		 or conditional instructions */
		bool isSpecialCharacter(const QChar &chr);

		//! \brief Filename that was loaded by the parser
		QString filename, search_path;

		/*! \brief Vectorial representation of the loaded file. This is the buffer that is
		 analyzed by de parser */
		QStringList buffer;

		//! \brief Current line where the parser reading is
		int line,

				 //! \brief Current column where the parser reading is
				column;

		attribs_map attributes;

		//! \brief PostgreSQL version currently used by the parser
		QString pgsql_version;

		/*! \brief Returns the boolean result of the comparison between left_val and right_val
		 * using the provided operator oper (see Token*Oper) */
		template<typename Type>
		bool getExpressionResult(const QString &oper, const QVariant &left_val, const QVariant &right_val);

		/*! \brief Parsers the token @include "file.sch". This method changes the original
		 *  structure of the buffer by replacing @include token with the code in the included file.
		 *  The "file.sch" portion in @include token is always treated as being a relative path.
		 *  Currently, you can have includes only in the main buffer (source file). If the included
		 *  files also have includes this method will raise an exception. This should be changed in the future. */
		bool parseInclude(const QString &include_ln, QString &src_buf, qint64 curr_stream_pos);

		/*! \brief Returns a formatted error message used prior to an exception raising.
		 * This method take into account if the error happened in the current schema file/buffer or
		 * in a file included via @include. ATTENTION: this method changes the current line and column
		 * variables so getCurrentLine() and getCurrentColumn() returns the proper line/columns of
		 * the source file. DO NOT CALL THIS METHOD outside an exception instantiation/throwing */
		template<typename ...ArgType>
		QString getParseError(ErrorCode err_code, const QString &extra_msg = "", const ArgType & ...args)
		{
			int actual_line = line + 1, incl_line = 0, incl_stmt_len = 0;
			QString file = filename, msg;
			bool is_incl_error = false;

			/* Checking if the current line is associated to a fragment of code
			 * loaded from an include file */
			for(auto &info : include_infos)
			{
				if(info.contains(line))
				{
					incl_line = info.start_line;
					actual_line = (line - info.start_line) + 1;
					file = info.include_file;
					incl_stmt_len = info.stmt_length;
					is_incl_error = true;
					break;
				}
			}

			if(is_incl_error)
			{
				msg = QString(QT_TR_NOOP("Failed to parse the file `%1' due to an error in the included file at line `%2'. Error detected:\n\n"))
									.arg(filename).arg(incl_line + 1);
			}

			/* Compile-time if: if the folded expression is not empty
			 * we use it as part of the error message arguments */
			if constexpr(sizeof...(args) > 0)
			{
				msg += Exception::getErrorMessage(err_code)
							 .arg(args...).arg(file).arg(actual_line).arg(column + 1) + " " + extra_msg;
			}
			else
			{
				msg += Exception::getErrorMessage(err_code)
							 .arg(file).arg(actual_line).arg(column + 1) + " " + extra_msg;
			}

			if(is_incl_error)
			{
				column = incl_stmt_len - 1;
				line = incl_line;
			}

			return msg;
		}

	public:
		static const QChar CharComment, //! \brief Character that starts a comment
		CharLineEnd, //! \brief Character that indicates end of line
		CharSpace, //! \brief Character that indicates spacing
		CharTabulation, //! \brief Character that indicates tabulation
		CharStartAttribute, //! \brief Character that indicates a reference to an attribute
		CharEndAttribute, //! \brief Character that delimits on the right the attribute name
		CharStartConditional, //! \brief Character that starts a conditional instruction
		CharStartMetachar, //! \brief Character that starts a metacharacter
		CharStartPlainText, //! \brief Character that starts a plain text
		CharEndPlainText, //! \brief Character that ends a plain text
		CharStartCompExpr, //! \brief Character that starts a comparison expression
		CharEndCompExpr, //! \brief Character that ends a comparison expression
		CharValueDelim, //! \brief Character that delimiters a value (string)
		CharValueOf, //! \brief Character that is used on %set instructions to create an attribute name based upon another attribute value
		CharToXmlEntity, //! \brief Character that is used on attributes, e.g. &{attribute}, to indicate that their content must be converted to xml entities
		CharStartEscaped; //! \brief Character that is used on escapade special characters, e.g. \# \$ \% #\$ \& \] \[ \{ \}

		//! \brief Tokens related to conditional instructions and operators
		static const QString	TokenIf,
		TokenThen,
		TokenElse,
		TokenEnd,
		TokenOr,
		TokenAnd,
		TokenNot,
		TokenSet,
		TokenUnset,
		TokenInclude,

		//! \brief Tokens related to metacharacters
		TokenMetaSp,// $sp (space)
		TokenMetaBr, // $br (line break)
		TokenMetaTb, // $tb (tabulation)
		TokenMetaOb, // $ob (open square bracket '[')
		TokenMetaCb, // $cb (close square bracket ']')
		TokenMetaOc, // $oc (open curly bracket '{')
		TokenMetaCc, // $cc (close curly bracket '}')
		TokenMetaMs, // $ms (money sign '$')
		TokenMetaHs, // $hs (hash/number sign '#')
		TokenMetaPs, // $ps (percentage sign '%')
		TokenMetaAt, // $at (at character '@')
		TokenMetaDs, // $ds (special data separator character '•')
		TokenMetaAm, // $am (ampersand character '&')
		TokenMetaBs, // $bs (backslash character '\')

		//! \brief Tokens related to comparison expressions
		TokenEqOper, // == (equal)
		TokenNeOper, // != (not equal)
		TokenGtOper, // > (greater than)
		TokenLtOper, // < (less than)
		TokenGtEqOper, // >= (greater or equal to)
		TokenLtEqOper; // <= (less or equal to)

		//! \brief Token related to schema files inclusion (@include)
		static const QRegularExpression TokenIncludeRegexp;

		//! \brief Constants used to get a specific object definition
		enum CodeType {
			SqlCode,
			XmlCode
		};

		SchemaParser();

		/*! \brief Set the version of PostgreSQL to be adopted by the parser in obtaining
		 the definition of the objects. This function should always be called at
		 software startup or when the user wants to change the default version
		 of the database */
		void setPgSQLVersion(const QString &pgsql_ver, bool ignore_db_version);

		/*! \brief Returns the complete xml/sql definition for an database object represented by the
		 map 'attributes'. For SQL definition is necessary to indicate the version of PostgreSQL
		 in order to the to correct schema be loaded */
		QString getSourceCode(const QString &obj_name, attribs_map &attribs, CodeType def_type);

		/*! \brief Generic method that loads a schema file and for a given map of attributes
		 this method returns the data of the file analyzed and filled with the values ​​of the
		 attributes map */
		QString getSourceCode(const QString &filename, attribs_map &attribs);

		/*! \brief Generic method that interprets a pre-specified buffer (see loadBuffer()) and for a given map
		 of attributes this method returns the data of the buffer analyzed and filled with the values ​​of the
		 attributes map */
		QString getSourceCode(const attribs_map &attribs);

		//! \brief Loads the schema code from a string buffer
		void loadBuffer(const QString &buf);

		//! \brief Loads a schema file and inserts its line into the parser's buffer
		void loadFile(const QString &filename);

		//! \brief Resets the parser in order to do new analysis
		void restartParser();

		//! \brief Set if the parser must ignore unknown attributes avoiding exception throwing
		void ignoreUnkownAttributes(bool ignore);

		//! \brief Set if the parser must ignore empty attributes avoiding exception throwing
		void ignoreEmptyAttributes(bool ignore);

		//! \brief Retorns the current PostgreSQL version used by the parser
		QString getPgSQLVersion();

		//! \brief Extracts the attributes names from the currently loaded buffer
		QStringList extractAttributes();

		//! \brief Returns the current line where the parser is reading
		int getCurrentLine();

		//! \brief Returns the current columnm of the current line where the parser is reading
		int getCurrentColumn();

		//! \brief Set where the schema parser should look for include files used via @include
		void setSearchPath(const QString &path);

		friend class Catalog;
};

#endif
