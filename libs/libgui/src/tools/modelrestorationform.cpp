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

#include "modelrestorationform.h"
#include "customuistyle.h"
#include "utilsns.h"
#include "globalattributes.h"
#include <QDir>
#include <QFileInfo>

ModelRestorationForm::ModelRestorationForm(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);

	connect(restore_btn, &QPushButton::clicked, this, &ModelRestorationForm::accept);
	connect(cancel_btn, &QPushButton::clicked, this, &ModelRestorationForm::reject);
	connect(tmp_files_tbw, &QTableWidget::itemSelectionChanged, this, &ModelRestorationForm::enableRestoration);
}

QStringList ModelRestorationForm::getTemporaryModels()
{
	//Returns if there is some .dbm file on the tmp dir
	QStringList list = QDir(GlobalAttributes::getTemporaryPath(),
													"*" + GlobalAttributes::DbModelExt,
													QDir::Time, QDir::Files | QDir::NoDotAndDotDot).entryList();

	for(auto &file : ignored_files)
		list.removeAll(file);

	return list;
}

int ModelRestorationForm::exec()
{
	QStringList file_list = this->getTemporaryModels(), tmp_info;
	QFileInfo info;
	QTableWidgetItem *item=nullptr;
	QString buffer, filename;
	QRegularExpression regexp("(\\<database)( )+(name)(=)(\")");
	QRegularExpressionMatch match;
	int start=-1, end=-1, col=0;

	while(!file_list.isEmpty())
	{
		info.setFile(GlobalAttributes::getTemporaryPath(), file_list.front());
		filename=GlobalAttributes::getTemporaryFilePath(file_list.front());

		buffer.append(UtilsNs::loadFile(filename));

		match = regexp.match(buffer);
		start = match.capturedStart() + match.capturedLength();
		end = buffer.indexOf("\"", start);

		tmp_info.append(buffer.mid(start, end - start));
		tmp_info.append(info.fileName());
		tmp_info.append(info.lastModified().toString("yyyy-MM-dd hh:mm:ss"));

		if(info.size() < 1024)
			tmp_info.append(QString("%1 bytes").arg(info.size()));
		else
			tmp_info.append(QString("%1 KB").arg(info.size()/1024));

		tmp_files_tbw->insertRow(tmp_files_tbw->rowCount());

		for(col=0; col < tmp_info.size(); col++)
		{
			item=new QTableWidgetItem;
			item->setText(tmp_info.at(col));
			item->setData(Qt::UserRole, filename);
			item->setSelected(true);
			tmp_files_tbw->setItem(tmp_files_tbw->rowCount()-1, col, item);
		}

		file_list.pop_front();
		buffer.clear();
		tmp_info.clear();
	}

	tmp_files_tbw->resizeColumnsToContents();
	tmp_files_tbw->resizeRowsToContents();

	return QDialog::exec();
}

void ModelRestorationForm::setIgnoredFiles(const QStringList &list)
{
	ignored_files = list;
}

bool ModelRestorationForm::hasTemporaryModels()
{
	return !this->getTemporaryModels().isEmpty();
}

void ModelRestorationForm::removeTemporaryFiles()
{
	QDir tmp_file;
	QStringList tmp_files = QDir(GlobalAttributes::getTemporaryPath(), "*",
															 QDir::Name, QDir::Files | QDir::NoDotAndDotDot).entryList();

	for(auto &file : tmp_files)
		tmp_file.remove(GlobalAttributes::getTemporaryFilePath(file));
}

void ModelRestorationForm::removeTemporaryModels()
{
	QStringList file_list=this->getTemporaryModels();
	QDir tmp_file;

	for(auto &file : file_list)
		tmp_file.remove(GlobalAttributes::getTemporaryFilePath(file));
}

void ModelRestorationForm::removeTemporaryModel(const QString &tmp_model)
{
	QDir tmp_file;
	QString file=QFileInfo(tmp_model).fileName();
	tmp_file.remove(GlobalAttributes::getTemporaryFilePath(file));
}

void ModelRestorationForm::enableRestoration()
{
	restore_btn->setEnabled(!tmp_files_tbw->selectedItems().isEmpty());
}

QStringList ModelRestorationForm::getSelectedModels()
{
	QStringList list;
	QList<QTableWidgetItem *> items;

	items=tmp_files_tbw->selectedItems();

	while(!items.isEmpty())
	{
		list.push_back(items.front()->data(Qt::UserRole).toString());
		items.pop_front();
	}

	list.removeDuplicates();
	return list;
}

