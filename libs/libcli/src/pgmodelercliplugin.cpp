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

#include "pgmodelercliplugin.h"

PgModelerCliPlugin::PgModelerCliPlugin()
{
	cli_app = nullptr;
}

bool PgModelerCliPlugin::isOpModeOption(const QString &opt)
{
	return getOpModeOptions().contains(opt);
}

QStringList PgModelerCliPlugin::getValidOptions()
{
	QStringList opts;

	for(auto &[opt, _] : getLongOptions())
		opts.append(opt);

	return opts;
}

void PgModelerCliPlugin::initPlugin(PgModelerCliApp *app)
{
	cli_app = app;
}
