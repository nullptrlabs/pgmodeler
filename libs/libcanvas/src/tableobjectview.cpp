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

#include "tableobjectview.h"
#include "rule.h"
#include "index.h"
#include "trigger.h"
#include "constraint.h"
#include "policy.h"
#include "physicaltable.h"
#include "utilsns.h"

bool TableObjectView::hide_sch_name_usr_type {false};

const QString	TableObjectView::TypeSeparator {" "};
const QString	TableObjectView::ConstrSeparator {" "};
const QString	TableObjectView::TextUnique {"uq"};
const QString	TableObjectView::TextExclude {"ex"};
const QString	TableObjectView::TextCheck {"ck"};
const QString	TableObjectView::TextPrimaryKey {"pk"};
const QString	TableObjectView::TextForeignKey {"fk"};
const QString	TableObjectView::TextNotNull {"nn"};
const QString	TableObjectView::ConstrDelimStart {"«"};
const QString	TableObjectView::ConstrDelimEnd {"»"};

TableObjectView::TableObjectView(TableObject *object) : BaseObjectView(object)
{
	descriptor=nullptr;
	fake_selection=false;

	for(auto & label : lables)
		label = new QGraphicsSimpleTextItem;

	delete obj_selection;
}

TableObjectView::~TableObjectView()
{
	delete descriptor;

	for(auto & label : lables)
		delete label;

	delete obj_selection;
}

