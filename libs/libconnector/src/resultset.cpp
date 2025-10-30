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

#include "resultset.h"
#include "exception.h"
#include <QDebug>

ResultSet::ResultSet()
{
	sql_result = nullptr;
	empty_result = false;
	current_tuple = -1;
}

ResultSet::ResultSet(PGresult *sql_result) : ResultSet()
{
	initResultSet(sql_result);
}

ResultSet::~ResultSet()
{
	clearResultSet();
}

void ResultSet::initResultSet(PGresult *sql_res)
{
	if(!sql_res)
		throw Exception(ErrorCode::AsgNotAllocatedSQLResult, PGM_FUNC, PGM_FILE, PGM_LINE);

	int res_state = -1;

	clearResultSet();
	this->sql_result = sql_res;
	res_state = PQresultStatus(sql_res);

	//Handling the status of the result
	switch(res_state)
	{
			//Generating an error in case the server returns an incomprehensible response
			case PGRES_BAD_RESPONSE:
					throw Exception(ErrorCode::IncomprehensibleDBMSResponse, PGM_FUNC, PGM_FILE, PGM_LINE);

			//Generating an error in case the server returns a fatal error
			case PGRES_FATAL_ERROR:
					throw Exception(Exception::getErrorMessage(ErrorCode::DBMSFatalError)
													.arg(PQresultErrorMessage(sql_res)),
													ErrorCode::DBMSFatalError, PGM_FUNC, PGM_FILE, PGM_LINE);

			//In case of sucess states the result will be created
			default:
					/* For any other result set status different from PGRES_TUPLES_OK
					 * we flag the result set as empty since they either return no tuples
					 * or aren't supported at the moment by this class */
					empty_result = res_state != PGRES_TUPLES_OK;
					current_tuple = -1;
			break;
	}
}

void ResultSet::clearResultSet()
{
  if(sql_result)
    PQclear(sql_result);

	//Reset the other attributes
  sql_result = nullptr;
  empty_result = false;
  current_tuple = -1;
}

QString ResultSet::getColumnName(int column_idx)
{
	//Throws an error in case the column index is invalid
	if(column_idx < 0 || column_idx >= getColumnCount())
		throw Exception(ErrorCode::RefTupleColumnInvalidIndex, PGM_FUNC, PGM_FILE, PGM_LINE);

	//Returns the column name on the specified index
	return { PQfname(sql_result, column_idx) };
}

QStringList ResultSet::getColumnNames()
{
	if(isEmpty() || !isValid())
		return {};

	QStringList names;

	for(int col_idx = 0; col_idx < getColumnCount(); col_idx++)
		names.append(QString(PQfname(sql_result, col_idx)));

	return names;
}

unsigned ResultSet::getColumnTypeId(int column_idx)
{
	//Throws an error in case the column index is invalid
	if(column_idx < 0 || column_idx >= getColumnCount())
		throw Exception(ErrorCode::RefTupleColumnInvalidIndex, PGM_FUNC, PGM_FILE, PGM_LINE);

	//Returns the column type id on the specified index
	return static_cast<unsigned>(PQftype(sql_result, column_idx));
}

int ResultSet::getColumnIndex(const QString &column_name)
{
	int col_idx=-1;

	//Get the column index using it's name
	col_idx=PQfnumber(sql_result, column_name.toStdString().c_str());

	/* In case the index is negative indicates that the column doesn't exists in the tuple
		thus an error will be raised */
	if(col_idx < 0)
		throw Exception(ErrorCode::RefTupleColumnInvalidName, PGM_FUNC, PGM_FILE, PGM_LINE);

	return col_idx;
}

