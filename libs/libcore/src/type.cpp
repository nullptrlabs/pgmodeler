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

#include "type.h"
#include "defaultlanguages.h"

Type::Type()
{
	obj_type=ObjectType::Type;
	setConfiguration(EnumerationType);

	attributes[Attributes::BaseType]="";
	attributes[Attributes::CompositeType]="";
	attributes[Attributes::RangeType]="";
	attributes[Attributes::TypeAttribute]="";
	attributes[Attributes::EnumType]="";
	attributes[Attributes::Labels]="";
	attributes[Attributes::InputFunc]="";
	attributes[Attributes::OutputFunc]="";
	attributes[Attributes::RecvFunc]="";
	attributes[Attributes::SendFunc]="";
	attributes[Attributes::TpmodInFunc]="";
	attributes[Attributes::TpmodOutFunc]="";
	attributes[Attributes::AnalyzeFunc]="";
	attributes[Attributes::InternalLength]="";
	attributes[Attributes::ByValue]="";
	attributes[Attributes::Alignment]="";
	attributes[Attributes::Storage]="";
	attributes[Attributes::DefaultValue]="";
	attributes[Attributes::Element]="";
	attributes[Attributes::Delimiter]="";
	attributes[Attributes::ReducedForm]="";
	attributes[Attributes::Category]="";
	attributes[Attributes::Preferred]="";
	attributes[Attributes::LikeType]="";
	attributes[Attributes::Collatable]="";
	attributes[Attributes::Subtype]="";
	attributes[Attributes::SubtypeDiffFunc]="";
	attributes[Attributes::CanonicalFunc]="";
	attributes[Attributes::OpClass]="";
}