void TableObjectView::configureDescriptor(ConstraintType constr_type)
{
	ObjectType obj_type=ObjectType::BaseObject;
	Column *column=dynamic_cast<Column *>(this->getUnderlyingObject());
	bool ellipse_desc=false;

	double factor=BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor();
	QPen pen;

	//Based upon the source object type the descriptor is allocated
	if(this->getUnderlyingObject())
		obj_type=this->getUnderlyingObject()->getObjectType();

	/* Elliptical descriptor is used to columns (with or without not-null constraint),
		for other object types, polygonal descriptor is usded */
	ellipse_desc=((column && constr_type==ConstraintType::Null) || (!TableObject::isTableObject(obj_type)));

	if(descriptor && ((ellipse_desc && !dynamic_cast<QGraphicsEllipseItem *>(descriptor)) ||
										(!ellipse_desc && dynamic_cast<QGraphicsEllipseItem *>(descriptor))))
	{
		delete descriptor;
		descriptor=nullptr;
	}

	if(!descriptor)
	{
		if(ellipse_desc)
			descriptor=new QGraphicsEllipseItem;
		else
			descriptor=new QGraphicsPolygonItem;
	}

	if(column)
	{
		QString attrib;
		QPolygonF pol;

		if(constr_type==ConstraintType::Null)
		{
			QGraphicsEllipseItem *desc=dynamic_cast<QGraphicsEllipseItem *>(descriptor);

			desc->setRect(QRectF(QPointF(0,0), QSizeF(10.0 * factor, 10.0 * factor)));

			if(column->isNotNull())
				attrib=Attributes::NnColumn;
			else
				attrib=Attributes::Column;

			desc->setBrush(this->getFillStyle(attrib));

			pen = this->getBorderStyle(attrib);
			pen.setWidthF(pen.widthF() * 1.15);
			desc->setPen(pen);
		}
		else
		{
			QGraphicsPolygonItem *desc=dynamic_cast<QGraphicsPolygonItem *>(descriptor);

			if(constr_type==ConstraintType::PrimaryKey)
			{
				attrib=Attributes::PkColumn;
				pol.append(QPointF(2,0)); pol.append(QPointF(0,2)); pol.append(QPointF(0,7));
				pol.append(QPointF(2,9)); pol.append(QPointF(3,8)); pol.append(QPointF(3,6));
				pol.append(QPointF(4,6)); pol.append(QPointF(5,7)); pol.append(QPointF(6,6));
				pol.append(QPointF(7,5)); pol.append(QPointF(9,7)); pol.append(QPointF(9,3));
				pol.append(QPointF(3,3)); pol.append(QPointF(3,1));				
			}
			else if(constr_type==ConstraintType::ForeignKey)
			{
				attrib=Attributes::FkColumn;
				pol.append(QPointF(0,3)); pol.append(QPointF(0,6)); pol.append(QPointF(4,6));
				pol.append(QPointF(4,9)); pol.append(QPointF(5,9)); pol.append(QPointF(9,5));
				pol.append(QPointF(9,4)); pol.append(QPointF(5,0)); pol.append(QPointF(4,0));
				pol.append(QPointF(4,3));
			}
			else if(constr_type==ConstraintType::Unique)
			{
				attrib=Attributes::UqColumn;
				pol.append(QPointF(4,0)); pol.append(QPointF(0,4)); pol.append(QPointF(0,5));
				pol.append(QPointF(4,9)); pol.append(QPointF(5,9)); pol.append(QPointF(9,5));
				pol.append(QPointF(9,4)); pol.append(QPointF(5,0));
			}

			resizePolygon(pol,
										(pol.boundingRect().width() + HorizSpacing) * factor,
										(pol.boundingRect().height() + VertSpacing) * factor);

			desc->setPolygon(pol);
			desc->setBrush(this->getFillStyle(attrib));

			pen = this->getBorderStyle(attrib);
			pen.setWidthF(pen.widthF() * 1.15);
			desc->setPen(pen);
		}
	}
	else if(obj_type != ObjectType::BaseObject)
	{
		TableObject *tab_obj=dynamic_cast<TableObject *>(this->getUnderlyingObject());
		QGraphicsPolygonItem *desc=dynamic_cast<QGraphicsPolygonItem *>(descriptor);
		QPolygonF pol;

		pol.append(QPointF(5,0)); pol.append(QPointF(0,5)); pol.append(QPointF(4,9));
		pol.append(QPointF(9,9)); pol.append(QPointF(9,4));

		resizePolygon(pol,
									(pol.boundingRect().width() + HorizSpacing) * factor,
									(pol.boundingRect().height() + VertSpacing) * factor);

		desc->setPolygon(pol);
		desc->setBrush(this->getFillStyle(tab_obj->getSchemaName()));

		pen = this->getBorderStyle(tab_obj->getSchemaName());
		pen.setWidthF(pen.widthF() * 1.15);
		desc->setPen(pen);
	}
	else
	{
		QGraphicsEllipseItem *desc=dynamic_cast<QGraphicsEllipseItem *>(descriptor);

		desc->setRect(QRectF(QPointF(0,0), QSizeF(9.0 * factor, 9.0 * factor)));
		desc->setBrush(this->getFillStyle(Attributes::Reference));

		pen = this->getBorderStyle(Attributes::Reference);
		pen.setWidthF(pen.widthF() * 1.15);
		desc->setPen(pen);
	}
}

QString TableObjectView::formatUserTypeName(PgSqlType type)
{
	QString fmt_type_name = *type;

	if(type.isUserType() && type.getObject() && hide_sch_name_usr_type)
	{
		BaseObject *obj = type.getObject();
		fmt_type_name.remove(obj->getSchema()->getName() + ".");
	}

	return fmt_type_name;
}

