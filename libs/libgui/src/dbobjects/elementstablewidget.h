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
\class ElementsTableWidget
\brief Implements the operations to create/edit constraints (exclude), indexes and partition keys elements via form storing them on a objects table (grid).
*/

#ifndef ELEMENTS_TABLES_WIDGET_H
#define ELEMENTS_TABLES_WIDGET_H

#include "widgets/customtablewidget.h"
#include "indexelement.h"
#include "excludeelement.h"
#include "partitionkey.h"
#include "baseform.h"
#include "elementwidget.h"

class __libgui ElementsTableWidget: public QWidget {
	Q_OBJECT

	private:
		/*! \brief Store an instance of the element being handled (see setAttributes())
		 * This one is used to determine the settings of the element widget open when the user
		 * wants to create new elements on the grid */
		Element *handled_elem;

		ElementWidget *element_wgt;

		BaseForm element_form;

		DatabaseModel *model;

		BaseObject *parent_obj;

		//! \brief Table widget used to control the index elements
		CustomTableWidget *elements_tab;
		
		//! \brief Shows the element data on the elements table at the specified line
		void showElementData(Element *elem, int elem_idx);

		//! \brief Copies the provided element storing it on a QVariant according to its real type (class)
		QVariant copyElementData(Element *elem);
		
		//! \brief Opens the element editing form using the attributes of the provided element
		int openElementForm(Element *elem);

	public:
		ElementsTableWidget(QWidget *parent = nullptr);
		~ElementsTableWidget() override;

		//! \brief Configures the grid based upon the template Class in use
		template<class Class>
		void setAttributes(DatabaseModel *model, BaseObject *parent_obj)
		{
			if(handled_elem && !dynamic_cast<Class *>(handled_elem))
			{
				delete handled_elem;
				handled_elem = nullptr;
			}

			if(!handled_elem)
				handled_elem = new Class;

			this->model = model;
			this->parent_obj = parent_obj;

			if(dynamic_cast<IndexElement *>(handled_elem))
				elements_tab->setHeaderVisible(2, false);

			if(dynamic_cast<ExcludeElement *>(handled_elem))
				elements_tab->setHeaderVisible(4, false);

			if(dynamic_cast<PartitionKey *>(handled_elem))
			{
				elements_tab->setHeaderVisible(2, false);
				elements_tab->setHeaderVisible(5, false);
				elements_tab->setHeaderVisible(6, false);
			}
		}

		//! \brief Fills the grid with the elements on the vector vector
		template<class Class>
		void setElements(std::vector<Class> elems)
		{
			elements_tab->blockSignals(true);
			for(auto &elem : elems)
			{
				elements_tab->addRow();
				showElementData(&elem, elements_tab->getRowCount() - 1);
			}
			elements_tab->clearSelection();
			elements_tab->blockSignals(false);
		}

		//! \brief Fills the provided vector with the elements on the grid
		template<class Class>
		void getElements(std::vector<Class> &elems)
		{
			if(elements_tab->getRowCount() > 0)
			{
				if(elements_tab->getRowData(0).canConvert<Class>())
				{
					elems.clear();
					for(unsigned i=0; i < elements_tab->getRowCount(); i++)
						elems.push_back(elements_tab->getRowData(i).value<Class>());
				}
			}
		}
		
	private slots:
		void addElement(int elem_idx);
		void editElement(int elem_idx);
};

#endif
