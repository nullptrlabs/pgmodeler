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

#include "view.h"

using namespace CompatNs;

View::View()
{
	obj_type=ObjectType::View;
	materialized=recursive=with_no_data=false;
	attributes[Attributes::Definition]="";
	attributes[Attributes::References]="";
	attributes[Attributes::SelectExp]="";
	attributes[Attributes::FromExp]="";
	attributes[Attributes::SimpleExp]="";
	attributes[Attributes::EndExp]="";
	attributes[Attributes::CteExpression]="";
	attributes[Attributes::Materialized]="";
	attributes[Attributes::Recursive]="";
	attributes[Attributes::WithNoData]="";
	attributes[Attributes::Columns]="";
}

void View::setMaterialized(bool value)
{
	setCodeInvalidated(materialized != value);
	materialized=value;
	if(materialized) recursive=false;
}

void View::setRecursive(bool value)
{
	setCodeInvalidated(recursive != value);
	recursive=value;
	if(recursive) materialized=false;
}

void View::setWithNoData(bool value)
{
	setCodeInvalidated(materialized && with_no_data != value);
	with_no_data=(materialized ? value : false);
}

bool View::isMaterialized()
{
	return materialized;
}

bool View::isRecursive()
{
	return recursive;
}

bool View::isWithNoData()
{
	return with_no_data;
}

SimpleColumn View::getColumn(const QString &name)
{
	for(auto &col : columns)
	{
		if(col.getName() == name)
			return col;
	}

	return { };
}

void View::setCommomTableExpression(const QString &expr)
{
	setCodeInvalidated(cte_expression != expr);
	cte_expression=expr;
}

bool View::hasDefinitionExpression()
{
	std::vector<Reference>::iterator itr;
	bool found=false;

	itr=references.begin();
	while(itr!=references.end() && !found)
	{
		found=((*itr).isDefinitionExpression());
		itr++;
	}

	return found;
}

QString View::getCommomTableExpression()
{
	return cte_expression;
}

int View::getReferenceIndex(Reference &refer)
{
	std::vector<Reference>::iterator itr, itr_end;
	bool found=false;
	int idx=-1;

	itr=references.begin();
	itr_end=references.end();

	while(itr!=itr_end && !found)
	{
		found=((*itr)==refer);
		itr++;
		idx++;
	}

	if(!found) idx=-1;
	return idx;
}

std::vector<unsigned> *View::getExpressionList(Reference::SqlType sql_type)
{
	if(sql_type==Reference::SqlSelect)
		return &exp_select;

	if(sql_type==Reference::SqlFrom)
		return &exp_from;

	if(sql_type==Reference::SqlWhere)
		return &exp_where;

	if(sql_type==Reference::SqlEndExpr)
		return &exp_end;

	return nullptr;
}

void View::generateColumns()
{
	unsigned col_id = 0, col_count = 0, expr_idx = 0;
	PhysicalTable *tab = nullptr;
	Reference ref;
	Column *col = nullptr;
	QString name, alias;

	columns.clear();

	if(hasDefinitionExpression())
	{
		std::vector<SimpleColumn> ref_cols = references[0].getColumns();

		if(ref_cols.empty())
			columns.push_back(SimpleColumn(QString("%1...").arg(references[0].getExpression().simplified().mid(0, 20)),
																		 Attributes::Expression,
																		 !references[0].getReferenceAlias().isEmpty() ? references[0].getReferenceAlias() : ""));
		else
			columns = ref_cols;
	}
	else
	{
		for(auto ref_id : exp_select)
		{
			ref = references[ref_id];

			if(!ref.getExpression().isEmpty())
			{
				if(!ref.getAlias().isEmpty())
					name = ref.getAlias();
				else
					name = QString("_expr%1_").arg(expr_idx++);

				name = getUniqueColumnName(name);
				columns.push_back(SimpleColumn(name,  Attributes::Expression,
																			 !ref.getReferenceAlias().isEmpty() ? ref.getReferenceAlias() : name));
			}
			else if(!ref.getColumn())
			{
				tab=ref.getTable();
				col_count=tab->getColumnCount();

				for(col_id=0; col_id < col_count; col_id++)
				{
					col = tab->getColumn(col_id);
					name = getUniqueColumnName(col->getName());
					columns.push_back(SimpleColumn(name, *col->getType(),
																				 !col->getAlias().isEmpty() ? col->getAlias() : col->getName()));
				}
			}
			else
			{
				col = ref.getColumn();

				if(!ref.getColumnAlias().isEmpty())
					name = getUniqueColumnName(ref.getColumnAlias());
				else
					name = getUniqueColumnName(col->getName());

				if(!ref.getReferenceAlias().isEmpty())
					alias = ref.getReferenceAlias();
				else
					alias = !col->getAlias().isEmpty() ? col->getAlias() : col->getName();

				columns.push_back(SimpleColumn(name, *col->getType(), alias));
			}
		}
	}
}

