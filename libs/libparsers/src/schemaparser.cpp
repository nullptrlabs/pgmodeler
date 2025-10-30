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

#include "schemaparser.h"
#include "attributes.h"
#include "utilsns.h"
#include "pgsqlversions.h"
#include "globalattributes.h"

const QRegularExpression SchemaParser::AttribRegExp {
	QRegularExpression::anchoredPattern("^([a-z])([a-z]*|(\\d)*|(\\-)*|(_)*)+"),
	QRegularExpression::CaseInsensitiveOption
};

const QChar SchemaParser::CharComment {'#'};
const QChar SchemaParser::CharLineEnd {'\n'};
const QChar SchemaParser::CharSpace {' '};
const QChar SchemaParser::CharTabulation {'\t'};
const QChar SchemaParser::CharStartAttribute {'{'};
const QChar SchemaParser::CharEndAttribute {'}'};
const QChar SchemaParser::CharStartConditional {'%'};
const QChar SchemaParser::CharStartMetachar {'$'};
const QChar SchemaParser::CharStartPlainText {'['};
const QChar SchemaParser::CharEndPlainText {']'};
const QChar SchemaParser::CharStartCompExpr {'('};
const QChar SchemaParser::CharEndCompExpr {')'};
const QChar SchemaParser::CharValueDelim {'"'};
const QChar SchemaParser::CharValueOf {'@'};
const QChar SchemaParser::CharToXmlEntity {'&'};
const QChar SchemaParser::CharStartEscaped {'\\'};

const QString	SchemaParser::TokenIf {"if"};
const QString	SchemaParser::TokenThen {"then"};
const QString	SchemaParser::TokenElse {"else"};
const QString	SchemaParser::TokenEnd {"end"};
const QString	SchemaParser::TokenOr {"or"};
const QString	SchemaParser::TokenAnd {"and"};
const QString	SchemaParser::TokenNot {"not"};
const QString	SchemaParser::TokenSet {"set"};
const QString	SchemaParser::TokenUnset {"unset"};
const QString	SchemaParser::TokenInclude{ QString(SchemaParser::CharValueOf) + "include"};

const QString	SchemaParser::TokenMetaSp {"sp"};
const QString	SchemaParser::TokenMetaBr {"br"};
const QString	SchemaParser::TokenMetaTb {"tb"};
const QString	SchemaParser::TokenMetaOb {"ob"};
const QString	SchemaParser::TokenMetaCb {"cb"};
const QString	SchemaParser::TokenMetaOc {"oc"};
const QString	SchemaParser::TokenMetaCc {"cc"};
const QString	SchemaParser::TokenMetaMs {"ms"};
const QString	SchemaParser::TokenMetaHs {"hs"};
const QString	SchemaParser::TokenMetaPs {"ps"};
const QString	SchemaParser::TokenMetaAt {"at"};
const QString	SchemaParser::TokenMetaDs {"ds"};
const QString	SchemaParser::TokenMetaAm {"am"};
const QString	SchemaParser::TokenMetaBs {"bs"};

const QString	SchemaParser::TokenEqOper {"=="};
const QString	SchemaParser::TokenNeOper {"!="};
const QString	SchemaParser::TokenGtOper {">"};
const QString	SchemaParser::TokenLtOper {"<"};
const QString	SchemaParser::TokenGtEqOper {">="};
const QString	SchemaParser::TokenLtEqOper {"<="};

const QRegularExpression SchemaParser::TokenIncludeRegexp { "^\\s*" +
																													 TokenInclude +
																													 "\\s+([\"])((.*)?)\\1\\s*$",
																													 QRegularExpression::MultilineOption };

SchemaParser::SchemaParser()
{
	line = column = 0;
	ignore_unk_atribs = ignore_empty_atribs = false;
	pgsql_version = PgSqlVersions::DefaulVersion;
}

