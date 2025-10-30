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

#include "layersconfigwidget.h"
#include "colorpickerwidget.h"
#include "guiutilsns.h"
#include "relationshipview.h"

LayersConfigWidget::LayersConfigWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	setModel(nullptr);

	curr_item = nullptr;
	curr_item = nullptr;
	curr_row = -1;

	layers_tab->installEventFilter(this);
	frame->installEventFilter(this);

	QAction *act = visibility_menu.addAction(tr("Show all"), this, &LayersConfigWidget::setLayersActive);
	act->setData(true);

	act = visibility_menu.addAction(tr("Hide all"), this, &LayersConfigWidget::setLayersActive);
	act->setData(false);

	add_btn->setToolTip(add_btn->toolTip() + QString(" (%1)").arg(add_btn->shortcut().toString()));
	remove_btn->setToolTip(remove_btn->toolTip() + QString(" (%1)").arg(remove_btn->shortcut().toString()));
	remove_all_btn->setToolTip(remove_all_btn->toolTip() + QString(" (%1)").arg(remove_all_btn->shortcut().toString()));

	visibility_btn->setMenu(&visibility_menu);
	GuiUtilsNs::createDropShadow(this, 5, 5, 30);

	connect(toggle_layers_rects_chk, &QCheckBox::toggled, this, &LayersConfigWidget::toggleLayersRects);
	connect(toggle_layers_rects_chk, &QCheckBox::toggled, toggle_layers_names_chk, &QCheckBox::setEnabled);
	connect(toggle_layers_names_chk, &QCheckBox::toggled, this, &LayersConfigWidget::toggleLayersRects);

	connect(hide_tb, &QToolButton::clicked, this, &LayersConfigWidget::s_visibilityChanged);

	connect(add_btn, &QPushButton::clicked, this, [this](){
		addLayer("", true);
	});

	connect(remove_btn, &QPushButton::clicked, this, &LayersConfigWidget::removeLayer);

	connect(layers_tab, &QTableWidget::itemDoubleClicked, this, &LayersConfigWidget::startLayerRenaming);
	connect(layers_tab, &QTableWidget::itemChanged, this, &LayersConfigWidget::updateActiveLayers);
	connect(layers_tab, &QTableWidget::itemSelectionChanged, this, &LayersConfigWidget::finishLayerRenaming);
	connect(layers_tab, &QTableWidget::itemSelectionChanged, this, &LayersConfigWidget::enableButtons);

	connect(remove_all_btn, &QPushButton::clicked, this, [this](){
		removeLayer(true);
	});

	connect(rels_tabs_visibility_chk, &QCheckBox::toggled, this, [this](bool checked){
		if(checked)
			updateRelsVisibility();
		else
			updateActiveLayers();
	});
}

bool LayersConfigWidget::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == layers_tab)
	{
		if(event->type() == QEvent::KeyPress)
		{
			QKeyEvent *k_event = dynamic_cast<QKeyEvent *>(event);

			if(curr_item && (k_event->key() == Qt::Key_Enter || k_event->key() == Qt::Key_Return))
				finishLayerRenaming();
		}
		else if(event->type() == QEvent::FocusIn && curr_item && curr_item != layers_tab->currentItem())
			finishLayerRenaming();
	}
	else if(watched == frame && event->type() == QEvent::MouseMove)
	{
		static GuiUtilsNs::WidgetCornerId corner_id;
		QMouseEvent *m_event = dynamic_cast<QMouseEvent *>(event);

		if(m_event->buttons() == Qt::NoButton)
		{
			corner_id = GuiUtilsNs::getWidgetHoveredCorner(this, frame, m_event,
																										 GuiUtilsNs::WidgetCornerId::TopRightCorner);
		}

		GuiUtilsNs::resizeFloatingWidget(this, m_event, corner_id);
	}

	return QWidget::eventFilter(watched, event);
}

void LayersConfigWidget::updateActiveLayers()
{
	QList<unsigned> active_layers;
	QTableWidgetItem *item = nullptr;

	for(int row = 0; row < layers_tab->rowCount(); row++)
	{
		item = layers_tab->item(row, 0);

		if(item->checkState() == Qt::Checked)
			active_layers.append(static_cast<unsigned>(row));
	}

	model->scene->setActiveLayers(active_layers);

	if(rels_tabs_visibility_chk->isChecked())
		updateRelsVisibility();

	model->getDatabaseModel()->setObjectsModified({ ObjectType::Schema });
	emit s_activeLayersChanged();
}