void Type::setName(const QString &name)
{
	QString prev_name;

	prev_name=this->getName(true);//this->nome;
	BaseObject::setName(name);
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void Type::setSchema(BaseObject *schema)
{
	QString prev_name;

	prev_name=this->getName(true);
	BaseObject::setSchema(schema);
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void Type::setCodeInvalidated(bool value)
{
	BaseObject::setCodeInvalidated(value);

	for(auto &typ_attr : type_attribs)
		typ_attr.setCodeInvalidated(value);
}

int Type::getAttributeIndex(const QString &attrib_name)
{
	std::vector<TypeAttribute>::iterator itr, itr_end;
	int idx=-1;

	itr=type_attribs.begin();
	itr_end=type_attribs.end();

	while(itr!=itr_end)
	{
		if(itr->getName()==attrib_name)
		{
			idx=(itr - type_attribs.begin());
			break;
		}

		itr++;
	}

	return idx;
}

void Type::addAttribute(TypeAttribute attrib)
{
	//Raises an error if the attribute has an empty name or null type
	if(attrib.getName().isEmpty() || attrib.getType()==PgSqlType::Null)
		throw Exception(ErrorCode::InsInvalidTypeAttribute,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the passed attribute has the same type as the defining type (this)
	if(PgSqlType::getUserTypeIndex(this->getName(true), this) == !attrib.getType())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvUserTypeSelfReference).arg(this->getName(true)),
										ErrorCode::InvUserTypeSelfReference,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	//Raises an error when the attribute already exists
	if(getAttributeIndex(attrib.getName()) >= 0)
		throw Exception(ErrorCode::InsDuplicatedItems,PGM_FUNC,PGM_FILE,PGM_LINE);

	type_attribs.push_back(attrib);
	setCodeInvalidated(true);
}

void Type::removeAttribute(unsigned attrib_idx)
{
	//Raises an error if the attribute index is out of bound
	if(attrib_idx >= type_attribs.size())
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	type_attribs.erase(type_attribs.begin() + attrib_idx);
	setCodeInvalidated(true);
}

void Type::removeAttributes()
{
	type_attribs.clear();
	setCodeInvalidated(true);
}

void Type::addEnumeration(const QString &enum_name)
{
	//Raises an error if the enumaration name is empty
	if(enum_name.isEmpty())
		throw Exception(ErrorCode::InsInvalidEnumerationItem,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the enumeration name is invalid (exceeds the maximum length)
	if(enum_name.size() > BaseObject::ObjectNameMaxLength)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgEnumLongName).arg(enum_name).arg(this->getName(true)),
										ErrorCode::AsgEnumLongName,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	//Raises an error if the enumeration already exists
	if(enumerations.contains(enum_name))
		throw Exception(ErrorCode::InsDuplicatedEnumerationItem,PGM_FUNC,PGM_FILE,PGM_LINE);

	enumerations.append(enum_name);
	setCodeInvalidated(true);
}

void Type::removeEnumeration(unsigned enum_idx)
{
	if(enum_idx >= static_cast<unsigned>(enumerations.size()))
		throw Exception(ErrorCode::RefEnumerationInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	enumerations.removeAt(enum_idx);
	setCodeInvalidated(true);
}

void Type::removeEnumerations()
{
	enumerations.clear();
	setCodeInvalidated(true);
}

void Type::setConfiguration(TypeConfig conf)
{
	//Raises an error if the configuration type is invalid
	if(conf > RangeType)
		throw Exception(ErrorCode::AsgInvalidTypeConfiguration,PGM_FUNC,PGM_FILE,PGM_LINE);

	type_attribs.clear();
	enumerations.clear();

	for(auto &func : functions)
		func = nullptr;

	setCollation(nullptr);
	subtype_opclass=nullptr;

	alignment="integer";
	delimiter='\0';
	storage=StorageType::Plain;
	element="\"any\"";
	internal_len=0;
	category=CategoryType::UserDefined;
	preferred=collatable=by_value=false;
	like_type="\"any\"";

	this->config=conf;
	setCodeInvalidated(true);
}

void Type::setFunction(FunctionId func_id, Function *func)
{
	unsigned param_count=0;

	//Raises an error if the function id is invalid
	if(func_id > SubtypeDiffFunc)
		throw Exception(ErrorCode::RefFunctionInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(func)
		param_count=func->getParameterCount();

	/* Raises an error if the user try to reference a function id which is incompatible according
	to the type's configuraiton */
	if((config==BaseType && func_id >= CanonicalFunc) ||
			(config==RangeType && func_id <= AnalyzeFunc))
		throw Exception(ErrorCode::RefInvalidFunctionIdTypeConfig,PGM_FUNC,PGM_FILE,PGM_LINE);

	/* Raises an error if the function isn't defined and the function id is INPUT or OUTPUT,
		because this function is mandatory for base types */
	if(!func && (func_id==InputFunc || func_id==OutputFunc))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedFunction)
										.arg(this->getName(true))
										.arg(BaseObject::getTypeName(ObjectType::Type)),
										ErrorCode::AsgNotAllocatedFunction,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	if(func)
	{
		/* Raises an error if the function language is not C.
		 Functions assigned to base type must be written in C */
		if((func_id!=CanonicalFunc && func_id!=SubtypeDiffFunc) &&
				func->getLanguage()->getName().toLower() != DefaultLanguages::C &&
				func->getLanguage()->getName().toLower() != DefaultLanguages::Internal)
			throw Exception(ErrorCode::AsgFunctionInvalidLanguage,PGM_FUNC,PGM_FILE,PGM_LINE);

		/* Raises an error if the parameter count for INPUT and RECV functions
		 is different from 1 or 3. */
		if((param_count!=1 && param_count!=3 &&
				 (func_id==InputFunc || func_id==RecvFunc)) ||
				(param_count!=2 && func_id==SubtypeDiffFunc) ||
				(param_count!=1 &&
				 (func_id==OutputFunc   || func_id==SendFunc ||
				  func_id==TpmodInFunc || func_id==TpmodOutFunc ||
				  func_id==AnalyzeFunc  || func_id==CanonicalFunc)))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParamCount)
											.arg(this->getName())
											.arg(BaseObject::getTypeName(ObjectType::Type)),
											ErrorCode::AsgFunctionInvalidParamCount,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		/* Checking the return types of function in relation to type.
		 INPUT, RECV and CANONICAL functions must return the data type that is being defined according to the
		 documentation, but to facilitate the implementation the function must return data type
		 'any' which will be replaced by the defined type name at the moment of generation of SQL.
		 OUTPUT and TPMOD_OUT should return cstring.
		 The other functions SEND, TPMOD_IN and ANALYZE should return bytea, integer and boolean,
		 respectively. Raises an error if some of conditions above is not satisfied. */
		if((func_id==InputFunc && func->getReturnType()!="\"any\"") ||
				(func_id==OutputFunc && func->getReturnType()!="cstring") ||
				(func_id==RecvFunc && func->getReturnType()!="\"any\"") ||
				(func_id==SendFunc && func->getReturnType()!="bytea") ||
				(func_id==TpmodInFunc && func->getReturnType()!="integer") ||
				(func_id==TpmodOutFunc && func->getReturnType()!="cstring") ||
				(func_id==AnalyzeFunc && func->getReturnType()!="boolean") ||
				(func_id==CanonicalFunc && func->getReturnType()!="\"any\"") ||
				(func_id==SubtypeDiffFunc && func->getReturnType()!="double precision"))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidReturnType)
											.arg(this->getName())
											.arg(BaseObject::getTypeName(ObjectType::Type)),
											ErrorCode::AsgFunctionInvalidReturnType,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		/* Validating the parameter types of function in relation to the type configuration.
		 The INPUT function must have parameters with type (cstring, oid, integer).
		 SEND, OUTPUT, CANONICAL must have a parameter of the same type being defined
		 in this case, to facilitate implementation simply use a type parameter "any".
		 The RECV function must have parameters (internal, oid, integer).
		 The function TPMOD_IN must have a type parameter (ctring []).
		 TPMOD_OUT function must have a parameter of type (integer).
		 The ANALYZE function must have a parameter of type (internal).
		 Raises an error if some of above conditions is not satisfied.*/
		if((func_id==InputFunc &&
				 (func->getParameter(0).getType()!="cstring" ||
				  (param_count==3 &&
					 (func->getParameter(1).getType()!="oid" ||
					func->getParameter(2).getType()!="integer")))) ||
				(func_id==RecvFunc &&
				 (func->getParameter(0).getType()!="internal" ||
				  (param_count==3 &&
					 (func->getParameter(1).getType()!="oid" ||
					func->getParameter(2).getType()!="integer")))) ||
				((func_id==SendFunc || func_id==CanonicalFunc || func_id==OutputFunc) &&
				 func->getParameter(0).getType()!="\"any\"") ||
				(func_id==TpmodInFunc && *(func->getParameter(0).getType())!="cstring[]") ||
				(func_id==TpmodOutFunc && func->getParameter(0).getType()!="integer") ||
				(func_id==AnalyzeFunc && func->getParameter(0).getType()!="internal") ||
				(func_id==SubtypeDiffFunc &&
				 (func->getParameter(0).getType()!=this->subtype ||
				  func->getParameter(1).getType()!=this->subtype)))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParameters)
											.arg(this->getName())
											.arg(this->getTypeName()),
											ErrorCode::AsgFunctionInvalidParameters,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		func->setProtected(false);
	}

	setCodeInvalidated(functions[func_id] != func);
	functions[func_id]=func;
}

