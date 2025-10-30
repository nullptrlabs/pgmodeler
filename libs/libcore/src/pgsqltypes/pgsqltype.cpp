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

#include "pgsqltype.h"
#include "attributes.h"
#include "../baseobject.h"

const QStringList PgSqlType::type_names {
	"", // Reserved for Class::Null

	//Types used by the class PgSQLType
	//offsets 1 to 63
	//Note: the type char is different from "char" (with quotes)
	//Reference: http://www.postgresql.org/docs/9.2/static/datatype-character.html
	"smallint", "integer", "bigint", "decimal", "numeric",
	"real", "double precision", "float", "serial", "bigserial", "money",
	"character varying", "varchar", "character",  "char", "\"char\"",
	"text", "bytea",
	"timestamp", "date", "time","timetz","timestamptz",
	"interval", "boolean", "bool",
	"point", "line", "lseg", "box", "path",
	"polygon", "circle", "cidr", "inet",
	"macaddr", "macaddr8", "bit", "bit varying", "varbit", "uuid", "xml", "json", "jsonb",
	"smallserial", "int2vector", "int2", "int4", "int8", "float4", "float8",
	"bpchar", "name", "abstime", "aclitem", "gtsvector", "refcursor",
	"reltime", "tinterval", "tsquery", "tsvector", "txid_snapshot", "pg_lsn",

	//Spatial type specifics for the PostGiS extension
	//offsets 64 to 82
	"box2d","box3d","box2df","box3df",
	"geometry", "geometry_dump","geography",
	"geomval", "addbandarg", "rastbandarg",
	"raster", "reclassarg",  "unionarg",
	"\"TopoGeometry\"",
	"getfaceedges_returntype",
	"validatetopology_returntype",
	"gidx", "spheroid", "valid_detail",

	//Range-types
	//offsets 83 to 93
	"int4range", "int8range", "numrange",
	"tsrange","tstzrange","daterange",
	"int4multirange","int8multirange",
	"nummultirange", "tsmultirange",
	"tstzmultirange",

	//Object Identification type (OID)
	//offsets 94 to 108
	"oid", "regproc", "regprocedure",
	"regoper", "regoperator", "regclass",
	"regrole", "regnamespace", "regtype",
	"regconfig", "regdictionary", "xid", "cid",
	"tid", "oidvector",

	//Pseudo-types
	//offsets 109 to 133
	"\"any\"", "anyelement", "anyarray", "anynonarray", "anyenum",
	"anyrange", "anymultirange", "anycompatible", "anycompatiblearray",
	"anycompatiblenonarray", "anycompatiblerange", "anycompatiblemultirange",
	"cstring", "internal", "language_handler", "fdw_handler", "table_am_handler",
	"index_am_handler", "tsm_handler", "record", "trigger", "event_trigger",
	"pg_ddl_command", "void", "unknown"
};

std::vector<UserTypeConfig> PgSqlType::user_types;

PgSqlType::PgSqlType()
{
	type_idx = type_names.indexOf("smallint");
	reset(true);
}

PgSqlType::PgSqlType(const QString &type_name) : PgSqlType()
{
	setType(type_name);
}

PgSqlType::PgSqlType(BaseObject *ptype) : PgSqlType()
{
	setUserType(ptype);
}

PgSqlType::PgSqlType(BaseObject *ptype, unsigned dimension, unsigned length, int precision, bool with_timezone, IntervalType interv_type, SpatialType spatial_type) : PgSqlType()
{
	setUserType(ptype);
	setDimension(dimension);
	setLength(length);
	setPrecision(precision);
	setWithTimezone(with_timezone);
	setIntervalType(interv_type);
	setSpatialType(spatial_type);
}

PgSqlType::PgSqlType(const QString &type_name, unsigned dimension, unsigned length, int precision, bool with_timezone, IntervalType interv_type, SpatialType spatial_type) : PgSqlType()
{
	setType(type_name);
	setDimension(dimension);
	setLength(length);
	setPrecision(precision);
	setWithTimezone(with_timezone);
	setIntervalType(interv_type);
	setSpatialType(spatial_type);
}

PgSqlType::PgSqlType(unsigned type_id, unsigned dimension, unsigned length, int precision, bool with_timezone, IntervalType interv_type, SpatialType spatial_type) : PgSqlType()
{
	setType(type_id);
	setDimension(dimension);
	setLength(length);
	setPrecision(precision);
	setWithTimezone(with_timezone);
	setIntervalType(interv_type);
	setSpatialType(spatial_type);
}