void TableObjectView::configureObject()
{
	if(!this->getUnderlyingObject())
		return;

	QTextCharFormat fmt;
	double px = 0;
	QString str_constr, tooltip, atribs_tip;
	TableObject *tab_obj=dynamic_cast<TableObject *>(this->getUnderlyingObject());
	Column *column=dynamic_cast<Column *>(tab_obj);
	ConstraintType constr_type=ConstraintType::Null;
	bool sql_disabled=false;

	tooltip = QString("`%1' (%2)").arg(tab_obj->getName(), tab_obj->getTypeName());
	tooltip += QString("\n%1 Id: %2").arg(UtilsNs::DataSeparator, QString::number(tab_obj->getObjectId()));
	sql_disabled = tab_obj->isSQLDisabled();
	fake_selection=false;

	if(column)
	{
		tooltip += tr("\n%1 Type: %2").arg(UtilsNs::DataSeparator, column->getType().getTypeSql());

		if(column->isAddedByRelationship())
			tooltip += tr("\n%1 Relationship: `%2'").arg(UtilsNs::DataSeparator, column->getParentRelationship()->getName());

		str_constr=this->getConstraintString(column);

		if(str_constr.indexOf(TextPrimaryKey)>=0)
		{
			fmt=font_config[Attributes::PkColumn];
			constr_type=ConstraintType::PrimaryKey;
		}
		else if(str_constr.indexOf(TextForeignKey)>=0)
		{
			fmt=font_config[Attributes::FkColumn];
			constr_type=ConstraintType::ForeignKey;
		}
		else if(str_constr.indexOf(TextUnique)>=0)
		{
			fmt=font_config[Attributes::UqColumn];
			constr_type=ConstraintType::Unique;
		}
		else if(str_constr.indexOf(TextNotNull)>=0)
			fmt=font_config[Attributes::NnColumn];
		else
			fmt=font_config[Attributes::Column];

		if(column->isAddedByRelationship())
			fmt=font_config[Attributes::InhColumn];
		else if(column->isProtected())
			fmt=font_config[Attributes::ProtColumn];

		if(str_constr.indexOf(TextPrimaryKey)>=0)
			atribs_tip+=(~ConstraintType(ConstraintType::PrimaryKey)).toLower() + ", ";

		if(str_constr.indexOf(TextForeignKey)>=0)
			atribs_tip+=(~ConstraintType(ConstraintType::ForeignKey)).toLower() + ", ";

		if(str_constr.indexOf(TextUnique)>=0)
			atribs_tip+=(~ConstraintType(ConstraintType::Unique)).toLower() + ", ";

		if(str_constr.indexOf(TextExclude)>=0)
			atribs_tip+=(~ConstraintType(ConstraintType::Exclude)).toLower() + ", ";

		if(str_constr.indexOf(TextNotNull)>=0)
			atribs_tip+="not null";
	}
	else
	{
		if(tab_obj->isAddedByRelationship())
			fmt=font_config[Attributes::InhColumn];
		else if(tab_obj->isProtected())
			fmt=font_config[Attributes::ProtColumn];
		else
			fmt=font_config[tab_obj->getSchemaName()];
	}

	configureDescriptor(constr_type);

	descriptor->setPos(HorizSpacing * 3, 0);
	px=descriptor->pos().x() + descriptor->boundingRect().width() + (2 * HorizSpacing);

	//Configuring the labels as follow: [object name] [type] [constraints]
	lables[0]->setText(compact_view && !tab_obj->getAlias().isEmpty() ? tab_obj->getAlias() : tab_obj->getName());

	//Strikeout the column name when its SQL is disabled
	QFont font=fmt.font();
	font.setStrikeOut(sql_disabled);
	fmt.setFont(font);

	lables[0]->setFont(fmt.font());
	lables[0]->setBrush(fmt.foreground());
	lables[0]->setPos(px, 0);
	px+=lables[0]->boundingRect().width();

	//Configuring the type label
	fmt=font_config[Attributes::ObjectType];

	if(compact_view)
		lables[1]->setText("");
	else
	{
		if(column)
			lables[1]->setText(TypeSeparator + formatUserTypeName(column->getType()));
		else
			lables[1]->setText(TypeSeparator + tab_obj->getSchemaName());
	}

	lables[1]->setFont(fmt.font());
	lables[1]->setBrush(fmt.foreground());
	lables[1]->setPos(px, 0);
	px+=lables[1]->boundingRect().width() + (3 * HorizSpacing);

	//Configuring the constraints label
	fmt=font_config[Attributes::Constraints];
	if(compact_view)
		lables[2]->setText("");
	else if(column)
		lables[2]->setText(!str_constr.isEmpty() ? str_constr : " ");
	else
	{
		Rule *rule=dynamic_cast<Rule *>(tab_obj);
		Trigger *trigger=dynamic_cast<Trigger *>(tab_obj);
		Index *index=dynamic_cast<Index *>(tab_obj);
		Constraint *constr=dynamic_cast<Constraint *>(tab_obj);
		Policy *policy = dynamic_cast<Policy *>(tab_obj);

		if(rule)
		{
			str_constr+=(~rule->getExecutionType()).mid(0,1);
			atribs_tip+=(~rule->getExecutionType()).toLower() + ", ";

			str_constr+=ConstrSeparator;

			str_constr+=(~rule->getEventType()).mid(3,1);
			atribs_tip+=(~rule->getEventType()).toLower();
			str_constr=str_constr.toLower();
		}
		else if(trigger)
		{
			str_constr+=(~trigger->getFiringType()).mid(0,1);
			str_constr+=ConstrSeparator;

			atribs_tip+=(~trigger->getFiringType()).toLower() + ", ";

			for(unsigned i=EventType::OnInsert; i <= EventType::OnTruncate; i++)
			{
				if(trigger->isExecuteOnEvent(EventType(i)))
				{
					str_constr+=(~EventType(i)).mid(3,1);
					atribs_tip+=(~EventType(i)).toLower() + ", ";
				}
			}
			str_constr=str_constr.toLower();
		}
		else if(index)
		{
			if(index->getIndexAttribute(Index::Unique))
			{
				str_constr+="u";
				atribs_tip += "unique, ";
			}

			if(index->getIndexAttribute(Index::Concurrent))
			{
				str_constr+="c";
				atribs_tip += "concurrent, ";
			}

			if(index->getIndexAttribute(Index::FastUpdate))
			{
				str_constr+="f";
				atribs_tip += "fast updated";
			}

			if(index->getIndexAttribute(Index::Buffering))
			{
				str_constr+="b";
				atribs_tip += "buffering";
			}
		}
		else if(constr)
		{
			ConstraintType type = constr->getConstraintType();

			if(type == ConstraintType::PrimaryKey)
				str_constr = TextPrimaryKey;
			else if(type == ConstraintType::ForeignKey)
				str_constr = TextForeignKey;
			else if(type == ConstraintType::Unique)
				str_constr = TextUnique;
			else if(type == ConstraintType::Exclude)
				str_constr = TextExclude;
			else if(type == ConstraintType::Check)
				str_constr = TextCheck;

			atribs_tip = (~type).toLower();
		}
		else if(policy)
		{
			if(policy->isPermissive())
			{
				str_constr += "p";
				atribs_tip += "permissive";
			}
			else
			{
				str_constr += "r";
				atribs_tip += "restrictive";
			}

			atribs_tip += ", ";
			str_constr += (~policy->getPolicyCommand()).toLower().at(0);
			atribs_tip += (~policy->getPolicyCommand()).toLower();
		}

		if(!str_constr.isEmpty())
			lables[2]->setText(ConstrDelimStart + " " +
												 str_constr + " " + ConstrDelimEnd);
		else
			lables[2]->setText("");
	}

	if(!atribs_tip.isEmpty())
	{
		if(atribs_tip.at(atribs_tip.length()-1)==' ')
			atribs_tip.remove(atribs_tip.length()-2, 2);

		atribs_tip="\n" + ConstrDelimStart +
							 " " + atribs_tip + " " + ConstrDelimEnd;

	}

	if(!tab_obj->getComment().isEmpty())
		atribs_tip += QString("\n\n%1").arg(tab_obj->getComment());

	lables[2]->setFont(fmt.font());
	lables[2]->setBrush(fmt.foreground());
	lables[2]->setPos(px, 0);

	calculateBoundingRect();

	tooltip += atribs_tip;
	this->setToolTip(UtilsNs::formatMessage(tooltip));
}