void Type::convertFunctionParameters(bool inverse_conv)
{
	unsigned i, conf_funcs[]={ InputFunc, RecvFunc, OutputFunc, SendFunc };
	Parameter param;
	Function *func=nullptr;

	for(i=0; i < 4; i++)
	{
		func=functions[conf_funcs[i]];

		if(func)
		{
			if(conf_funcs[i]==OutputFunc || conf_funcs[i]==SendFunc)
			{
				param=func->getParameter(0);
				func->removeParameter(0);

				if(!inverse_conv)
				{
					param.setType(PgSqlType(this));
					func->addParameter(param);
				}
				else
				{
					param.setType(PgSqlType("\"any\""));
					func->addParameter(param);
				}
			}
			else if(conf_funcs[i]==InputFunc || conf_funcs[i]==RecvFunc)
			{
				if(!inverse_conv)
					func->setReturnType(PgSqlType(this));
				else
					func->setReturnType(PgSqlType("\"any\""));
			}
		}
	}

	setCodeInvalidated(true);
}

void Type::setInternalLength(unsigned length)
{
	setCodeInvalidated(internal_len != length);
	internal_len=length;
}

void Type::setByValue(bool value)
{
	setCodeInvalidated(by_value != value);
	by_value=value;
}

void Type::setAlignment(PgSqlType type)
{
	QString tp=(*type);

	//Raises an error if the type assigned to the alignment is invalid according to the rule
	if(tp!="char" && tp!="smallint" && tp!="integer" && tp!="double precision")
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidAlignmentType).arg(this->getName(true)),
						ErrorCode::AsgInvalidAlignmentType,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(alignment != type);
	alignment=tp;
}

