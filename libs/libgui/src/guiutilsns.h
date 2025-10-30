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
\namespace GuiUtilsNs
\brief This namespace is used to organize all functions or constants used in the libgui package.
*/

#ifndef GUI_UTILS_NS_H
#define GUI_UTILS_NS_H

#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QPixmap>
#include <QFileDialog>
#include "baseobject.h"
#include "widgets/numberedtexteditor.h"
#include "csvdocument.h"
#include "settings/appearanceconfigwidget.h"
#include <QMargins>

namespace GuiUtilsNs {
	/*! \brief WidgetCornerId used by resizeFloatingWidget() to determine
	 *  which corners of an widget can be used to resize it */
	enum WidgetCornerId: unsigned {
		NoCorners = 0,
		LeftCorner = 2,
		TopCorner = 4,
		RightCorner = 8,
		BottomCorner = 16,
		TopLeftCorner = TopCorner | LeftCorner,
		TopRightCorner = TopCorner | RightCorner,
		BottomLeftCorner = BottomCorner | LeftCorner,
		BottomRightCorner = BottomCorner | RightCorner,
		AllCorners = 31
	};

	static constexpr int LtMargin = 5,
	LtSpacing = 5;

	static const QMargins LtMargins { LtMargin, LtMargin, LtMargin, LtMargin };

	enum FontFactorId: unsigned {
		SmallFontFactor,
		MediumFontFactor,
		BigFontFactor,
		HugeFontFactor
	};

	extern __libgui void configureWidgetFont(QWidget *widget, FontFactorId factor_id);
	extern __libgui void __configureWidgetFont(QWidget *widget, double factor);
	extern __libgui void configureWidgetsFont(const QWidgetList widgets, FontFactorId factor_id);

	/*! \brief Creates an item in the specified QTreeWidget instance.
		The new item is automatically inserted on the QTreeWidget object.
		Setting word_wrap will create a QLabel instance into item's and assign the text to it. */
	extern __libgui QTreeWidgetItem *createOutputTreeItem(QTreeWidget *output_trw, const QString &text, const QPixmap &ico=QPixmap(),
																													QTreeWidgetItem *parent=nullptr, bool expand_item=true, bool word_wrap=false);

	//! \brief Creates an item in the specified QListWidget instance with the specified text and ico
	extern __libgui void createOutputListItem(QListWidget *output_lst, const QString &text, const QPixmap &ico=QPixmap(), bool is_formated=true);

	/*! \brief Toggles the SQL code for the object. This function also toggles the SQL of the references
		related to the input object */
	extern __libgui void disableObjectSQL(BaseObject *object, bool disable);

	//! \brief Recursively toggles the specified object's references SQL
	extern __libgui void disableReferencesSQL(BaseObject *object);

	/*! \brief Fills a tree widget with all the stack trace provided by the passed Exception. A root item
	can be specified so all created child items are appended to it */
	extern __libgui void createExceptionsTree(QTreeWidget *exceptions_trw, Exception &e, QTreeWidgetItem *root);

	//! \brief Returns the path, in the icon resource, to the provided icon name
	extern __libgui QString getIconPath(const QString &icon);

	/*! \brief Returns the path, in the icon resource, to the icon of the provided object type
	 *  The parameter sub_type is used to retrive the sub type icon. Currently only relationships
	 *  and constraints have sub types being for relationships the Relationship::RelType enum and
	 *  constraints the class ConstraintType */
	extern __libgui QString getIconPath(ObjectType obj_type, int sub_type = -1);

	//! \brief Resizes the provided widget considering font dpi changes as well screen size
	extern __libgui void resizeWidget(QWidget *widget);

	//! brief Changes the values of the grid selection at once
	extern __libgui void openColumnDataForm(QTableWidget *results_tbw);

	//! brief Opens the bulk data edit in readonly mode and display the text of the provided model index
	extern __libgui void openColumnDataForm(const QModelIndex &index);

	//! \brief Creates drop shadown on a widget
	extern __libgui void createDropShadow(QWidget *wgt, int x_offset = 2, int y_offset = 2, int radius = 5, const QColor &color = QColor(0, 0, 0, 100));

