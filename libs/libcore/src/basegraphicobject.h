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

/**
\ingroup libcore
\class BaseGraphicObject
\brief This is a basic implementation to control graphical objects such as tables, relationships, textboxes, views.
The full graphical representation of these objects are made in an separated layer
this is treated in the BaseObjectView class on libobjrenderer library
\note <strong>Creation date:</strong> 17/09/2006
*/

#ifndef BASE_GRAPHIC_OBJECT_H
#define BASE_GRAPHIC_OBJECT_H

#include <QObject>
#include <QPointF>
#include "baseobject.h"

//! \brief Enum used to reference object color ids
enum class ColorId: unsigned {
	FillColor1,
	FillColor2,
	BorderColor,
};

class __libcore BaseGraphicObject: public QObject, public BaseObject {
	Q_OBJECT

	private:
		/*! \brief Stores the position of the object on the model. This attribute is used only
		 to memorize the position of the object in xml code generation */
		QPointF position;

		/*! \brief Indicates that the object structure was modified somehow
		 and it is needed to be updated or specially treated */
		bool is_modified,

		//! \brief Indicates if the graphical representation of this object is faded out
		is_faded_out;

		/*! \brief Stores a reference to the object which is currently the receiver
		 of signals emitted by the instance of this class. The receiver is an object that
		 represents the 'this' object in the QGraphicsScene. This attribute breaks
		 some concepts of OO but is required for some cases when is necessary to
		 recover the graphical object in a fast way (rather than find
		 it in the QGraphcisScene) */
		QObject *receiver_object;

		static bool updates_enabled;

	protected:
		//! \brief This attribute holds the layers in which the object is visible.
		QList<unsigned> layers;

		//! \brief Stores the table's Z position in the canvas
		int z_value;

		/*! \brief Method that defines the objects position attributes used in generation
		 of XML code definition */
		void setPositionAttribute();

		//! \brief Defines the receveir objects that represents the 'this' object on the QGraphicsScene
		void setReceiverObject(QObject *obj);

		void setFadedOutAttribute();

		void setLayersAttribute();

	public:
		static constexpr int MaxZValue = 50,
		MinZValue = -50;

		BaseGraphicObject();
		~BaseGraphicObject() override = default;

		/*! \brief Sets whether the object is protected or not (method overloading
		 from base class BaseObject) the difference is that this method
		 emits the signal s_objectProtected() */
		void setProtected(bool value) override;

		/*! \brief Sets whether the object is system or not (method overloading
		 from base class BaseObject) the difference is that this method
		 emits the same signal s_objectProtected() */
		void setSystemObject(bool value) override;

		//! \brief Sets the object's position
		virtual void setPosition(const QPointF &pos);

		/*! \brief Sets if the objects is modified or not.
		 This method emits the signal s_objectModified() */
		virtual void setModified(bool value);

		void setSQLDisabled(bool value) override;

		//! \brief Sets the fade out status of the receiver object
		void setFadedOut(bool value);

		//! \brief Returns the modified status of the object
		bool isModified();

		//! \brief Returns the fade out status of the object
		bool isFadedOut();

		//! \brief Returns the current position of the object
		QPointF getPosition();

		//! \brief Assigns on object to other mading the correct attribute copy
		void operator =(BaseGraphicObject &obj);

		//! \brief Gets the current overlying (top object, scene object) that graphically represents the 'this' object
		QObject *getOverlyingObject();

		//! \brief Returns the code definition of the object
		QString getSourceCode(SchemaParser::CodeType) override = 0;

		//! \brief Returns if the passed type one that has a graphical representation (table, view, schema, relationship or textbox)
		static bool isGraphicObject(ObjectType type);

		//! \brief Defines in which layers the object is visible
		void setLayers(QStringList list);
		void setLayers(QList<unsigned> list);

		//! \brief Add the object to the layer specified by the id
		void addToLayer(unsigned layer_id);

		//! \brief Remove the object from the layer specified by the id
		void removeFromLayer(unsigned layer_id);

		//! \brief Clear all the ids and moves the object to default layer (0)
		void resetLayers();

		//! \brief Returns the layers that the object is visible
		QList<unsigned> getLayers();

		//! \brief Returns true if the object is in the provided layer
		bool isInLayer(unsigned layer_id);

		int getLayersCount();

		virtual void setZValue(int z_value);

		int getZValue();

		static void setUpdatesEnabled(bool value);

		static bool isUpdatesEnabled();

	signals:
		//! \brief Signal emitted when the user calls the setModified() method
		void s_objectModified();

		//! \brief Signal emitted when the user calls the setProtected() method
		void s_objectProtected(bool);

		friend class BaseObjectView;
		friend class DatabaseModel;
		friend class OperationList;
};

#endif