void Type::setStorage(StorageType strg)
{
	setCodeInvalidated(storage != strg);
	storage=strg;
}

void Type::setDefaultValue(const QString &value)
{
	QString def=value.trimmed();

	setCodeInvalidated(default_value != def);
	this->default_value=def;
}

void Type::setElement(PgSqlType elem)
{
	if(PgSqlType::getUserTypeIndex(this->getName(true), this) == !elem)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvUserTypeSelfReference).arg(this->getName(true)),
										ErrorCode::InvUserTypeSelfReference,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	if(elem!="\"any\"" &&
			(elem.isOidType() || elem.isPseudoType() ||
			 elem.isUserType() || elem.isArrayType()))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidElementType).arg(this->getName(true)),
										ErrorCode::AsgInvalidElementType,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	elem.reset();
	setCodeInvalidated(element != elem);
	this->element=elem;
}

void Type::setDelimiter(char delim)
{
	setCodeInvalidated(delimiter != delim);
	delimiter=delim;
}

void Type::setElementsAttribute(SchemaParser::CodeType def_type)
{
	QString str_elem;
	unsigned i, count;

	count=type_attribs.size();
	for(i=0; i < count; i++)
		str_elem+=type_attribs[i].getSourceCode(def_type);

	if(def_type==SchemaParser::SqlCode)
		str_elem.remove(str_elem.lastIndexOf(','), str_elem.size());

	attributes[Attributes::TypeAttribute]=str_elem;
}

void Type::setEnumerationsAttribute(SchemaParser::CodeType def_type)
{
	QStringList str_enum;
	attribs_map attribs;

	for(auto &enum_attr : enumerations)
	{
		if(def_type == SchemaParser::SqlCode)
			str_enum.append("'" + enum_attr + "'");
		else
		{
			attribs[Attributes::Label] = enum_attr;
			str_enum.append(schparser.getSourceCode(Attributes::EnumType, attribs, def_type));
		}
	}

	attributes[Attributes::Labels] = str_enum.join(def_type == SchemaParser::SqlCode ? "," : "");
}

void Type::setCategory(CategoryType categ)
{
	setCodeInvalidated(category != categ);
	this->category=categ;
}

void Type::setPreferred(bool value)
{
	setCodeInvalidated(preferred != value);
	this->preferred=value;
}

void Type::setCollatable(bool value)
{
	setCodeInvalidated(collatable != value);
	this->collatable=value;
}

void Type::setLikeType(PgSqlType like_type)
{
	if(PgSqlType::getUserTypeIndex(this->getName(true), this) == !like_type)
		throw Exception(Exception::getErrorMessage(ErrorCode::InvUserTypeSelfReference).arg(this->getName(true)),
						ErrorCode::InvUserTypeSelfReference,PGM_FUNC,PGM_FILE,PGM_LINE);

	like_type.reset();
	setCodeInvalidated(this->like_type != like_type);
	this->like_type=like_type;
}

void Type::setSubtype(PgSqlType subtype)
{
	if(PgSqlType::getUserTypeIndex(this->getName(true), this) == !subtype)
		throw Exception(Exception::getErrorMessage(ErrorCode::InvUserTypeSelfReference).arg(this->getName(true)),
						ErrorCode::InvUserTypeSelfReference,PGM_FUNC,PGM_FILE,PGM_LINE);

	subtype.reset();
	setCodeInvalidated(this->subtype != subtype);
	this->subtype=subtype;
}

void Type::setSubtypeOpClass(OperatorClass *opclass)
{
	if(opclass && opclass->getIndexingType()!=IndexingType::Btree)
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidOpClassObject)
						.arg(this->getName(true))
						.arg(this->getTypeName()),
						ErrorCode::AsgInvalidOpClassObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(subtype_opclass != opclass);
	subtype_opclass=opclass;
}

TypeAttribute Type::getAttribute(unsigned attrib_idx)
{
	if(attrib_idx >= type_attribs.size())
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return type_attribs[attrib_idx];
}

unsigned Type::getAttributeCount()
{
	return type_attribs.size();
}