PgSqlType PgSqlType::parseString(const QString &str)
{
	QString type_str=str.toLower().simplified(), sptype, interv;
	bool with_tz=false;
	unsigned dim=0, srid=0;
	int prec=-1, len = -1;
	int start=-1, end=-1;
	QStringList value;
	PgSqlType type;

	//Checking if the string contains one of interval types
	for(auto &interv : IntervalType::getTypes())
	{
		start = type_str.indexOf(QRegularExpression("( )" + interv.toLower()));

		if(start >= 0)
		{
			type_str.remove(start, interv.size()+1);
			break;
		}
	}

	//Check if the type contains "with time zone" descriptor
	with_tz = QRegularExpression(QRegularExpression::anchoredPattern("(.)*(with time zone)(.)*")).match(type_str).hasMatch();

	//Removes the timezone descriptor
	type_str.remove(QRegularExpression("(with)(out)*( time zone)"));

	//Count the dimension of the type and removes the array descriptor
	dim=type_str.count("[]");
	type_str.remove("[]");

	//Check if the type is a variable length type, e.g varchar(200)
	if(QRegularExpression("(.)+\\(( )*[0-9]+( )*\\)").match(type_str).hasMatch())
	{
		start=type_str.indexOf('(');
		end=type_str.indexOf(')', start);
		len=type_str.mid(start+1, end-start-1).toInt();
	}
	//Check if the type is a numeric type, e.g, numeric(10,2)
	else if(QRegularExpression("(.)+\\(( )*[0-9]+( )*(,)( )*[0-9]+( )*\\)").match(type_str).hasMatch())
	{
		start=type_str.indexOf('(');
		end=type_str.indexOf(')', start);
		value=type_str.mid(start+1, end-start-1).split(',');
		len=value[0].toInt();
		prec=value[1].toUInt();
	}
	//Check if the type is a spatial type (PostGiS), e.g, geography(POINTZ, 4296)
	else if(QRegularExpression("(.)+\\(( )*[a-z]+(( )*(,)( )*[0-9]+( )*)?\\)",
														 QRegularExpression::CaseInsensitiveOption).match(type_str).hasMatch())
	{
		start=type_str.indexOf('(');
		end=type_str.indexOf(')', start);
		value=type_str.mid(start+1, end-start-1).split(',');
		sptype=value[0].toUpper();

		if(value.size() > 1)
			srid=value[1].toUInt();
	}

	//If the string matches one of the regexp above remove the analyzed parts
	if(start >=0 && end>=0)
		type_str.remove(start, end-start+1);

	/* The resultant string must be only the name of the type without [] and ().
	NOTE: Since the string was converted to lower case at start it's necessary to get
	it's original form from the input string in order to correctly create the type. */
	type_str=str.mid(str.indexOf(type_str, 0, Qt::CaseInsensitive),type_str.length()).trimmed();

	try
	{
		try
		{
			//Creates the type based on the extracted values
			type=PgSqlType(type_str);
		}
		catch(Exception &)
		{
			/* In case of error (specially with PostGiS types) split the string to remove
				the schema name and try to create the type once more */
			QStringList typname=type_str.split('.');

			if(typname.size()==2)
				type=PgSqlType(typname[1]);
			else
			{
				/* One last try it to check if the type has an entry on user defined types
			 as pg_catalog.[type name] */
				type=PgSqlType("pg_catalog." + type_str);
			}
		}

		type.setWithTimezone(with_tz);
		type.setDimension(dim);

		if(type.isNumericType() && len > 0 && prec >=0)
		{
			type.setLength(len);
			type.setPrecision(prec);
		}
		else if(type.isDateTimeType() && len >= 0)
			type.setPrecision(len);
		else if(type.hasVariableLength() && len > 0)
			type.setLength(len);

		if(!interv.isEmpty())
			type.setIntervalType(IntervalType(interv));
		else if(!sptype.isEmpty())
			type.setSpatialType(SpatialType(sptype, srid));

		return type;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, str);
	}
}

QStringList PgSqlType::getTypes(bool oids, bool pseudos)
{
	QStringList type_list;
	unsigned total = type_names.size();

	for(unsigned idx = 1; idx < total; idx++)
	{
		if(idx < OidStart ||
			 (oids && idx >= OidStart && idx <= OidEnd) ||
			 (pseudos && idx >= PseudoStart && idx <= PseudoEnd))
			type_list.push_back(type_names[idx]);
	}

	return type_list;
}

