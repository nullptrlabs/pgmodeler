#include "extension.h"
#include "pgsqltypes/pgsqltype.h"

Extension::Extension()
{
	obj_type = ObjectType::Extension;
	attributes[Attributes::CurVersion] = "";
	attributes[Attributes::OldVersion] = "";
	attributes[Attributes::Objects] = "";
}

void Extension::setSchema(BaseObject *schema)
{
	/* We raise an error if the user tries to move the extension to a schema
	 * which is created by the extension itself */
	if(schema && schema->isDependingOn(this))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvSchemaExtension).arg(schema->getName(), obj_name),
										ErrorCode::AsgInvSchemaExtension, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	if(!schema)
		this->schema = schema;
	else
	{
		QString new_type_sig;

		/* Checking if the schema changing will cause name conflicts with other data types
		 * that are already registered in that schema */
		for(auto &ext_obj : ext_objects[ObjectType::Type])
		{
			if(!ext_obj.getParent().isEmpty() &&
				 ext_obj.getParent() != schema->getName())
				continue;

			// Configuring a new data type signature taking into account the new schema
			new_type_sig = QString("%1.%2").arg(schema->getName(true, false), BaseObject::formatName(ext_obj.getName()));

			// If we find a data type with that signature
			if(PgSqlType::getUserTypeIndex(new_type_sig, nullptr, getDatabase()) != PgSqlType::Null)
			{
				PgSqlType user_type(new_type_sig);

				// If the type is not a child of this extension we raise a conflict error
				if(!user_type.getObject()->isDependingOn(this))
				{
					throw Exception(Exception::getErrorMessage(ErrorCode::AsgSchExtTypeConflict)
													.arg(schema->getName(), obj_name, ext_obj.getName(), BaseObject::getTypeName(ObjectType::Type)),
													ErrorCode::AsgSchExtTypeConflict, PGM_FUNC, PGM_FILE, PGM_LINE);
				}
			}
		}

		BaseObject::setSchema(schema);
	}
}

void Extension::addObject(const ExtObject &ext_obj)
{
	if(!ext_obj.isValid())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvExtensionObject).arg(obj_name),
										ErrorCode::InvExtensionObject, PGM_FUNC, PGM_FILE, PGM_LINE, nullptr,
										QString(QT_TR_NOOP("Invalid object: %1 (%2)")).arg(ext_obj.getSignature(),
																																			 BaseObject::getTypeName(ext_obj.getType())));
	}

	if(containsObject(ext_obj))
		return;

	ext_objects[ext_obj.getType()].push_back(ext_obj);
	setCodeInvalidated(true);
}

void Extension::removeObjects(ObjectType obj_type)
{
	if(obj_type == ObjectType::BaseObject)
		ext_objects.clear();
	else
		ext_objects.erase(obj_type);
}

const std::vector<Extension::ExtObject> &Extension::getObjects(ObjectType obj_type)
{
	if(obj_type != ObjectType::Type && obj_type != ObjectType::Schema)
	{
		throw Exception(ErrorCode::ObtObjectInvalidType, PGM_FUNC, PGM_FILE, PGM_LINE, nullptr,
										QString(QT_TR_NOOP("Invalid object type: %1")).arg(BaseObject::getTypeName(obj_type)));
	}

	return ext_objects[obj_type];
}

void Extension::setVersion(VersionId ver, const QString &value)
{
	if(ver > OldVersion)
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(versions[ver] != value);
	versions[ver]=value;
}

QString Extension::getVersion(VersionId ver)
{
	if(ver > OldVersion)
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return versions[ver];
}

bool Extension::containsObject(const ExtObject &ext_obj)
{
	if(!ext_obj.isValid() || !ext_objects.count(ext_obj.getType()))
		return false;

	QString obj_signature;

	for(auto &obj : ext_objects[ext_obj.getType()])
	{
		/* If the extension object is a schema child one and the attribute
		 * parent is empty, we adopt the name of the same schema of the
		 * parent extension in the comparison below */
		if(BaseObject::isChildObjectType(ObjectType::Schema, obj.getType()) &&
			 obj.getParent().isEmpty())
		{
			obj_signature = BaseObject::formatName(schema->getName()) + "." +
											BaseObject::formatName(obj.getName());
		}
		else
			obj_signature = obj.getSignature();

		/* If the extension objects themselves aren't equal,
		 * as a fallback, we compare the signature and their types */
		if(obj == ext_obj ||
			 (!obj_signature.isEmpty() &&
				obj_signature == ext_obj.getSignature()))
			return true;
	}

	return false;
}

QString Extension::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	attributes[Attributes::Name] = getName(def_type==SchemaParser::SqlCode, false);
	attributes[Attributes::CurVersion] = versions[CurVersion];
	attributes[Attributes::OldVersion] = versions[OldVersion];
	attributes[Attributes::Types] = "";

	if(def_type == SchemaParser::XmlCode && !ext_objects.empty())
	{
		attribs_map obj_attr;

		for(auto &ext_obj_typ : { ObjectType::Schema, ObjectType::Type })
		{
			for(auto &ext_obj : ext_objects[ext_obj_typ])
			{
				obj_attr[Attributes::Name] = ext_obj.getName();
				obj_attr[Attributes::Type] = BaseObject::getSchemaName(ext_obj.getType());
				obj_attr[Attributes::Parent] = ext_obj.getParent();

				schparser.ignoreUnkownAttributes(true);
				schparser.ignoreEmptyAttributes(true);
				attributes[Attributes::Objects] += schparser.getSourceCode(Attributes::Object, obj_attr, def_type);
			}
		}
	}

	return BaseObject::__getSourceCode(def_type);
}

QString Extension::getAlterCode(BaseObject *object)
{
	Extension *ext=dynamic_cast<Extension *>(object);

	if(!ext)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		attributes[Attributes::AlterCmds]=BaseObject::getAlterCode(object);
		attributes[Attributes::NewVersion]="";

		if(!this->versions[CurVersion].isEmpty() && !ext->versions[CurVersion].isEmpty() &&
				this->versions[CurVersion].isEmpty() < ext->versions[CurVersion].isEmpty())
			attributes[Attributes::NewVersion]=ext->versions[CurVersion];

		return BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QString Extension::getDropCode(bool cascade)
{
	attributes[Attributes::Name] = getName(true);
	return BaseObject::getDropCode(cascade);
}

QString Extension::getSignature(bool format)
{
	return getName(format, false);
}

QString Extension::getName(bool format, bool prepend_schema)
{
	return BaseObject::getName(format, prepend_schema);
}
