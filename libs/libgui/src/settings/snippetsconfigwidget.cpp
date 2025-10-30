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

#include "snippetsconfigwidget.h"
#include "baseobject.h"
#include "messagebox.h"
#include "guiutilsns.h"
#include <QMenu>

std::map<QString, attribs_map> SnippetsConfigWidget::config_params;

const QRegularExpression SnippetsConfigWidget::IdFormatRegExp {
	QRegularExpression::anchoredPattern("^([a-z])([a-z]*|(\\d)*|(_)*)+"),
	QRegularExpression::CaseInsensitiveOption
};

SnippetsConfigWidget::SnippetsConfigWidget(QWidget * parent) : BaseConfigWidget(parent)
{
	QPixmap ico;
	QString gen_purpose=tr("General purpose");
	std::map<QString, ObjectType> types_map;
	std::vector<ObjectType> types=BaseObject::getObjectTypes(true, {ObjectType::Relationship, ObjectType::Tag, ObjectType::Textbox,
																														 ObjectType::Permission, ObjectType::BaseRelationship });

	setupUi(this);

	for(ObjectType type : types)
		types_map[BaseObject::getTypeName(type)]=type;

	//Creates a combo with the accepted object type
	for(auto &itr : types_map)
	{
		ico.load(GuiUtilsNs::getIconPath(itr.second));
		applies_to_cmb->addItem(ico, itr.first, enum_t(itr.second));
		filter_cmb->addItem(ico, itr.first, enum_t(itr.second));
	}

	applies_to_cmb->insertItem(0, gen_purpose, enum_t(ObjectType::BaseObject));
	applies_to_cmb->setCurrentIndex(0);

	filter_cmb->insertItem(0, gen_purpose, enum_t(ObjectType::BaseObject));
	filter_cmb->insertItem(0, tr("All snippets"));
	filter_cmb->setCurrentIndex(0);

	snippet_txt=GuiUtilsNs::createNumberedTextEditor(snippet_wgt, true);

	snippet_hl=new SyntaxHighlighter(snippet_txt);
	snippet_hl->loadConfiguration(GlobalAttributes::getSchHighlightConfPath());

	enableEditMode(false);

	connect(new_tb, &QToolButton::clicked, this, &SnippetsConfigWidget::resetForm);
	connect(edit_tb, &QToolButton::clicked, this, &SnippetsConfigWidget::editSnippet);
	connect(remove_tb, &QToolButton::clicked, this, &SnippetsConfigWidget::removeSnippet);
	connect(remove_all_tb, &QToolButton::clicked, this, &SnippetsConfigWidget::removeAllSnippets);

	connect(cancel_tb, &QToolButton::clicked, this, [this](){
		enableEditMode(false);
	});

	connect(snippets_cmb, &QComboBox::currentTextChanged, this, [this](){
		enableEditMode(false);
	});

	connect(id_edt, &QLineEdit::textChanged, this, &SnippetsConfigWidget::enableSaveButtons);
	connect(label_edt, &QLineEdit::textChanged, this, &SnippetsConfigWidget::enableSaveButtons);
	connect(snippet_txt, &NumberedTextEditor::textChanged, this, &SnippetsConfigWidget::enableSaveButtons);
	connect(parsable_chk, &QCheckBox::toggled, this, &SnippetsConfigWidget::enableSaveButtons);
	connect(filter_cmb, &QComboBox::currentIndexChanged, this, &SnippetsConfigWidget::filterSnippets);
	connect(update_tb, &QPushButton::clicked, this, &SnippetsConfigWidget::handleSnippet);
	connect(add_tb, &QPushButton::clicked, this, &SnippetsConfigWidget::handleSnippet);
	connect(parse_tb, &QPushButton::clicked, this, qOverload<>(&SnippetsConfigWidget::parseSnippet));

	connect(parsable_chk, &QCheckBox::toggled, placeholders_chk, &QCheckBox::setEnabled);
}

std::map<QString, attribs_map> SnippetsConfigWidget::getConfigurationParams()
{
	return config_params;
}

attribs_map SnippetsConfigWidget::getSnippetById(const QString &snip_id)
{
	if(config_params.count(snip_id))
		return config_params[snip_id];

	return {};
}

QStringList SnippetsConfigWidget::getSnippetsIdsByObject(ObjectType obj_type)
{
	QStringList ids;
	QString type_name=(obj_type==ObjectType::BaseObject ?
						   Attributes::General : BaseObject::getSchemaName(obj_type));

	for(auto &snip : config_params)
	{
		if(snip.second[Attributes::Object]==type_name)
			ids.push_back(snip.second[Attributes::Id]);
	}

	return ids;
}