PgSqlType::TypeCategory PgSqlType::getCategory()
{
	std::map<TypeCategory, std::function<bool(void)>> type_check_func = {
		{ TypeCategory::OidType, std::bind(&PgSqlType::isOidType, this) },
		{	TypeCategory::PolymorphicType, std::bind(&PgSqlType::isPolymorphicType, this) },
		{ TypeCategory::PseudoType, std::bind(&PgSqlType::isPseudoType, this) },
		{ TypeCategory::TimezoneType, std::bind(&PgSqlType::isTimezoneType, this) },
		{ TypeCategory::DateTimeType, std::bind(&PgSqlType::isDateTimeType, this) },
		{ TypeCategory::NumericType, std::bind(&PgSqlType::isNumericType, this) },
		{	TypeCategory::IntegerType, std::bind(&PgSqlType::isIntegerType, this) },
		{	TypeCategory::FloatPointType, std::bind(&PgSqlType::isFloatPointType, this) },
		{	TypeCategory::CharacterType, std::bind(&PgSqlType::isCharacterType, this) },
		{	TypeCategory::NetworkType, std::bind(&PgSqlType::isNetworkType, this) },
		{	TypeCategory::MonetaryType, std::bind(&PgSqlType::isMonetaryType, this) },
		{	TypeCategory::BinaryType, std::bind(&PgSqlType::isBinaryType, this) },
		{	TypeCategory::BooleanType, std::bind(&PgSqlType::isBooleanType, this) },
		{	TypeCategory::GeometricType, std::bind(&PgSqlType::isGeometricType, this) },
		{	TypeCategory::BitStringType, std::bind(&PgSqlType::isBitStringType, this) },
		{ TypeCategory::TextSearchType, std::bind(&PgSqlType::isTextSearchType, this) },
		{	TypeCategory::UuidType, std::bind(&PgSqlType::isUuidType, this) },
		{ TypeCategory::XmlType, std::bind(&PgSqlType::isXmlType, this) },
		{	TypeCategory::JsonType, std::bind(&PgSqlType::isJsonType, this) },
		{	TypeCategory::RangeType, std::bind(&PgSqlType::isRangeType, this) },
		{	TypeCategory::PostGiSType, std::bind(&PgSqlType::isPostGiSType, this) },
		{ TypeCategory::SerialType, std::bind(&PgSqlType::isSerialType, this) },
		{	TypeCategory::UserType, std::bind(&PgSqlType::isUserType, this) },
	};

	for(auto &itr : type_check_func)
	{
		if(itr.second())
			return itr.first;
	}

	return TypeCategory::OtherType;
}