void LayersConfigWidget::removeLayer(bool clear)
{
	QTableWidgetItem *item = nullptr;
	QString msg;

	if(clear)
		msg = tr("This action will delete all layers (except the default one), and the objects within them will be moved to the default layer. Do you want to proceed?");
	else
		msg = tr("Deleting the selected layer will cause the objects within it to be moved to the default layer. Do you want to proceed?");

	if(!Messagebox::isAccepted(Messagebox::confirm(msg)))
		return;

	if(clear)
	{
		model->scene->removeLayers();

		while(layers_tab->rowCount() > 1)
		{
			disconnect(rect_color_pickers.last(), nullptr, this, nullptr);
			rect_color_pickers.removeLast();

			disconnect(name_color_pickers.last(), nullptr, this, nullptr);
			name_color_pickers.removeLast();

			layers_tab->setRowCount(layers_tab->rowCount() - 1);
		}
	}
	else if(layers_tab->currentRow() > 0)
	{
		int row = layers_tab->currentRow();

		item = layers_tab->item(layers_tab->currentRow(), 0);
		model->scene->removeLayer(item->text());

		disconnect(rect_color_pickers.at(row), nullptr, this, nullptr);
		rect_color_pickers.removeAt(row);

		disconnect(name_color_pickers.at(row), nullptr, this, nullptr);
		name_color_pickers.removeAt(row);

		layers_tab->removeRow(row);
	}

	layers_tab->clearSelection();
	enableButtons();
}

void LayersConfigWidget::updateLayerColors(int layer_idx)
{
	if(layer_idx < 0)
	{
		layer_idx = rect_color_pickers.indexOf(dynamic_cast<ColorPickerWidget *>(sender()));

		if(layer_idx < 0)
			layer_idx = name_color_pickers.indexOf(dynamic_cast<ColorPickerWidget *>(sender()));
	}

	if(layer_idx >= 0 && layer_idx < layers_tab->rowCount())
	{
		model->scene->setLayerColors(layer_idx,
																 name_color_pickers[layer_idx]->getColor(0),
																 rect_color_pickers[layer_idx]->getColor(0));
		model->updateModelLayersInfo();
	}
}

void LayersConfigWidget::enableButtons()
{
	remove_btn->setEnabled(layers_tab->currentRow() > 0);
	remove_all_btn->setEnabled(layers_tab->rowCount() > 1);
}

void LayersConfigWidget::setLayersActive()
{
	QAction *act = qobject_cast<QAction *>(sender());
	Qt::CheckState chk_state = act->data().toBool() ? Qt::Checked : Qt::Unchecked;

	layers_tab->blockSignals(true);

	for(int row = 0; row < layers_tab->rowCount(); row++)
		layers_tab->item(row, 0)->setCheckState(chk_state);

	layers_tab->blockSignals(false);
	updateActiveLayers();
}

void LayersConfigWidget::setVisible(bool value)
{
	QWidget::setVisible(value);
	emit s_visibilityChanged(value);
}

void LayersConfigWidget::updateLayersRects()
{
	if(!model)
		return;

	model->getObjectsScene()->clearSelection();
	model->getObjectsScene()->updateLayerRects();
}

void LayersConfigWidget::toggleLayersRects()
{
	if(!model)
		return;

	model->getObjectsScene()->setLayerRectsVisible(toggle_layers_rects_chk->isChecked());
	model->getObjectsScene()->setLayerNamesVisible(toggle_layers_names_chk->isChecked());
	model->updateModelLayersInfo();
	model->getDatabaseModel()->setObjectsModified({ ObjectType::Schema });

	emit s_activeLayersChanged();
}

void LayersConfigWidget::updateLayersList()
{
	if(!model)
		return;

	toggle_layers_rects_chk->blockSignals(true);
	toggle_layers_rects_chk->setChecked(model->getObjectsScene()->isLayerRectsVisible());
	toggle_layers_rects_chk->blockSignals(false);

	layers_tab->blockSignals(true);
	for(auto &layer : model->scene->getLayers())
	{
		__addLayer(layer,
							 model->scene->isLayerActive(layer) ? Qt::Checked : Qt::Unchecked);
	}
	layers_tab->blockSignals(false);

	int idx = 0, p_idx = 0;
	QList<QStringList> colors_lists = {
		model->getDatabaseModel()->getLayerNameColors(),
		model->getDatabaseModel()->getLayerRectColors()
	};

	QList<QList<ColorPickerWidget*>*> pickers = { &name_color_pickers,
																								&rect_color_pickers };

	for(auto &cl_list : colors_lists)
	{
		idx = 0;

		for(auto &cl : cl_list)
		{
			if(idx >= pickers[p_idx]->size())
				break;

			pickers[p_idx]->at(idx)->blockSignals(true);
			pickers[p_idx]->at(idx)->setColor(0, QColor(cl));
			pickers[p_idx]->at(idx)->blockSignals(false);
			idx++;
		}

		p_idx++;
	}

	toggle_layers_names_chk->blockSignals(true);
	toggle_layers_names_chk->setChecked(model->getDatabaseModel()->isLayerNamesVisible());
	toggle_layers_names_chk->setEnabled(model->getDatabaseModel()->isLayerNamesVisible());
	toggle_layers_names_chk->blockSignals(false);

	toggle_layers_rects_chk->blockSignals(true);
	toggle_layers_rects_chk->setChecked(model->getDatabaseModel()->isLayerRectsVisible());
	toggle_layers_rects_chk->blockSignals(false);

}

