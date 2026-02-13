/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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
\class EventTriggerWidget
\brief Implements the operations to create/edit event triggers via form.
*/

#ifndef EVENT_TRIGGER_WIDGET_H
#define EVENT_TRIGGER_WIDGET_H

#include "baseobjectwidget.h"
#include "widgets/customtablewidget.h"
#include "ui_eventtriggerwidget.h"

class __libgui EventTriggerWidget: public BaseObjectWidget, public Ui::EventTriggerWidget {
	Q_OBJECT

	private:	
		CustomTableWidget *filter_tab;
		
		ObjectSelectorWidget *function_sel;
		
	public:
		EventTriggerWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, EventTrigger *event_trig);
		
	public slots:
		void applyConfiguration() override;
};

#endif