unsigned PgSqlType::setType(unsigned type_id)
{
	if(type_id == Null)
		throw Exception(ErrorCode::AsgInvalidTypeObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(type_id >= static_cast<unsigned>(type_names.size()))
		return setUserType(type_id);

	return TemplateType<PgSqlType>::setType(type_id, type_names);
}

unsigned PgSqlType::setType(const QString &type_name)
{
	unsigned type_id = Null, usr_type_id = Null;

	type_id = getBaseTypeIndex(type_name);
	usr_type_id = getUserTypeIndex(type_name, nullptr);

	if(type_id == Null && usr_type_id == Null)
		throw Exception(ErrorCode::AsgInvalidTypeObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(type_id != Null)
		return TemplateType<PgSqlType>::setType(type_id, type_names);

	return setUserType(usr_type_id);
}

unsigned PgSqlType::operator = (unsigned type_id)
{
	return setType(type_id);
}

unsigned PgSqlType::operator = (const QString &type_name)
{
	return setType(type_name);
}

BaseObject *PgSqlType::getObject()
{
	if(this->isUserType())
	{
		/* return reinterpret_cast<BaseObject *>(
				user_types[this->type_idx - (PseudoEnd + 1)].ptype);*/

		return user_types[this->type_idx - (PseudoEnd + 1)].ptype;
	}

	return nullptr;
}

unsigned PgSqlType::getUserTypeConfig()
{
	if(this->isUserType())
		return (user_types[this->type_idx - (PseudoEnd + 1)].type_conf);

	return PgSqlType::Null;
}

unsigned PgSqlType::getTypeId()
{
	return !(*this);
}

QString PgSqlType::getTypeName(bool incl_dimension)
{
	if(incl_dimension)
	{
		QString type;

		type=~(*this);

		if(type!="void" && dimension > 0)
			type+=QString("[]").repeated(dimension);

		return type;
	}

	return ~(*this);
}

QString PgSqlType::getTypeSql()
{
	QString fmt_type, type, aux;
	//unsigned idx;

	type = ~(*this);
	fmt_type = type;

	//Generation the definition for the spatial types (PostGiS)
	if(type=="geometry" || type=="geography")
		fmt_type=type + (*spatial_type);
	else if(hasVariableLength())
	{
		//Configuring the precision
		if((type=="numeric" || type=="decimal") && length >= 1 && precision>=0 && precision<=static_cast<int>(length))
			aux=QString("%1(%2,%3)").arg(type_names[type_idx]).arg(length).arg(precision);
		//Configuring the length for the type
		else if(length >= 1)
			aux=QString("%1(%2)").arg(type_names[type_idx]).arg(length);
		else
			aux=type;

		fmt_type=aux;
	}
	else if(type!="numeric" && type!="decimal" && acceptsPrecision())
	{
		if(type!="interval")
		{
			aux = type_names[type_idx];

			if(precision >= 0)
				aux+=QString("(%1)").arg(precision);

			if(with_timezone)
				aux+=" with time zone";
		}
		else
		{
			aux = type_names[type_idx];

			if(interval_type!=IntervalType::Null)
				aux+=QString(" %1 ").arg(~interval_type);

			if(precision >= 0)
				aux+=QString("(%1)").arg(precision);
		}

		fmt_type=aux;
		}


	if(type!="void" && dimension > 0)
	{
		//for(idx=0; idx < dimension; idx++)
		fmt_type+=QString("[]").repeated(dimension);
	}

	return fmt_type;
}

QStringList PgSqlType::getTypes()
{
	return TemplateType<PgSqlType>::getTypes(type_names);
}

bool PgSqlType::isRegistered(const QString &type, BaseObject *pmodel)
{
	if(getBaseTypeIndex(type) != PgSqlType::Null)
		return true;

	return (getUserTypeIndex(type, nullptr, pmodel) != PgSqlType::Null);
}

bool PgSqlType::operator == (unsigned type_id)
{
	return (this->type_idx==type_id);
}

bool PgSqlType::operator == (const QString &type_name)
{
	return (type_idx == static_cast<unsigned>(type_names.indexOf(type_name)));
}

bool PgSqlType::operator != (const QString &type_name)
{
	return (!((*this)==type_name));
}

bool PgSqlType::operator != (PgSqlType type)
{
	return (type_idx != type.type_idx);
}

bool PgSqlType::operator != (unsigned type_id)
{
	return (type_idx != type_id);
}

bool PgSqlType::operator == (PgSqlType type)
{
	return (type_idx == type.type_idx);
}

bool PgSqlType::operator == (BaseObject *ptype)
{
	int idx = getUserTypeIndex("", ptype);
	return (static_cast<int>(type_idx) == idx);
}

IntervalType PgSqlType::getIntervalType()
{
	return interval_type;
}

SpatialType PgSqlType::getSpatialType()
{
	return spatial_type;
}

bool PgSqlType::isWithTimezone()
{
	return with_timezone;
}

bool PgSqlType::isOidType()
{
	return (type_idx>=OidStart && type_idx<=OidEnd);
}

bool PgSqlType::isPseudoType()
{
	return (type_idx>=PseudoStart && type_idx<=PseudoEnd);
}

unsigned PgSqlType::operator << (BaseObject *ptype)
{
	return setUserType(ptype);
}

void PgSqlType::setIntervalType(IntervalType interv_type)
{
	interval_type=interv_type;
}

void PgSqlType::setSpatialType(SpatialType spat_type)
{
	spatial_type=spat_type;
}

void PgSqlType::setWithTimezone(bool with_tz)
{
	this->with_timezone = with_tz && acceptsTimezone();
}

unsigned PgSqlType::setUserType(unsigned type_id)
{
	unsigned lim1 = PseudoEnd + 1,
					 lim2 = lim1 + PgSqlType::user_types.size();

	if(user_types.size() > 0 &&
		 (type_id >= lim1 && type_id < lim2))
	{
		type_idx = type_id;
		return type_idx;
	}

	throw Exception(ErrorCode::AsgInvalidTypeObject,PGM_FUNC,PGM_FILE,PGM_LINE);
}

unsigned PgSqlType::setUserType(BaseObject *ptype)
{
	int idx = getUserTypeIndex("", ptype);

	if(idx <= 0)
		throw Exception(ErrorCode::AsgInvalidTypeObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	type_idx = idx;
	return type_idx;
}

//void PgSqlType::addUserType(const QString &type_name, BaseObject *ptype, DatabaseModel *pmodel, UserTypeConfig::TypeConf type_conf)
void PgSqlType::addUserType(const QString &type_name, BaseObject *ptype, UserTypeConfig::TypeConf type_conf)
{
	if(!type_name.isEmpty() && ptype && ptype->getDatabase() &&
			type_conf != UserTypeConfig::AllUserTypes &&
			getUserTypeIndex(type_name, ptype, ptype->getDatabase()) == Null)
	{
		UserTypeConfig cfg;

		cfg.name = type_name;
		cfg.ptype = ptype;
		cfg.pmodel = ptype->getDatabase();
		cfg.type_conf = type_conf;
		PgSqlType::user_types.push_back(cfg);
	}
}

void PgSqlType::removeUserType(const QString &type_name, BaseObject *ptype)
{
	if(PgSqlType::user_types.size() > 0 &&
			!type_name.isEmpty() && ptype)
	{
		std::vector<UserTypeConfig>::iterator itr, itr_end;

		itr=PgSqlType::user_types.begin();
		itr_end=PgSqlType::user_types.end();

		while(itr!=itr_end)
		{
			if(itr->name==type_name && itr->ptype==ptype)
				break;

			itr++;
		}

		if(itr!=itr_end)
		{
			itr->name="__invalidated_type__";
			itr->ptype=nullptr;
			itr->invalidated=true;
		}
	}
}

void PgSqlType::renameUserType(const QString &type_name, BaseObject *ptype, const QString &new_name)
{
	if(PgSqlType::user_types.empty() ||
		 type_name.isEmpty() || !ptype || type_name == new_name)
		return;

	for(auto &tp : user_types)
	{
		if(!tp.invalidated && tp.name == type_name && tp.ptype == ptype)
		{
			tp.name = new_name;
			break;
		}
	}
}

void PgSqlType::removeUserTypes(BaseObject *pmodel)
{
	if(!pmodel)
		return;

	std::vector<UserTypeConfig>::iterator itr;
	unsigned idx=0;

	itr=user_types.begin();
	while(itr!=user_types.end())
	{
		if(itr->pmodel==pmodel)
		{
			user_types.erase(itr);
			itr=user_types.begin() + idx;
		}
		else
		{
			idx++;
			itr++;
		}
	}
}

unsigned PgSqlType::getBaseTypeIndex(const QString &type_name)
{
	QString aux_name=type_name;

	aux_name.remove("[]");
	aux_name.remove(QRegularExpression("( )(with)(out)?(.)*"));
	aux_name=aux_name.trimmed();
	return getType(aux_name, type_names);
}

unsigned PgSqlType::getUserTypeIndex(const QString &type_name, BaseObject* ptype, BaseObject *pmodel)
{
	if(user_types.size() == 0 || (type_name.isEmpty() && !ptype))
		return PgSqlType::Null;

	std::vector<UserTypeConfig>::iterator itr, itr_end;
	int idx=0;

	itr=user_types.begin();
	itr_end=user_types.end();

	while(itr!=itr_end)
	{
		if(!itr->invalidated &&
				(((!type_name.isEmpty() && itr->name==type_name) || (ptype && itr->ptype==ptype)) &&
				 ((pmodel && itr->pmodel==pmodel) || !pmodel)))
			break;

		idx++;
		itr++;
	}

	if(itr != itr_end)
		return (PseudoEnd + 1 + idx);

	return PgSqlType::Null;
}

QString PgSqlType::getUserTypeName(unsigned type_id)
{
	unsigned lim1, lim2;

	lim1=PseudoEnd + 1;
	lim2=lim1 + user_types.size();

	if(user_types.size() > 0 &&
			(type_id >= lim1 && type_id < lim2))
		return (user_types[type_id - lim1].name);

	return "";
}

void PgSqlType::getUserTypes(QStringList &type_list, BaseObject *pmodel, unsigned inc_usr_types)
{
	type_list.clear();

	for(auto &cfg : user_types)
	{
		//Only the user defined types of the specified model are retrieved
		if(!cfg.invalidated && cfg.pmodel==pmodel &&
				((inc_usr_types & cfg.type_conf) == cfg.type_conf))
			type_list.push_back(cfg.name);
	}
}

void PgSqlType::getUserTypes(std::vector<BaseObject *> &ptypes, BaseObject *pmodel, unsigned inc_usr_types)
{
	ptypes.clear();

	for(auto &cfg : user_types)
	{
		//Only the user defined types of the specified model are retrieved
		if(!cfg.invalidated && cfg.pmodel == pmodel &&
				((inc_usr_types & cfg.type_conf) == cfg.type_conf))
			ptypes.push_back(cfg.ptype);
	}
}

QString PgSqlType::operator ~ ()
{
	if(type_idx >= PseudoEnd + 1)
		return (user_types[type_idx - (PseudoEnd + 1)].name);

	QString name = type_names[type_idx];

	if(with_timezone && (name=="time" || name=="timestamp"))
		 name+=" with time zone";

	return name;
}

bool PgSqlType::isArrayType()
{
	return (dimension > 0);
}

bool PgSqlType::isUserType()
{
	return (type_idx > PseudoEnd);
}

bool PgSqlType::isNetworkType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() &&
				 (curr_type=="cidr" ||
					curr_type=="inet" ||
					curr_type=="macaddr" ||
					curr_type=="macaddr8"));
}

bool PgSqlType::isPostGisGeoType(const QString &type_name)
{
	return (type_name=="geography" ||
					type_name=="geometry" ||
					type_name=="geometry_dump");
}

bool PgSqlType::isPostGisBoxType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() && isPostGisBoxType(curr_type));
}