	//! \brief Creates drop shadown on a widget based on the current UI theme
	extern __libgui void updateDropShadow(QWidget *wgt);

	//! \brief Changes drop shadows color and offset to match the current theme.
	extern __libgui void updateDropShadows(QWidgetList widgets, const QString &class_name = "QToolButton");

	/*! \brief Handles the currently provided file dialog state to file.
	 * If save_state is true, then the dialog's state is saved to file
	 * otherwise is restored */
	extern __libgui void handleFileDialogState(QFileDialog *file_dlg, bool save_state);

	//! \brief Saves the currently provided file dialog state to file
	extern  __libgui void saveFileDialogState(QFileDialog *file_dlg);

	//! \brief Restores the saved filed dialog from file/memory and applies to the provided file dialog
	extern __libgui void restoreFileDialogState(QFileDialog *file_dlg);

	/*! \brief Lists the objects in a QTableView using a list of database model objects as data source.
	 * The first column on each row of table contains the reference to the object.
	 * The parameter search_attr is used to display the attribute value in which the search was performed. */
	extern __libgui void populateObjectsTable(QTableView *table_vw, const std::vector<BaseObject *> &objects, const QString &search_attr = "");

	//! \brief Lists the objects in a QTableView using a list of object attributes (see Catalog::getObjects) as data source.
	extern __libgui void populateObjectsTable(QTableView *table_vw, const std::vector<attribs_map> &attribs);

	//! \brief Populates the provided table widget with the data in a parsed CSV document.
	extern __libgui void populateTable(QTableWidget *tab_wgt, const CsvDocument &csv_doc);

	/*! \brief Opens an instance of QFileDialog with the provided attributes.
	 *  The method returns the files/directories selected by the user in the dialog */
	extern __libgui QStringList selectFiles(const QString &title = QT_TR_NOOP("Select file..."), QFileDialog::FileMode file_mode = QFileDialog::AnyFile,
																					QFileDialog::AcceptMode accept_mode = QFileDialog::AcceptOpen,	const QStringList &name_filters = {},
																					const QStringList &mime_filters = {}, const QString &default_suffix = "", const QString &selected_file = "");

	/*! \brief Writes the provided buffer to a file which path is selected via QFileDialog.
	 * Custom attributes for the QFileDialog instance can be provided.
	 * Raises an exception in case of the file couldn't be saved.
	 * The method does nothing if file_mode is different from QFileDialog::ExistingFile and QFileDialog::AnyFile */
	extern __libgui bool selectAndSaveFile(const QByteArray &buffer, const QString &title = QT_TR_NOOP("Save file..."),
																				 QFileDialog::FileMode file_mode = QFileDialog::AnyFile, const QStringList &name_filters = {},
																				 const QStringList &mime_filters = {}, const QString &default_suffix = "", const QString &selected_file = "");

	/*! \brief Loads a file which path is selected via QFileDialog returning the buffer with the contents of the file.
	 * Custom attributes for the QFileDialog instance can be provided.
	 * Raises an exception in case of the file couldn't be loaded.
	 * The method does nothing if file_mode is different from QFileDialog::ExistingFile and QFileDialog::AnyFile */
	extern __libgui  bool selectAndLoadFile(QByteArray &buffer,
																					const QString &title = QT_TR_NOOP("Load file..."), QFileDialog::FileMode file_mode = QFileDialog::AnyFile,
																					const QStringList &name_filters = {}, const QStringList &mime_filters = {}, const QString &selected_file = "");

	//! \brief Creates an action in a QLineEdit that controls the visibility of passwords
	extern __libgui void createPasswordShowAction(QLineEdit *parent_edt);

	/*! \brief Resizes the toolbuttons under the widget wgt according to its width.
	 *  If the widget's width isn't enough to display the tool buttons with icons and texts
	 *  the this function changes the tool buttons to icon-only style */
	extern __libgui void resizeChildToolButtons(QWidget *wgt, const QSize &new_size);

	/*! \brief Returns the corner id (see WidgetCornerId) that is currently hovered in the provided widget.
	 *  The event_wgt is the widget that has triggered the mouse event.
	 *  The corners params holds the corner ids that the hovered is allowed for the widget */
	extern __libgui WidgetCornerId getWidgetHoveredCorner(QWidget *widget, QWidget *event_wgt, QMouseEvent *event, WidgetCornerId corners);

