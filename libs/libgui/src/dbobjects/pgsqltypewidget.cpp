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

#include "pgsqltypewidget.h"
#include <QCompleter>

const QString PgSQLTypeWidget::InvalidType {"invalid_type"};

PgSQLTypeWidget::PgSQLTypeWidget(QWidget *parent, const QString &label) : QWidget(parent)
{
	QStringList interval_lst, spatial_lst;

	setupUi(this);

	allow_qualifiers = true;

	if(!label.isEmpty())
		groupBox->setTitle(label);

	this->setWindowTitle(groupBox->title());

	format_hl=nullptr;
	format_hl=new SyntaxHighlighter(format_txt, true, false, font().pointSizeF());
	format_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
	this->adjustSize();

	interval_lst = IntervalType::getTypes();
	interval_cmb->addItem("");
	interval_cmb->addItems(interval_lst);

	spatial_lst = SpatialType::getTypes();
	spatial_lst.sort();
	spatial_cmb->addItem(tr("NONE"));
	spatial_cmb->addItems(spatial_lst);

	type_cmb->installEventFilter(this);
	type_cmb->completer()->setFilterMode(Qt::MatchContains);
	type_cmb->completer()->setCompletionMode(QCompleter::PopupCompletion);

	connect(type_cmb, &QComboBox::currentIndexChanged, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(type_cmb, &QComboBox::currentTextChanged, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(precision_sb, &QSpinBox::valueChanged, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(length_sb, &QSpinBox::valueChanged, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(dimension_sb, &QSpinBox::valueChanged, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(interval_cmb, &QComboBox::currentIndexChanged, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(timezone_chk, &QCheckBox::toggled, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(spatial_cmb,  &QComboBox::currentIndexChanged, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(var_m_chk, &QCheckBox::toggled, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(var_z_chk, &QCheckBox::toggled, this, &PgSQLTypeWidget::updateTypeFormat);
	connect(srid_spb, &QSpinBox::valueChanged, this, &PgSQLTypeWidget::updateTypeFormat);
}

bool PgSQLTypeWidget::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::KeyRelease && object == type_cmb)
		updateTypeFormat();

	return QWidget::eventFilter(object, event);
}

void PgSQLTypeWidget::updateTypeFormat()
{
	try
	{
		QVariant data;

		//Gets the data related to the current selected data type
		data = type_cmb->itemData(type_cmb->currentIndex());

		//If the data value (index) is 0 indicates that the type is a built-in one
		if(data.toUInt() == 0)
			type = type_cmb->currentText();
		else
			//Case the index is greated than zero indicates that the type is a user-defined one
			type = data.toUInt();

		length_sb->setEnabled(allow_qualifiers && type.hasVariableLength());
		timezone_chk->setVisible(type.acceptsTimezone());
		timezone_lbl->setVisible(type.acceptsTimezone());
		precision_sb->setEnabled(allow_qualifiers && type.acceptsPrecision());
		dimension_sb->setEnabled(type != "void");

		interval_cmb->setVisible(type == "interval");
		interval_lbl->setVisible(interval_cmb->isVisible());
		interval_cmb->setEnabled(allow_qualifiers);

		spatial_cmb->setEnabled(allow_qualifiers);
		spatial_cmb->setVisible(type.isPostGisGeoType());
		spatial_lbl->setVisible(type.isPostGisGeoType());
		variation_lbl->setVisible(type.isPostGisGeoType());

		srid_lbl->setVisible(type.isPostGisGeoType());
		srid_spb->setEnabled(allow_qualifiers);
		srid_spb->setVisible(type.isPostGisGeoType());

		var_m_chk->setEnabled(allow_qualifiers);
		var_m_chk->setVisible(type.isPostGisGeoType());

		var_z_chk->setEnabled(allow_qualifiers);
		var_z_chk->setVisible(type.isPostGisGeoType());

		if(spatial_cmb->isVisible())
		{
			SpatialType spatial_tp;
			QString sp_type_name = spatial_cmb->currentIndex() > 0 ? spatial_cmb->currentText() : "";
			spatial_tp = SpatialType(sp_type_name, srid_spb->value());

			if(var_z_chk->isChecked() && var_m_chk->isChecked())
				spatial_tp.setVariation(SpatialType::VarZm);
			else if(var_m_chk->isChecked())
				spatial_tp.setVariation(SpatialType::VarM);
			else if(var_z_chk->isChecked())
				spatial_tp.setVariation(SpatialType::VarZ);

			type.setSpatialType(spatial_tp);
		}

		type.setLength(length_sb->value());
		type.setPrecision(precision_sb->value());
		type.setDimension(dimension_sb->value());
		type.setIntervalType(interval_cmb->currentText());
		type.setWithTimezone(timezone_chk->isChecked());

		format_txt->setPlainText(*type);
	}
	catch(Exception &)
	{
		format_txt->setPlainText(InvalidType);
	}
}

void PgSQLTypeWidget::listPgSQLTypes(QComboBox *combo, DatabaseModel *model, UserTypeConfig::TypeConf user_type_conf, bool oid_types, bool pseudo_types)
{
	if(combo)
	{
		QStringList types;
		int idx, count;

		combo->blockSignals(true);
		combo->clear();

		//Getting the user defined type adding them into the combo
		PgSqlType::getUserTypes(types,model, user_type_conf);
		types.sort();
		count=types.size();

		for(idx=0; idx < count; idx++)
			combo->addItem(types[idx], QVariant(PgSqlType::getUserTypeIndex(types[idx],nullptr,model)));

		//Getting the built-in type adding them into the combo
		types = PgSqlType::getTypes(oid_types, pseudo_types);
		types.sort();
		combo->addItems(types);
		combo->blockSignals(false);
	}
}

void PgSQLTypeWidget::setAttributes(PgSqlType type, DatabaseModel *model, bool allow_qualifiers,  UserTypeConfig::TypeConf usr_type_conf, bool oid_types, bool pseudo_types)
{
	QWidgetList child_wgts {
			type_cmb, precision_sb, length_sb,
			dimension_sb, interval_cmb, timezone_chk,
			spatial_cmb, var_m_chk, var_z_chk, srid_spb
	};

	try
	{
		int idx;
		QString type_name;

		this->allow_qualifiers = allow_qualifiers;

		//type_cmb->blockSignals(true);
		listPgSQLTypes(type_cmb, model, usr_type_conf, oid_types, pseudo_types);
		//type_cmb->blockSignals(false);

		//Get the passed type index
		type_name=~type;
		type_name.remove(QRegularExpression("( )(with)(out)?(.)*"));
		idx = type_cmb->findText(type_name);

		for(auto &wgt : child_wgts)
			wgt->blockSignals(true);

		//Select the type on the combo
		type_cmb->setCurrentIndex(idx);

		length_sb->setValue(type.getLength());
		precision_sb->setValue(type.getPrecision());
		dimension_sb->setValue(type.getDimension());

		idx = interval_cmb->findText(~(type.getIntervalType()));
		interval_cmb->setCurrentIndex(idx);

		idx = spatial_cmb->findText(~(type.getSpatialType()));
		if(idx < 0) idx = 0;
		spatial_cmb->setCurrentIndex(idx);

		timezone_chk->setChecked(type.isWithTimezone());

		this->type = type;
		updateTypeFormat();

		for(auto &wgt : child_wgts)
			wgt->blockSignals(false);
	}
	catch(Exception &e)
	{
		for(auto &wgt : child_wgts)
			wgt->blockSignals(false);

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

PgSqlType PgSQLTypeWidget::getPgSQLType()
{
	if(format_txt->toPlainText() == InvalidType)
		throw Exception(ErrorCode::AsgInvalidTypeObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	return type;
}