bool PgSqlType::isPostGisBoxType(const QString &type_name)
{
	return (type_name=="box2d" || type_name=="box3d" ||
					type_name=="box2df" || type_name=="box3df");
}

bool PgSqlType::isPostGiSType()
{
	return (type_idx >= PostGiSStart && type_idx <= PostGiSEnd);
}

bool PgSqlType::isPostGisGeoType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() && isPostGisGeoType(curr_type));
}

bool PgSqlType::isRangeType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() &&
					(curr_type=="int4range" || curr_type=="int8range" ||
					 curr_type=="numrange" ||	curr_type=="tsrange" ||
					 curr_type=="tstzrange" || curr_type=="daterange" ||
					 curr_type=="int4multirange" || curr_type=="int8multirange" ||
					 curr_type=="nummultirange" || curr_type=="tsmultirange" ||
					 curr_type=="tstzmultirange"));
}

bool PgSqlType::isSerialType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() &&
					(curr_type=="serial" ||
					 curr_type=="smallserial" ||
					 curr_type=="bigserial"));
}

bool PgSqlType::isDateTimeType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() &&
					(isTimezoneType() ||
						(curr_type=="time" || curr_type=="timestamp" ||
						 curr_type=="interval" || curr_type=="date")));
}

bool PgSqlType::isTimezoneType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() &&
					(curr_type == "timetz" || curr_type == "timestamptz" ||
					 curr_type == "time with time zone" || curr_type == "timestamp with time zone"));
}