std::vector<attribs_map> SnippetsConfigWidget::getSnippetsByObject(ObjectType obj_type)
{
	std::vector<attribs_map> snippets;
	QString type_name=(obj_type==ObjectType::BaseObject ?
						   Attributes::General : BaseObject::getSchemaName(obj_type));

	for(auto &snip : config_params)
	{
		if(snip.second[Attributes::Object]==type_name)
			snippets.push_back(snip.second);
	}

	return snippets;
}

QStringList SnippetsConfigWidget::getAllSnippetsAttribute(const QString &attrib)
{
	QStringList attribs;

	for(auto &snip : config_params)
	{
		if(snip.second.count(attrib))
			attribs.push_back(snip.second[attrib]);
	}

	return attribs;
}

std::vector<attribs_map> SnippetsConfigWidget::getAllSnippets()
{
	std::vector<attribs_map> snippets;

	for(auto &snip : config_params)
		snippets.push_back(snip.second);

	return snippets;
}

QString SnippetsConfigWidget::parseSnippet(attribs_map snippet, attribs_map attribs)
{ 
	SchemaParser schparser;
	QStringList aux_attribs;
	QString buf=snippet[Attributes::Contents];

	if(snippet[Attributes::Parsable]!=Attributes::True)
		return buf;

	try
	{
		schparser.loadBuffer(buf);

		//Assigning dummy values for empty attributes
		if(snippet[Attributes::Placeholders]==Attributes::True)
		{
			aux_attribs=schparser.extractAttributes();
			for(QString attr : aux_attribs)
			{
				if(attribs.count(attr)==0 ||
						(attribs.count(attr) && attribs[attr].isEmpty()))
					attribs[attr]=QString("{%1}").arg(attr);
			}
		}

		schparser.ignoreEmptyAttributes(true);
		schparser.ignoreUnkownAttributes(true);
		return schparser.getSourceCode(attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QString SnippetsConfigWidget::getParsedSnippet(const QString &snip_id, attribs_map attribs)
{
	if(config_params.count(snip_id))
	{
		try
		{
			return parseSnippet(config_params[snip_id], attribs);
		}
		catch(Exception &e)
		{
			return tr("/* Error parsing the snippet '%1':\n\n %2 */").arg(snip_id, e.getErrorMessage());
		}
	}

	return "";
}

void SnippetsConfigWidget::fillSnippetsCombo(std::map<QString, attribs_map> &config)
{
	snippets_cmb->clear();

	for(auto &cfg : config)
		snippets_cmb->addItem(QString("[%1] %2").arg(cfg.first, cfg.second.at(Attributes::Label)), cfg.first);
}

bool SnippetsConfigWidget::isSnippetValid(attribs_map &attribs, const QString &orig_id)
{
	QString snip_id=attribs.at(Attributes::Id),
			err_msg;

	if(!orig_id.isEmpty() && snip_id!=orig_id && config_params.count(snip_id)!=0)
		err_msg=tr("Duplicated snippet id <strong>%1</strong> detected. Please, specify a different one!").arg(snip_id);
	else if(!IdFormatRegExp.match(snip_id).hasMatch())
		err_msg=tr("Invalid ID pattern detected <strong>%1</strong>. This one must start with at leat one letter and be composed by letters, numbers and/or underscore!").arg(snip_id);
	else if(attribs[Attributes::Label].isEmpty())
		err_msg=tr("Empty label for snippet <strong>%1</strong>. Please, specify a value for it!").arg(snip_id);
	else if(attribs[Attributes::Contents].isEmpty())
		err_msg=tr("Empty code for snippet <strong>%1</strong>. Please, specify a value for it!").arg(snip_id);
	else if(attribs[Attributes::Parsable]==Attributes::True)
	{
		try
		{
			QString buf=snippet_txt->toPlainText();
			attribs_map attribs;
			SchemaParser schparser;

			schparser.loadBuffer(buf);
			schparser.ignoreEmptyAttributes(true);
			schparser.ignoreUnkownAttributes(true);
			schparser.getSourceCode(attribs);
		}
		catch(Exception &e)
		{
			err_msg=tr("The dynamic snippet contains syntax error(s). Additional info: <br/><em>%1</em>").arg(e.getErrorMessage());
		}
	}

	if(!err_msg.isEmpty())
	{
		Messagebox::error(err_msg);
		return false;
	}

	return true;
}

void SnippetsConfigWidget::hideEvent(QHideEvent *)
{
	enableEditMode(false);
}

void SnippetsConfigWidget::loadConfiguration()
{
	try
	{
		QStringList inv_snippets;

		this->resetForm();
		BaseConfigWidget::loadConfiguration(GlobalAttributes::SnippetsConf, config_params, { Attributes::Id });

		//Check if there are invalid snippets loaded
		for(auto &snip : config_params)
		{
			if(!isSnippetValid(snip.second,""))
				inv_snippets.push_back(snip.first);
		}

		//Destroy any invalid snippets
		for(auto &id : inv_snippets)
			config_params.erase(id);

		fillSnippetsCombo(config_params);
		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, e.getExtraInfo());
	}
}

void SnippetsConfigWidget::resetForm()
{
	snippet_txt->clear();
	id_edt->clear();
	label_edt->clear();
	applies_to_cmb->setCurrentIndex(0);
	parsable_chk->setChecked(false);
	placeholders_chk->setChecked(false);
}

attribs_map SnippetsConfigWidget::getSnippetAttributes()
{
	QString object_id=BaseObject::getSchemaName(static_cast<ObjectType>(applies_to_cmb->currentData().toUInt()));

	if(object_id.isEmpty())
		object_id=Attributes::General;

	return attribs_map{ {Attributes::Id, id_edt->text()},
						{Attributes::Label, label_edt->text()},
						{Attributes::Object, object_id},
						{Attributes::Parsable, (parsable_chk->isChecked() ? Attributes::True : Attributes::False)},
						{Attributes::Placeholders, (parsable_chk->isChecked() && placeholders_chk->isChecked() ?
						 Attributes::True : Attributes::False)},
						{Attributes::Contents, snippet_txt->toPlainText()} };
}

void SnippetsConfigWidget::editSnippet()
{
	QString snip_id=snippets_cmb->currentData().toString();
	ObjectType obj_type=BaseObject::getObjectType(config_params[snip_id].at(Attributes::Object));

	enableEditMode(true);
	snippet_txt->setPlainText(config_params[snip_id].at(Attributes::Contents));
	id_edt->setText(snip_id);
	label_edt->setText(config_params[snip_id].at(Attributes::Label));
	parsable_chk->setChecked(config_params[snip_id].at(Attributes::Parsable)==Attributes::True);
	placeholders_chk->setChecked(config_params[snip_id].at(Attributes::Placeholders)==Attributes::True);
	applies_to_cmb->setCurrentText(BaseObject::getTypeName(obj_type));
}

void SnippetsConfigWidget::handleSnippet()
{
	QString orig_id=snippets_cmb->currentData().toString();
	attribs_map snippet;

	snippet=getSnippetAttributes();

	if(isSnippetValid(snippet, orig_id))
	{
		config_params[id_edt->text()]=snippet;

		//If the operation is update and the snippet id changed, remove the original one
		if(sender()==update_tb && id_edt->text() != orig_id)
			config_params.erase(orig_id);

		filterSnippets(filter_cmb->currentIndex());
		resetForm();
		setConfigurationChanged(true);
	}
}

void SnippetsConfigWidget::removeSnippet()
{
	config_params.erase(snippets_cmb->currentData().toString());
	filterSnippets(filter_cmb->currentIndex());
	setConfigurationChanged(true);
}

void SnippetsConfigWidget::removeAllSnippets()
{
	int res = Messagebox::confirm(tr("Do you really want to remove all snippets?"));

	if(Messagebox::isAccepted(res))
	{
		config_params.clear();
		filterSnippets(0);
		setConfigurationChanged(true);
	}
}

void SnippetsConfigWidget::enableEditMode(bool enable)
{
	bool has_snippets=(snippets_cmb->count() > 0);

	cancel_tb->setVisible(enable);
	new_tb->setVisible(!enable);
	snippets_cmb->setEnabled(!enable);
	filter_cmb->setEnabled(!enable);
	add_tb->setVisible(!enable);
	update_tb->setVisible(enable);

	edit_tb->setEnabled(!enable && has_snippets);
	remove_tb->setEnabled(!enable && has_snippets);
	remove_all_tb->setEnabled(!enable && has_snippets);

	if(!enable) resetForm();
}

void SnippetsConfigWidget::enableSaveButtons()
{
	bool enable=(!id_edt->text().isEmpty() &&
				 !label_edt->text().isEmpty() &&
				 !snippet_txt->toPlainText().isEmpty());

	add_tb->setEnabled(enable);
	update_tb->setEnabled(enable);
	parse_tb->setEnabled(enable && parsable_chk->isChecked());
}

void SnippetsConfigWidget::filterSnippets(int idx)
{
	if(idx <= 0)
		fillSnippetsCombo(config_params);
	else
	{
		ObjectType obj_type=static_cast<ObjectType>(filter_cmb->currentData().toUInt());
		std::map<QString, attribs_map> flt_snippets;
		QString object_id=BaseObject::getSchemaName(obj_type);

		if(object_id.isEmpty())
			object_id=Attributes::General;

		for(auto &cfg : config_params)
		{
			if(cfg.second.at(Attributes::Object)==object_id)
				flt_snippets[cfg.first]=cfg.second;
		}

		fillSnippetsCombo(flt_snippets);
	}
}

void SnippetsConfigWidget::parseSnippet()
{
	try
	{
		parseSnippet(getSnippetAttributes(), attribs_map());
		Messagebox::info(tr("No syntax errors found in the code snippet."));
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SnippetsConfigWidget::saveConfiguration()
{ 
	try
	{
		QString snippet_sch = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir,
																																				 Attributes::Snippet + GlobalAttributes::SchemaExt);

		attribs_map attribs;
		ObjectType obj_type;
		std::vector<attribs_map> snippets;

		//Saving the snippets sorting them by object type in the output file
		for(int i=0; i < applies_to_cmb->count(); i++)
		{
			obj_type=static_cast<ObjectType>(applies_to_cmb->itemData(i).toUInt());
			snippets=getSnippetsByObject(obj_type);

			for(auto &snip : snippets)
				attribs[Attributes::Snippet] +=	schparser.getSourceCode(snippet_sch, snip);
		}

		config_params[GlobalAttributes::SnippetsConf]=attribs;
		BaseConfigWidget::saveConfiguration(GlobalAttributes::SnippetsConf, config_params);
		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void SnippetsConfigWidget::restoreDefaults()
{
	try
	{
		BaseConfigWidget::restoreDefaults(GlobalAttributes::SnippetsConf, false);
		this->loadConfiguration();
		setConfigurationChanged(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void SnippetsConfigWidget::configureSnippetsMenu(QMenu *snip_menu, std::vector<ObjectType> types)
{
	std::vector<attribs_map> snippets, vet_aux;
	QAction *act=nullptr;
	QMenu *menu=nullptr;
	std::map<QString, QMenu *> submenus;
	QString object, snip_id, type_name;
	QPixmap ico;

	//Retrieve all snippets if the 'types' is empty
	if(types.empty())
		snippets=SnippetsConfigWidget::getAllSnippets();
	else
	{
		//Retreving only the snippets of the type specified by user
		for(ObjectType type : types)
		{
			vet_aux=SnippetsConfigWidget::getSnippetsByObject(type);
			snippets.insert(snippets.end(), vet_aux.begin(), vet_aux.end());
		}
	}

	snip_menu->clear();

	for(attribs_map snip : snippets)
	{
		object=snip[Attributes::Object];
		snip_id=snip[Attributes::Id];

		//Creating the snippet submenu for the current object type
		if(submenus.count(object)==0)
		{
			type_name=BaseObject::getTypeName(object);

			if(type_name.isEmpty())
			{
				ico=QPixmap();
				type_name=tr("General");
			}
			else
				ico=QPixmap(GuiUtilsNs::getIconPath(object));

			menu=new QMenu(type_name, snip_menu);
			menu->setIcon(ico);
			menu->setToolTipsVisible(true);
			submenus[object]=menu;

			/* If the current group (object) is general does not include the submenu yet.
		 This will be included as the last submenu */
			if(object!=Attributes::General)
				snip_menu->addMenu(menu);
		}

		//Creating the action for the current snippet
		act=new QAction(QPixmap(GuiUtilsNs::getIconPath("codesnippet")), snip_id, submenus[object]);
		act->setToolTip(snip[Attributes::Label]);
		submenus[object]->addAction(act);
	}

	//Include the "general" submenu at the end of snippet menu
	if(submenus.count(Attributes::General)!=0)
	{
		if(snip_menu->isEmpty())
		  snip_menu->addMenu(submenus[Attributes::General]);
		else
		{
		  //Inserting the "general" submenu at the top of snippets actions
		  snip_menu->insertMenu(snip_menu->actions().at(0), submenus[Attributes::General]);
		  snip_menu->insertSeparator(snip_menu->actions().at(1));
		}
	}
}

bool SnippetsConfigWidget::isSnippetExists(const QString &snip_id)
{
	return (config_params.count(snip_id)!=0);
}