void LayersConfigWidget::setModel(ModelWidget *model)
{
	bool enable = model != nullptr;

	this->model = model;
	layers_tab->setRowCount(0);
	name_color_pickers.clear();
	rect_color_pickers.clear();
	add_btn->setEnabled(enable);
	updateLayersList();
}

void LayersConfigWidget::__addLayer(const QString &name, Qt::CheckState chk_state)
{
	ColorPickerWidget *color_picker = nullptr;
	QTableWidgetItem *item = nullptr;

	int row = layers_tab->rowCount();
	layers_tab->insertRow(row);

	item = new QTableWidgetItem;
	item->setText(name);
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(chk_state);
	layers_tab->setItem(row, 0, item);

	color_picker = new ColorPickerWidget(1, layers_tab);
	color_picker->setButtonToolTip(0, tr("Layer name color"));
	color_picker->layout()->setContentsMargins(GuiUtilsNs::LtMargins);
	color_picker->setColor(0, QColor(0,0,0));
	name_color_pickers.append(color_picker);

	connect(color_picker, &ColorPickerWidget::s_colorChanged, this, &LayersConfigWidget::updateLayerColors);
	connect(color_picker, &ColorPickerWidget::s_colorsChanged, this, [this]() {
		updateLayerColors();
	});

	layers_tab->setCellWidget(row, 1, color_picker);

	color_picker = new ColorPickerWidget(1, layers_tab);
	color_picker->setButtonToolTip(0, tr("Layer rectangle color"));
	color_picker->layout()->setContentsMargins(GuiUtilsNs::LtMargins);
	color_picker->generateRandomColors();
	rect_color_pickers.append(color_picker);

	connect(color_picker, &ColorPickerWidget::s_colorChanged, this, &LayersConfigWidget::updateLayerColors);
	connect(color_picker, &ColorPickerWidget::s_colorsChanged, this, [this]() {
		updateLayerColors();
	});

	layers_tab->setCellWidget(row, 2, color_picker);
	layers_tab->horizontalHeader()->setStretchLastSection(false);
	layers_tab->resizeRowsToContents();
	layers_tab->resizeColumnsToContents();
	layers_tab->horizontalHeader()->setStretchLastSection(true);
	layers_tab->clearSelection();

	enableButtons();
}

QString LayersConfigWidget::addLayer(const QString &name, bool config_obj_sel)
{
	QString fmt_name = name.isEmpty() ? tr("New layer") : name;
	QStringList act_layers = model->scene->getActiveLayers();

	fmt_name = model->scene->addLayer(fmt_name);

	__addLayer(fmt_name);
	act_layers.prepend(fmt_name);

	updateLayerColors(layers_tab->rowCount() - 1);
	model->scene->setActiveLayers(act_layers);

	/* Reconfigure the model's menu if we have selected items so the new layer can
	 * appear in the "Move to layer" quick action */
	if(config_obj_sel && !model->scene->selectedItems().isEmpty())
		model->configureObjectSelection();

	return fmt_name;
}

void LayersConfigWidget::startLayerRenaming()
{
	if(layers_tab->currentRow() < 0)
		return;

	QTableWidgetItem *item = layers_tab->item(layers_tab->currentRow(), 0);

	curr_item = item;
	curr_text = item->text();
	curr_row = layers_tab->currentRow();
	layers_tab->openPersistentEditor(item);
}

void LayersConfigWidget::finishLayerRenaming()
{
	if(layers_tab->currentRow() < 0)
		return;

	if(curr_item)
	{
		layers_tab->closePersistentEditor(curr_item);

		if(curr_item->text().isEmpty())
			curr_item->setText(curr_text);
		else
			curr_item->setText(model->scene->renameLayer(curr_row, curr_item->text()));

		curr_item = nullptr;
		curr_text.clear();
		curr_row = -1;

		/* Reconfigure the model's menu if we have selected items so the renamed layer can
		 * appear in the "Move to layer" quick action */
		if(!model->scene->selectedItems().isEmpty())
			model->configureObjectSelection();
	}
}

void LayersConfigWidget::updateRelsVisibility()
{
	if(!model)
		return;

	RelationshipView *rel_view = nullptr;

	for(auto &item : model->scene->items())
	{
		rel_view = dynamic_cast<RelationshipView *>(item);

		if(!rel_view)
			continue;

		if(rel_view->isVisible() &&
				(!rel_view->isTableVisible(BaseRelationship::SrcTable) ||
				 !rel_view->isTableVisible(BaseRelationship::DstTable)))
		{
			rel_view->setVisible(false);
		}
	}
}