int ResultSet::validateColumnName(const QString &column_name)
{
	try
	{
		/* Raises an error if the user try to get the value of a column in
		 a tuple of an empty result or generated from an INSERT, DELETE, UPDATE,
		 that is, which command do not return lines but only do updates or removal */
		if(getTupleCount() == 0 || empty_result)
			throw Exception(ErrorCode::RefInvalidTuple, PGM_FUNC, PGM_FILE, PGM_LINE);

		if(current_tuple < 0 || current_tuple >= getTupleCount())
			throw Exception(ErrorCode::RefInvalidTupleColumn, PGM_FUNC, PGM_FILE, PGM_LINE);

		//Get the column index through its name
		return getColumnIndex(column_name);
	}
	catch(Exception &e)
	{
		//Capture and redirect any generated exception
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

char *ResultSet::getColumnValue(const QString &column_name)
{
	//Returns the column value on the current tuple
	return PQgetvalue(sql_result, current_tuple, validateColumnName(column_name));
}

void ResultSet::validateColumnIndex(int column_idx)
{
	//Raise an error in case the column index is invalid
	if(column_idx < 0 || column_idx >= getColumnCount())
		throw Exception(ErrorCode::RefTupleColumnInvalidIndex, PGM_FUNC, PGM_FILE, PGM_LINE);

	/* Raises an error if the user try to get the value of a column in
		a tuple of an empty result or generated from an INSERT, DELETE, UPDATE,
		that is, which command do not return lines but only do updates or removal */
	if(getTupleCount() == 0 || empty_result)
		throw Exception(ErrorCode::RefInvalidTuple, PGM_FUNC, PGM_FILE, PGM_LINE);

	if(current_tuple < 0 || current_tuple >= getTupleCount())
		throw Exception(ErrorCode::RefInvalidTupleColumn, PGM_FUNC, PGM_FILE, PGM_LINE);
}

char *ResultSet::getColumnValue(int column_idx)
{
	validateColumnIndex(column_idx);

	//Returns the column value on the current tuple
	return PQgetvalue(sql_result, current_tuple, column_idx);
}

bool ResultSet::isColumnValueNull(int column_idx)
{
	validateColumnIndex(column_idx);

	//Returns the null state of the column on the current tuple
	return PQgetisnull(sql_result, current_tuple, column_idx);
}

bool ResultSet::isColumnValueNull(const QString &column_name)
{
	//Returns the null state of the column on the current tuple
	return PQgetisnull(sql_result, current_tuple, validateColumnName(column_name));
}

int ResultSet::getColumnSize(const QString &column_name)
{
	//Returns the column value length on the current tuple
	return PQgetlength(sql_result, current_tuple, validateColumnName(column_name));
}

int ResultSet::getColumnSize(int column_idx)
{
	//Raise an error in case the column index is invalid
	if(column_idx < 0 || column_idx >= getColumnCount())
		throw Exception(ErrorCode::RefTupleColumnInvalidIndex, PGM_FUNC, PGM_FILE, PGM_LINE);

	if(current_tuple < 0 || current_tuple >= getTupleCount())
		throw Exception(ErrorCode::RefInvalidTupleColumn, PGM_FUNC, PGM_FILE, PGM_LINE);

	//Retorns the column value length on the current tuple
	return PQgetlength(sql_result, current_tuple, column_idx);
}

attribs_map ResultSet::getTupleValues()
{
	attribs_map tup_vals;

	if(current_tuple < 0 || current_tuple >= getTupleCount())
		throw Exception(ErrorCode::RefInvalidTuple, PGM_FUNC, PGM_FILE, PGM_LINE);

	for(int col=0; col < getColumnCount(); col++)
		tup_vals[getColumnName(col)]=getColumnValue(col);

	return tup_vals;
}

int ResultSet::getTupleCount()
{
	//In case the result has some tuples
	if(!empty_result)
		//Returns the tuple count gathered after the SQL command
		return PQntuples(sql_result);

	/* Returns the line amount that were affected by the SQL command
		(only for INSERT, DELETE, UPDATE) */
	return atoi(PQcmdTuples(sql_result));
}

int ResultSet::getColumnCount()
{
	return PQnfields(sql_result);
}

int ResultSet::getCurrentTuple()
{
	return current_tuple;
}

bool ResultSet::isColumnBinaryFormat(const QString &column_name)
{
	int col_idx=-1;

	try
	{
		col_idx=getColumnIndex(column_name);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}

	/* Returns the column format in the current tuple.
		According to libpq documentation, value = 0, indicates column text format,
		value = 1 the column has binary format, the other values are reserved */
	return (PQfformat(sql_result, col_idx) == 1);
}

bool ResultSet::isColumnBinaryFormat(int column_idx)
{
	//Raise an error in case the column index is invalid
	if(column_idx < 0 || column_idx >= getColumnCount())
		throw Exception(ErrorCode::RefTupleColumnInvalidIndex, PGM_FUNC, PGM_FILE, PGM_LINE);

	/* Returns the column format in the current tuple.
		According to libpq documentation, value = 0, indicates column text format,
	value = 1 the column has binary format, the other values are reserved.

	One additional check is made, if the type of the column is bytea. */
	return (PQfformat(sql_result, column_idx)==1 || PQftype(sql_result, column_idx)==BYTEAOID);
}

bool ResultSet::accessTuple(TupleId tuple_id)
{
	int tuple_count = getTupleCount();

	/* Raises an error if trying to access the tuple with
	 * an invalid tuple id */
	if(tuple_id > NextTuple)
		throw Exception(ErrorCode::RefInvalidTuple, PGM_FUNC, PGM_FILE, PGM_LINE);

	/* If we have an empty result (generated from a DDL command for example)
	 * or we have no tuples in the result set (generated from a DML but without returned rows) */
	if(empty_result || tuple_count == 0)
		return false;

	bool accessed = true;

	switch(tuple_id)
	{
		case FirstTuple:
			current_tuple = 0;
		break;

		case LastTuple:
			current_tuple = tuple_count-1;
		break;

		case PreviousTuple:
			accessed = (current_tuple > 0);
			if(accessed) current_tuple--;
		break;

		case NextTuple:
			accessed = (current_tuple < (tuple_count-1));
			if(accessed) current_tuple++;
		break;
	}

	return accessed;
}

bool ResultSet::isEmpty()
{
	return empty_result;
}

bool ResultSet::isValid()
{
	return (sql_result != nullptr);
}

