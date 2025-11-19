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
#include "widgets/objectassociationswidget.h"
#include "guiutilsns.h"

class __libgui BaseObjectWidget: public QWidget, public Ui::BaseObjectWidget {
	Q_OBJECT

	private:
		struct FieldLayoutCfg {
			QLabel *label = nullptr;
			QWidget *widget = nullptr, *append_widget = nullptr;
			int row = 0, col = 0, row_span = 1, col_span = 1;

			FieldLayoutCfg(QLabel *label, QWidget *widget, QWidget *append_widget, 
										 int row, int col, int row_span = 1, int col_span = 1) :
				label(label), widget(widget), append_widget(append_widget), 
				row(row), col(col), row_span(row_span), col_span(col_span) {}

			FieldLayoutCfg(QLabel *label, QWidget *widget, int row, int col, 
										 int row_span = 1, int col_span = 1) :
				FieldLayoutCfg(label, widget, nullptr, row, col, row_span, col_span) {}
		};

		void configureBaseLayout();

	protected:
		static constexpr int MaxObjectSize=16777215;

		bool object_protected;

		/*! \brief This horizontal layout arranges the buttons "Custom" SQL and "Edit permissions"
		 *  as well as the checkbox "Disable SQL code" */
		QHBoxLayout *misc_btns_lt;

		/*! \brief This vertical layout, initially empty, can be used by derived classes to
		 *  insert custom widgets/layouts in the middle of the form, between the main fields
		 *  (name, schema, comment, etc) and the layout misc_btns_lt */
		QVBoxLayout *extra_wgts_lt;

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

		//! \brief Object selectors for schema, owner, tablespace and collation
		ObjectAssociationsWidget *obj_assoc_wgt;

		/*! \brief Holds the reference to the SQL preview tab when in tabbed layout
		 *  This attribute is nullptr for objects that don't generate SQL code */
		QWidget *sql_preview_pg;
		
		/*! \brief Merges the specified layout with the 'baseobject_grid' creating a single form.
		 * The obj_type parameter must be specified to show the object type icon */
		template<class LayoutClass, std::enable_if_t<std::is_base_of_v<QLayout, LayoutClass>, bool> = true>
		void configureFormLayout(LayoutClass *layout, ObjectType obj_type)
		{
			if(!layout)
			{
				setLayout(baseobject_grid);
				return;
			}

			if constexpr(std::is_same_v<QGridLayout, LayoutClass>)
			{
				QLayoutItem *item = nullptr;
				int lin = 0, col = 0, col_span = 0,
						row_span = 0, item_id = 0, item_count = 0;

				/* Move all the widgets of the passed grid layout one row down,
				 * permiting the insertion of the 'baseobject_grid' at the top
				 * of the items */
				item_count = layout->count();

				for(item_id = item_count - 1; item_id >= 0; item_id--)
				{
					item = layout->itemAt(item_id);
					layout->getItemPosition(item_id, &lin, &col, &row_span, &col_span);
					layout->removeItem(item);
					layout->addItem(item, lin + 1, col, row_span, col_span);
				}

				//Adding the base layout on the top
				layout->addLayout(baseobject_grid, 0, 0, 1, 0);
				baseobject_grid = layout;
			}
			else
			{
				layout->insertLayout(0, baseobject_grid);
			}

			// Configuring QTextEdit to accept tabs as focus changes.
			for(auto &txt_wgt : baseobject_grid->findChildren<QTextEdit *>())
				txt_wgt->setTabChangesFocus(true);

			baseobject_grid->setContentsMargins(QMargins(0, 0, 0, 0));
			baseobject_grid->setSpacing(layout->spacing());
			configureFormFields(obj_type, obj_type != ObjectType::BaseObject);
		}

		/*! \brief Configures a form layout in tabbed way.
		 *  The tab_widget is the QTabWidget instance that will hold
		 *  all pages. By default, this method add three new pages in
		 *  the tab widget: General (baseobject_grid), Associations and SQL preview.
		 *  If create_general_pg is false then the general page is not created. */
		void configureTabbedLayout(QTabWidget *tab_widget, bool create_general_pg = true);

		/*! \brief Configures a tabbed layout from a simple QHBoxLayout-ed
		 *  or a QVBoxLayout-ed widget. This method creates a QTabWidget instance and
		 *  move all widgets/sublayouts of the orignal widget's layout to the "General" page. */
		void configureTabbedLayout(bool create_attr_page, const QString &attr_pg_name = "", const QString &attr_pg_icon = "");

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
		
		[[deprecated]]
		void configureTabOrder(std::vector<QWidget *> widgets={});

		BaseObject *getHandledObject();

		/*! \brief This method can be reimplemented in each derived class
		 * to return the a custom SQL code preview of the object being
		 * handled so it can be displayed in the SQL preview tab.
		 * The default implementation returns the current SQL code of the object. */
		virtual QString getSQLCodePreview();
			
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
		[[deprecated]]
		static QFrame *generateVersionWarningFrame(std::map<QString, std::vector<QWidget *> > &fields, std::map<QWidget *, std::vector<QString> > *values=nullptr);
		
		//! \brief Generates a informative frame containing the specified message
		[[deprecated]]
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
void BaseObjectWidget::startConfiguration()
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