void SchemaParser::setPgSQLVersion(const QString &pgsql_ver, bool ignore_db_version)
{
	try
	{
		pgsql_version = PgSqlVersions::parseString(pgsql_ver, ignore_db_version);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QString SchemaParser::getPgSQLVersion()
{
	return pgsql_version;
}

QStringList SchemaParser::extractAttributes()
{
	QStringList attribs;
	int start = 0, end = 0;

	for(auto &line : buffer)
	{
		//Find the first occurrence of '{' in the line
		start = line.indexOf(CharStartAttribute, start);

		while(start >= 0 && start < line.size())
		{
			end = line.indexOf(CharEndAttribute, start);

			if(end >= 0)
			{
				//Extract the name between {} and push it into the list
				attribs.push_back(line.mid(start + 1, end - start -1));
				//Start searching new attribute start now from the last position
				start = line.indexOf(CharStartAttribute, end);
			}
			else
				break;
		}

		start = /* end = */ 0;
	}

	attribs.removeDuplicates();
	return attribs;
}

int SchemaParser::getCurrentLine()
{
	return line + 1;
}

int SchemaParser::getCurrentColumn()
{
	return column + 1;
}

void SchemaParser::setSearchPath(const QString &path)
{
	search_path = path;
}

void SchemaParser::restartParser()
{
	/* Clears the buffer and resets the counters for line,
		column and include statement infos */
	buffer.clear();
	attributes.clear();
	include_infos.clear();
	line = column = 0;
}

void SchemaParser::loadBuffer(const QString &buf)
{
	QString buf_aux = buf, lin;
	QTextStream ts(&buf_aux);
	bool open_plain_txt = false;
	QChar prev_chr;
	QString orig_cwd = QDir::currentPath();

	// Prepares the parser to do new reading
	restartParser();

	if(filename.isEmpty())
		filename = QT_TR_NOOP("[memory buffer]");

	if(!search_path.isEmpty())
		QDir::setCurrent(search_path);

	// While the input file doesn't reach the end
	while(!ts.atEnd())
	{
		// Get one line from stream (until the last char before \n)
		ts.readLineInto(&lin);

		/* We need to make a pre processing on plaintext expressions []
		 * in order to check if they have comment char inside them, e.g, [ # foo bar ]
		 *
		 * In positive case, we do not remove it (and the rest of the line as well)
		 * because it would generate syntax error. */
		for(int pos = 0; pos < lin.size(); pos++)
		{
			if(!open_plain_txt && lin[pos] == CharStartPlainText)
				open_plain_txt = true;
			else if(open_plain_txt && lin[pos] == CharEndPlainText)
				open_plain_txt = false;
			else if(lin[pos] == CharComment &&
							// Avoids removing a escaped hash \# wrongly
							prev_chr != CharStartEscaped &&
							!open_plain_txt)
			{
				// We remove the line only if there's no open plaintext expression
				lin.remove(pos, lin.size());
				break;
			}

			prev_chr = lin[pos];
		}

		// Add a line break in case the last character is not
		if(!lin.endsWith(CharLineEnd))
			lin += CharLineEnd;

		try
		{
			/* If we find a @include that is not inside a plaintext statement
			 * we parse it, and in case of successful parsing, we just
			 * jump to the next buffer line which may be the first line of
			 * the included file */
			if(!open_plain_txt && parseInclude(lin, buf_aux, ts.pos()))
				continue;
		}
		catch(Exception &e)
		{
			QDir::setCurrent(orig_cwd);
			throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
		}

		// Add the treated line in the buffer
		buffer.push_back(lin);
		prev_chr = QChar::Null;
	}
}

bool SchemaParser::parseInclude(const QString &include_ln, QString &src_buf, qint64 curr_stream_pos)
{
	QRegularExpressionMatch match = TokenIncludeRegexp.match(include_ln);

	// The line does't contain a well formed include statement
	if(!match.hasMatch() && !include_ln.contains(TokenInclude))
		return false;

	/* Making the current line point to the last line of the actual buffer
	 * and the column point to the last character of the @include "" statement (include_ln),
	 * so in case of any error parsing the include statment we can point the exact
	 * location of the error when calling getCurrentLine and getCurrentColumn */
	line = buffer.size();
	column = include_ln.length() - 1;

	// The line contains a malformed include statement
	if(!match.hasMatch() && include_ln.contains(TokenInclude))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvalidSyntax)
										.arg(filename).arg(getCurrentLine()).arg(getCurrentColumn()) + " " +
										QString(QT_TR_NOOP("Expected a valid include statement in the form `%1 \"file.sch\"'.")).arg(TokenInclude),
										ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	QDir dir;
	QStringList texts = match.capturedTexts();
	QString incl_file = dir.absoluteFilePath(texts.last());
	QFileInfo fi(incl_file);

	/* If the user specificed the include file without the extension
	 * we set the default extension .sch in order to locate the file */
	if(fi.suffix().isEmpty())
		fi.setFile(incl_file + GlobalAttributes::SchemaExt);

	// Check if the included file exists, if not, abort the parsing
	if(!fi.isFile() || !fi.isReadable())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvalidInclude)
										.arg(filename).arg(getCurrentLine()).arg(getCurrentColumn()) + " " +
										QString(QT_TR_NOOP("Include file `%1' not found.")).arg(fi.absoluteFilePath()),
										ErrorCode::InvalidInclude, PGM_FUNC, PGM_FILE, PGM_LINE,
										nullptr, fi.absoluteFilePath());
	}

	try
	{
		// Load the code of the included file
		QString incl_buf = UtilsNs::loadFile(fi.absoluteFilePath());

		/* Appending a line feed char to the loaded include file to avoid
		 * errors when the loaded include is followed by another @include */
		incl_buf.append(QChar::LineFeed);

		/* If the loaded code contains one or more @include statements we abort
		 * the parsing because chained/nested file inclusion is not yet supported. */
		if(incl_buf.contains(TokenIncludeRegexp))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::InvalidInclude)
											.arg(filename).arg(getCurrentLine()).arg(getCurrentColumn()) + " " +
											QString(QT_TR_NOOP("The included file `%1' contains one or more `%2' statements. Nested file inclusion is not currently supported!")).arg(fi.absoluteFilePath(), TokenInclude),
											ErrorCode::InvalidInclude, PGM_FUNC, PGM_FILE, PGM_LINE,
											nullptr, fi.absoluteFilePath());
		}

		src_buf.insert(curr_stream_pos, incl_buf);

		/* Registering the included buffer start and end lines based upon
		 * the current number of lines in the main buffer where @include was found
		 * and by counting the number of line breaks in the included buffer, this is
		 * enough to know where the included buffer starts and ends */
		include_infos.push_back(IncludeInfo { fi.absoluteFilePath(),
																					static_cast<int>(buffer.size()),
																					static_cast<int>(buffer.size() + incl_buf.count(QChar::LineFeed)),
																					static_cast<int>(include_ln.length()) });

		/* Reset the line/column values so the parser can start the parsing of
		 * the loaded buffer as soon as it exits this methods and no lines is left
		 * to be loaded from the source file */
		line = column = 0;

		return true;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void SchemaParser::loadFile(const QString &filename)
{
	if(filename.isEmpty())
		return;

	try
	{
		QString buf(UtilsNs::loadFile(filename));
		setSearchPath(QFileInfo(filename).absolutePath());
		loadBuffer(buf);
		SchemaParser::filename = filename;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QString SchemaParser::getAttribute(bool &found_conv_to_xml)
{
	QString atrib, current_line;
	bool start_attrib, end_attrib, error = false;

	//Get the current line from the buffer
	current_line = buffer[line];

	/* Only start extracting an attribute if it starts with a { or &{
		even if the current character is an attribute delimiter */
	if((current_line[column] != CharStartAttribute &&
			current_line[column] != CharToXmlEntity) ||

		 (current_line[column] == CharToXmlEntity &&
			column + 1 < current_line.length() -  1 &&
			current_line[column + 1] != CharStartAttribute))
	{
		error = true;
	}
	else
	{
		//If the starting char is the & we need to move two chars to reach the starting of the attribute name
		if(current_line[column] == CharToXmlEntity)
		{
			found_conv_to_xml = true;
			column += 2;
		}
		//Otherwise move only one char after {
		else
		{
			found_conv_to_xml = false;
			column++;
		}

		//Marks the flag indicating start of attribute
		start_attrib = true;

		//Unmarks the flag indicating end of attribute
		end_attrib = false;

		/* Attempt to extract an attribute until a space, end of line
		 * or attribute is encountered */
		while(column < current_line.size() &&
					current_line[column]!=CharLineEnd &&
					current_line[column]!=CharSpace &&
					current_line[column]!=CharTabulation &&
					!end_attrib && !error)
		{
			if(current_line[column] != CharEndAttribute)
				atrib += current_line[column];
			else if(current_line[column] == CharEndAttribute && !atrib.isEmpty())
				end_attrib = true;
			else
				error = true;

			column++;
		}

		/* If the attribute has been started but not finished
		 * ie absence of the } in its statement (ie. {attr),
		 * generates an error. */
		if(start_attrib && !end_attrib)
			error = true;
	}

	if(error)
	{
		throw Exception(getParseError(ErrorCode::InvalidSyntax,
										QString(QT_TR_NOOP("Expected a valid attribute token enclosed by `%1%2'."))
										.arg(CharStartAttribute).arg(CharEndAttribute)),
										ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	if(!AttribRegExp.match(atrib).hasMatch())
	{
		throw Exception(getParseError(ErrorCode::InvalidAttribute),
										ErrorCode::InvalidAttribute, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	return atrib;
}

QString SchemaParser::getWord()
{
	QString word, current_line;

	//Gets the current line buffer
	current_line = buffer[line];

	/* Attempt to extract a word if the first character is not
		a special character. */
	if(!isSpecialCharacter(current_line[column]))
	{
		/* Extract the word while it is not end of line, space or
		 * special character */
		while(current_line[column] != CharLineEnd &&
					!isSpecialCharacter(current_line[column]) &&
					current_line[column] != CharSpace &&
					current_line[column] != CharTabulation)
		{
			word+=current_line[column];
			column++;
		}
	}

	return word;
}

QString SchemaParser::getPlainText()
{
	QString text, current_line;
	bool error = false;
	int start_col = column, start_line = line;

	current_line = buffer[line];

	//Attempt to extract a pure text if the first character is a [
	if(current_line[column] == CharStartPlainText)
	{
		//Moves to the next character that contains the beginning of the text
		column++;

		/* Extracts the text while the end of pure text (]), end of buffer or
		 beginning of other pure text ([) is reached */
		while(current_line[column] != CharEndPlainText &&
					line < buffer.size() &&
					current_line[column] != CharStartPlainText)
		{
			text += current_line[column];

			/* In case a escaped start char \ in current position
			 * of plain text expression is found, we convert look for
			 * [ or ] to determined that the user espacted the plain text
			 * operators, e.g., \[ or \], this way we add them in the
			 * extracted text instead of parsing as the start or end
			 * of the expression */
			if(current_line[column] == CharStartEscaped)
			{
				column++;

				/* If the the current char is [ or ] we then proceed with
				 * the conversion of the escaped char */
				if(column < current_line.size() &&
					 (current_line[column] == CharStartPlainText ||
						current_line[column] == CharEndPlainText))
				{
					#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
						text.removeLast();
					#else
						text.remove(text.length() - 1, 1);
					#endif

					text += current_line[column];
					column++;
				}
			}
			/* Special case to end of line. Unlike other elements of
			 * language, a pure text can be extracted until the end of the buffer,
			 * thus, this method also controls the lines transitions */
			else if(current_line[column] == CharLineEnd)
			{
				//Step to the next line
				line++;
				column=0;

				if(line < buffer.size())
					current_line = buffer[line];
			}
			else
				column++;
		}

		if(current_line[column] == CharEndPlainText)
			column++;
		else
			error = true;
	}
	else
		error = true;

	if(error)
	{
		column = start_col;
		line = start_line;

		throw Exception(getParseError(ErrorCode::InvalidSyntax,
										QString(QT_TR_NOOP("Plain text expression is unbalanced or not properly enclosed by `%1%2'."))
										.arg(CharStartPlainText).arg(CharEndPlainText)),
										ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	return text;
}

QString SchemaParser::getConditional()
{
	QString conditional, current_line;
	bool error = false;

	current_line = buffer[line];

	//Will initiate extraction if a % is found
	if(current_line[column] == CharStartConditional)
	{
		// The next char should be the start of the conditional instruction name
		column++;

		/* Moves to the next character that is the beginning of
		 the name of the conditional word */
		while(current_line[column] != CharLineEnd &&
					current_line[column] != CharSpace &&
					current_line[column] != CharTabulation)
		{
			conditional += current_line[column];
			column++;
		}

		//If no word was extracted an error is raised
		if(conditional.isEmpty())
			error = true;
	}
	else
		error = true;

	if(error)
	{
		throw Exception(getParseError(ErrorCode::InvalidSyntax,
										QString(QT_TR_NOOP("Expected a valid conditional instruction token starting with `%1' and followed by at least one letter.")).arg(CharStartConditional)),
										ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	return conditional;
}

QString SchemaParser::getMetaCharacter()
{
	try
	{
		return getMetaOrEscapedToken(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QString SchemaParser::getEscapedCharacter()
{
	try
	{
		return getMetaOrEscapedToken(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QString SchemaParser::getMetaOrEscapedToken(bool is_escaped)
{
	QString chr_token, current_line;
	bool error = false;
	QChar start_chr = is_escaped ? CharStartEscaped : CharStartMetachar;

	current_line = buffer[line];

	//Begins the extraction in case of a $ is found
	if(current_line[column] == start_chr)
	{
		//Moves to the next character that is the beginning of the metacharacter
		column++;

		/* Extracts the metacharacter until doesn't finds a space or end of line
		 * or another char starting another escaped/metachar sequence.
		 * Contiguous escaped sequences are allowed, e.g., \#\s\t\n.
		 * Also, contiguous metachars are allowed (despite the poor legibility), e.g., $br$hs$tb */
		while(current_line[column] != CharLineEnd &&
					current_line[column] != CharSpace &&
					current_line[column] != CharTabulation &&
					current_line[column] != start_chr &&
					((!is_escaped && chr_token.size() < 2) ||
					 (is_escaped && chr_token.isEmpty())))
		{
			chr_token += current_line[column];
			column++;
		}

		//If no metacharacter was extracted an error is raised
		if(chr_token.isEmpty())
			error = true;
	}
	else
		error = true;

	if(error)
	{
		QString extra_msg;

		if(is_escaped)
			extra_msg = QString(QT_TR_NOOP("Expected a valid escaped character token starting with `%1' and followed by a single character.")).arg(start_chr);
		else
			extra_msg = QString(QT_TR_NOOP("Expected a valid metacharacter token starting with `%1' and followed by at least one letter.")).arg(start_chr);

		throw Exception(getParseError(ErrorCode::InvalidSyntax, extra_msg),
										ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	return chr_token;
}

bool SchemaParser::isSpecialCharacter(const QChar &chr)
{
	return chr == CharStartAttribute || chr == CharEndAttribute ||
				 chr == CharStartConditional || chr == CharStartMetachar ||
				 chr == CharStartPlainText || chr == CharEndPlainText ||
				 chr == CharToXmlEntity || chr == CharStartEscaped;
}

bool SchemaParser::evaluateComparisonExpr()
{
	QString curr_line, attrib, value, oper, valid_op_chrs="=!<>fi", extra_error_msg;
	bool error = false, end_eval = false, expr_is_true = true, to_xml_entity = false;
	static QStringList opers = { TokenEqOper, TokenNeOper, TokenGtOper,
															 TokenLtOper, TokenGtEqOper, TokenLtEqOper };
	QChar chr;

	try
	{
		curr_line = buffer[line];
		column++;

		while(!end_eval && !error)
		{
			ignoreBlankChars(curr_line);
			chr = curr_line[column];

			/* If the scan reached the end of the line and the expression was not closed raises an syntax error
			 * Comparison expr must start and end in the same line */
			if(chr == CharLineEnd && !end_eval)
				error = true;

			if(chr == CharToXmlEntity || chr == CharStartAttribute)
			{
				/* Extract the attribute (the first element in the expression) only
				 * if the comparison operator and values aren't extracted */
				if(attrib.isEmpty() && oper.isEmpty() && value.isEmpty())
					attrib = getAttribute(to_xml_entity);
				else
					error = true;
			}
			else if(chr == CharValueDelim)
			{
				/* Extract the value (the last element in the expression) only
				 * if the attribute and operator were extracted */
				if(value.isEmpty() && !attrib.isEmpty() && !oper.isEmpty())
				{
					value += curr_line[column++];

					while(column < curr_line.size())
					{
						value += curr_line[column++];

						if(value.endsWith(CharValueDelim))
							break;
					}

					// If the value is not properly closed by double quotes
					if(!value.isEmpty() && !value.endsWith(CharValueDelim))
						error = true;
				}
				else
					error = true;
			}
			else if(chr == CharEndCompExpr)
			{
				column++;

				//If one of the elements are missing, raise an syntax error
				if(attrib.isEmpty() || oper.isEmpty() || value.isEmpty())
				{
					error = true;
				}
				else if(!opers.contains(QString(oper).remove('f').remove('i')))
				{
					throw Exception(getParseError(ErrorCode::InvalidOperatorInExpression, "", oper),
													ErrorCode::InvalidOperatorInExpression, PGM_FUNC, PGM_FILE, PGM_LINE);
				}
				else if(attributes.count(attrib)==0 && !ignore_unk_atribs)
				{
					throw Exception(getParseError(ErrorCode::UnkownAttribute, "", attrib),
													ErrorCode::UnkownAttribute, PGM_FUNC, PGM_FILE, PGM_LINE);
				}
				else
				{
					QVariant left_val, right_val;
					QString attr_val = to_xml_entity ?
																 UtilsNs::convertToXmlEntities(attributes[attrib]) : attributes[attrib];

					to_xml_entity = false;
					value.remove(CharValueDelim);

					//Evaluating the attribute value against the one captured on the expression without casting
					if(oper.endsWith('f'))
					{
						left_val = QVariant(attr_val.toFloat());
						right_val = QVariant(value.toFloat());
						oper.remove('f');
						expr_is_true = getExpressionResult<float>(oper, left_val, right_val);
					}
					else if(oper.endsWith('i'))
					{
						left_val = QVariant(attr_val.toInt());
						right_val = QVariant(value.toInt());
						oper.remove('i');
						expr_is_true = getExpressionResult<int>(oper, left_val, right_val);
					}
					else
					{
						left_val = QVariant(attr_val);
						right_val = QVariant(value);
						expr_is_true = getExpressionResult<QString>(oper, left_val, right_val);
					}

					end_eval = true;
				}
			}
			else
			{
				/* Extract the operator (the second element in the expression) only
				 * if the attribute was extracted and the value not */
				if(oper.size() <= 3 && !attrib.isEmpty() && value.isEmpty())
				{
					//If the current char is a valid operator capture it otherwise raise an error
					if(valid_op_chrs.indexOf(curr_line[column]) >= 0)
						oper += curr_line[column++];
					else
					{
						error = true;
						extra_error_msg = QString(QT_TR_NOOP("Expected a valid operator token composed of at least two characters from the set `%1'.")).arg(valid_op_chrs);
					}
				}
				else
					error = true;
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),
										PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}

	if(error && !end_eval && extra_error_msg.isEmpty())
		extra_error_msg = QString(QT_TR_NOOP("Expected a valid comparison expression `%1 [attribute] [operator] [value] %2'.")).arg(CharStartCompExpr).arg(CharEndCompExpr);

	if(error)
	{
		throw Exception(getParseError(ErrorCode::InvalidSyntax, extra_error_msg),
										ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	return expr_is_true;
}

void SchemaParser::defineAttribute()
{
	QString curr_line = buffer[line], attrib, value, new_attrib;
	bool error=false, end_def=false, use_val_as_name=false, to_xml_entity = false;
	int curr_ln_idx = line;
	QChar chr;

	try
	{
		while(!end_def && !error)
		{
			ignoreBlankChars(curr_line);
			chr = curr_line[column];

			if(chr == CharLineEnd)
			{
				end_def=true;
			}
			else if(chr == CharValueOf)
			{
				if(!use_val_as_name)
				{
					use_val_as_name = true;
					column++;
					new_attrib = getAttribute(to_xml_entity);
				}
				else
					error = true;
			}
			else if(chr == CharStartConditional)
			{
				error = true;
			}
			else if(chr == CharToXmlEntity || chr == CharStartAttribute)
			{
				if(new_attrib.isEmpty())
					new_attrib = getAttribute(to_xml_entity);
				else
				{
					//Get the attribute in the middle of the value
					attrib = getAttribute(to_xml_entity);

					if(attributes.count(attrib)==0 && !ignore_unk_atribs)
					{
						throw Exception(getParseError(ErrorCode::UnkownAttribute, "", attrib),
														ErrorCode::UnkownAttribute, PGM_FUNC, PGM_FILE, PGM_LINE);
					}

					value += to_xml_entity ? UtilsNs::convertToXmlEntities(attributes[attrib]) : attributes[attrib];
					to_xml_entity = false;
				}
			}
			else if(chr == CharStartPlainText)
			{
				value += getPlainText();

				/* If we finished the extraction of a plain text
				 * in which contains multiple lines, we need to stop
				 * parsing the current line and return the defined
				 * attribute */
				end_def = line != curr_ln_idx;
			}
			else if(chr == CharStartMetachar)
			{
				value += convertMetaCharacter(getMetaCharacter());
			}
			else if(chr == CharStartEscaped)
			{
				value += convertEscapedCharacter(getEscapedCharacter());
			}
			else
			{
				value += getWord();
			}

			//If the attribute name was not extracted yet returns a error
			if(new_attrib.isEmpty())
				error = true;
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}

	if(!error)
	{
		attrib = (use_val_as_name ? attributes[new_attrib] : new_attrib);

		//Checking if the attribute has a valid name
		if(!AttribRegExp.match(attrib).hasMatch())
		{
			throw Exception(getParseError(ErrorCode::InvalidAttribute),
											ErrorCode::InvalidAttribute, PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		/* Creates the attribute in the attribute map of the schema, making the attribute
		 * available on the rest of the script being parsed */
		attributes[attrib] = value;
	}
	else
	{
		throw Exception(getParseError(ErrorCode::InvalidSyntax),
										ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SchemaParser::unsetAttribute()
{
	QString curr_line, attrib;
	bool end_def=false, to_xml_entity = false;
	QChar chr;

	try
	{
		curr_line=buffer[line];

		while(!end_def)
		{
			ignoreBlankChars(curr_line);
			chr = curr_line[column];

			if(chr == CharLineEnd && !attrib.isEmpty())
			{
				end_def = true;
			}
			else if(chr == CharStartAttribute)
			{
				attrib = getAttribute(to_xml_entity);

				if(attributes.count(attrib)==0 && !ignore_unk_atribs)
				{
					throw Exception(getParseError(ErrorCode::UnkownAttribute, "", attrib),
													 ErrorCode::UnkownAttribute, PGM_FUNC, PGM_FILE, PGM_LINE);
				}

				if(!AttribRegExp.match(attrib).hasMatch())
				{
					throw Exception(getParseError(ErrorCode::InvalidAttribute, "", attrib),
													 ErrorCode::InvalidAttribute, PGM_FUNC, PGM_FILE, PGM_LINE);
				}

				attributes[attrib]="";
			}
			else
			{
				throw Exception(getParseError(ErrorCode::InvalidSyntax),
												 ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),
										PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}
bool SchemaParser::evaluateExpression()
{
	QString current_line, cond, attrib, prev_cond;
	bool error = false, end_eval = false, expr_is_true = true,
			attrib_true = true, comp_true = true, to_xml_entity = false;
	unsigned attrib_count = 0, and_or_count = 0;
	QChar chr;

	try
	{
		current_line = buffer[line];

		while(!end_eval && !error)
		{
			ignoreBlankChars(current_line);

			if(current_line[column] == CharLineEnd)
			{
				line++;

				if(line < buffer.size())
				{
					current_line = buffer[line];
					column = 0;
					ignoreBlankChars(current_line);
				}
				else if(!end_eval)
					error = true;
			}

			chr = current_line[column];

			//Extract the next conditional token
			if(chr == CharStartConditional)
			{
				prev_cond = cond;
				cond = getConditional();

				//Error 1: %if {a} %or %or %then
				error=(cond == prev_cond ||
							//Error 2: %if {a} %and %or %then
							(cond == TokenAnd && prev_cond == TokenOr) ||
							//Error 3: %if {a} %or %and %then
							(cond == TokenOr && prev_cond == TokenAnd) ||
							//Error 4: %if %and {a} %then
							(attrib_count == 0 && (cond == TokenAnd || cond == TokenOr)));

				if(cond == TokenThen)
				{
					/* Returns the parser to the token %then because additional
					 * operations is done whe this token is found */
					column -= cond.length() + 1;
					end_eval = true;

					//Error 1: %if {a} %not %then
					error = (prev_cond == TokenNot ||
								//Error 2: %if %then
								attrib_count == 0 ||
								//Error 3: %if {a} %and %then
								(and_or_count != attrib_count-1));
				}
				else if(cond == TokenOr || cond == TokenAnd)
					and_or_count++;
			}
			else if(chr == CharToXmlEntity || chr == CharStartAttribute)
			{
				attrib = getAttribute(to_xml_entity);

							 //Raises an error if the attribute does is unknown
				if(attributes.count(attrib)==0 && !ignore_unk_atribs)
				{
					throw Exception(getParseError(ErrorCode::UnkownAttribute, "", attrib),
													ErrorCode::UnkownAttribute, PGM_FUNC, PGM_FILE, PGM_LINE);
				}

				//Error 1: A conditional token other than %or %not %and if found on conditional expression
				error=(!cond.isEmpty() && cond != TokenOr && cond != TokenAnd && cond != TokenNot) ||
								//Error 2: A %not token if found after an attribute: %if {a} %not %then
								(attrib_count > 0 && cond == TokenNot && prev_cond.isEmpty()) ||
								//Error 3: Two attributes not separated by any conditional token: %if {a} {b} %then
								(attrib_count > 0 && cond.isEmpty());

				//Increments the extracted attribute counter
				attrib_count++;

				if(!error)
				{
					//Appliyng the NOT operator if found
					attrib_true = (cond == TokenNot ? attributes[attrib].isEmpty() : !attributes[attrib].isEmpty());

					//Executing the AND operation if the token is found
					if(cond == TokenAnd || prev_cond == TokenAnd)
						expr_is_true=(expr_is_true && attrib_true);
					else if(cond == TokenOr || prev_cond == TokenOr)
						expr_is_true = (expr_is_true || attrib_true);
					else
						expr_is_true = attrib_true;

					cond.clear();
					prev_cond.clear();
				}
			}
			else if(chr == CharStartCompExpr)
			{
				comp_true = evaluateComparisonExpr();

				//Appliyng the NOT operator if found
				if(cond == TokenNot)
					comp_true = !comp_true;

				//Executing the AND operation if the token is found
				if(cond == TokenAnd || prev_cond == TokenAnd)
					expr_is_true = (expr_is_true && comp_true);
				else if(cond == TokenOr || prev_cond == TokenOr)
					expr_is_true = (expr_is_true || comp_true);
				else
					expr_is_true = comp_true;

				//Consider the comparison expression as an attribute evaluation
				attrib_count++;
				cond.clear();
				prev_cond.clear();
			}
			else
			{
				error=true;
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),	PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}

	if(error)
	{
		throw Exception(getParseError(ErrorCode::InvalidSyntax),
										ErrorCode::InvalidSyntax,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	return expr_is_true;
}

void SchemaParser::ignoreBlankChars(const QString &line)
{
	while(column < line.size() &&
				(line[column]==CharSpace ||
				 line[column]==CharTabulation)) column++;
}

QString SchemaParser::convertMetaCharacter(const QString &meta)
{
	static attribs_map metas={{ TokenMetaSp, CharSpace },
														{ TokenMetaTb, CharTabulation },
														{ TokenMetaBr, CharLineEnd },
														{ TokenMetaOb, CharStartPlainText },
														{ TokenMetaCb, CharEndPlainText },
														{ TokenMetaOc, CharStartAttribute },
														{ TokenMetaCc, CharEndAttribute },
														{ TokenMetaMs, CharStartMetachar },
														{ TokenMetaHs, CharComment },
														{ TokenMetaPs, CharStartConditional },
														{ TokenMetaAt, CharValueOf },
														{ TokenMetaDs, UtilsNs::DataSeparator },
														{ TokenMetaAm, CharToXmlEntity },
														{ TokenMetaBs, CharStartEscaped }};

	if(metas.count(meta) == 0)
	{
		throw Exception(getParseError(ErrorCode::InvalidMetacharacter, "", CharStartMetachar + meta),
										ErrorCode::InvalidMetacharacter, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	return metas.at(meta);
}

QString SchemaParser::convertEscapedCharacter(const QString &escaped)
{
	static QStringList esc_chars = {
		"s", /* CharSpace */
		"t", /* CharTabulation */
		"n", /* CharLineEnd */
		CharStartPlainText,
		CharEndPlainText,
		CharStartAttribute,
		CharEndAttribute,
		CharStartMetachar,
		CharComment,
		CharStartConditional,
		CharValueOf,
		CharToXmlEntity,
		CharStartEscaped
	};

	static QStringList non_print_chars = {
		CharSpace,
		CharTabulation,
		CharLineEnd
	};

	/* Special case for escaped \* which returns the special
	 * character UtilsNs::DataSeparator (•) */
	if(escaped == "*")
		return UtilsNs::DataSeparator;

	int idx = esc_chars.indexOf(escaped);

	if(idx < 0)
	{
		throw Exception(getParseError(ErrorCode::InvalidEscapedCharacter, "", CharStartEscaped + escaped),
										ErrorCode::InvalidEscapedCharacter, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	if(idx <= 2)
		return non_print_chars[idx];

	return esc_chars[idx];
}

QString SchemaParser::convertMetaOrEscaped(const QString &token, bool is_escaped)
{
	if(is_escaped)
		return convertEscapedCharacter(token);

	return convertMetaCharacter(token);
}

QString SchemaParser::getSourceCode(const QString & obj_name, attribs_map &attribs, CodeType def_type)
{
	try
	{
		QString filename;

		if(def_type == SqlCode)
		{
			filename = GlobalAttributes::getSchemaFilePath(GlobalAttributes::SQLSchemaDir, obj_name);
			attribs[Attributes::PgSqlVersion] = pgsql_version;
		}
		else
			filename = GlobalAttributes::getSchemaFilePath(GlobalAttributes::XMLSchemaDir, obj_name);

#ifdef DEMO_VERSION
		#warning "DEMO VERSION: truncating generated SQL code."
		QString code_def = getSourceCode(filename, attribs);

		if(def_type == SchemaParser::SqlCode && !code_def.isEmpty() &&
			 obj_name != Attributes::DbModel)
		{
			bool ends_with_lf = code_def.endsWith("\n");
			code_def = code_def.mid(0, code_def.size() * 0.60);
			code_def += "...";
			code_def += ends_with_lf ? "\n" : "";
		}

		return code_def;
#else
		//Try to get the object definitin from the specified path
		return getSourceCode(filename, attribs);
#endif
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),	PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void SchemaParser::ignoreUnkownAttributes(bool ignore)
{
	ignore_unk_atribs=ignore;
}

void SchemaParser::ignoreEmptyAttributes(bool ignore)
{
	ignore_empty_atribs=ignore;
}

QString SchemaParser::getSourceCode(const attribs_map &attribs)
{
	QString object_def;
	unsigned end_cnt = 0, if_cnt = 0;
	int if_level = -1, prev_if_level = -1;
	QString atrib, cond, prev_cond, word, meta;
	bool error = false, if_expr = false, to_xml_entity = false;
	QChar chr;
	std::vector<bool> vet_expif, vet_tk_if, vet_tk_then, vet_tk_else;
	std::map<int, std::vector<QString> > if_map, else_map;
	std::vector<QString>::iterator itr, itr_end;
	std::vector<int> vet_prev_level;
	std::vector<QString> *vet_aux;

	//In case the file was successfuly loaded
	if(buffer.size() > 0)
	{
		//Init the control variables
		attributes = attribs;
		error = if_expr = false;
		if_level = -1;
		end_cnt = if_cnt = 0;

		while(line < buffer.size())
		{
			chr = buffer[line][column];

			/* Increments the number of rows causing the parser
			to get the next line buffer for analysis */
			if(chr == CharLineEnd)
			{
				line++;
				column = 0;
			}
			else if(chr == CharTabulation || chr == CharSpace)
			{
				//The parser will ignore the spaces that are not within pure texts
				ignoreBlankChars(buffer[line]);
			}
			//Metacharacter extraction
			else if(chr == CharStartMetachar || chr == CharStartEscaped)
			{
				meta = getMetaOrEscapedToken(chr == CharStartEscaped);

				//Checks whether the metacharacter is part of the  'if' expression (this is an error)
				if(if_level>=0 && vet_tk_if[if_level] && !vet_tk_then[if_level])
				{
					throw Exception(getParseError(ErrorCode::InvalidSyntax),
													ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
				}

				//Converting the metacharacter/escaped char to the character that it represents
				meta = convertMetaOrEscaped(meta, chr == CharStartEscaped);

				//If the parser is inside an 'if / else' extracting tokens
				if(if_level>=0)
				{
					/* If the parser is in 'if' section,
						 places the metacharacter on the word map of the current 'if' */
					if(vet_tk_if[if_level] &&
							vet_tk_then[if_level] &&
							!vet_tk_else[if_level])
						if_map[if_level].push_back(meta);

					/* If the parser is in 'else' section,
					 * places the metacharacter on the word map of the current 'else'*/
					else if(vet_tk_else[if_level])
						else_map[if_level].push_back(meta);
				}
				else
					/* If the parsers is not in a 'if / else', puts the metacharacter
					 * in the definition sql */
					object_def += meta;
			}
			//Attribute extraction
			else if(chr == CharToXmlEntity ||
							chr == CharStartAttribute ||
							chr == CharEndAttribute)
			{
				atrib = getAttribute(to_xml_entity);

				//Checks if the attribute extracted belongs to the passed list of attributes
				if(attributes.count(atrib) == 0)
				{
					if(!ignore_unk_atribs)
					{
						throw Exception(getParseError(ErrorCode::UnkownAttribute, "", atrib),
														ErrorCode::UnkownAttribute, PGM_FUNC, PGM_FILE, PGM_LINE);
					}

					attributes[atrib]	= "";
				}

				//If the parser is inside an 'if / else' extracting tokens
				if(if_level >= 0)
				{
					//If the parser evaluated the 'if' conditional and is inside the current if block
					if(!(!if_expr && vet_tk_if[if_level] && !vet_tk_then[if_level]))
					{
						word = atrib;
						atrib = "";

						if(to_xml_entity)
							atrib += CharToXmlEntity;

						atrib += CharStartAttribute;
						atrib += word;
						atrib += CharEndAttribute;

									 //If the parser is in the 'if' section
						if(vet_tk_if[if_level] &&	vet_tk_then[if_level] &&	!vet_tk_else[if_level])
							//Inserts the attribute value in the map of the words of current the 'if' section
							if_map[if_level].push_back(atrib);
						else if(vet_tk_else[if_level])
							//Inserts the attribute value in the map of the words of current the 'else' section
							else_map[if_level].push_back(atrib);
					}
				}
				else
				{
					/* If the attribute has no value set and parser must not ignore empty values
					 * raises an exception */
					if(attributes[atrib].isEmpty() && !ignore_empty_atribs)
					{
						throw Exception(getParseError(ErrorCode::UndefinedAttributeValue, "", atrib),
														ErrorCode::UndefinedAttributeValue, PGM_FUNC, PGM_FILE, PGM_LINE);
					}

					/* If the parser is not in an if / else, concatenates the value of the attribute
					 * directly in definition in sql */
					object_def += to_xml_entity ? UtilsNs::convertToXmlEntities(attributes[atrib]) : attributes[atrib];
					to_xml_entity = false;
				}
			}
			//Conditional instruction extraction
			else if(chr == CharStartConditional)
			{
				prev_cond = cond;
				cond = getConditional();

				//Checks whether the extracted token is a valid conditional
				if(cond != TokenIf && cond != TokenElse &&
					 cond != TokenThen && cond != TokenEnd &&
					 cond != TokenOr && cond != TokenNot &&
					 cond != TokenAnd && cond != TokenSet &&
					 cond != TokenUnset)
				{
					throw Exception(getParseError(ErrorCode::InvalidInstruction, "", cond),
													ErrorCode::InvalidInstruction, PGM_FUNC, PGM_FILE, PGM_LINE);
				}

				if(cond == TokenSet || cond == TokenUnset)
				{
					bool extract = false;

					/* Extracts or unset the attribute only if the process is not in the middle of a 'if-then-else' or
						if the parser is inside the 'if' part and the expression is evaluated as true, or in the 'else' part
						and the related 'if' is false. Otherwise the line where %set is located will be completely ignored */
					extract=(if_level < 0 || vet_expif.empty());

					if(!extract && if_level >= 0)
					{
						//If in 'else' the related 'if' is false, extracts the attribute
						if(prev_cond == TokenElse && !vet_expif[if_level])
							extract = true;
						else if(prev_cond != TokenElse)
						{
							//If in the 'if' part all the previous ifs until the current must be true
							extract = true;
							for(int i = 0; i <= if_level && extract; i++)
							{
								extract=(vet_expif[i] && !vet_tk_else[i]) ||
													(!vet_expif[i] && vet_tk_else[i]);
							}
						}
					}

					if(extract)
					{
						if(cond == TokenSet)
							defineAttribute();
						else
							unsetAttribute();
					}
					else
					{
						ignoreBlankChars(buffer[line]);

						/* When the %set instruction is ignored due to the fact of it being under a if expression evaluated as false
						 * there's the need to create an empty representation of it in the set of attributes so in further expressions
						 * evaluations the parser isn't broke by a unknow attribute exception */
						atrib = getAttribute(to_xml_entity);

						if(attributes.count(atrib) == 0)
							attributes[atrib] = "";

						column = 0;
						line++;
						atrib.clear();
					}
				}
				else
				{
					//If the toke is an 'if'
					if(cond==TokenIf)
					{
						//Evaluates the if expression storing the result on the vector
						if_expr = true;
						vet_expif.push_back(evaluateExpression());

						/* Inserts the value of the current 'if' level  in the previous 'if' levels vector.
						 This vector is used to know which 'if' the parser was in before entering current if */
						vet_prev_level.push_back(if_level);

						/* Mark the flags indicating that an  'if' was found and a
						 * 'then' and 'else' have not been found yet */
						vet_tk_if.push_back(true);
						vet_tk_then.push_back(false);
						vet_tk_else.push_back(false);

						//Defines the current if level as the size of list 'if' tokens found  -1
						if_level = (vet_tk_if.size()-1);

						//Increases the number of found 'if's
						if_cnt++;
					}
					//If the parser is in 'if / else' and one 'then' token is found
					else if(cond == TokenThen && if_level >= 0)
					{
						//Marks the then token flag of the current 'if'
						vet_tk_then[if_level]=true;

						/* Clears the  expression extracted flag from the 'if - then',
						 * so that the parser does not generate an error when it encounters another
						 * 'if - then' with an expression still unextracted */
						if_expr = false;
					}
					//If the parser is in 'if / else' and a 'else' token is found
					else if(cond == TokenElse && if_level >= 0)
						//Mark the  o flag do token else do if atual
						vet_tk_else[if_level] = true;
					//Case the parser is in 'if/else' and a 'end' token was found
					else if(cond == TokenEnd && if_level >= 0)
					{
						//Increments the number of 'end' tokes found
						end_cnt++;

									 //Get the level of the previous 'if' where the parser was
						prev_if_level = vet_prev_level[if_level];

									 //In case the current 'if' be internal (nested) (if_level > 0)
						if(if_level > 0)
						{
							//In case the current 'if' doesn't in the 'else' section of the above 'if' (previous if level)
							if(!vet_tk_else[prev_if_level])
								//Get the extracted word vector on the above 'if'
								vet_aux=&if_map[prev_if_level];
							else
								//Get the extracted word vector on the above 'else'
								vet_aux=&else_map[prev_if_level];
						}
						else
							vet_aux = nullptr;

						/* Initializes the iterators to scan
							 the auxiliary vector if necessary */
						itr=itr_end=if_map[0].end();

						/* In case the expression of the current 'if' has the value true
						 * then the parser will scan the list of words on the 'if' part of the
						 * current 'if' */
						if(vet_expif[if_level])
						{
							itr = if_map[if_level].begin();
							itr_end = if_map[if_level].end();
						}

						/* If there is a 'else' part on the current 'if'
						 * then the parser will scan the list of words on the 'else' part */
						else if(else_map.count(if_level)>0)
						{
							itr = else_map[if_level].begin();
							itr_end = else_map[if_level].end();
						}

						/* This iteration scans the list of words selected above
						 * inserting them in 'if' part  of the 'if' or 'else' superior to current.
						 * This is done so that only the words extracted based on
						 * ifs expressions of the buffer are embedded in defining sql */
						while(itr!=itr_end)
						{
							//If the auxiliary vector is allocated, inserts the word on above 'if / else'
							if(vet_aux)
								vet_aux->push_back((*itr));
							else
							{
								word=(*itr);

											 //Check if the word is not an attribute
								if(!word.isEmpty() &&
										(word.startsWith(CharStartAttribute) ||
										 word.startsWith(CharToXmlEntity)) &&
										word.endsWith(CharEndAttribute))
								{
									//If its an attribute, extracts the name between { } and checks if the same has empty value
									bool conv_entity = word.startsWith(CharToXmlEntity);
									int pos = conv_entity ? 2 : 1;

									atrib = word.mid(pos, word.size() - (pos + 1));
									word = conv_entity ? UtilsNs::convertToXmlEntities(attributes[atrib]) : attributes[atrib];

									/* If the attribute has no value set and parser must not ignore empty values
									 * raises an exception */
									if(word.isEmpty() && !ignore_empty_atribs)
									{
										throw Exception(getParseError(ErrorCode::UndefinedAttributeValue, "", atrib),
																		ErrorCode::UndefinedAttributeValue, PGM_FUNC, PGM_FILE, PGM_LINE);
									}
								}

								//Else, insert the word directly on the object definition
								object_def += word;
							}

							itr++;
						}

						//Case the current if is nested (internal)
						if(if_level > 0)
						{
							//Causes the parser to return to the earlier 'if'
							if_level=prev_if_level;
						}
						/* In case the 'if' be the uppermost (level 0) indicates that all
						 * the if's  has already been checked, so the parser will clear the
						 * used auxiliary structures*/
						else
						{
							if_map.clear();
							else_map.clear();
							vet_tk_if.clear();
							vet_tk_then.clear();
							vet_tk_else.clear();
							vet_expif.clear();
							vet_prev_level.clear();

							//Resets the ifs levels
							if_level = /* prev_if_level = */ -1;
						}
					}
					else
						error=true;

					if(!error)
					{
						/* Verifying that the conditional words appear in a valid  order if not
						 * the parser generates an error. Correct order means IF before THEN,
						 * ELSE after IF and before END */
						if((prev_cond == TokenIf && cond != TokenThen) ||
								(prev_cond == TokenElse && cond != TokenIf && cond != TokenEnd) ||
								(prev_cond == TokenThen && cond == TokenThen))
							error = true;
					}

					if(error)
					{
						throw Exception(getParseError(ErrorCode::InvalidSyntax),
														ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
					}
				}
			}
			//Extraction of pure text or simple words
			else
			{
				if(chr == CharStartPlainText || chr == CharEndPlainText)
					word = getPlainText();
				else
					word = getWord();

				//Case the parser is in 'if/else'
				if(if_level>=0)
				{
					/* In case the word/text be inside 'if' expression, the parser returns an error
					 * because only an attribute must be on the 'if' expression  */
					if(vet_tk_if[if_level] && !vet_tk_then[if_level])
					{
						throw Exception(getParseError(ErrorCode::InvalidSyntax),
														ErrorCode::InvalidSyntax, PGM_FUNC, PGM_FILE, PGM_LINE);
					}

					//Case the parser is in 'if' section
					if(vet_tk_if[if_level] &&
						 vet_tk_then[if_level] &&
						 !vet_tk_else[if_level])
					{
						//Inserts the word on the words map extracted on 'if' section
						if_map[if_level].push_back(word);
					}
					else if(vet_tk_else[if_level])
					{
						//Inserts the word on the words map extracted on 'else' section
						else_map[if_level].push_back(word);
					}
				}
				else
					//Case the parser is not in 'if/else' concatenates the word/text directly on the object definition
					object_def+=word;
			}
		}

		/* If has more 'if' toknes than  'end' tokens, this indicates that some 'if' in code
		 * was not closed thus the parser returns an error */
		if(if_cnt!=end_cnt)
		{
			throw Exception(getParseError(ErrorCode::InvalidSyntax),
											ErrorCode::InvalidSyntax, PGM_FUNC , PGM_FILE, PGM_LINE);
		}
	}

	restartParser();
	ignore_unk_atribs = false;
	ignore_empty_atribs = false;
	return object_def;
}

QString SchemaParser::getSourceCode(const QString &filename, attribs_map &attribs)
{
	try
	{
		loadFile(filename);
		attribs[Attributes::PgSqlVersion]=pgsql_version;
		return getSourceCode(attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

template<typename Type>
bool SchemaParser::getExpressionResult(const QString &oper, const QVariant &left_val, const QVariant &right_val){
	return ((oper==TokenEqOper && (left_val.value<Type>() == right_val.value<Type>())) ||
					(oper==TokenNeOper && (left_val.value<Type>() != right_val.value<Type>())) ||
					(oper==TokenGtOper && (left_val.value<Type>() > right_val.value<Type>())) ||
					(oper==TokenLtOper && (left_val.value<Type>() < right_val.value<Type>())) ||
					(oper==TokenGtEqOper && (left_val.value<Type>() >= right_val.value<Type>())) ||
					(oper==TokenLtEqOper && (left_val.value<Type>() <= right_val.value<Type>())));
}