std::vector<SimpleColumn> View::getColumns()
{
	return columns;
}

std::vector<Reference> View::getViewReferences()
{
	return references;
}

void View::addReference(Reference &refer, Reference::SqlType sql_type, int expr_id)
{
	int idx;
	std::vector<unsigned> *expr_list=nullptr;
	Column *col=nullptr;

	//Specific tests for expressions used as view definition
	if(sql_type==Reference::SqlViewDef)
	{
		//Raises an error if the expression is empty
		if(refer.getExpression().isEmpty())
			throw Exception(ErrorCode::AsgInvalidViewDefExpression,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Raises an error if already exists a definition expression
		if(hasDefinitionExpression())
			throw Exception(ErrorCode::AsgSecondViewDefExpression,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Raises an error if the user try to add a definition expression when already exists another references
		if(!references.empty())
			throw Exception(ErrorCode::MixingViewDefExprsReferences,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	//Raises an error if the user try to add a ordinary reference when there is a reference used as definition expression
	if(hasDefinitionExpression())
		throw Exception(ErrorCode::MixingViewDefExprsReferences,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Checks if the reference already exists
	idx=getReferenceIndex(refer);

	//If not exists
	if(idx < 0)
	{
		//Inserts the reference on the view
		refer.setDefinitionExpression(sql_type==Reference::SqlViewDef);
		references.push_back(refer);
		idx=references.size()-1;
	}

	if(sql_type!=Reference::SqlViewDef)
	{
		//Gets the expression list
		expr_list=getExpressionList(sql_type);

		//Avoiding the insertion of a duplicated reference in the expression list
		if(std::find(expr_list->begin(), expr_list->end(), idx) != expr_list->end())
			return;

		//Inserts the reference id on the expression list
		if(expr_id >= 0 && expr_id < static_cast<int>(expr_list->size()))
			expr_list->insert(expr_list->begin() + expr_id, static_cast<unsigned>(idx));
		//Raises an error if the expression id is invalid
		else if(expr_id >= 0 && expr_id >= static_cast<int>(expr_list->size()))
			throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);
		else
			expr_list->push_back(static_cast<unsigned>(idx));

		col=refer.getColumn();
		if(col && col->isAddedByRelationship() &&
				col->getObjectId() > this->object_id)
			this->object_id=BaseObject::getGlobalId();
	}

	generateColumns();
	setCodeInvalidated(true);
}

unsigned View::getReferenceCount()
{
	return references.size();
}

unsigned View::getReferenceCount(Reference::SqlType sql_type, int ref_type)
{
	std::vector<unsigned> *vect_idref=getExpressionList(sql_type);

	if(!vect_idref)
	{
		if(sql_type==Reference::SqlViewDef)
			return references.size();

		return 0;
	}
	
	if(ref_type < 0)
		return vect_idref->size();

	unsigned count=0;

	for(auto &ref : *vect_idref)
	{
		if(references[ref].getReferenceType()== static_cast<Reference::ReferType>(ref_type))
			count++;
	}

	return count;
}

Reference View::getReference(unsigned ref_id)
{
	//Raises an error if the reference id is out of bound
	if(ref_id >= references.size())
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return references[ref_id];
}

Reference View::getReference(unsigned ref_id, Reference::SqlType sql_type)
{
	std::vector<unsigned> *vect_idref=getExpressionList(sql_type);

	//Raises an error if the reference id is out of bound
	if(ref_id >= references.size())
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(sql_type==Reference::SqlViewDef || vect_idref)
		return references[ref_id];

	return references[vect_idref->at(ref_id)];
}

/* void View::removeReference(unsigned ref_id)
{
	std::vector<unsigned> *vect_idref[4]={&exp_select, &exp_from, &exp_where, &exp_end};
	std::vector<unsigned>::iterator itr, itr_end;
	unsigned i;

	//Raises an error if the reference id is out of bound
	if(ref_id >= references.size())
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	for(i=0; i < 3; i++)
	{
		itr=vect_idref[i]->begin();
		itr_end=vect_idref[i]->end();

		while(itr!=itr_end && !vect_idref[i]->empty())
		{
			//Removes the reference id from the expression list
			if(references[*itr]==references[ref_id])
				itr = vect_idref[i]->erase(itr);
			else
				itr++;
		}
	}

	//Removes the reference from the view
	references.erase(references.begin() + ref_id);
	generateColumns();
	setCodeInvalidated(true);
} */

void View::removeReferences()
{
	/* for(auto &ref : references)
	{
		if(ref.getColumn())
			unsetReference(ref.getColumn());
		else if(ref.getTable())
			unsetReference(ref.getTable());
	} */

	references.clear();
	exp_select.clear();
	exp_from.clear();
	exp_where.clear();
	exp_end.clear();
	columns.clear();
	setCodeInvalidated(true);
}

/* void View::removeReference(unsigned expr_id, Reference::SqlType sql_type)
{
	std::vector<unsigned> *vect_idref=getExpressionList(sql_type);

	if(expr_id >= vect_idref->size())
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	vect_idref->erase(vect_idref->begin() + expr_id);
	setCodeInvalidated(true);
} */

int View::getReferenceIndex(Reference &ref, Reference::SqlType sql_type)
{
	std::vector<unsigned> *vet_idref=getExpressionList(sql_type);
	std::vector<unsigned>::iterator itr, itr_end;
	int idx_ref;
	bool found=false;

	idx_ref=getReferenceIndex(ref);

	if(sql_type==Reference::SqlViewDef &&
			idx_ref >=0 && ref.isDefinitionExpression())
		return idx_ref;

	if(sql_type!=Reference::SqlViewDef)
	{
		itr=vet_idref->begin();
		itr_end=vet_idref->end();

		while(itr!=itr_end && !found)
		{
			found=(static_cast<int>(*itr)==idx_ref);
			if(!found) itr++;
		}

		if(!found)
			return -1;
		
		return (itr-vet_idref->begin());
	}
	
	return -1;
}

void View::setDefinitionAttribute()
{
	QString decl;

	if(!references.empty())
	{
		if(exp_select.empty())
		{
			decl=references[0].getExpression();
		}
		else
		{
			std::vector<unsigned> *refs_vect[4]={&exp_select, &exp_from, &exp_where, &exp_end};
			std::vector<unsigned>::iterator itr, itr_end;
			QString keywords[4]={"SELECT\n", "\nFROM\n", "\nWHERE\n", "\n"};
			unsigned i, cnt, idx;
			Reference::SqlType sql_type[4]={ Reference::SqlSelect,
																						Reference::SqlFrom,
																						Reference::SqlWhere,
																						Reference::SqlEndExpr };

			for(i=0; i < 4; i++)
			{
				if(refs_vect[i]->size() > 0)
				{
					decl+=keywords[i];

					itr=refs_vect[i]->begin();
					itr_end=refs_vect[i]->end();
					while(itr!=itr_end)
					{
						idx=(*itr);
						decl+=references[idx].getSQLDefinition(sql_type[i]);
						itr++;
					}

					if(sql_type[i]==Reference::SqlSelect ||
							sql_type[i]==Reference::SqlFrom)
					{
						//Removing the final comma from SELECT / FROM declarations
						cnt=decl.size();
						if(decl[cnt-2]==',')
							decl.remove(cnt-2,2);
					}
				}
			}
		}
	}

	decl=decl.trimmed();
	if(!decl.isEmpty() && !decl.endsWith(QChar(';')) && !with_no_data)
		decl.append(QChar(';'));

	attributes[Attributes::Definition]=decl;
}

void View::setReferencesAttribute()
{
	QString str_aux;
	QString attribs[]={ Attributes::SelectExp,
											Attributes::FromExp,
											Attributes::SimpleExp,
											Attributes::EndExp};
	std::vector<unsigned> *vect_exp[]={&exp_select, &exp_from, &exp_where, &exp_end};
	int cnt, i, i1;

	cnt=references.size();
	for(i=0; i < cnt; i++)
		str_aux+=references[i].getXMLDefinition();
	attributes[Attributes::References]=str_aux;

	for(i=0; i < 4; i++)
	{
		str_aux="";
		cnt=vect_exp[i]->size();
		for(i1=0; i1 < cnt; i1++)
		{
			str_aux+=QString("%1").arg(vect_exp[i]->at(i1));
			if(i1 < cnt-1) str_aux+=",";
		}
		attributes[attribs[i]]=str_aux;
	}
}

QString View::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	attributes[Attributes::CteExpression]=cte_expression;
	attributes[Attributes::Materialized]=(materialized ? Attributes::True : "");
	attributes[Attributes::Recursive]=(recursive ? Attributes::True : "");
	attributes[Attributes::WithNoData]=(with_no_data ? Attributes::True : "");
	attributes[Attributes::Columns]="";
	attributes[Attributes::Tag]="";
	attributes[Attributes::Pagination]=(pagination_enabled ? Attributes::True : "");
	attributes[Attributes::CollapseMode]=QString::number(collapse_mode);
	attributes[Attributes::AttribsPage]=(pagination_enabled ? QString::number(curr_page[AttribsSection]) : "");
	attributes[Attributes::ExtAttribsPage]=(pagination_enabled ? QString::number(curr_page[ExtAttribsSection]) : "");

	setSQLObjectAttribute();
	setLayersAttribute();

	// We use column names only if the view has references that aren't its whole definition (Reference::SqlViewDef)
	if(recursive && !hasDefinitionExpression())
	{
		QStringList fmt_names;

		//for(auto &name : col_names)
		//	fmt_names.push_back(formatName(name));

		for(auto &col : columns)
			fmt_names.push_back(formatName(col.getName()));

		attributes[Attributes::Columns]=fmt_names.join(',');
	}

	if(tag && def_type==SchemaParser::XmlCode)
		attributes[Attributes::Tag]=tag->getSourceCode(def_type, true);

	if(def_type==SchemaParser::SqlCode)
		setDefinitionAttribute();
	else
	{
		setPositionAttribute();
		setFadedOutAttribute();
		setReferencesAttribute();
		attributes[Attributes::ZValue]=QString::number(z_value);
		attributes[Attributes::MaxObjCount]=QString::number(static_cast<unsigned>(getMaxObjectCount() * 1.20));
	}

	return BaseObject::__getSourceCode(def_type);
}

void View::setSQLObjectAttribute()
{
	if(materialized)
		attributes[Attributes::SqlObject]="MATERIALIZED " + BaseObject::getSQLName(ObjectType::View);
}

QString View::getUniqueColumnName(const QString &name)
{
	unsigned idx = 1;
	QString fmt_name = name;
	std::vector<SimpleColumn>::iterator itr, itr_end;

	itr = columns.begin();
	itr_end = columns.end();

	while(itr != itr_end)
	{
		if(itr->getName() == fmt_name)
		{
			fmt_name = name + QString::number(idx);
			idx++;
			itr = columns.begin();
		}
		else
			itr++;
	}

	return fmt_name;
}

QString View::getDropCode(bool cascade)
{
	setSQLObjectAttribute();
	return BaseObject::getDropCode(cascade);
}