bool PgSqlType::isNumericType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() &&
					(curr_type=="numeric" || curr_type=="decimal"));
}

bool PgSqlType::isFloatPointType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() &&
					(curr_type=="real" || curr_type=="double precision" ||
					 curr_type=="float4" || curr_type=="float8"));
}

bool PgSqlType::isIntegerType()
{
	QString curr_type = getTypeName(false);

	return (!isUserType() &&
					(curr_type=="smallint" || curr_type=="integer" ||
					 curr_type=="bigint" || curr_type=="int4" ||
					 curr_type=="int8" || curr_type=="int2"));
}

bool PgSqlType::hasVariableLength()
{
	QString curr_type=(!isUserType() ? type_names[this->type_idx] : "");

	return (!isUserType() &&
					(curr_type=="numeric" || curr_type=="decimal" ||
					 curr_type=="character varying" || curr_type=="varchar" ||
					 curr_type=="character" || curr_type=="char" ||
					 curr_type=="bit" || curr_type=="bit varying" ||
					 curr_type=="varbit"));
}

bool PgSqlType::isCharacterType()
{
	QString curr_type = getTypeName(false); //(!isUserType() ? type_names[this->type_idx] : "");

	return !isUserType() &&
				 (curr_type=="\"char\"" || curr_type=="char" ||
					curr_type=="character" || curr_type=="varchar" ||
					curr_type=="character varying" || curr_type=="text");
}

