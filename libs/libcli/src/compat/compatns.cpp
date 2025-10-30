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

#include "compatns.h"

namespace CompatNs {
	View *createLegacyView(const QString &xml_code, DatabaseModel *dbmodel)
	{
		attribs_map attribs, aux_attribs;
		View *view=nullptr;
		Column *column=nullptr;
		PhysicalTable *table=nullptr;
		QString elem, str_aux;
		QStringList list_aux;
		std::vector<Reference> refs;
		BaseObject *tag=nullptr;
		Reference::SqlType sql_type;
		int ref_idx, i, count;
		bool refs_in_expr=false;
		Reference reference;
		XmlParser *xmlparser = dbmodel->getXMLParser();

		try
		{
			xmlparser->restartParser();
			xmlparser->loadXMLBuffer(xml_code);

			view = new View;
			setBasicAttributes(view, dbmodel);

			xmlparser->getElementAttributes(attribs);
			view->setObjectListsCapacity(attribs[Attributes::MaxObjCount].toUInt());
			view->setMaterialized(attribs[Attributes::Materialized]==Attributes::True);
			view->setRecursive(attribs[Attributes::Recursive]==Attributes::True);
			view->setWithNoData(attribs[Attributes::WithNoData]==Attributes::True);
			view->setCollapseMode(attribs[Attributes::CollapseMode].isEmpty() ? BaseTable::NotCollapsed : static_cast<BaseTable::CollapseMode>(attribs[Attributes::CollapseMode].toUInt()));
			view->setPaginationEnabled(attribs[Attributes::Pagination]==Attributes::True);
			view->setCurrentPage(BaseTable::AttribsSection, attribs[Attributes::AttribsPage].toUInt());
			view->setCurrentPage(BaseTable::ExtAttribsSection, attribs[Attributes::ExtAttribsPage].toUInt());
			view->setFadedOut(attribs[Attributes::FadedOut]==Attributes::True);
			view->setLayers(attribs[Attributes::Layers].split(','));

			if(xmlparser->accessElement(XmlParser::ChildElement))
			{
				do
				{
					if(xmlparser->getElementType()==XML_ELEMENT_NODE)
					{
						elem=xmlparser->getElementName();

						if(elem==Attributes::Reference)
						{
							xmlparser->getElementAttributes(attribs);

										 //If the table name is specified tries to create a reference to a table/column
							if(!attribs[Attributes::Table].isEmpty())
							{
								column=nullptr;
								table=dynamic_cast<PhysicalTable *>(dbmodel->getObject(attribs[Attributes::Table], {ObjectType::Table, ObjectType::ForeignTable}));

											 //Raises an error if the table doesn't exists
								if(!table)
								{
									str_aux=Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
																.arg(view->getName())
																.arg(BaseObject::getTypeName(ObjectType::View))
																.arg(attribs[Attributes::Table])
																.arg(BaseObject::getTypeName(ObjectType::Table));

									throw Exception(str_aux,ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
								}

								if(!attribs[Attributes::Column].isEmpty())
								{
									column=table->getColumn(attribs[Attributes::Column]);

									if(!column)
										column=table->getColumn(attribs[Attributes::Column], true);

												 //Raises an error if the view references an inexistant column
									if(!column)
									{
										str_aux=Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
																	.arg(view->getName())
																	.arg(BaseObject::getTypeName(ObjectType::View))
																	.arg(attribs[Attributes::Table] + "." +
																				attribs[Attributes::Column])
																	.arg(BaseObject::getTypeName(ObjectType::Column));

										throw Exception(str_aux,ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
									}
								}

											 //Adds the configured reference to a temporarily list
								reference = Reference(table, column,
																			 attribs[Attributes::Alias],
																			 attribs[Attributes::ColumnAlias]);
								reference.setReferenceAlias(attribs[Attributes::RefAlias]);
								refs.push_back(reference);
							}
							else
							{
								xmlparser->savePosition();
								str_aux=attribs[Attributes::Alias];

											 // Retrieving the reference expression
								xmlparser->accessElement(XmlParser::ChildElement);
								xmlparser->savePosition();
								xmlparser->accessElement(XmlParser::ChildElement);
								reference = Reference(xmlparser->getElementContent(), str_aux);
								reference.setReferenceAlias(attribs[Attributes::RefAlias]);
								xmlparser->restorePosition();

											 // Creating the columns related to the expression
								if(xmlparser->accessElement(XmlParser::NextElement))
								{
									do
									{
										elem = xmlparser->getElementName();
										xmlparser->savePosition();

										if(elem == Attributes::Column)
										{
											column = dbmodel->createColumn();
											reference.addColumn(column);
											delete column;
										}
										else if(elem == Attributes::RefTableTag)
										{
											xmlparser->getElementAttributes(aux_attribs);
											table = dbmodel->getTable(aux_attribs[Attributes::Name]);

											if(!table)
												table = dbmodel->getForeignTable(aux_attribs[Attributes::Name]);

											reference.addReferencedTable(table);
										}

										xmlparser->restorePosition();
									}
									while(xmlparser->accessElement(XmlParser::NextElement));
								}

								refs.push_back(reference);
								xmlparser->restorePosition();
							}
						}
						else if(elem==Attributes::Expression)
						{
							xmlparser->savePosition();
							xmlparser->getElementAttributes(attribs);
							xmlparser->accessElement(XmlParser::ChildElement);

							if(attribs[Attributes::Type]==Attributes::CteExpression)
								view->setCommomTableExpression(xmlparser->getElementContent());
							else
							{
								if(attribs[Attributes::Type]==Attributes::SelectExp)
									sql_type=Reference::SqlSelect;
								else if(attribs[Attributes::Type]==Attributes::FromExp)
									sql_type=Reference::SqlFrom;
								else if(attribs[Attributes::Type]==Attributes::SimpleExp)
									sql_type=Reference::SqlWhere;
								else
									sql_type=Reference::SqlEndExpr;

								list_aux=xmlparser->getElementContent().split(',');
								count=list_aux.size();

											 //Indicates that some of the references were used in the expressions
								if(count > 0 && !refs_in_expr)
									refs_in_expr=true;

								for(i=0; i < count; i++)
								{
									ref_idx=list_aux[i].toInt();
									view->addReference(refs[ref_idx],sql_type);
								}
							}

							xmlparser->restorePosition();
						}
						else if(elem==BaseObject::getSchemaName(ObjectType::Tag))
						{
							xmlparser->getElementAttributes(aux_attribs);
							tag=dbmodel->getObject(aux_attribs[Attributes::Name] ,ObjectType::Tag);

							if(!tag)
							{
								throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
																		 .arg(attribs[Attributes::Name])
																		 .arg(BaseObject::getTypeName(ObjectType::Table))
																		 .arg(aux_attribs[Attributes::Table])
																		 .arg(BaseObject::getTypeName(ObjectType::Tag))
																 , ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
							}

							view->setTag(dynamic_cast<Tag *>(tag));
						}
					}
				}
				while(xmlparser->accessElement(XmlParser::NextElement));
			}

			/** Special case for refereces used as view definition **

						 If the flag 'refs_in_expr' isn't set indicates that the user defined a reference
					 but has no used to define the view declaration, this way pgModeler will consider these
					 references as View definition expressions and will force the insertion of them as
						 Reference::SQL_VIEW_DEFINITION.

						This process can raise errors because if the user defined more than one reference the view
						cannot accept the two as it's SQL definition, also the defined references MUST be expressions in
						order to be used as view definition */
			if(!refs.empty() && !refs_in_expr)
			{
				std::vector<Reference>::iterator itr;

				itr=refs.begin();
				while(itr!=refs.end())
				{
					view->addReference(*itr, Reference::SqlViewDef);
					itr++;
				}
			}
		}
		catch(Exception &e)
		{
			delete view;
			throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}

		return view;
	}

	void setBasicAttributes(BaseObject *object, DatabaseModel *dbmodel)
	{
		attribs_map attribs, attribs_aux;
		QString elem_name;
		BaseObject *tabspc=nullptr, *owner=nullptr, *collation=nullptr;
		Schema *schema=nullptr;
		ObjectType obj_type=ObjectType::BaseObject, obj_type_aux;
		bool has_error=false, protected_obj=false, sql_disabled=false;
		ForeignObject *frn_object = dynamic_cast<ForeignObject *>(object);
		XmlParser *xmlparser = dbmodel->getXMLParser();

		if(!object)
			throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		xmlparser->getElementAttributes(attribs);
		obj_type_aux=object->getObjectType();

		if(obj_type_aux!=ObjectType::Cast && obj_type_aux != ObjectType::UserMapping)
			object->setName(attribs[Attributes::Name]);

		if(BaseGraphicObject::isGraphicObject(obj_type_aux) && !attribs[Attributes::ZValue].isEmpty())
			dynamic_cast<BaseGraphicObject *>(object)->setZValue(attribs[Attributes::ZValue].toInt());

		if(BaseObject::acceptsAlias(obj_type_aux))
			object->setAlias(attribs[Attributes::Alias]);

		protected_obj=attribs[Attributes::Protected]==Attributes::True;
		sql_disabled=attribs[Attributes::SqlDisabled]==Attributes::True;

		if(frn_object)
		{
			QStringList opt_val;

			for(auto &option : attribs[Attributes::Options].split(ForeignObject::OptionsSeparator))
			{
				opt_val = option.split(UserMapping::OptionValueSeparator);

				if(opt_val.size() < 2)
					continue;

				frn_object->setOption(opt_val[0], opt_val[1]);
			}
		}

		xmlparser->savePosition();

		if(xmlparser->accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser->getElementType()==XML_ELEMENT_NODE)
				{
					elem_name=xmlparser->getElementName();

								 //Defines the object's comment
					if(elem_name==Attributes::Comment)
					{
						xmlparser->savePosition();
						xmlparser->accessElement(XmlParser::ChildElement);
						object->setComment(xmlparser->getElementContent());
						xmlparser->restorePosition();
					}
					//Defines the object's schema
					else if(elem_name==Attributes::Schema)
					{
						obj_type=ObjectType::Schema;
						xmlparser->getElementAttributes(attribs_aux);
						schema=dynamic_cast<Schema *>(dbmodel->getObject(attribs_aux[Attributes::Name], obj_type));
						object->setSchema(schema);
						has_error=(!schema && !attribs_aux[Attributes::Name].isEmpty());
					}
					//Defines the object's tablespace
					else if(elem_name==Attributes::Tablespace)
					{
						obj_type=ObjectType::Tablespace;
						xmlparser->getElementAttributes(attribs_aux);
						tabspc=dbmodel->getObject(attribs_aux[Attributes::Name], obj_type);
						object->setTablespace(tabspc);
						has_error=(!tabspc && !attribs_aux[Attributes::Name].isEmpty());
					}
					//Defines the object's owner
					else if(elem_name==Attributes::Role)
					{
						obj_type=ObjectType::Role;
						xmlparser->getElementAttributes(attribs_aux);
						owner=dbmodel->getObject(attribs_aux[Attributes::Name], obj_type);
						object->setOwner(owner);
						has_error=(!owner && !attribs_aux[Attributes::Name].isEmpty());
					}
					//Defines the object's schema
					else if(elem_name==Attributes::Collation)
					{
						obj_type=ObjectType::Collation;
						xmlparser->getElementAttributes(attribs_aux);
						collation=dbmodel->getObject(attribs_aux[Attributes::Name], obj_type);
						object->setCollation(collation);
						has_error=(!collation && !attribs_aux[Attributes::Name].isEmpty());
					}
					else if(elem_name==Attributes::AppendedSql)
					{
						xmlparser->savePosition();
						xmlparser->accessElement(XmlParser::ChildElement);
						object->setAppendedSQL(xmlparser->getElementContent());
						xmlparser->restorePosition();
					}
					else if(elem_name==Attributes::PrependedSql)
					{
						xmlparser->savePosition();
						xmlparser->accessElement(XmlParser::ChildElement);
						object->setPrependedSQL(xmlparser->getElementContent());
						xmlparser->restorePosition();
					}
					//Defines the object's position (only for graphical objects)
					else if(elem_name==Attributes::Position)
					{
						xmlparser->getElementAttributes(attribs);

						if(elem_name==Attributes::Position &&
								(obj_type_aux!=ObjectType::Relationship &&
								 obj_type_aux!=ObjectType::BaseRelationship))
						{
							dynamic_cast<BaseGraphicObject *>(object)->setPosition(QPointF(attribs[Attributes::XPos].toDouble(),
																																								attribs[Attributes::YPos].toDouble()));


						}
					}
				}
			}
			while(!has_error && xmlparser->accessElement(XmlParser::NextElement));
		}

		xmlparser->restorePosition();
		object->setProtected(protected_obj);
		object->setSQLDisabled(sql_disabled);

		if(has_error)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
													 .arg(object->getName())
													 .arg(object->getTypeName())
													 .arg(attribs_aux[Attributes::Name])
													 .arg(BaseObject::getTypeName(obj_type)),
											 ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		if(!object->getSchema() && (BaseObject::acceptsSchema(obj_type_aux) && obj_type_aux != ObjectType::Extension))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::InvObjectAllocationNoSchema)
											.arg(object->getName())
											.arg(object->getTypeName()),
											ErrorCode::InvObjectAllocationNoSchema,PGM_FUNC,PGM_FILE,PGM_LINE);
		}
	}

	QString convertToNewView(View *old_view)
	{
		::View *new_view = new ::View;

		new_view->setName(old_view->getName());
		new_view->setAlias(old_view->getAlias());
		new_view->setPosition(old_view->getPosition());
		new_view->setProtected(old_view->isProtected());
		new_view->setSQLDisabled(old_view->isSQLDisabled());
		new_view->setZValue(old_view->getZValue());
		new_view->setSchema(old_view->getSchema());
		new_view->setOwner(old_view->getOwner());
		new_view->setTag(old_view->getTag());
		new_view->setLayers(old_view->getLayers());
		new_view->setMaterialized(old_view->isMaterialized());
		new_view->setWithNoData(old_view->isWithNoData());
		new_view->setCollapseMode(old_view->getCollapseMode());
		new_view->setPaginationEnabled(old_view->isPaginationEnabled());
		new_view->setAppendedSQL(old_view->getAppendedSQL());
		new_view->setPrependedSQL(old_view->getPrependedSQL());

		/* Forcing the generation of only the CREATE VIEW ... command
		 * without ALTER VIEW SET OWNER and any other prepended or appended
		 * commands, this will help to extract exactly the SQL that defines the view */
		old_view->setOwner(nullptr);
		old_view->setAppendedSQL("");
		old_view->setPrependedSQL("");
		old_view->setSQLDisabled(false);

		QString sql_code = old_view->getSourceCode(SchemaParser::SqlCode);
		sql_code.remove(0, sql_code.indexOf("\nAS") + 3);
		sql_code.remove(sql_code.indexOf(Attributes::DdlEndToken), sql_code.length());
		new_view->setSqlDefinition(sql_code.trimmed());

		std::vector<SimpleColumn> columns;
		for(auto &col : old_view->getColumns())
		{
			/* Columns of the type "expression" are converted to "text"
			 * since the type "expression" is not a valid PgSqlType. This
			 * will avoid invalid type errors when trying to edit view columns
			 * that came from the legacy structure */
			columns.push_back(SimpleColumn(col.getName(),
																		 col.getType() == Attributes::Expression ? "text" : col.getType(),
																		 col.getAlias()));
		}

		new_view->setCustomColumns(columns);

		std::vector<::Reference> refs;
		BaseObject *obj = nullptr;
		QString ref_name, ref_alias;
		unsigned col_idx = 1, tab_idx = 1;

		for(auto &old_ref : old_view->getViewReferences())
		{
			obj = nullptr;

			if(old_ref.getColumn())
			{
				obj = old_ref.getColumn();
				// Appending an incremented value to avoid reference name ambiguity
				ref_name = obj->getName() + QString::number(col_idx++);
				ref_alias = old_ref.getColumnAlias();
			}
			else if(old_ref.getTable())
			{
				obj = old_ref.getTable();
				// Appending an incremented value to avoid reference name ambiguity
				ref_name = obj->getName() + QString::number(tab_idx++);
				ref_alias = old_ref.getAlias();
			}

			if(!obj)
				continue;

			refs.push_back(::Reference(obj, ref_name, ref_alias, true, true, false));
		}

		new_view->setReferences(refs);

		return new_view->getSourceCode(SchemaParser::XmlCode);
	}
}