void TableObjectView::configureObject(const SimpleColumn &col)
{
	QTextCharFormat fmt;
	double px = 0;

	configureDescriptor();
	descriptor->setPos(HorizSpacing * 3, 0);
	px = descriptor->pos().x() + descriptor->boundingRect().width() + (2 * HorizSpacing);

	fmt = font_config[Attributes::Column];

	if(compact_view && !col.getAlias().isEmpty())
		lables[0]->setText(col.getAlias());
	else
		lables[0]->setText(col.getName());

	lables[0]->setFont(fmt.font());
	lables[0]->setBrush(fmt.foreground());
	lables[0]->setPos(px, 0);
	px+=lables[0]->boundingRect().width() + (4 * HorizSpacing);

	if(!compact_view && !col.getType().isEmpty())
	{
		fmt=font_config[Attributes::ObjectType];

		if(col.getType() == Attributes::Expression)
			lables[1]->setText(col.getType());
		else
			lables[1]->setText(formatUserTypeName(PgSqlType::parseString(col.getType())));

		lables[1]->setFont(fmt.font());
		lables[1]->setBrush(fmt.foreground());
		lables[1]->setPos(px, 0);
		//px += lables[1]->boundingRect().width() + (4 * HorizSpacing);
	}
	else
		lables[1]->setText("");

	lables[2]->setText("");
	calculateBoundingRect();

	setToolTip(	UtilsNs::formatMessage(tr("`%1' (%2)\n%3 Type: %4")
																			.arg(col.getName(),
																					 BaseObject::getTypeName(ObjectType::Column),
																					 UtilsNs::DataSeparator,
																					 col.getType())));
}

