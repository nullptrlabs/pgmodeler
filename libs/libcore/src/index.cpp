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

#include "index.h"
#include "utilsns.h"

Index::Index()
{
	obj_type=ObjectType::Index;

	for(unsigned idx = Unique; idx <= NullsNotDistinct; idx++)
		index_attribs[idx] = false;

	fill_factor=90;
	attributes[Attributes::Unique]="";
	attributes[Attributes::Concurrent]="";
	attributes[Attributes::Table]="";
	attributes[Attributes::IndexType]="";
	attributes[Attributes::Columns]="";
	attributes[Attributes::Expression]="";
	attributes[Attributes::Factor]="";
	attributes[Attributes::Predicate]="";
	attributes[Attributes::OpClass]="";
	attributes[Attributes::NullsFirst]="";
	attributes[Attributes::AscOrder]="";
	attributes[Attributes::DeclInTable]="";
	attributes[Attributes::Elements]="";
	attributes[Attributes::FastUpdate]="";
	attributes[Attributes::Buffering]="";
	attributes[Attributes::StorageParams]="";
	attributes[Attributes::IncludedCols]="";
	attributes[Attributes::NullsNotDistinct]="";
}

void Index::setIndexElementsAttribute(SchemaParser::CodeType def_type)
{
	QString str_elem;
	unsigned i, count;

	count=idx_elements.size();
	for(i=0; i < count; i++)
	{
		str_elem+=idx_elements[i].getSourceCode(def_type);
		if(i < (count-1) && def_type==SchemaParser::SqlCode) str_elem+=',';
	}

	attributes[Attributes::Elements]=str_elem;
}

int Index::getElementIndex(IndexElement elem)
{
	int idx=0;
	bool found=false;

	while(idx < static_cast<int>(idx_elements.size()) && !found)
	{
		found=(idx_elements[idx]==elem);
		if(!found) idx++;
	}

	return (found ? idx : -1);
}

