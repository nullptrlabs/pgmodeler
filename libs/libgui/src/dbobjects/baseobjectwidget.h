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
\ingroup libgui
\class BaseObjectWidget
\brief Implements the basic operations to create/edit database objects via form.
*/

#ifndef BASE_OBJECT_WIDGET_H
#define BASE_OBJECT_WIDGET_H

#include "databasemodel.h"
#include "operationlist.h"
#include "widgets/objectselectorwidget.h"
#include "ui_baseobjectwidget.h"
#include "qtconnectmacros.h"

class __libgui BaseObjectWidget: public QWidget, public Ui::BaseObjectWidget {
	Q_OBJECT

	protected:
		static constexpr int MaxObjectSize=16777215;

		bool object_protected;

		QHBoxLayout *misc_btns_lt;

		//! \brief Store the kind of object being handled by the widget (configured in the constructor)
		ObjectType handled_obj_type;
		
		/*! \brief Operation list element count before editing an object. This attribute
			is used to know, in case of cancel the edition, the operation (count) that is needed to
			be removed. This attribute applies only if the object creation is chained (like in tables, views or relationships)
			See: cancelChainedConfiguration() */
		unsigned operation_count;
				
		//! \brief Reference database model
		DatabaseModel *model;
		
		//! \brief Reference table/view (used only when editing table objects)
		BaseTable *table;
		
		//! \brief Stores the object previous name (used to validate schema renaming)
		QString prev_name;
		
		//! \brief Store the object previous schema (used to update the schemas when moving tables/views from one to another)
		Schema *prev_schema;
		
		//! \brief Reference relationship (used only when editing relationship attributes)
		Relationship *relationship;
		
		//! \brief Reference operation list where all modifications in the form are registered
		OperationList *op_list;
		
		//! \brief Object that is being edited / created
		BaseObject *object;
		
		/*! \brief Stores the object position (generally the mouse position) when the dialog was called
			(used only when creating graphical objects) */
		double object_px, object_py;
		
		//! \brief Grid layout used to organize the widgets over the form
		QGridLayout *baseobject_grid;
		
		//! \brief Indicates if the object is a new one or is being edited
		bool new_object;
		
		//! \brief Object selectors for schema, owner, tablespace and collation
		ObjectSelectorWidget *schema_sel,
		*owner_sel,
		*tablespace_sel,
		*collation_sel;
		
		/*! \brief Merges the specified grid layout with the 'baseobject_grid' creating a single form.
			The obj_type parameter must be specified to show the object type icon */
		void configureFormLayout(QGridLayout *grid=nullptr, ObjectType obj_type=ObjectType::BaseObject);
		
		/*! \brief Configures the state of commom fields related to database objects enabling/disabling/hidding
		 * according to the object type. The parameter inst_ev_filter indicates if the special event filter
		 * must be installed on input fields. The event filter calls applyConfiguration() when ENTER/RETURN is pressed */
		void configureFormFields(ObjectType obj_type, bool inst_ev_filter = true);

		/*! \brief Starts a object configuration, alocating a new one if necessary, registering
			the object on the operation list. This method doens't applies to database model edition */
		template<class Class>
		void startConfiguration();
		
		/*! \brief Finishes the edition / creation of object, registering it on the operation list
			and inserts is on the parent object */
		void finishConfiguration();
			
		//! \brief Apply the basic configurations to the object (name, schema, comment, owner, tablespace)
		virtual void applyConfiguration();

		void showEvent(QShowEvent *) override;

		void setAttributes(DatabaseModel *model, OperationList *op_list,
						   BaseObject *object, BaseObject *parent_obj=nullptr,
						   double obj_px=DNaN, double obj_py=DNaN, bool uses_op_list=true);
		
		/*! \brief This method is a simplification of the original setAttributes. This method must be used
		only on forms that does not make use of operaton list and not treat graphical objects, since it calls
		this original one whit the op_list=nullptr and obj_px=DoubleNaN, obj_py=DoubleNaN */
		void setAttributes(DatabaseModel *model, BaseObject *object, BaseObject *parent_obj);
		
		//! \brief Disable the object's refereces SQL code
		void disableReferencesSQL(BaseObject *object);
		
		void configureTabOrder(std::vector<QWidget *> widgets={});

		BaseObject *getHandledObject();
			
	public:
		//! \brief Constants used to generate version intervals for version alert frame
		static constexpr unsigned UntilVersion=0,
		VersionsInterval=1,
		AfterVersion=2;
		
		BaseObjectWidget(QWidget * parent = nullptr, ObjectType obj_type=ObjectType::BaseObject);
		~BaseObjectWidget() override = default;

		//! \brief Generates a string containing the specified version interval
		static QString generateVersionsInterval(unsigned ver_interv_id, const QString &ini_ver, const QString &end_ver="");
		
		/*! \brief Generates a alert frame highlighting the fields of exclusive use on the specified
			PostgreSQL versions. On the first map (fields) the key is the PostgreSQL versions and
			the values are the reference to the widget. The second map is used to specify the values
			of widgets specific for each version. */
		static QFrame *generateVersionWarningFrame(std::map<QString, std::vector<QWidget *> > &fields, std::map<QWidget *, std::vector<QString> > *values=nullptr);
		
		//! \brief Generates a informative frame containing the specified message
		static QFrame *generateInformationFrame(const QString &msg);

		static void highlightVersionSpecificFields(std::map<QString, std::vector<QWidget *> > &fields, std::map<QWidget *, std::vector<QString> > *values=nullptr);
		
		//! \brief Highlights the specified widget as a required field
		static void setRequiredField(QWidget *widget);
		
		//! \brief Filters the ENTER/RETURN key press forcing the button "Apply" to be clicked
		bool eventFilter(QObject *obj, QEvent *event) override;

		//! \brief Returns the kind of database object handled
		ObjectType getHandledObjectType();

		virtual bool isHandledObjectProtected();
		
	protected slots:
		void editPermissions();
		void editCustomSQL();
		
		//! \brief Register the new object in the operation history if it is not registered already
		void registerNewObject();

		/*! \brief Aborts the object configuration, deallocation it if necessary or restoring it to
			its previous configuration */
		virtual void cancelConfiguration();

		//! \brief Executes the proper actions to cancel chained operations.
		virtual void cancelChainedOperation();
		
	signals:
		//! \brief Signal emitted whenever a object is created / edited using the form
		void s_objectManipulated();

		//! \brief Signal emitted whenever the object editing was successful and the form need to be closed
		void s_closeRequested();

	friend class BaseForm;
	friend class ModelWidget;
};

template<class Class>
void BaseObjectWidget::startConfiguration(void)
{
	try
	{
		Class *new_tmpl_obj=nullptr;
		
		//! \brief If the object is already allocated
		if(this->object && op_list &&
				this->object->getObjectType()!=ObjectType::Database)
		{
			if(this->table)
				op_list->registerObject(this->object, Operation::ObjModified, -1, this->table);
			else
				op_list->registerObject(this->object, Operation::ObjModified, -1, this->relationship);
			new_object=false;
		}
		//! \brief If there is need to allocate the object
		else if(!this->object)
		{
			new_tmpl_obj=new Class;
			this->object=new_tmpl_obj;
			new_object=true;
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

#endif