void TableObjectView::setChildObjectXPos(ChildObjectId obj_id, double px)
{
	if(obj_id > ConstrAliasLabel)
		throw Exception(ErrorCode::RefObjectInvalidIndex, PGM_FUNC, PGM_FILE, PGM_LINE);

	if(obj_id == ObjDescriptor)
		descriptor->setPos(px, descriptor->pos().y());
	else
		lables[obj_id-1]->setPos(px, lables[obj_id-1]->pos().y());

	calculateBoundingRect();
}

void TableObjectView::calculateBoundingRect()
{
	double width = 0, height = 0, curr_w = 0, py = 0;

	width = descriptor->pos().x() + descriptor->boundingRect().width();
	height = lables[0]->boundingRect().height();

	for(auto & label : lables)
	{
		if(label->text().isEmpty())
			continue;

		curr_w = label->pos().x() + label->boundingRect().width();

		if(width < curr_w)
			width = label->pos().x() + label->boundingRect().width();
	}

	bounding_rect = QRectF(QPointF(0,0), QSizeF(width + (4 * HorizSpacing), height + (VertSpacing * 0.80)));

	//Adjusting the Y position of the objects in order to center them on the new bouding rect
	descriptor->setPos(descriptor->pos().x(),
										 (bounding_rect.height() - descriptor->boundingRect().height() + (VertSpacing * 0.4))/2);

	py = (bounding_rect.height() - lables[0]->boundingRect().height())/2;

	for(auto & label : lables)
		label->setPos(label->pos().x(), py);
}

QGraphicsItem *TableObjectView::getChildObject(ChildObjectId obj_id)
{
	if(obj_id > ConstrAliasLabel)
		throw Exception(ErrorCode::RefObjectInvalidIndex, PGM_FUNC, PGM_FILE, PGM_LINE);

	if(obj_id == ObjDescriptor)
		return descriptor;

	return lables[obj_id - 1];
}