bool PgSqlType::isPolymorphicType()
{
	QString curr_type = getTypeName(false); //(!isUserType() ? type_names[this->type_idx] : "");

	return !isUserType() &&
				 (curr_type=="anyarray" || curr_type=="anyelement" ||
					curr_type=="anyenum" || curr_type=="anynonarray" ||
					curr_type=="anyrange" || curr_type=="\"any\"");
}

bool PgSqlType::isMonetaryType()
{
	QString curr_type = getTypeName(false); //(!isUserType() ? type_names[this->type_idx] : "");

	return !isUserType() && curr_type=="money";
}

bool PgSqlType::isBinaryType()
{
	QString curr_type = getTypeName(false); //(!isUserType() ? type_names[this->type_idx] : "");

	return !isUserType() && curr_type=="bytea";
}

bool PgSqlType::isBooleanType()
{
	QString curr_type = getTypeName(false);

	return !isUserType() &&
			(curr_type=="bool" || curr_type == "boolean");
}

bool PgSqlType::isGeometricType()
{
	QString curr_type = getTypeName(false);

	return !isUserType() &&
			(curr_type=="point" || curr_type=="line" ||
			 curr_type=="lseg" ||	curr_type=="box" ||
			 curr_type=="path" || 	curr_type=="polygon" ||
			 curr_type=="circle");
}

bool PgSqlType::isBitStringType()
{
	QString curr_type = getTypeName(false);

	return !isUserType() &&
			(curr_type=="bit" || curr_type=="bit varying" || curr_type == "varbit");
}

bool PgSqlType::isTextSearchType()
{
	QString curr_type = getTypeName(false);

	return !isUserType() &&
			(curr_type=="tsquery" || curr_type=="tsvector");
}

bool PgSqlType::isUuidType()
{
	QString curr_type = getTypeName(false);

	return !isUserType() && curr_type=="uuid";
}

bool PgSqlType::isXmlType()
{
	QString curr_type = getTypeName(false);

	return !isUserType() && curr_type=="xml";
}

bool PgSqlType::isJsonType()
{
	QString curr_type = getTypeName(false);

	return !isUserType() &&
			(curr_type=="json" || curr_type=="jsonb");
}

bool PgSqlType::acceptsPrecision()
{
	return (isNumericType() ||
					(!isUserType() && type_names[this->type_idx]!="date" && isDateTimeType()));
}

bool PgSqlType::acceptsTimezone()
{
	return !isUserType() &&
				 (type_names[this->type_idx] == "time"  ||
					type_names[this->type_idx] == "timestamp");
}

void PgSqlType::reset(bool all_attrs)
{
	setIntervalType(IntervalType::Null);
	setSpatialType(SpatialType());
	setPrecision(-1);
	setLength(0);

	if(all_attrs)
	{
		setWithTimezone(false);
		setDimension(0);
	}
}

bool PgSqlType::canCastTo(PgSqlType type)
{
	// If the types are the same of belongs to the same category they naturally can be casted
	if(this->type_idx==type.type_idx ||
		(isCharacterType() && type.isCharacterType()) ||
		(isDateTimeType() && type.isDateTimeType()) ||
		(isNumericType() && type.isNumericType()) ||
		(isNetworkType() && type.isNetworkType()) ||

		//Polymorphics anyarray, anyrange, anynoarray, anyenum to anyelement
		((isPolymorphicType() && type=="anyelement") ||
		 ((*this)=="anyelement" && type.isPolymorphicType())) ||

		//Character to network address
		((isCharacterType() || isNetworkType()) &&
		 (type.isCharacterType() || type.isNetworkType())) ||

		//Integer to OID
		((isIntegerType() || isOidType()) &&
		 (type.isIntegerType() || type.isOidType())) ||

		//abstime to integer
		((((*this)=="integer" || (*this)=="int4") && type=="abstime") ||
		 (((*this)=="abstime" && (type=="integer" || type=="int4")))))

		return true;

	return false;
}

bool PgSqlType::isEquivalentTo(PgSqlType type)
{
	unsigned this_idx=0, type_idx=0;
	static std::vector<QStringList> types={{"int2","smallint"},
																		{"int4","integer"},
																		{"int8","bigint"},
																		{"decimal","numeric"},
																		{"character varying","varchar"},
																		{"character", "char"},
																		{"bool", "boolean"},
																		{"bit varying","varbit"},
																		{"oid","regproc","regprocedure",
																		 "regoper","regoperator","regclass",
																		 "regtype","regconfig","regdictionary"},
																		{"timestamptz","timestamp with time zone"},
																		{"timestamp","timestamp without time zone"}};

	//If the types are equal there is no need to perform further operations
	if(*this == type)
		return true;

	if(isUserType() == type.isUserType() &&
		 getTypeSql() == type.getTypeSql())
		return true;

	//Getting the index which the this type is in
	for(auto &list : types)
	{
		if(list.contains(~(*this))) break;
		this_idx++;
	}

	//Getting the index which 'type' is in
	for(auto &list : types)
	{
		if(list.contains(~type)) break;
		type_idx++;
	}

	return (this_idx < types.size() && type_idx < types.size() &&
					this_idx==type_idx &&
					this->isArrayType()==type.isArrayType());
}