	/*! \brief Resizes a widget based a mouse move event.
	 *  The corner param is used to identify from which corner the widget must be resized */
	extern __libgui void resizeFloatingWidget(QWidget *widget, QMouseEvent *event, WidgetCornerId corner);

	/*! \brief Moves a widget based a mouse move event that is captured by a handle widget
	 *  in the the edge of the widget */
	extern __libgui void moveFloatingWidget(QWidget *widget, QWidget *event_wgt, QMouseEvent *event);

	/*! \brief Configures the font family/size of the provided QPlainTextEdit instance
	 *  to use the global settings defined in AppearanceSettingsWidget */
	template<class Class, std::enable_if_t<std::is_base_of_v<QPlainTextEdit, Class> ||
																				 std::is_same_v<Class, QPlainTextEdit> ||
																				 std::is_same_v<Class, QTextEdit>, bool> = true>
	void configureTextEditFont(Class *txt, double custom_fnt_size = 0)
	{
		if(!txt)
			return;

		if constexpr (std::is_base_of_v<QPlainTextEdit, Class> ||
									std::is_class_v<QPlainTextEdit> ||
									std::is_class_v<QTextEdit>)
		{
			std::map<QString, attribs_map> confs = AppearanceConfigWidget::getConfigurationParams();

			//Configuring font style for output widget
			if(!confs[Attributes::Code][Attributes::Font].isEmpty())
			{
				double size =
						custom_fnt_size > 0 ? custom_fnt_size :
						confs[Attributes::Code][Attributes::FontSize].toDouble();

				if(size < 5.0)
					size = 5.0;

				QFont fnt = txt->font();
				fnt.setFamily(confs[Attributes::Code][Attributes::Font]);
				fnt.setPointSizeF(size);
				txt->setFont(fnt);
			}
		}
	}

	/*! \brief Creates a wiget in a parent. If the parent has no layout configured then
	 * this function also creates a layout for the parent and puts the new widget there.
	 * The user can specify the layout margins. If no parent is provided the object is an
	 * orphan one, meaning the user needs to take care of its destruction */
	template<class WgtClass, typename ...CtorArgs,
					 std::enable_if_t<std::is_base_of_v<QWidget, WgtClass>, bool> = true>
	WgtClass *createWidgetInParent(int lt_margins, CtorArgs... new_wgt_ctor_args)
	{
		WgtClass *new_wgt = new WgtClass(new_wgt_ctor_args...);
		QWidget *parent = qobject_cast<QWidget *>(new_wgt->parent());

		if(parent && !parent->layout())
		{
			QVBoxLayout *vbox = new QVBoxLayout(parent);
			vbox->addWidget(new_wgt);
			vbox->setContentsMargins(lt_margins, lt_margins, lt_margins, lt_margins);
			vbox->setSpacing(LtSpacing);
		}

		return new_wgt;
	}

	/*! \brief Creates a wiget in a parent. If the parent has no layout configured then
	 * this function also creates a layout for the parent and puts the new widget there.
	 * This version, creates the layout in parent with no margins. If no parent is provided
	 * the object is an orphan one, meaning the user needs to take care of its destruction */
	template<class WgtClass, typename ...CtorArgs,
					 std::enable_if_t<std::is_base_of_v<QWidget, WgtClass>, bool> = true>
	WgtClass *createWidgetInParent(CtorArgs... new_wgt_ctor_args)
	{
		return createWidgetInParent<WgtClass>(0, new_wgt_ctor_args...);
	}

	/*! \brief Creates a NumberedTextEditor instance automatically assigning it to 'parent'.
	 * This function will create a layout if 'parent' doesn't has one. If parent has a layout
	 * the function will do nothing. If parent is null creates an orphan object which means the
	 * user must take care of the destruction of the object */
	extern __libgui NumberedTextEditor *createNumberedTextEditor(QWidget *parent, bool act_btns_enabled = false, qreal custom_fnt_size = 0);
}

#endif
