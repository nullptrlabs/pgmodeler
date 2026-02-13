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

#include "operatorfamily.h"

OperatorFamily::OperatorFamily()
{
	obj_type=ObjectType::OpFamily;
	attributes[Attributes::IndexType]="";
}

void OperatorFamily::setIndexingType(IndexingType idx_type)
{
	setCodeInvalidated(indexing_type != idx_type);
	indexing_type=idx_type;
}

IndexingType OperatorFamily::getIndexingType()
{
	return indexing_type;
}

QString OperatorFamily::getSourceCode(SchemaParser::CodeType def_type)
{
	return this->getSourceCode(def_type, false);
}

QString OperatorFamily::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	QString code_def=getCachedCode(def_type, reduced_form);
	if(!code_def.isEmpty()) return code_def;

	attributes[Attributes::Signature]=getSignature();
	attributes[Attributes::IndexType]=(~indexing_type);
	return BaseObject::getSourceCode(def_type,reduced_form);
}

QString OperatorFamily::getSignature(bool format)
{
	return BaseObject::getSignature(format) + QString(" USING %1").arg(~indexing_type);
}
