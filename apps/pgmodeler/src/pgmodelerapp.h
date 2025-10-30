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
\ingroup pgmodeler
\class PgModelerApp
\brief This class inherits from QApplication and has the notify() method modified
 to treat the exceptions raised by pgModeler components.
*/

#ifndef PGMODELER_APP_H
#define PGMODELER_APP_H

#include "application.h"
#include <QTextStream>
#include <QTranslator>
#include <QFile>

class PgModelerApp: public Application {
	Q_OBJECT

	public:
		PgModelerApp(int & argc, char ** argv);
		bool notify(QObject *receiver, QEvent *event) override;
};

#endif