QString Type::getEnumeration(unsigned idx_enum)
{
	if(idx_enum >= static_cast<unsigned>(enumerations.size()))
		throw Exception(ErrorCode::RefEnumerationInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return enumerations[idx_enum];
}

unsigned Type::getEnumerationCount()
{
	return enumerations.size();
}

Function *Type::getFunction(FunctionId func_id)
{
	if(func_id > SubtypeDiffFunc)
		throw Exception(ErrorCode::RefFunctionInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	return functions[func_id];
}

unsigned Type::getInternalLength()
{
	return internal_len;
}

bool Type::isByValue()
{
	return by_value;
}

PgSqlType Type::getAlignment()
{
	return alignment;
}

StorageType Type::getStorage()
{
	return storage;
}

QString Type::getDefaultValue()
{
	return default_value;
}

PgSqlType Type::getElement()
{
	return element;
}

char Type::getDelimiter()
{
	return delimiter;
}

Type::TypeConfig Type::getConfiguration()
{
	return config;
}

CategoryType Type::getCategory()
{
	return category;
}

bool Type::isPreferred()
{
	return preferred;
}

bool Type::isCollatable()
{
	return collatable;
}

PgSqlType Type::getLikeType()
{
	return like_type;
}

PgSqlType Type::getSubtype()
{
	return subtype;
}

OperatorClass *Type::getSubtypeOpClass()
{
	return subtype_opclass;
}

QString Type::getSourceCode(SchemaParser::CodeType def_type)
{
	return this->getSourceCode(def_type, false);
}

QString Type::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	QString code_def=getCachedCode(def_type, reduced_form);
	if(!code_def.isEmpty()) return code_def;

	if(config==EnumerationType)
	{
		attributes[Attributes::EnumType]=Attributes::True;
		setEnumerationsAttribute(def_type);
	}
	else if(config==CompositeType)
	{
		attributes[Attributes::CompositeType]=Attributes::True;
		setElementsAttribute(def_type);
	}
	else if(config==RangeType)
	{
		attributes[Attributes::RangeType]=Attributes::True;

		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::Subtype]=(*subtype);
		else
			attributes[Attributes::Subtype]=subtype.getSourceCode(SchemaParser::XmlCode);

		if(subtype_opclass)
		{
			if(def_type==SchemaParser::SqlCode)
				attributes[Attributes::OpClass]=subtype_opclass->getName(true);
			else
				attributes[Attributes::OpClass]=subtype_opclass->getSourceCode(def_type, true);
		}
	}
	else
	{
		attributes[Attributes::BaseType]=Attributes::True;

		if(internal_len==0 && def_type==SchemaParser::SqlCode)
			attributes[Attributes::InternalLength]="VARIABLE";
		else
			attributes[Attributes::InternalLength]=QString("%1").arg(internal_len);

		attributes[Attributes::ByValue]=(by_value ? Attributes::True : "");
		attributes[Attributes::Alignment]=(*alignment);
		attributes[Attributes::Storage]=(~storage);
		attributes[Attributes::DefaultValue]=default_value;

		if(element!="\"any\"")
			attributes[Attributes::Element]=(*element);

		if(delimiter!='\0')
			attributes[Attributes::Delimiter]=delimiter;

		attributes[Attributes::Category]=~(category);

		attributes[Attributes::Preferred]=(preferred ? Attributes::True : "");
		attributes[Attributes::Collatable]=(collatable ? Attributes::True : "");

		if(like_type!="\"any\"")
		{
			if(def_type==SchemaParser::SqlCode)
				attributes[Attributes::LikeType]=(*like_type);
			else
				attributes[Attributes::LikeType]=like_type.getSourceCode(SchemaParser::XmlCode);
		}
	}

	if(config==BaseType || config==RangeType)
	{
		unsigned i;
		QString func_attrib[]={Attributes::InputFunc,
							   Attributes::OutputFunc,
							   Attributes::RecvFunc,
							   Attributes::SendFunc,
							   Attributes::TpmodInFunc,
							   Attributes::TpmodOutFunc,
							   Attributes::AnalyzeFunc,
							   Attributes::CanonicalFunc,
							   Attributes::SubtypeDiffFunc};

		for(i=0; i < sizeof(functions)/sizeof(Function *); i++)
		{
			if(functions[i])
			{
				if(def_type==SchemaParser::SqlCode)
					attributes[func_attrib[i]]=functions[i]->getName();
				else
				{
					functions[i]->setAttribute(Attributes::RefType, func_attrib[i]);
					attributes[func_attrib[i]]=functions[i]->getSourceCode(def_type, true);
				}
			}
		}
	}

	return BaseObject::getSourceCode(def_type, reduced_form);
}

