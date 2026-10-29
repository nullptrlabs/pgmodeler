#include "csvdocument.h"
#include "utilsns.h"
#include "exception.h"

const QChar CsvDocument::TextDelimiter {'"'};
const QChar CsvDocument::Separator {';'};
const QChar CsvDocument::LineBreak { QChar::LineFeed };

CsvDocument::CsvDocument()
{
	setSpecialChars(Separator, TextDelimiter, LineBreak);
}

CsvDocument::CsvDocument(const QChar &sep, const QChar &txt_delim, const QChar &ln_break)
{
	setSpecialChars(sep, txt_delim, ln_break);
}

int CsvDocument::getRowCount() const
{
	return rows.size();
}

int CsvDocument::getColumnCount() const
{
	if(!columns.isEmpty())
		return columns.size();

	if(!rows.isEmpty())
		return rows.at(0).size();

	return 0;
}

QStringList CsvDocument::getColumnNames() const
{
	return columns;
}

void CsvDocument::setColumns(const QStringList &cols)
{
	columns = cols;
}

void CsvDocument::addRow(const QStringList &row)
{
	if(row.isEmpty())
		return;

	if(getColumnCount() > 0 && row.size() != getColumnCount())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::MalformedCsvInvalidCols)
										.arg(getColumnCount()).arg(rows.size() + 1).arg(row.size()),
										ErrorCode::MalformedCsvInvalidCols, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	rows.append(row);
}

void CsvDocument::setSpecialChars(const QChar &sep, const QChar &txt_delim, const QChar &ln_break)
{
	if(sep == txt_delim || sep == ln_break || ln_break == txt_delim)
		throw Exception(ErrorCode::InvCsvParserOptions, PGM_FUNC, PGM_FILE, PGM_LINE);

	separator = sep;
	text_delim = txt_delim;
	line_break = ln_break;
}

QString CsvDocument::getValue(int row, int col) const
{
	if(row >= rows.size() || col >= getColumnCount())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::RefInvCsvDocumentValue).arg(row).arg(col),
										ErrorCode::RefInvCsvDocumentValue, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	return rows.at(row).at(col);
}

bool CsvDocument::isEmpty() const
{
	return rows.isEmpty();
}

void CsvDocument::saveToFile(const QString &filename) const
{
	try
	{
		QByteArray buffer;
		QString data;
		QList<QStringList> rows = this->rows;

		rows.prepend(columns);

		for(auto &row : rows)
		{
			row.replaceInStrings(text_delim, QString("%1%1").arg(text_delim));

			for(auto &val : row)
			{
				val.prepend(text_delim);
				val.append(text_delim);
			}

			data.append(row.join(separator));
			data.append(line_break);
			buffer.append(data.toUtf8());
			data.clear();
		}

		UtilsNs::saveFile(filename, buffer);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}
