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

#include "baserelationship.h"
#include "table.h"
#include "doublenan.h"
#include <QApplication>

BaseRelationship::BaseRelationship(BaseRelationship *rel)
{
	if(!rel)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	for(auto & lable : lables)
		lable = nullptr;

	src_table=dst_table=nullptr;

	(*(this))=(*rel);

	custom_color=QColor(Qt::transparent);
	reference_fk = nullptr;
}

BaseRelationship::BaseRelationship(RelType rel_type, BaseTable *src_tab, BaseTable *dst_tab, bool src_mandatory, bool dst_mandatory)
{
	try
	{
		QString str_aux;

		this->connected=false;
		this->src_mandatory=src_mandatory;
		this->dst_mandatory=dst_mandatory;
		this->rel_type=rel_type;
		this->custom_color=QColor(Qt::transparent);
		this->reference_fk=nullptr;
		this->src_table=src_tab;
		this->dst_table=dst_tab;


		for(unsigned i=0; i < 3; i++)
		{
			lables[i]=nullptr;
			lables_dist[i]=QPointF(DNaN, DNaN);
		}

		configureRelationship();

		str_aux = qApp->translate("BaseRelationship","rel_%1_%2","")
				.arg(src_tab->getName()).arg(dst_tab->getName());

		if(str_aux.size() > BaseObject::ObjectNameMaxLength)
			str_aux.resize(BaseObject::ObjectNameMaxLength);

		setName(str_aux);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void BaseRelationship::configureRelationship()
{
	obj_type=ObjectType::BaseRelationship;

	attributes[Attributes::Type]="";
	attributes[Attributes::SrcRequired]="";
	attributes[Attributes::DstRequired]="";
	attributes[Attributes::SrcTable]="";
	attributes[Attributes::DstTable]="";
	attributes[Attributes::Points]="";
	attributes[Attributes::Columns]="";
	attributes[Attributes::Constraints]="";
	attributes[Attributes::Elements]="";
	attributes[Attributes::Identifier]="";
	attributes[Attributes::ReducedForm]="";
	attributes[Attributes::Deferrable]="";
	attributes[Attributes::DeferType]="";
	attributes[Attributes::TableName]="";
	attributes[Attributes::SpecialPkCols]="";
	attributes[Attributes::RelationshipNn]="";
	attributes[Attributes::RelationshipGen]="";
	attributes[Attributes::RelationshipDep]="";
	attributes[Attributes::RelationshipPart]="";
	attributes[Attributes::Relationship1n]="";
	attributes[Attributes::Relationship11]="";
	attributes[Attributes::Constraints]="";
	attributes[Attributes::Table]="";
	attributes[Attributes::AncestorTable]="";
	attributes[Attributes::CopyOptions]="";
	attributes[Attributes::CopyMode]="";
	attributes[Attributes::SrcColPattern]="";
	attributes[Attributes::DstColPattern]="";
	attributes[Attributes::PkPattern]="";
	attributes[Attributes::UqPattern]="";
	attributes[Attributes::SrcFkPattern]="";
	attributes[Attributes::DstFkPattern]="";
	attributes[Attributes::PkColPattern]="";
	attributes[Attributes::SinglePkColumn]="";
	attributes[Attributes::UpdAction]="";
	attributes[Attributes::DelAction]="";
	attributes[Attributes::CustomColor]="";
	attributes[Attributes::ReferenceFk]="";
	attributes[Attributes::PartitionBoundExpr]="";
	attributes[Attributes::OriginalPk]="";
	attributes[Attributes::FkIdxType]="";
	attributes[Attributes::FkIdxPattern]="";
	attributes[Attributes::Index]="";

	//Check if the relationship type is valid
	if(rel_type <= RelationshipFk)
	{
		//Raises an error if one of the tables is not allocated
		if(!src_table || !dst_table)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedTable)
							.arg(this->getName())
							.arg(BaseObject::getTypeName(ObjectType::BaseRelationship)),
							ErrorCode::AsgNotAllocatedTable,PGM_FUNC,PGM_FILE,PGM_LINE);

		/* Raises an error if the relationship type is generalization or dependency
			and the source and destination table are the same. */
		if((rel_type==RelationshipGen || rel_type==RelationshipDep || rel_type==RelationshipPart) && src_table==dst_table)
			throw Exception(ErrorCode::InvInheritCopyPartRelationship,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Allocates the textbox for the name label
		lables[RelNameLabel]=new Textbox;
		lables[RelNameLabel]->setTextAttribute(Textbox::ItalicText, true);

		//Allocates the cardinality labels only when the relationship is not generalization or dependency (copy)
		if(rel_type!=RelationshipGen &&
			 rel_type!=RelationshipDep &&
			 rel_type!=RelationshipPart)
		{
			lables[SrcCardLabel]=new Textbox;
			lables[DstCardLabel]=new Textbox;
			lables[SrcCardLabel]->setTextAttribute(Textbox::ItalicText, true);
			lables[DstCardLabel]->setTextAttribute(Textbox::ItalicText, true);

			//Configures the mandatory participation for both tables
			setMandatoryTable(SrcTable,src_mandatory);
			setMandatoryTable(DstTable,dst_mandatory);
		}
	}
	else
		//Raises an error if the specified relationship typ is invalid
		throw Exception(ErrorCode::AllocationObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
}

BaseRelationship::~BaseRelationship()
{
	//Deallocates the labels
	for(auto & label : lables)
	{
		delete label;
		label = nullptr;
	}
}

void BaseRelationship::setName(const QString &name)
{
	try
	{
		BaseObject::setName(name);

		if(lables[RelNameLabel])
			lables[RelNameLabel]->setComment(name);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void BaseRelationship::setMandatoryTable(TableId table_id, bool value)
{
	QString cmin, aux;
	LabelId label_id;

	/* Raises an error if the user tries to create an relationship
		One to One where both tables are mandatory partitipation
		(1,1)-<>-(1,1). This type of relationship is not implemented because
		it requires the table fusion. */
	if(rel_type==Relationship11 &&
			((table_id==SrcTable && value && dst_mandatory) ||
			 (table_id==DstTable && value && src_mandatory)))
		throw Exception(ErrorCode::NotImplementedRelationshipType,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Case the source table is mandatory
	if(table_id==SrcTable)
	{
		src_mandatory=value;
		//Indicates that the source cardinality label will be configured
		label_id=SrcCardLabel;
	}
	else
	{
		if(rel_type!=Relationship1n)
			dst_mandatory=value;
		else
			/* For One to many (1-n) relationship the entity on the "many" side
			will be always in the format (0,n) */
			dst_mandatory=false;

		//Indicates that the destination cardinality label will be configured
		label_id=DstCardLabel;
	}

	if(!value) cmin="0";
	else cmin="1";

	if(lables[label_id])
	{
		if(rel_type==Relationship11)
			lables[label_id]->setComment(cmin + ":1");
		else if(rel_type==Relationship1n)
		{
			aux=(table_id==SrcTable ? "1" : "n");
			lables[label_id]->setComment(cmin + ":" + aux);
		}
		else if(rel_type==RelationshipFk)
		{
			if((table_id==SrcTable && dynamic_cast<Table *>(src_table)->isReferTableOnForeignKey(dynamic_cast<Table *>(dst_table))) ||
				 (!isSelfRelationship() && table_id==DstTable && dynamic_cast<Table *>(dst_table)->isReferTableOnForeignKey(dynamic_cast<Table *>(src_table))))
			{
				if(table_id == SrcTable && canSimulateRelationship11())
					aux = "1";
				else
					aux = "n";
			}
			else
				aux = "1";

			if((table_id == DstTable && dst_mandatory) ||
				 (table_id == SrcTable && src_mandatory))
				aux.prepend("1:");
			else
				aux.prepend("0:");

			lables[label_id]->setComment(aux);
		}
		else if(rel_type==RelationshipNn)
			lables[label_id]->setComment("n");

		lables[label_id]->setModified(true);
	}
}

BaseTable *BaseRelationship::getTable(TableId table_id)
{
	if(table_id==SrcTable)
		return src_table;

	if(table_id==DstTable)
		return dst_table;

	return nullptr;
}

bool BaseRelationship::isTableMandatory(TableId table_id)
{
	if(table_id==SrcTable)
		return src_mandatory;

	return dst_mandatory;
}

void BaseRelationship::setConnected(bool value)
{
	connected = value;
}

void BaseRelationship::disconnectRelationship()
{
	if(connected)
	{
		setConnected(false);
		setCodeInvalidated(true);
	}
}

void BaseRelationship::connectRelationship()
{
	if(!connected)
	{
		setConnected(true);
		setCodeInvalidated(true);

		src_table->setModified(true);

		if(src_table != dst_table)
			dst_table->setModified(true);
	}
}

Textbox *BaseRelationship::getLabel(LabelId label_id)
{
	if(label_id > RelNameLabel)
		throw Exception(ErrorCode::RefLabelInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return lables[label_id];
}

BaseRelationship::RelType BaseRelationship::getRelationshipType()
{
	return rel_type;
}

bool BaseRelationship::isRelationshipConnected()
{
	return connected;
}

bool BaseRelationship::isSelfRelationship()
{
	return (dst_table==src_table);
}

void BaseRelationship::setRelationshipAttributes()
{
	unsigned count, i;
	QString str_aux,
			label_attribs[3]={ Attributes::SrcLabel,
							   Attributes::DstLabel,
							   Attributes::NameLabel};

	attributes[Attributes::Type]=getRelTypeAttribute();
	attributes[Attributes::SrcRequired]=(src_mandatory ? Attributes::True : "");
	attributes[Attributes::DstRequired]=(dst_mandatory ? Attributes::True : "");

	if(src_table)
		attributes[Attributes::SrcTable]=src_table->getName(true);

	if(dst_table)
		attributes[Attributes::DstTable]=dst_table->getName(true);


	count=points.size();
	for(i=0; i < count; i++)
	{
		attributes[Attributes::XPos]=QString("%1").arg(points[i].x());
		attributes[Attributes::YPos]=QString("%1").arg(points[i].y());
		str_aux+=schparser.getSourceCode(Attributes::Position, attributes, SchemaParser::XmlCode);
	}
	attributes[Attributes::Points]=str_aux;

	str_aux="";
	for(i=0; i < 3; i++)
	{
		if(!std::isnan(lables_dist[i].x()))
		{
			attributes[Attributes::XPos]=QString("%1").arg(lables_dist[i].x());
			attributes[Attributes::YPos]=QString("%1").arg(lables_dist[i].y());
			attributes[Attributes::Position]=schparser.getSourceCode(Attributes::Position, attributes, SchemaParser::XmlCode);
			attributes[Attributes::RefType]=label_attribs[i];
			str_aux+=schparser.getSourceCode(Attributes::Label, attributes, SchemaParser::XmlCode);
		}
	}

	attributes[Attributes::LabelsPos]=str_aux;
	attributes[Attributes::CustomColor]=(custom_color!=Qt::transparent ? custom_color.name() : "");
	attributes[Attributes::ReferenceFk]=(reference_fk ? reference_fk->getName() : "");

	setFadedOutAttribute();
	setLayersAttribute();
}

QString BaseRelationship::getCachedCode(unsigned def_type)
{
	if(!code_invalidated &&
			((!cached_code[def_type].isEmpty()) ||
			 (def_type==SchemaParser::XmlCode  && !cached_reduced_code.isEmpty())))
	{
		if(def_type==SchemaParser::XmlCode  && !cached_reduced_code.isEmpty())
			return cached_reduced_code;
		
		return cached_code[def_type];
	}
	
	return "";
}

void BaseRelationship::setReferenceForeignKey(Constraint *ref_fk)
{
	this->reference_fk = ref_fk;
}

void BaseRelationship::configureSearchAttributes()
{
	search_attribs[Attributes::SrcTable] = src_table->getSignature();
	search_attribs[Attributes::DstTable] = dst_table->getSignature();
	search_attribs[Attributes::RelatedForeignKey] = reference_fk ? reference_fk->getSignature(true) : "";
	BaseGraphicObject::configureSearchAttributes();
}

Constraint *BaseRelationship::getReferenceForeignKey()
{
	return reference_fk;
}

bool BaseRelationship::canSimulateRelationship11()
{
	if(rel_type != BaseRelationship::RelationshipFk)
		return false;

	PhysicalTable *table = dynamic_cast<PhysicalTable *>(getTable(BaseRelationship::SrcTable));

	if(table)
	{
		Constraint *fk_constr = nullptr, *uq_constr = nullptr;

		for(auto &tab_obj : *table->getObjectList(ObjectType::Constraint))
		{
			fk_constr = dynamic_cast<Constraint *>(tab_obj);

			if(fk_constr->getConstraintType() == ConstraintType::ForeignKey)
			{
				for(auto &tab_obj1 : *table->getObjectList(ObjectType::Constraint))
				{
					uq_constr = dynamic_cast<Constraint *>(tab_obj1);

					if(uq_constr->getConstraintType() == ConstraintType::Unique &&
						 uq_constr->isColumnsExist(fk_constr->getColumns(Constraint::SourceCols), Constraint::SourceCols, true))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

QString BaseRelationship::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def = getCachedCode(def_type);

	if(!code_def.isEmpty()) 
		return code_def;

	if(def_type==SchemaParser::SqlCode)
	{
		if(rel_type!=RelationshipFk)
			return "";
		
		cached_code[def_type] = reference_fk->getSourceCode(SchemaParser::SqlCode);
		return cached_code[def_type];
	}
	
	bool reduced_form;
	setRelationshipAttributes();
	reduced_form=(attributes[Attributes::Points].isEmpty() &&
								attributes[Attributes::LabelsPos].isEmpty());

	if(!reduced_form)
		cached_reduced_code.clear();

	return BaseObject::getSourceCode(SchemaParser::XmlCode,reduced_form);	
}

void BaseRelationship::setPoints(const std::vector<QPointF> &points)
{
	setCodeInvalidated(true);
	this->points=points;
}

void BaseRelationship::setLabelDistance(LabelId label_id, QPointF label_dist)
{
	if(label_id > RelNameLabel)
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	lables_dist[label_id]=label_dist;
	setCodeInvalidated(true);
}

QPointF BaseRelationship::getLabelDistance(LabelId label_id)
{
	if(label_id > RelNameLabel)
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return lables_dist[label_id];
}

void BaseRelationship::setCustomColor(const QColor &color)
{
	custom_color=color;
	setCodeInvalidated(color != custom_color);
}

QColor BaseRelationship::getCustomColor()
{
	return custom_color;
}

void BaseRelationship::resetLabelsDistance()
{
	for(unsigned i = SrcCardLabel; i < RelNameLabel; i++)
		this->setLabelDistance(static_cast<LabelId>(i), QPointF(DNaN,DNaN));
}

std::vector<QPointF> BaseRelationship::getPoints()
{
	return points;
}

void BaseRelationship::operator = (BaseRelationship &rel)
{
	(*dynamic_cast<BaseGraphicObject *>(this))=dynamic_cast<BaseGraphicObject &>(rel);
	this->connected=false;
	this->src_table=rel.src_table;
	this->dst_table=rel.dst_table;
	this->rel_type=rel.rel_type;
	this->points=rel.points;
	this->custom_color=rel.custom_color;

	for(int i=0; i < 3; i++)
	{
		if(rel.lables[i])
		{
			if(!this->lables[i])
				this->lables[i]=new Textbox;

			(*this->lables[i])=(*rel.lables[i]);
		}
		this->lables_dist[i]=rel.lables_dist[i];
	}

	this->setMandatoryTable(SrcTable, false);
	this->setMandatoryTable(DstTable, false);

	this->setMandatoryTable(SrcTable, rel.src_mandatory);
	this->setMandatoryTable(DstTable, rel.dst_mandatory);
}

QString BaseRelationship::getRelTypeAttribute()
{
	switch(rel_type)
	{
		case Relationship11: return Attributes::Relationship11;
		case Relationship1n: return Attributes::Relationship1n;
		case RelationshipNn: return Attributes::RelationshipNn;
		case RelationshipGen: return Attributes::RelationshipGen;
		case RelationshipPart: return Attributes::RelationshipPart;
		case RelationshipFk: return Attributes::RelationshipFk;
		default:
		{
			if(src_table->getObjectType()==ObjectType::View)
				return Attributes::RelationshipTabView;
			
			return Attributes::RelationshipDep;
		}
	}
}

QString BaseRelationship::getRelationshipTypeName(RelType rel_type, bool is_view)
{
  switch(rel_type)
  {
		case Relationship11: return tr("One-to-one");
		case Relationship1n: return tr("One-to-many");
		case RelationshipNn: return tr("Many-to-many");
		case RelationshipGen: return tr("Inheritance");
		case RelationshipPart: return tr("Partitioning");
		case RelationshipFk: return tr("FK relationship");
	  default:
		{
			if(is_view)
				return tr("Dependency");
			
			return tr("Copy");
		}
  }
}

QString BaseRelationship::getRelationshipTypeName()
{
	return getRelationshipTypeName(rel_type, src_table->getObjectType()==ObjectType::View);
}

void BaseRelationship::updateDependencies()
{
	BaseObject::updateDependencies({ src_table, dst_table });
}

void BaseRelationship::setCodeInvalidated(bool value)
{
	if(src_table)
		src_table->setCodeInvalidated(value);

	if(dst_table)
		dst_table->setCodeInvalidated(value);

	BaseGraphicObject::setCodeInvalidated(value);
}