QString TableObjectView::getConstraintString(Column *column)
{
	if(column && column->getParentTable())
	{
		PhysicalTable *table=dynamic_cast<PhysicalTable *>(column->getParentTable());
		QString str_constr;
		Constraint *constr=nullptr;
		std::vector<TableObject *>::iterator itr,itr_end;
		ConstraintType constr_type;

		itr=table->getObjectList(ObjectType::Constraint)->begin();
		itr_end=table->getObjectList(ObjectType::Constraint)->end();

		while(itr!=itr_end)
		{
			constr=dynamic_cast<Constraint *>(*itr);
			itr++;

			//Check if the column is referecend by the constraint
			if((constr->getConstraintType()!=ConstraintType::Exclude && constr->isColumnExists(column, Constraint::SourceCols)) ||
				 (constr->getConstraintType()==ConstraintType::Exclude && constr->isColumnReferenced(column, false)))
			{
				constr_type=constr->getConstraintType();

				if(constr_type==ConstraintType::PrimaryKey)
					str_constr=TextPrimaryKey + ConstrSeparator + str_constr;

				if(constr_type==ConstraintType::ForeignKey && str_constr.indexOf(TextForeignKey) < 0)
					str_constr+=TextForeignKey + ConstrSeparator;

				if(constr_type==ConstraintType::Unique && str_constr.indexOf(TextUnique) < 0)
					str_constr+=TextUnique + ConstrSeparator;

				if(constr_type==ConstraintType::Exclude && str_constr.indexOf(TextExclude) < 0)
					str_constr+=TextExclude + ConstrSeparator;
			}
		}

		if(column->isNotNull() && !str_constr.contains(TextPrimaryKey))
			str_constr+=TextNotNull + ConstrSeparator;

		if(!str_constr.isEmpty())
			str_constr= ConstrDelimStart + ConstrSeparator + str_constr + ConstrDelimEnd;

		return str_constr;
	}

	return "";
}

void TableObjectView::setFakeSelection(bool value)
{
	// Fake selection is used only by instances that own and underlying object (column, constratin, trigger,etc)
	if(!this->getUnderlyingObject())
		return;

	fake_selection = value;

	if(value)
	{
		configureObjectSelection();
		sel_order=++BaseObjectView::global_sel_order;
	}
	else
		sel_order = 0;

	update();
}

bool TableObjectView::hasFakeSelection()
{
	return fake_selection;
}

void TableObjectView::setSchemaNameUserType(bool value)
{
	hide_sch_name_usr_type = value;
}

bool TableObjectView::isHideSchemaNameUserType()
{
	return hide_sch_name_usr_type;
}

void TableObjectView::configureObjectSelection()
{
	QGraphicsItem *parent = this->parentItem();
	RoundedRectItem *rect_item=nullptr;
	QRectF rect = this->boundingRect();

	/* In order to avoid unnecessary memory usage by items that eventually will
	 * get selection we allocate the object selection rectangle only if the object
	 * itself is selected by the user, and it'll be allocated until the object's destruction */
	if(!obj_selection)
		obj_selection=new RoundedRectItem;

	rect_item = dynamic_cast<RoundedRectItem *>(obj_selection);
	rect.setX(0);
	rect.setY(0);
	rect.setHeight(rect.height() - VertSpacing);

	// An small hack to capture the width of the table in which the item is child of
	if(parent->parentItem())
		rect.setWidth(parent->parentItem()->boundingRect().width() - (2.5 * HorizSpacing));
	else
		rect.setWidth(rect.width() - (3.5 * HorizSpacing));

	rect_item->setBorderRadius(4);
	rect_item->setRect(rect);
	rect_item->setPos(0, VertSpacing/2);
	rect_item->setBrush(this->getFillStyle(Attributes::ObjSelection));
	rect_item->setPen(this->getBorderStyle(Attributes::ObjSelection));
}

void TableObjectView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();
	painter->translate(descriptor->pos());
	descriptor->paint(painter, option, widget);
	painter->restore();

	for(auto & label : lables)
	{
		if(label->text().isEmpty())
			continue;

		painter->save();
		painter->translate(label->pos());
		label->paint(painter, option, widget);
		painter->restore();
	}

	if(fake_selection)
	{
		painter->translate(obj_selection->pos());
		obj_selection->paint(painter, option, widget);
	}
}

QRectF TableObjectView::boundingRect() const
{
	return bounding_rect;
}

