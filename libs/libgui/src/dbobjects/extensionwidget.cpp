#include "extensionwidget.h"
#include "guiutilsns.h"

ExtensionWidget::ExtensionWidget(QWidget * parent) : BaseObjectWidget(parent, ObjectType::Extension)
{
	Ui_ExtensionWidget::setupUi(this);

	types_tab = new CustomTableWidget(CustomTableWidget::AddButton |
																		 CustomTableWidget::RemoveButton |
																		 CustomTableWidget::RemoveAllButton, true, this);
	types_tab->setCellsEditable(true);
	types_tab->setColumnCount(2);
	types_tab->setHeaderLabel(tr("Name"), 0);
	types_tab->setHeaderIcon(QIcon(GuiUtilsNs::getIconPath("uid")), 0);
	types_tab->setHeaderLabel(tr("Schema"), 1);
	types_tab->setHeaderIcon(QIcon(GuiUtilsNs::getIconPath("schema")), 1);

	QVBoxLayout *vbox = new QVBoxLayout(types_gb);
	vbox->addWidget(types_tab);
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);

	configureFormLayout(extension_grid, ObjectType::Extension);
	configureTabOrder({ cur_ver_edt, old_ver_edt, types_tab });

	setMinimumSize(500, 500);
}

void ExtensionWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Extension *ext)
{
	BaseObjectWidget::setAttributes(model, op_list, ext, schema);

	if(ext)
	{
		cur_ver_edt->setText(ext->getVersion(Extension::CurVersion));
		old_ver_edt->setText(ext->getVersion(Extension::OldVersion));

		for(auto &ext_obj : ext->getObjects(ObjectType::Type))
		{
			types_tab->addRow();
			types_tab->setCellText(ext_obj.getName(), types_tab->getRowCount() - 1, 0);
			types_tab->setCellText(ext_obj.getParent(), types_tab->getRowCount() - 1, 1);
		}

		types_tab->clearSelection();
	}
}

void ExtensionWidget::applyConfiguration()
{
	try
	{
		startConfiguration<Extension>();

		Extension *extension = dynamic_cast<Extension *>(this->object);

		BaseObjectWidget::applyConfiguration();

		extension->setVersion(Extension::CurVersion, cur_ver_edt->text());
		extension->setVersion(Extension::OldVersion, old_ver_edt->text());

		extension->removeObjects();
		QString sch_name, typ_name;

		for(unsigned row = 0; row < types_tab->getRowCount(); row++)
		{
			typ_name = types_tab->getCellText(row, 0);
			sch_name = types_tab->getCellText(row, 1);

			if(!sch_name.isEmpty())
				extension->addObject(Extension::ExtObject(sch_name, ObjectType::Schema));

			extension->addObject(Extension::ExtObject(typ_name, ObjectType::Type, sch_name));
		}

		finishConfiguration();

		// Updating the extension objects in the model if the extension is being updated
		if(!new_object && !model->updateExtensionObjects(extension))
			Messagebox::alert(tr("Some removed objects were restored because they are still being referenced in the model! Please, undo the link between them and their referrer objects in the database model before trying to remove them again."));
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