QString Type::getAlterCode(BaseObject *object)
{
	Type *type=dynamic_cast<Type *>(object);

	if(!type)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		attribs_map attribs;
		QString alter_def, prev_val;
		int attrib_idx=-1;

		alter_def=BaseObject::getAlterCode(object);

		if(this->config==type->config)
		{
			if(config==EnumerationType)
			{
				for(auto &enum_val : type->enumerations)
				{
					if(!enumerations.contains(enum_val))
					{
						attribs[Attributes::Before]="";
						if(prev_val.isEmpty())
						{
							attribs[Attributes::Before]=Attributes::True;
							prev_val=this->enumerations[0];
						}

						attribs[Attributes::Value]=enum_val;
						attribs[Attributes::ExistingValue]=prev_val;
						copyAttributes(attribs);
						alter_def+=BaseObject::getAlterCode(this->getSchemaName(), attributes, true, true);
						attribs.clear();
					}

					prev_val=enum_val;
				}
			}
			else if(config==CompositeType)
			{
				//Removing type attributes
				for(TypeAttribute attrib : this->type_attribs)
				{
					if(type->getAttributeIndex(attrib.getName()) < 0)
					{
						attribs[Attributes::Drop]=Attributes::True;
						attribs[Attributes::Attribute]=attrib.getName(true);
						copyAttributes(attribs);
						alter_def+=BaseObject::getAlterCode(this->getSchemaName(), attributes, true, true);
						attribs.clear();
						attributes[Attributes::Drop]="";
					}
				}

				for(TypeAttribute attrib : type->type_attribs)
				{
					attrib_idx=this->getAttributeIndex(attrib.getName());

					//Creating type attributes
					if(attrib_idx < 0)
					{
						attribs[Attributes::Attribute]=attrib.getName(true);
						attribs[Attributes::Type]=attrib.getType().getSourceCode(SchemaParser::SqlCode);
						attribs[Attributes::Collation]="";

						if(attrib.getCollation())
							attribs[Attributes::Collation]=attrib.getCollation()->getName(true);

						copyAttributes(attribs);
						alter_def+=BaseObject::getAlterCode(this->getSchemaName(), attributes, true, true);
					}
					//Changing type attributes
					else
					{
						attribs[Attributes::Change]=Attributes::True;

						if(!type_attribs[attrib_idx].getType().isEquivalentTo(attrib.getType()))
						{
							attribs[Attributes::Attribute]=attrib.getName(true);
							attribs[Attributes::Type]=attrib.getType().getSourceCode(SchemaParser::SqlCode);
						}

						copyAttributes(attribs);
						alter_def+=BaseObject::getAlterCode(this->getSchemaName(), attributes, true, true);
						attributes[Attributes::Change]="";
					}

					attribs.clear();
				}
			}
		}

		return alter_def;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void Type::operator = (Type &type)
{
	QString prev_name;
	unsigned i=0;

	prev_name=this->getName(true);
	*(dynamic_cast<BaseObject *>(this))=dynamic_cast<BaseObject &>(type);

	this->config=type.config;
	this->type_attribs=type.type_attribs;
	this->enumerations=type.enumerations;
	this->internal_len=type.internal_len;
	this->by_value=type.by_value;
	this->alignment=type.alignment;
	this->element=type.element;
	this->storage=type.storage;
	this->default_value=type.default_value;
	this->category=type.category;
	this->preferred=type.preferred;
	this->like_type=type.like_type;
	this->delimiter=type.delimiter;
	this->collatable=type.collatable;
	this->subtype=type.subtype;
	this->subtype_opclass=type.subtype_opclass;

	while(i < sizeof(functions)/sizeof(Function *))
	{
		this->functions[i]=type.functions[i];
		i++;
	}

	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void Type::updateDependencies()
{
	std::vector<BaseObject *> deps = {
		like_type.getObject(), subtype.getObject(), subtype_opclass
	};

	for(auto &tp_attr : type_attribs)
		deps.push_back(tp_attr.getType().getObject());

	BaseObject::updateDependencies(deps);
}

