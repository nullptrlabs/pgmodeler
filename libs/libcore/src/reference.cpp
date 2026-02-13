/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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

Reference::Reference()
{
	object = nullptr;
	use_signature = format_name = use_columns = false;
}

Reference::Reference(BaseObject *_object, const QString &_ref_name, const QString &_ref_alias,
					 bool _use_signature, bool _format_name, bool _use_columns)
{
	ref_name = _ref_name;
	ref_alias = _ref_alias;
	object = _object;
	use_signature = _use_signature;
	format_name = _format_name;
	use_columns = _use_columns;
}

BaseObject *Reference::getObject() const
{
	return object;
}

QString Reference::getRefName() const
{
	return ref_name;
}

QString Reference::getRefAlias() const
{
	return ref_alias;
}

bool Reference::isUseSignature() const
{
	return use_signature;
}

bool Reference::isFormatName() const
{
	return format_name;
}

bool Reference::isUseColumns() const
{
	return use_columns;
}

bool Reference::isValid() const
{
	return object && !ref_name.isEmpty();
}

QString Reference::getXmlCode() const
{
	SchemaParser schparser;
	attribs_map ref_attrs;

	ref_attrs[Attributes::Object] = object->getSignature();
	ref_attrs[Attributes::Type] = object->getSchemaName();
	ref_attrs[Attributes::RefName] = ref_name;
	ref_attrs[Attributes::RefAlias] = ref_alias;
	ref_attrs[Attributes::FormatName] = format_name ? Attributes::True : "";
	ref_attrs[Attributes::UseSignature] = use_signature ? Attributes::True : "";
	ref_attrs[Attributes::UseColumns] = use_columns ? Attributes::True : "";

	return schparser.getSourceCode(Attributes::Reference, ref_attrs, SchemaParser::XmlCode);
}
