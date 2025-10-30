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
\ingroup libconnector
\class ResultSet
\brief This class encapsulates some functions implemented by libpq for manipulation of resultsets returned by PostgreSQL SQL commands.
It the resultset contains data the user must call ResultSet::accessTuple() to access the first tuple in order to perform operations on it.
\note <strong>Creation date:</strong> 14/07/2009
*/

#ifndef RESULTSET_H
#define RESULTSET_H

#include "connectorglobal.h"
#include "attribsmap.h"
#include <libpq-fe.h>
#include <cstdlib>

//This constant is defined on PostgreSQL source code src/catalog/pg_type.h
#define BYTEAOID 17

class __libconnector ResultSet {
	private:
		void validateColumnIndex(int column_idx);

		int validateColumnName(const QString &column_name);

        void initResultSet(PGresult *sql_res);

        void clearResultSet();

    protected:
		//! \brief Stores the current tuple index, just for navigation
		int current_tuple;

		/*! \brief Indicates that the result was generated from a command
		 * which generates no tuples. Example: INSERT, DELETE, CREATE .. */
		bool empty_result;

		//! \brief Stores the result object of a SQL command
		PGresult *sql_result;

        /*! \brief This class may be constructed from a
         * result of SQL command generated in Connection class.
         * The ResultSet takes the ownership of the provided sql_result,
         * so there's no need to call PQclear() over it after instantiate
         * an object of this class. When destroyed, the instance will
         * free the PGresult. (see clearResultSet) */
		ResultSet(PGresult *sql_result);

	public:
		//! \brief Constants used to navigate through the resultset
		enum TupleId: unsigned {
			FirstTuple = 0,
			LastTuple = 1,
			PreviousTuple = 2,
			NextTuple = 3
		};

		ResultSet();

         /*! \brief Destroys the result set using clearResultSet.
          * The internal PostgreSQL result set (sql_result) is automatically freed */
		~ResultSet();

		//! \brief Returns the value of a column (searching by name or index)
		char *getColumnValue(const QString &column_name);
		char *getColumnValue(int column_idx);

		//! \brief Returns the data allocated size of a column (searching by name or index)
		int getColumnSize(const QString &column_name);
		int getColumnSize(int column_idx);

		//! \brief Returns all the column names / values for the current tuple.
		attribs_map getTupleValues();

        /*! \brief Returns the number of rows affected by the command that generated
         * the result if it is an INSERT, DELETE, UPDATE or the number of
         * tuples returned if the command was a SELECT */
		int getTupleCount();

		//! \brief Returns the column count present in one tuple
		int getColumnCount();

		//! \brief Returns the name of the column specified by it's index
		QString getColumnName(int column_idx);

		//! \brief Returns a list containing the name of the columns in this result set
		QStringList getColumnNames();

		//! \brief Returns the type OID of the column specified by it's index
		unsigned getColumnTypeId(int column_idx);

		//! \brief Returns the index of the column specified by it's name
		int getColumnIndex(const QString &column_name);

		//! \brief Returns the current tuple where the navigation is
		int getCurrentTuple();

		//! \brief Informs if the column is in binary format
		bool isColumnBinaryFormat(const QString &column_name);
		bool isColumnBinaryFormat(int column_idx);

		//! \brief Informs if the column has a null value. In PostgreSQL null =/= empty
		bool isColumnValueNull(int column_idx);
		bool isColumnValueNull(const QString &column_name);

		//! \brief Access on tuple on result set via navigation constants
		bool accessTuple(TupleId tuple_id);

		//! \brief Returns if the result set is empty due a DML command that does not returned any data
		bool isEmpty();

		//! \brief Returns if the result set is valid (created from a valid result set)
		bool isValid();

		ResultSet &operator = (const PGresult *) = delete;
		ResultSet &operator = (const ResultSet &) = delete;

		friend class Connection;
};

#endif
