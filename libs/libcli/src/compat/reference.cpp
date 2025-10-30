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

#include "reference.h"

using namespace CompatNs;

Reference::Reference()
{
	this->table=nullptr;
	this->column=nullptr;
	this->is_def_expr=false;
}

Reference::Reference(PhysicalTable *table, Column *column, const QString &tab_alias, const QString &col_alias)
{
	//Raises an error if the table is not allocated
	if(!table)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the table/column alias has an invalid name
	if((!tab_alias.isEmpty() && !BaseObject::isValidName(tab_alias)) ||
			(!col_alias.isEmpty() && !BaseObject::isValidName(col_alias)))
		throw Exception(ErrorCode::AsgInvalidNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the column parent table differs from the passed table
	if(column && column->getParentTable()!=table)
		throw Exception(ErrorCode::AsgObjectBelongsAnotherTable ,PGM_FUNC,PGM_FILE,PGM_LINE);

	this->table=table;
	this->column=column;
	this->alias=tab_alias;
	this->column_alias=col_alias;
	this->is_def_expr=false;
}

Reference::Reference(const QString &expression, const QString &expr_alias)
{
	//Raises an error if the user try to create an reference using an empty expression
	if(expression.isEmpty())
		throw Exception(ErrorCode::AsgInvalidExpressionObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the expression alias has an invalid name
	if(!expr_alias.isEmpty() && !BaseObject::isValidName(expr_alias))
		throw Exception(ErrorCode::AsgInvalidNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	table=nullptr;
	column=nullptr;
	alias=expr_alias;
	this->expression=expression;
	this->is_def_expr=false;
}

void Reference::setDefinitionExpression(bool value)
{
	is_def_expr = value;
	if(!value) clearReferencedTables();
}

void Reference::addReferencedTable(PhysicalTable *ref_table)
{
	if(!ref_table)
		return;

	if(std::find(ref_tables.begin(), ref_tables.end(), ref_table) == ref_tables.end())
		ref_tables.push_back(ref_table);
}

int Reference::getReferencedTableIndex(PhysicalTable *ref_table)
{
	int idx = -1;
	std::vector<PhysicalTable *>::iterator itr = std::find(ref_tables.begin(), ref_tables.end(), ref_table);

	if(itr != ref_tables.end())
		idx = itr - ref_tables.begin();

	return idx;
}

void Reference::clearReferencedTables()
{
	ref_tables.clear();
}

std::vector<PhysicalTable *> Reference::getReferencedTables()
{
	return ref_tables;
}

bool Reference::isDefinitionExpression()
{
	return is_def_expr;
}

void Reference::addColumn(const QString &name, PgSqlType type, const QString &alias)
{
	QString aux_name = name;

	if(aux_name.startsWith(QChar('\"')) &&
		 aux_name.endsWith(QChar('\"')))
	{
		aux_name.remove(0, 1);
		aux_name.remove(aux_name.length(), 1);
	}

	// Validating the column name
	if(!BaseObject::isValidName(name))
	{
		if(aux_name.isEmpty())
			throw Exception(ErrorCode::AsgEmptyNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(aux_name.size() > BaseObject::ObjectNameMaxLength)
			throw Exception(ErrorCode::AsgLongNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		throw Exception(ErrorCode::AsgInvalidNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	// Checking if the column already exists
	for(auto &col : columns)
	{
		if(col.getName() == name)
			throw Exception(ErrorCode::InsDuplicatedElement,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	columns.push_back(SimpleColumn(name, *type, alias));
}

void Reference::addColumn(Column *col)
{
	if(!col)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	addColumn(col->getName(), col->getType(), col->getAlias());
}

void Reference::removeColumns()
{
	columns.clear();
}

std::vector<SimpleColumn> Reference::getColumns()
{
	return columns;
}

PhysicalTable *Reference::getTable()
{
	return table;
}

Column *Reference::getColumn()
{
	return column;
}

QString Reference::getColumnAlias()
{
	return column_alias;
}

QString Reference::getAlias()
{
	return alias;
}

QString Reference::getExpression()
{
	return expression;
}

Reference::ReferType Reference::getReferenceType()
{
	if(expression.isEmpty())
		return ReferColumn;

	return ReferExpression;
}

void Reference::setReferenceAlias(const QString &alias)
{
	if(alias.size() > BaseObject::ObjectNameMaxLength)
		throw Exception(ErrorCode::AsgLongNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	ref_alias = alias;
}

QString Reference::getReferenceAlias()
{
	return ref_alias;
}

QString Reference::getSQLDefinition(SqlType sql_type)
{
	QString sql_def, tab_name;
	ReferType refer_type;

	refer_type=getReferenceType();

	//Case the reference is between the SELECT-FROM keywords
	if(sql_type==SqlSelect)
	{
		//Case the reference is linked to a column
		if(refer_type==ReferColumn)
		{
			/* Generated SQL definition:
			[TABLE_ALIAS.]{COLUMN_NAME | *} [AS COLUMN_ALIAS] */

			if(!alias.isEmpty())
				tab_name=BaseObject::formatName(alias) + ".";
			else
				tab_name=table->getSignature() + ".";

			/* Case there is no column definede the default behavior is consider
			all the table columns (e.g. table.*) */
			if(!column)
				sql_def=tab_name + "*";
			else
			{
				//Case there is an column concatenates its name to the code definition
				sql_def=tab_name + column->getName(true);

				//Case there is a column alias concatenate it to the definition
				if(!column_alias.isEmpty())
					sql_def+=" AS " + BaseObject::formatName(column_alias);
			}
		}
		//Case the reference is linked to an expression
		else
		{
			/* Generated SQL definition:
			{expression} [AS ALIAS] */
			sql_def=expression;
			if(!alias.isEmpty())
				sql_def+=" AS " + BaseObject::formatName(alias);
		}
		sql_def+=",\n";
	}
	//Case the reference is between the FROM-[JOIN | WHERE] keywords
	else if(sql_type==SqlFrom)
	{
		/* Case the reference is linked to a column only the table name is used.
		 For expression the complete code is used thus the generated code is:

		 ... FROM {TABLE_NAME} [AS ALIAS] or
		 ... FROM {EXPRESSION} */
		if(refer_type==ReferColumn)
		{
			sql_def+=table->getName(true);

			if(!alias.isEmpty())
				sql_def+=" AS " + BaseObject::formatName(alias);
		}
		else
			sql_def=expression;

		sql_def+=",\n";
	}
	//Case the reference is after [JOIN | WHERE] keywords
	else
	{
		//Case the column is allocated
		if(refer_type==ReferColumn && column)
		{
			/* Generated SQL definition:
			... WHERE {TABLE_NAME | ALIAS}.{COLUMN_NAME} */

			if(alias.isEmpty())
				sql_def=table->getName(true);
			else
				sql_def=BaseObject::formatName(alias);

			sql_def+=".";

			if(column)
				sql_def+=column->getName(true);
		}
		else if(refer_type==ReferExpression)
			sql_def=expression;
	}

	sql_def="   " + sql_def;
	return sql_def;
}

QString Reference::getXMLDefinition()
{
	attribs_map attribs, aux_attribs;
	SchemaParser schparser;
	Column col_aux;
	QStringList ref_tab_names;

	attribs[Attributes::Table]="";
	attribs[Attributes::Column]="";

	if(table)
		attribs[Attributes::Table]=table->getName(true);

	if(column)
		attribs[Attributes::Column]=column->getName();

	attribs[Attributes::RefAlias]=ref_alias;
	attribs[Attributes::Expression]=expression;
	attribs[Attributes::Alias]=alias;
	attribs[Attributes::ColumnAlias]=column_alias;
	attribs[Attributes::Columns]="";
	attribs[Attributes::RefTables]="";

	for(auto &col : columns)
	{
		col_aux.setName(col.getName());
		col_aux.setType(PgSqlType::parseString(col.getType()));
		col_aux.setAlias(col.getAlias());
		attribs[Attributes::Columns]+=col_aux.getSourceCode(SchemaParser::XmlCode);
	}

	if(is_def_expr)
	{
		for(auto &tab : ref_tables)
		{
			aux_attribs[Attributes::Name] = tab->getSignature();
			attribs[Attributes::RefTables] += schparser.getSourceCode(Attributes::RefTableTag, aux_attribs, SchemaParser::XmlCode);
		}
	}

	return schparser.getSourceCode(Attributes::Reference, attribs, SchemaParser::XmlCode);
}

bool Reference::operator == (Reference &refer)
{
	unsigned ref_type;

	ref_type=this->getReferenceType();

	if(ref_type==refer.getReferenceType())
	{
		if(ref_type==ReferColumn)
		{
			return (this->table==refer.table &&
							this->column==refer.column &&
							this->alias==refer.alias &&
							this->column_alias==refer.column_alias);
		}

		return (this->expression == refer.expression &&
						this->alias == refer.alias &&
					  this->is_def_expr == refer.is_def_expr);
	
	}

	return false;
}

std::vector<BaseObject *> Reference::getDependencies(bool incl_indirect_deps)
{
	std::vector<BaseObject *> deps, aux_deps;

	if(column)
		deps.push_back(column);
	else if(table)
		deps.push_back(table);

	if(incl_indirect_deps && (column || table))
	{
		aux_deps = column ? column->getDependencies(true) : table->getDependencies(true);
		deps.insert(deps.end(), aux_deps.begin(), aux_deps.end());
	}

	return deps;
}

