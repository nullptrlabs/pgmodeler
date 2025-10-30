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
#include "physicaltable.h"

const QString View::ExtraSCRegExp {"((\\;)+(\\s|\\t)*)+$"};

View::View()
{
	obj_type = ObjectType::View;
	materialized = recursive = with_no_data = false;
	security_invoker = security_barrier = false;
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
	attributes[Attributes::CheckOption]="";
	attributes[Attributes::Options]="";
	attributes[Attributes::SecurityBarrier]="";
	attributes[Attributes::SecurityInvoker]="";
}

View::~View()
{
	for(auto &obj : getObjects())
		delete obj;

	indexes.clear();
	triggers.clear();
	rules.clear();
}

void View::setName(const QString &name)
{
	QString prev_name=this->getName(true);
	BaseObject::setName(name);
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void View::setSchema(BaseObject *schema)
{
	QString prev_name=this->getName(true);
	BaseObject::setSchema(schema);
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void View::setProtected(bool value)
{
	//Protected the table child objects
	for(auto &obj : getObjects())
		obj->setProtected(value);

	//Protects the view itself
	BaseGraphicObject::setProtected(value);
}

void View::setMaterialized(bool value)
{
	setCodeInvalidated(materialized != value);
	materialized = value;

	if(materialized)
	{
		recursive = false;
		check_option = CheckOptionType::Null;
	}
}

void View::setRecursive(bool value)
{
	setCodeInvalidated(recursive != value);
	recursive = value;

	if(recursive)
	{
		materialized = false;
		check_option = CheckOptionType::Null;
	}
}

void View::setWithNoData(bool value)
{
	setCodeInvalidated(materialized && with_no_data != value);
	with_no_data=(materialized ? value : false);
}

void View::setSecurityBarrier(bool value)
{
	setCodeInvalidated(security_barrier != value);
	security_barrier = value;
}

void View::setSecurityInvoker(bool value)
{
	setCodeInvalidated(security_invoker != value);
	security_invoker = value;
}

void View::setCheckOption(CheckOptionType check_opt)
{
	if(materialized || recursive)
		check_option = CheckOptionType::Null;

	setCodeInvalidated(check_option != check_opt);
	check_option = check_opt;
}

CheckOptionType View::getCheckOption()
{
	return check_option;
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

bool View::isSecurityInvoker()
{
	return security_invoker;
}

bool View::isSecurityBarrier()
{
	return security_barrier;
}

void View::setReferences(const std::vector<Reference> &obj_refs)
{
	this->references = obj_refs;
	generateColumns();
	setCodeInvalidated(true);
}

void View::setCustomColumns(const std::vector<SimpleColumn> &cols)
{
	custom_cols = cols;
	generateColumns();
	setCodeInvalidated(true);
}

void View::setSqlDefinition(const QString &sql_def)
{
	setCodeInvalidated(sql_def != sql_definition);
	sql_definition = sql_def;
}

QString View::getSqlDefinition()
{
	return sql_definition;
}

SimpleColumn View::getColumn(const QString &name)
{
	for(auto &col : gen_columns)
	{
		if(col.getName() == name)
			return col;
	}

	return {};
}

void View::generateColumns()
{
	PhysicalTable *tab = nullptr;
	View *view = nullptr;
	ObjectType ref_obj_type;
	BaseObject *ref_obj = nullptr;
	Column *col = nullptr;

	gen_columns.clear();

	for(auto &ref : references)
	{
		if(!ref.isUseColumns())
			continue;

		ref_obj_type = ref.getObject()->getObjectType();
		ref_obj = ref.getObject();

		if(ref_obj_type == ObjectType::Column)
		{
			col = dynamic_cast<Column *>(ref_obj);

			gen_columns.push_back(SimpleColumn(getUniqueColumnName(col->getName()),
																		 *col->getType(), ref.getRefName()));
		}
		else if(ref_obj_type == ObjectType::View)
		{
			view = dynamic_cast<View *>(ref_obj);

			for(auto &col : view->getColumns())
			{
				gen_columns.push_back(SimpleColumn(getUniqueColumnName(col.getName()),
																						 col.getType(), col.getAlias()));
			}
		}
		else if(PhysicalTable::isPhysicalTable(ref_obj_type))
		{
			tab = dynamic_cast<PhysicalTable *>(ref_obj);

			for(auto &obj : *tab->getObjectList(ObjectType::Column))
			{
				col = dynamic_cast<Column *>(obj);
				gen_columns.push_back(SimpleColumn(getUniqueColumnName(col->getName()),
																			 *col->getType(), ""));
			}
		}
	}

	gen_columns.insert(gen_columns.end(), custom_cols.begin(), custom_cols.end());
}

std::vector<SimpleColumn> View::getColumns()
{
	return gen_columns;
}

std::vector<SimpleColumn> View::getCustomColumns()
{
	return custom_cols;
}

std::vector<Reference> View::getObjectReferences()
{
	return references;
}

std::vector<BaseTable *> View::getReferencedTables()
{
	std::vector<BaseTable *> tables;

	for(auto &obj : getDependencies(false, {}, true))
	{
		if(BaseTable::isBaseTable(obj->getObjectType()))
			tables.push_back(dynamic_cast<BaseTable *>(obj));
		else if(obj->getObjectType() == ObjectType::Column)
			tables.push_back(dynamic_cast<TableObject *>(obj)->getParentTable());
	}

	return tables;
}

bool View::isReferRelationshipAddedColumn()
{
	Column *col = nullptr;

	for(auto &ref : references)
	{
		col = dynamic_cast<Column *>(ref.getObject());

		if(col && col->isAddedByRelationship())
			return true;
	}

	return false;
}

std::vector<Column *> View::getRelationshipAddedColumns()
{
	std::vector<Column *> cols;
	Column *col = nullptr;

	for(auto &ref : references)
	{
		col = dynamic_cast<Column *>(ref.getObject());

		if(col && col->isAddedByRelationship())
			cols.push_back(col);
	}

	return cols;
}

bool View::isReferencingTable(BaseTable *tab)
{
	for(auto &obj : getDependencies(false, {}, true))
	{
		if(dynamic_cast<BaseTable *>(obj) == tab)
			return true;
	}

	return false;
}

void View::setOptionsAttributes(SchemaParser::CodeType def_type)
{
	attribs_map opts_map = {{ Attributes::CheckOption, ~check_option },
													{ Attributes::SecurityBarrier, security_barrier ? Attributes::True : "" },
													{ Attributes::SecurityInvoker, security_invoker ? Attributes::True : "" }};

	if(def_type == SchemaParser::SqlCode)
	{
		QStringList fmt_opts;

		for(auto &itr : opts_map)
		{
			if(!itr.second.isEmpty())
			{
				fmt_opts.append(QString("%1=%2")
												.arg(QString(itr.first).replace('-', '_'), itr.second));
			}
		}

		attributes[Attributes::Options] = fmt_opts.join(", ");
	}
	else
	{
		for(auto &itr : opts_map)
			attributes[itr.first] = itr.second;
	}
}

QString View::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	attributes[Attributes::Materialized] = (materialized ? Attributes::True : "");
	attributes[Attributes::Recursive] = (recursive ? Attributes::True : "");
	attributes[Attributes::WithNoData] = (with_no_data ? Attributes::True : "");
	attributes[Attributes::Columns] = "";
	attributes[Attributes::Tag] = "";
	attributes[Attributes::References] = "";
	attributes[Attributes::Pagination] = (pagination_enabled ? Attributes::True : "");
	attributes[Attributes::CollapseMode] = QString::number(collapse_mode);
	attributes[Attributes::AttribsPage] = (pagination_enabled ? QString::number(curr_page[AttribsSection]) : "");
	attributes[Attributes::ExtAttribsPage] = (pagination_enabled ? QString::number(curr_page[ExtAttribsSection]) : "");

	setSQLObjectAttribute();
	setLayersAttribute();
	setOptionsAttributes(def_type);

	if(recursive)
	{
		QStringList fmt_names;

		for(auto &col : gen_columns)
			fmt_names.push_back(formatName(col.getName()));

		attributes[Attributes::Columns] = fmt_names.join(',');
	}

	if(tag && def_type == SchemaParser::XmlCode)
		attributes[Attributes::Tag] = tag->getSourceCode(def_type, true);

	if(def_type==SchemaParser::SqlCode)
	{
		GenericSQL view_def_obj;
		QString fmt_sql_def = sql_definition.trimmed();

		// Removing unneeded semicolons at the end of the view's definition command
		fmt_sql_def.remove(QRegularExpression(ExtraSCRegExp));

		view_def_obj.setHideDescription(true);
		view_def_obj.setDefinition(fmt_sql_def);
		view_def_obj.addReferences(references);
		attributes[Attributes::Definition] = view_def_obj.getSourceCode(def_type).trimmed();
	}
	else
	{
		for(auto &ref : references)
			attributes[Attributes::References] += ref.getXmlCode();

		for(auto &col : custom_cols)
			attributes[Attributes::Columns] += col.getXmlCode();

		setPositionAttribute();
		setFadedOutAttribute();

		attributes[Attributes::Definition] = sql_definition;
		attributes[Attributes::ZValue] = QString::number(z_value);
		attributes[Attributes::MaxObjCount] = QString::number(static_cast<unsigned>(getMaxObjectCount() * 1.20));
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

	itr = gen_columns.begin();
	itr_end = gen_columns.end();

	while(itr != itr_end)
	{
		if(itr->getName() == fmt_name)
		{
			fmt_name = name + QString::number(idx);
			idx++;
			itr = gen_columns.begin();
		}
		else
			itr++;
	}

	return fmt_name;
}

void View::setObjectListsCapacity(unsigned capacity)
{
  if(capacity < DefMaxObjectCount || capacity > DefMaxObjectCount * 10)
		capacity = DefMaxObjectCount;

	references.reserve(capacity);
	indexes.reserve(capacity/2);
	rules.reserve(capacity/2);
	triggers.reserve(capacity/2);
}

unsigned View::getMaxObjectCount()
{
	unsigned count = 0, max = references.size();
  std::vector<ObjectType> types = { ObjectType::Index, ObjectType::Rule, ObjectType::Trigger };

  for(auto type : types)
  {
	count = getObjectList(type)->size();
	if(count > max) max = count;
  }

	return max;
}

QString View::getDropCode(bool cascade)
{
	setSQLObjectAttribute();
	return BaseObject::getDropCode(cascade);
}

int View::getObjectIndex(BaseObject *obj)
{
	TableObject *tab_obj=dynamic_cast<TableObject *>(obj);

	if(!obj || (tab_obj && tab_obj->getParentTable()!=this))
		return -1;

	std::vector<TableObject *>::iterator itr, itr_end;
	std::vector<TableObject *> *obj_list=getObjectList(obj->getObjectType());
	bool found=false;

	if(!obj_list)
		return -1;

	itr=obj_list->begin();
	itr_end=obj_list->end();

	while(itr!=itr_end && !found)
	{
		found=((*itr)==tab_obj);
		if(!found) itr++;
	}

	if(found)
		return (itr - obj_list->begin());

	return -1;
}

int View::getObjectIndex(const QString &name, ObjectType obj_type)
{
	if(name.isEmpty())
		return -1;

	std::vector<TableObject *>::iterator itr, itr_end;
	std::vector<TableObject *> *obj_list=getObjectList(obj_type);
	bool found=false, format=name.contains('"');

	if(!obj_list)
		return -1;

	itr=obj_list->begin();
	itr_end=obj_list->end();

	while(itr!=itr_end && !found)
	{
		found=((*itr)->getName(format)==name);
		if(!found) itr++;
	}

	if(found)
		return (itr - obj_list->begin());

	return -1;
}

void View::addObject(BaseObject *obj, int obj_idx)
{
	if(!obj)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		std::vector<TableObject *> *obj_list = getObjectList(obj->getObjectType());
		TableObject *tab_obj=dynamic_cast<TableObject *>(obj);

		//Raises an error if already exists a object with the same name and type
		if(getObjectIndex(obj->getName(), tab_obj->getObjectType()) >= 0)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgDuplicatedObject)
											.arg(obj->getName(true))
											.arg(obj->getTypeName())
											.arg(this->getName(true))
											.arg(this->getTypeName()),
											ErrorCode::AsgDuplicatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		//Validates the object definition
		tab_obj->setParentTable(this);
		tab_obj->getSourceCode(SchemaParser::SqlCode);

		//Make a additional validation if the object is a trigger
		if(tab_obj->getObjectType()==ObjectType::Trigger)
			dynamic_cast<Trigger *>(tab_obj)->validateTrigger();

		//Inserts the object at specified position
		if(obj_idx < 0 || obj_idx >= static_cast<int>(obj_list->size()))
			obj_list->push_back(tab_obj);
		else
			obj_list->insert(obj_list->begin() + obj_idx, tab_obj);

		tab_obj->updateDependencies();
		setCodeInvalidated(true);
	}
	catch(Exception &e)
	{
		if(e.getErrorCode()==ErrorCode::UndefinedAttributeValue)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgObjectInvalidDefinition)
											.arg(obj->getName())
											.arg(obj->getTypeName()),
											ErrorCode::AsgObjectInvalidDefinition,PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void View::addTrigger(Trigger *trig, int obj_idx)
{
	try
	{
		addObject(trig, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void View::addRule(Rule *rule, int obj_idx)
{
	try
	{
		addObject(rule, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void View::addIndex(Index *index, int obj_idx)
{
	try
	{
		addObject(index, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void View::removeObject(unsigned obj_idx, ObjectType obj_type)
{
	std::vector<TableObject *> *obj_list = getObjectList(obj_type);
	std::vector<TableObject *>::iterator itr;

	//Raises an error if the object index is out of bound
	if(obj_idx >= obj_list->size())
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	itr=obj_list->begin() + obj_idx;

	(*itr)->clearAllDepsRefs();
	(*itr)->setParentTable(nullptr);

	obj_list->erase(itr);
	setCodeInvalidated(true);
}

void View::removeObject(BaseObject *obj)
{
	try
	{
		removeObject(getObjectIndex(obj), obj->getObjectType());
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void View::removeObject(const QString &name, ObjectType obj_type)
{
	try
	{
		removeObject(getObjectIndex(name, obj_type), obj_type);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void View::removeTrigger(unsigned idx)
{
	try
	{
		removeObject(idx, ObjectType::Trigger);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void View::removeRule(unsigned idx)
{
	try
	{
		removeObject(idx, ObjectType::Rule);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void View::removeIndex(unsigned idx)
{
	try
	{
		removeObject(idx, ObjectType::Index);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

TableObject *View::getObject(unsigned obj_idx, ObjectType obj_type)
{
	std::vector<TableObject *> *obj_list=getObjectList(obj_type);

	//Raises an error if the object index is out of bound
	if(obj_idx >= obj_list->size())
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return obj_list->at(obj_idx);
}

TableObject *View::getObject(const QString &name, ObjectType obj_type)
{
	try
	{
		int idx = getObjectIndex(name, obj_type);

		if(idx >= 0)
			return getObject(idx, obj_type);

		return nullptr;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Trigger *View::getTrigger(unsigned obj_idx)
{
	try
	{
		return dynamic_cast<Trigger *>(getObject(obj_idx, ObjectType::Trigger));
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Rule *View::getRule(unsigned obj_idx)
{
	try
	{
		return dynamic_cast<Rule *>(getObject(obj_idx, ObjectType::Rule));
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Index *View::getIndex(unsigned obj_idx)
{
	try
	{
		return dynamic_cast<Index *>(getObject(obj_idx, ObjectType::Index));
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

unsigned View::getObjectCount(ObjectType obj_type, bool)
{
	std::vector<TableObject *> *obj_list = getObjectList(obj_type);
	return (!obj_list ? 0 : obj_list->size());
}

unsigned View::getTriggerCount()
{
	return triggers.size();
}

unsigned View::getRuleCount()
{
	return rules.size();
}

unsigned View::getIndexCount()
{
	return indexes.size();
}

std::vector<TableObject *> *View::getObjectList(ObjectType obj_type)
{
	if(obj_type==ObjectType::Trigger)
		return &triggers;

	if(obj_type==ObjectType::Rule)
		return &rules;

	if(obj_type==ObjectType::Index)
		return &indexes;

	return nullptr;
}

void View::removeObjects()
{
	while(!triggers.empty())
	{
		triggers.back()->setParentTable(nullptr);
		triggers.pop_back();
	}

	while(!rules.empty())
	{
		rules.back()->setParentTable(nullptr);
		rules.pop_back();
	}

	while(!indexes.empty())
	{
		indexes.back()->setParentTable(nullptr);
		indexes.pop_back();
	}
}

void View::operator = (View &view)
{
	QString prev_name = this->getName(true);

	(*dynamic_cast<BaseTable *>(this))=reinterpret_cast<BaseTable &>(view);

	this->pagination_enabled = view.pagination_enabled;
	this->layers = view.layers;
	this->materialized=view.materialized;
	this->recursive=view.recursive;
	this->with_no_data=view.with_no_data;
	this->references=view.references;
	this->custom_cols=view.custom_cols;

	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

std::vector<BaseObject *> View::getObjects(const std::vector<ObjectType> &excl_types)
{
	std::vector<BaseObject *> list;
	std::vector<ObjectType> types={ ObjectType::Trigger, ObjectType::Index, ObjectType::Rule };

	for(auto type : types)
	{
		if(std::find(excl_types.begin(), excl_types.end(), type) != excl_types.end())
			continue;

		list.insert(list.end(), getObjectList(type)->begin(), getObjectList(type)->end()) ;
	}

	return list;
}

QString View::getDataDictionary(bool split, bool md_format, const attribs_map &extra_attribs)
{
	attribs_map attribs, aux_attrs;
	QStringList tab_names, col_names;
	QString link_dict_file = GlobalAttributes::getDictSchemaFilePath(md_format, Attributes::Link);

	attribs.insert(extra_attribs.begin(), extra_attribs.end());
	attribs[Attributes::Type] = getTypeName();
	attribs[Attributes::TypeClass] = getSchemaName();
	attribs[Attributes::Split] = split ? Attributes::True : "";
	attribs[Attributes::Name] = obj_name;
	attribs[Attributes::Schema] = schema ? schema->getName() : "";
	attribs[Attributes::Comment] = comment;
	attribs[Attributes::Objects] = "";

	aux_attrs[Attributes::Split] = attribs[Attributes::Split];

	try
	{
		for(auto &ref : references)
		{
			if(ref.getObject())
			{
				Column *col = dynamic_cast<Column *>(ref.getObject());
				BaseTable *tab = dynamic_cast<BaseTable *>(ref.getObject());

				aux_attrs[Attributes::Name] = col ? col->getParentTable()->getSignature() : tab->getSignature();
				aux_attrs[Attributes::Name].remove('"');
				tab_names.push_back(schparser.getSourceCode(link_dict_file, aux_attrs));
			}
		}

		tab_names.removeDuplicates();
		attribs[Attributes::References] = tab_names.join(", ");
		attribs[Attributes::Columns] = "";
		aux_attrs.clear();

		for(auto &col : gen_columns)
		{
			aux_attrs[Attributes::Parent] = getSchemaName();
			aux_attrs[Attributes::Name] = col.getName();
			aux_attrs[Attributes::Type] = col.getType();

			schparser.ignoreUnkownAttributes(true);
			attribs[Attributes::Columns] += schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, BaseObject::getSchemaName(ObjectType::Column)), aux_attrs);
			aux_attrs.clear();
		}

		for(auto &obj : triggers)
		{
			attribs[Attributes::Triggers] +=
					dynamic_cast<Trigger *>(obj)->getDataDictionary(md_format, {{ Attributes::Split, attribs[Attributes::Split] }});
		}

		for(auto &obj : indexes)
			attribs[Attributes::Indexes] +=  dynamic_cast<Index *>(obj)->getDataDictionary(md_format);

		schparser.ignoreUnkownAttributes(true);
		attribs[Attributes::Objects] += schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, Attributes::Objects), attribs);

		schparser.ignoreEmptyAttributes(true);
		return schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, getSchemaName()), attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QString View::getAlterCode(BaseObject *object)
{
	try
	{
		attributes[Attributes::Materialized] = (materialized ? Attributes::True : "");
		return BaseTable::getAlterCode(object);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

bool View::acceptsReplaceCommand()
{
	// Materialized views don't accept CREATE OR REPLACE command
	return !materialized;
}

void View::updateDependencies()
{
	std::vector<BaseObject *> deps;

	for(auto &ref : references)
		deps.push_back(ref.getObject());

	std::sort(deps.begin(), deps.end());
	auto end =std::unique(deps.begin(), deps.end());
	deps.erase(end, deps.end());

	BaseTable::updateDependencies(deps);
}