void Index::addIndexElement(IndexElement elem)
{
	if(getElementIndex(elem) >= 0)
		throw Exception(ErrorCode::InsDuplicatedElement,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(elem.getExpression().isEmpty() && !elem.getColumn() && !elem.getSimpleColumn().isValid())
		throw Exception(ErrorCode::AsgInvalidExpressionObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	idx_elements.push_back(elem);
	setCodeInvalidated(true);
	validateElements();
}

void Index::addIndexElement(const QString &expr, Collation *coll, OperatorClass *op_class, bool use_sorting, bool asc_order, bool nulls_first)
{
	try
	{
		IndexElement elem;

		//Raises an error if the expression is empty
		if(expr.isEmpty())
			throw Exception(ErrorCode::AsgInvalidExpressionObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Configures the element
		elem.setExpression(expr);
		elem.setOperatorClass(op_class);
		elem.setCollation(coll);
		elem.setSortingEnabled(use_sorting);
		elem.setSortingAttribute(IndexElement::NullsFirst, nulls_first);
		elem.setSortingAttribute(IndexElement::AscOrder, asc_order);

		if(getElementIndex(elem) >= 0)
			throw Exception(ErrorCode::InsDuplicatedElement,PGM_FUNC,PGM_FILE,PGM_LINE);

		idx_elements.push_back(elem);
		setCodeInvalidated(true);
		validateElements();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void Index::addIndexElement(Column *column, Collation *coll, OperatorClass *op_class, bool use_sorting, bool asc_order, bool nulls_first)
{
	try
	{
		IndexElement elem;

		//Case the column is not allocated raises an error
		if(!column)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedColumn)
							.arg(this->getName(), this->getTypeName()),
							ErrorCode::AsgNotAllocatedColumn, PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		//Configures the element
		elem.setColumn(column);
		elem.setOperatorClass(op_class);
		elem.setCollation(coll);
		elem.setSortingEnabled(use_sorting);
		elem.setSortingAttribute(IndexElement::NullsFirst, nulls_first);
		elem.setSortingAttribute(IndexElement::AscOrder, asc_order);

		if(getElementIndex(elem) >= 0)
			throw Exception(ErrorCode::InsDuplicatedElement,PGM_FUNC,PGM_FILE,PGM_LINE);

		idx_elements.push_back(elem);
		setCodeInvalidated(true);
		validateElements();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void Index::addIndexElements(std::vector<IndexElement> &elems)
{
	std::vector<IndexElement> elems_bkp=idx_elements;

	try
	{
		idx_elements.clear();

		for(const auto & elem : elems)
			addIndexElement(elem);
	}
	catch(Exception &e)
	{
		idx_elements = elems_bkp;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void Index::removeIndexElement(unsigned idx_elem)
{
	if(idx_elem >= idx_elements.size())
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	idx_elements.erase(idx_elements.begin() + idx_elem);
	setCodeInvalidated(true);
}

void Index::removeIndexElements()
{
	idx_elements.clear();
	setCodeInvalidated(true);
}

IndexElement Index::getIndexElement(unsigned elem_idx)
{
	if(elem_idx >= idx_elements.size())
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return idx_elements[elem_idx];
}

std::vector<IndexElement> Index::getIndexElements()
{
	return idx_elements;
}

unsigned Index::getIndexElementCount()
{
	return idx_elements.size();
}

void Index::setIndexAttribute(IndexAttrib attrib_id, bool value)
{
	if(attrib_id > NullsNotDistinct)
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(index_attribs[attrib_id] != value);
	index_attribs[attrib_id]=value;
}

void Index::setFillFactor(unsigned factor)
{
	setCodeInvalidated(fill_factor != factor);
	fill_factor=factor;
}

void Index::setIndexingType(IndexingType idx_type)
{
	setCodeInvalidated(indexing_type != idx_type);
	this->indexing_type=idx_type;
	validateElements();
}

void Index::setPredicate(const QString &expr)
{
	setCodeInvalidated(predicate != expr);
	predicate=expr;
}

unsigned Index::getFillFactor()
{
	return fill_factor;
}

bool Index::getIndexAttribute(IndexAttrib attrib_id)
{
	if(attrib_id > NullsNotDistinct)
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return index_attribs[attrib_id];
}

IndexingType Index::getIndexingType()
{
	return indexing_type;
}

QString Index::getPredicate()
{
	return predicate;
}

bool Index::isReferRelationshipAddedColumn()
{
	//Checks if some of the elements if referencing columns added by relationship
	for(auto &elem : idx_elements)
	{
		if(elem.getColumn() &&
			 elem.getColumn()->isAddedByRelationship())
			return true;
	}

	for(auto &col : included_cols)
	{
		if(col->isAddedByRelationship())
			return true;
	}

	return false;
}

std::vector<Column *> Index::getRelationshipAddedColumns()
{
	std::vector<Column *> cols;
	Column *col = nullptr;

	for(auto &elem : idx_elements)
	{
		col = elem.getColumn();

		if(col && col->isAddedByRelationship())
			cols.push_back(col);
	}

	for(auto &col : included_cols)
	{
		if(col->isAddedByRelationship())
			cols.push_back(col);
	}

	return cols;
}

bool Index::isReferCollation(Collation *coll)
{
	std::vector<IndexElement>::iterator itr, itr_end;
	bool found=false;

	if(!coll) return false;

	itr=idx_elements.begin();
	itr_end=idx_elements.end();

	//Checks if some of the elements is referencing the collation
	while(itr!=itr_end && !found)
	{
		found=((*itr).getCollation()==coll);
		itr++;
	}

	return found;
}

bool Index::isReferColumn(Column *column)
{
	if(!column)
		return false;

	for(auto &elem : idx_elements)
	{
		if(elem.getColumn() == column)
			return true;
	}

	for(auto &col : included_cols)
	{
		if(col == column)
			return true;
	}

	return false;
}

void Index::addColumn(Column *col)
{
	if(!col)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedColumn).arg(getName(), getTypeName()),
										ErrorCode::AsgNotAllocatedColumn, PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	// We ignore the column if it alread exits in the list
	if(std::find(included_cols.begin(), included_cols.end(), col) == included_cols.end())
	{
		incl_simple_cols.clear();
		included_cols.push_back(col);
		setCodeInvalidated(true);
	}
}

void Index::addSimpleColumn(const SimpleColumn &col)
{
	if(!col.isValid())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedColumn).arg(getName(), getTypeName()),
										ErrorCode::AsgNotAllocatedColumn, PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	// We ignore the column if it alread exits in the list
	if(std::find(incl_simple_cols.begin(), incl_simple_cols.end(), col) == incl_simple_cols.end())
	{
		included_cols.clear();
		incl_simple_cols.push_back(col);
		setCodeInvalidated(true);
	}
}

void Index::setColumns(const std::vector<Column *> &cols)
{
	try
	{
		included_cols.clear();

		for(auto &col : cols)
			addColumn(col);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void Index::setSimpleColumns(const std::vector<SimpleColumn> &cols)
{
	try
	{
		incl_simple_cols.clear();

		for(auto &col : cols)
			addSimpleColumn(col);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

std::vector<Column *> Index::getColumns()
{
	return included_cols;
}

std::vector<SimpleColumn> Index::getSimpleColumns()
{
	return incl_simple_cols;
}

QString Index::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	setIndexElementsAttribute(def_type);
	attributes[Attributes::Unique]=(index_attribs[Unique] ? Attributes::True : "");
	attributes[Attributes::Concurrent]=(index_attribs[Concurrent] ? Attributes::True : "");
	attributes[Attributes::NullsNotDistinct]=(index_attribs[NullsNotDistinct] ? Attributes::True : "");
	attributes[Attributes::IndexType]=(~indexing_type);
	attributes[Attributes::Predicate]=predicate;
	attributes[Attributes::StorageParams]="";

	if(getParentTable())
	{
		attributes[Attributes::Table]=getParentTable()->getName(true);

		if(def_type==SchemaParser::SqlCode && getParentTable()->getSchema())
			attributes[Attributes::Schema]=getParentTable()->getSchema()->getName(true);
	}

	if(this->indexing_type==IndexingType::Gin)
		attributes[Attributes::StorageParams]=attributes[Attributes::FastUpdate]=(index_attribs[FastUpdate] ? Attributes::True : "");

	if(this->indexing_type==IndexingType::Gist)
		attributes[Attributes::StorageParams]=attributes[Attributes::Buffering]=(index_attribs[Buffering] ? Attributes::True : "");

	if(this->indexing_type!=IndexingType::Gin && fill_factor >= 10)
	{
		attributes[Attributes::Factor]=QString("%1").arg(fill_factor);
		attributes[Attributes::StorageParams]=Attributes::True;
	}
	else if(def_type==SchemaParser::XmlCode)
		attributes[Attributes::Factor]="0";

	QStringList incl_cols;

	for(auto &col : included_cols)
		incl_cols.append(col->getName(true));

	for(auto &col : incl_simple_cols)
		incl_cols.append(BaseObject::formatName(col.getName()));

	attributes[Attributes::IncludedCols] = incl_cols.join(',');

	/* Case the index doesn't referece some column added by relationship it will be declared
		inside the parent table construction by the use of 'decl-in-table' schema attribute */
	if(!isReferRelationshipAddedColumn())
		attributes[Attributes::DeclInTable]=Attributes::True;

	return BaseObject::__getSourceCode(def_type);
}

QString Index::getSignature(bool format)
{
	if(!getParentTable() || !getParentTable()->getSchema())
		return BaseObject::getSignature(format);

	return QString("%1.%2").arg(getParentTable()->getSchema()->getName(format), this->getName(format));
}

QString Index::getAlterCode(BaseObject *object)
{
	Index *index=dynamic_cast<Index *>(object);

	if(!index)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		attribs_map attribs;
		attributes[Attributes::AlterCmds] = TableObject::getAlterCode(object);

		if(this->indexing_type==index->indexing_type)
		{
			if(this->indexing_type != IndexingType::Gin &&
				 this->fill_factor!=index->fill_factor && index->fill_factor >= 10)
				attribs[Attributes::Factor]=QString::number(index->fill_factor);

			if(this->indexing_type==IndexingType::Gin &&
					this->index_attribs[FastUpdate] != index->index_attribs[FastUpdate])
				attribs[Attributes::FastUpdate]=(index->index_attribs[FastUpdate] ? Attributes::True : Attributes::Unset);

			if(this->indexing_type==IndexingType::Gist &&
					this->index_attribs[Buffering] != index->index_attribs[Buffering])
				attribs[Attributes::Buffering]=(index->index_attribs[Buffering] ? Attributes::True : Attributes::Unset);
		}

		copyAttributes(attribs);
		return BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void Index::validateElements()
{
	if(indexing_type!=IndexingType::Btree)
	{
		for(auto & idx_element : idx_elements)
		{
			if(idx_element.isSortingEnabled())
			{
				idx_element.setSortingEnabled(false);
				setCodeInvalidated(true);
			}
		}
	}
}

QString Index::getDataDictionary(bool md_format, const attribs_map &extra_attribs)
{
	try
	{
		attribs_map attribs;
		QStringList exprs, col_names;

		attribs.insert(extra_attribs.begin(), extra_attribs.end());
		attribs[Attributes::Name] = obj_name;
		attribs[Attributes::Type] = ~indexing_type;
		attribs[Attributes::Comment] = comment;
		attribs[Attributes::Predicate] = predicate;

		for(auto &elem : idx_elements)
		{
			if(elem.getColumn())
				col_names.append(elem.getColumn()->getName());
			else if(elem.getSimpleColumn().isValid())
				col_names.append(elem.getSimpleColumn().getName());
			else
				exprs.append(elem.getExpression());
		}

		attribs[Attributes::Columns] = col_names.join(", ");
		attribs[Attributes::Expressions] = exprs.join(", ");

		schparser.ignoreEmptyAttributes(true);
		return schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, getSchemaName()), attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void Index::updateDependencies()
{
	std::vector<BaseObject *> deps, aux_deps;

	for(auto &elem : idx_elements)
	{
		aux_deps = elem.getDependencies();
		deps.insert(deps.end(), aux_deps.begin(), aux_deps.end());
	}

	for(auto &col : included_cols)
		deps.push_back(col);

	TableObject::updateDependencies(deps);
}

void Index::generateHashCode()
{
	QString str_attr;
	TableObject::generateHashCode();

	for(auto &attr : index_attribs)
		str_attr.append(QString::number(attr));

	hash_code = UtilsNs::getStringHash(hash_code + str_attr);
}
