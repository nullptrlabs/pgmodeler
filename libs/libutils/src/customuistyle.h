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
\ingroup libutils
\class CustomUiStyle
\brief Implements a derivative class of QProxyStyle that can be used to override the application UI style at rendering level.
One instance of this class is created in the moment the application is instantiated and the custom style is applied to
all widgets in the application.
*/

#ifndef CUSTOM_UI_STYLE_H
#define CUSTOM_UI_STYLE_H

#include "utilsglobal.h"
#include <QProxyStyle>
#include <QMap>

class __libutils CustomUiStyle : public QProxyStyle {
	private:
		static QMap<PixelMetric, int> pixel_metrics;

	public:
		CustomUiStyle();

		CustomUiStyle(const QString &key);

		virtual ~CustomUiStyle();

		/*! \brief Defines a custom pixel metric attribute value globally. Which means, all instancess
		 *  of this class will share the same pixel metrics values */
		static void setPixelMetricValue(PixelMetric metric, int value);

		int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0) const;
};

#endif