bool PgSqlType::isExactTo(PgSqlType type)
{
	return (this->type_idx == type.type_idx &&
				 this->dimension == type.dimension &&
				 this->length == type.length &&
				 this->precision == type.precision &&
				 this->with_timezone == type.with_timezone &&
				 this->interval_type == type.interval_type &&
				 this->spatial_type == type.spatial_type);
}

PgSqlType PgSqlType::getAliasType()
{
	if(!isUserType())
	{
		if(type_names[this->type_idx]=="serial")
			return { "integer" };

		if(type_names[this->type_idx]=="smallserial")
			return { "smallint" };

		if(type_names[this->type_idx]=="bigserial")
			return { "bigint" };

		return { type_names[this->type_idx] };
	}

	return *this;
}

void PgSqlType::setDimension(unsigned dim)
{
	if(dim > 0 && this->isUserType())
	{
		int idx=getUserTypeIndex(~(*this), nullptr) - (PseudoEnd + 1);
		if(static_cast<unsigned>(idx) < user_types.size() &&
				user_types[idx].type_conf==UserTypeConfig::SequenceType)
			throw Exception(ErrorCode::AsgInvalidSequenceTypeArray,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	dimension=dim;
}

void PgSqlType::setLength(unsigned len)
{
	this->length=len;
}

void PgSqlType::setPrecision(int prec)
{
	if(!isUserType())
	{
		//Raises an error if the user tries to specify a precision > length
		if(((type_names[type_idx]=="numeric" ||
			 type_names[type_idx]=="decimal") && prec > static_cast<int>(length)))
			throw Exception(ErrorCode::AsgInvalidPrecision,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Raises an error if the precision is greater thant 6
		if(((type_names[type_idx]=="time" ||
					type_names[type_idx]=="timestamp" ||
					type_names[type_idx]=="interval") && prec > 6))
			throw Exception(ErrorCode::AsgInvalidPrecisionTimestamp,PGM_FUNC,PGM_FILE,PGM_LINE);

		this->precision=prec;
	}
}

unsigned PgSqlType::getDimension()
{
	return dimension;
}

unsigned PgSqlType::getLength()
{
	return length;
}

int PgSqlType::getPrecision()
{
	return precision;
}

QString PgSqlType::getSourceCode(SchemaParser::CodeType def_type, QString ref_type)
{
	if(def_type==SchemaParser::SqlCode)
		return getTypeSql();

	attribs_map attribs;
	SchemaParser schparser;

	attribs[Attributes::Length]="";
	attribs[Attributes::Dimension]="";
	attribs[Attributes::Precision]="";
	attribs[Attributes::WithTimezone]="";
	attribs[Attributes::IntervalType]="";
	attribs[Attributes::SpatialType]="";
	attribs[Attributes::Variation]="";
	attribs[Attributes::Srid]="";
	attribs[Attributes::RefType]=ref_type;

	attribs[Attributes::Name]=(~(*this));
	attribs[Attributes::Length]=QString("%1").arg(this->length);

	if(dimension > 0)
		attribs[Attributes::Dimension]=QString("%1").arg(this->dimension);

	if(precision >= 0)
		attribs[Attributes::Precision]=QString("%1").arg(this->precision);

	if(interval_type != IntervalType::Null)
		attribs[Attributes::IntervalType]=(~interval_type);

	if(isPostGisGeoType())
	{
		attribs[Attributes::SpatialType]=(~spatial_type);
		attribs[Attributes::Variation]=QString("%1").arg(spatial_type.getVariation());
		attribs[Attributes::Srid]=QString("%1").arg(spatial_type.getSRID());
	}

	if(with_timezone)
		attribs[Attributes::WithTimezone]=Attributes::True;

	return schparser.getSourceCode(Attributes::PgSqlBaseType, attribs, def_type);
}

QString PgSqlType::operator * ()
{
	return getTypeSql();
}
