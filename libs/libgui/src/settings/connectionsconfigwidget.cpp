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

#include "connectionsconfigwidget.h"
#include "guiutilsns.h"
#include "utilsns.h"
#include "baseform.h"

const QString ConnectionsConfigWidget::DefaultFor {"default-for-%1"};
std::vector<Connection *> ConnectionsConfigWidget::connections;
std::map<QString, attribs_map> ConnectionsConfigWidget::config_params;

ConnectionsConfigWidget::ConnectionsConfigWidget(QWidget * parent) : BaseConfigWidget(parent)
{
	Ui_ConnectionsConfigWidget::setupUi(this);

	GuiUtilsNs::createPasswordShowAction(passwd_edt);
	GuiUtilsNs::configureWidgetsFont({ alias_lbl, conn_db_lbl, host_lbl,
																		user_lbl, passwd_lbl, timeout_lbl,
																		set_role_lbl, other_params_lbl, default_for_lbl,
																		ssl_mode_lbl, client_cert_lbl, client_key_lbl,
																		root_cert_lbl, crl_lbl, krb_server_lbl },
																	 GuiUtilsNs::SmallFontFactor, true);

	one_time_conn_edit = false;
	one_time_test_tb->setVisible(false);

	connect(ssl_mode_cmb, &QComboBox::currentIndexChanged, this, &ConnectionsConfigWidget::enableCertificates);

	connect(new_tb, &QToolButton::clicked, this, &ConnectionsConfigWidget::newConnection);
	connect(cancel_tb, &QToolButton::clicked, this, &ConnectionsConfigWidget::newConnection);

	connect(duplicate_tb, &QToolButton::clicked, this, __slot(this, ConnectionsConfigWidget::duplicateConnection));
	connect(update_tb, &QToolButton::clicked, this, __slot(this, ConnectionsConfigWidget::handleConnection));

	connect(edit_tb, &QToolButton::clicked, this, &ConnectionsConfigWidget::editConnection);
	connect(remove_tb, &QToolButton::clicked, this, &ConnectionsConfigWidget::removeConnection);

	connect(test_tb, &QPushButton::clicked, this, &ConnectionsConfigWidget::testConnection);
	connect(one_time_test_tb, &QPushButton::clicked, this, &ConnectionsConfigWidget::testConnection);
	connect(add_tb, &QPushButton::clicked, this, __slot(this, ConnectionsConfigWidget::handleConnection));

	connect(alias_edt, &QLineEdit::textChanged, this, &ConnectionsConfigWidget::enableConnectionTest);
	connect(host_edt, &QLineEdit::textChanged, this, &ConnectionsConfigWidget::enableConnectionTest);
	connect(user_edt, &QLineEdit::textChanged, this, &ConnectionsConfigWidget::enableConnectionTest);
	connect(passwd_edt, &QLineEdit::textChanged, this, &ConnectionsConfigWidget::enableConnectionTest);
	connect(conn_db_edt, &QLineEdit::textChanged, this, &ConnectionsConfigWidget::enableConnectionTest);

	update_tb->setVisible(false);
	cancel_tb->setVisible(false);
}

void ConnectionsConfigWidget::hideEvent(QHideEvent *event)
{
	if(!event->spontaneous())
	{
		newConnection();
		one_time_conn_edit = false;
		host_edt->setEnabled(true);
		port_sbp->setEnabled(true);
		conn_btns_grp->setVisible(true);
		add_tb->setVisible(true);
	}
}

void ConnectionsConfigWidget::showEvent(QShowEvent *event)
{
	if(!event->spontaneous())
	{
		updateConnectionsCombo();
		newConnection();
	}
}

void ConnectionsConfigWidget::updateConnectionsCombo()
{
	connections_cmb->clear();

	for(auto &conn : connections)
		connections_cmb->addItem(GuiUtilsNs::getIcon("server"), conn->getConnectionId());
}

void ConnectionsConfigWidget::destroyConnections()
{
	Connection *conn=nullptr;

	while(!connections.empty())
	{
		conn=connections.back();
		connections.pop_back();
		connections_cmb->removeItem(0);
		delete conn;
	}
}

std::map<QString, attribs_map> ConnectionsConfigWidget::getConfigurationParams()
{
	return config_params;
}

void ConnectionsConfigWidget::loadConfiguration()
{
	try
	{
		Connection *conn=nullptr;

		destroyConnections();

		BaseConfigWidget::loadConfiguration(GlobalAttributes::ConnectionsConf, config_params, { Attributes::Alias });

		for(auto &itr : config_params)
		{
			conn=new Connection;

			conn->setConnectionParam(Connection::ParamAlias, itr.second[Attributes::Alias]);
			conn->setConnectionParam(Connection::ParamServerFqdn, itr.second[Connection::ParamServerFqdn]);
			conn->setConnectionParam(Connection::ParamPort, itr.second[Connection::ParamPort]);
			conn->setConnectionParam(Connection::ParamUser, itr.second[Connection::ParamUser]);
			conn->setConnectionParam(Connection::ParamPassword,itr.second[Connection::ParamPassword]);
			conn->setConnectionParam(Connection::ParamDbName, itr.second[Connection::ParamDbName]);
			conn->setConnectionParam(Connection::ParamConnTimeout, itr.second[Attributes::ConnectionTimeout]);
			conn->setConnectionParam(Connection::ParamSslMode, itr.second[Connection::ParamSslMode]);
			conn->setConnectionParam(Connection::ParamSslRootCert, itr.second[Connection::ParamSslRootCert]);
			conn->setConnectionParam(Connection::ParamSslCert, itr.second[Connection::ParamSslCert]);
			conn->setConnectionParam(Connection::ParamSslKey, itr.second[Connection::ParamSslKey]);
			conn->setConnectionParam(Connection::ParamSslCrl, itr.second[Connection::ParamSslCrl]);
			conn->setConnectionParam(Connection::ParamLibGssapi, itr.second[Connection::ParamLibGssapi]);
			conn->setConnectionParam(Connection::ParamKerberosServer, itr.second[Connection::ParamKerberosServer]);
			conn->setConnectionParam(Connection::ParamOthers, itr.second[Connection::ParamOthers]);
			conn->setConnectionParam(Connection::ParamSetRole, itr.second[Connection::ParamSetRole]);

			conn->setAutoBrowseDB(itr.second[Attributes::AutoBrowseDb]==Attributes::True);
			conn->setDefaultForOperation(Connection::OpDiff, itr.second[DefaultFor.arg(Attributes::Diff)]==Attributes::True);
			conn->setDefaultForOperation(Connection::OpExport, itr.second[DefaultFor.arg(Attributes::Export)]==Attributes::True);
			conn->setDefaultForOperation(Connection::OpImport, itr.second[DefaultFor.arg(Attributes::Import)]==Attributes::True);
			conn->setDefaultForOperation(Connection::OpValidation, itr.second[DefaultFor.arg(Attributes::Validation)]==Attributes::True);

			connections.push_back(conn);
		}

		edit_tb->setEnabled(!connections.empty());
		remove_tb->setEnabled(!connections.empty());
		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, e.getExtraInfo());
	}
}

void ConnectionsConfigWidget::setOneTimeEditMode(bool one_time_edit, const QString &conn_alias, const QString &dbname, const QString &host, int port, const QString &username, const QString &password)
{
	one_time_conn_edit = one_time_edit;
	conn_btns_grp->setVisible(!one_time_edit);
	one_time_test_tb->setVisible(one_time_edit);
	add_tb->setVisible(!one_time_edit);
	host_edt->setDisabled(one_time_edit && !host.isEmpty());
	port_sbp->setDisabled(one_time_edit && port > 0);
	alias_edt->setText(conn_alias);
	conn_db_edt->setText(dbname);
	host_edt->setText(host);
	port_sbp->setValue(port);
	user_edt->setText(username);
	passwd_edt->setText(password);
}

void ConnectionsConfigWidget::enableCertificates()
{
	client_cert_lbl->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	client_cert_edt->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	root_cert_lbl->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	root_cert_edt->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	crl_edt->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	crl_lbl->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	client_key_lbl->setEnabled(ssl_mode_cmb->currentIndex()!=0);
	client_key_edt->setEnabled(ssl_mode_cmb->currentIndex()!=0);
}

void ConnectionsConfigWidget::enableConnectionTest()
{
	test_tb->setEnabled(!alias_edt->text().isEmpty() &&
											!host_edt->text().isEmpty() &&
											!user_edt->text().isEmpty() &&
											!conn_db_edt->text().isEmpty());

	one_time_test_tb->setEnabled(test_tb->isEnabled());

	add_tb->setEnabled(test_tb->isEnabled());
	update_tb->setEnabled(test_tb->isEnabled());

	if(!isConfigurationChanged() && test_tb->isEnabled())
		setConfigurationChanged(true);
}

void ConnectionsConfigWidget::newConnection()
{
	/* If we aren't in one time edit mode
	 * we clear the fields below, otherwise we avoid clear
	 * the fields and reset the flag one_time_conn_edit in the
	 * first call to this method */
	if(!one_time_conn_edit)
	{
		conn_db_edt->clear();
		alias_edt->clear();
		user_edt->clear();
		host_edt->clear();
		port_sbp->setValue(5432);
		passwd_edt->clear();
		passwd_edt->clearFocus();
	}

	other_params_edt->clear();
	set_role_edt->clear();

	auto_browse_chk->setChecked(false);
	diff_chk->setChecked(false);
	export_chk->setChecked(false);
	import_chk->setChecked(false);
	validation_chk->setChecked(false);

	ssl_mode_cmb->setCurrentIndex(0);
	client_cert_edt->setText("~/.postgresql/postgresql.crt");
	root_cert_edt->setText("~/.postgresql/root.crt");
	crl_edt->setText("~/.postgresql/root.crl");
	client_key_edt->setText("~/.postgresql/postgresql.key");

	gssapi_auth_chk->setChecked(false);
	krb_server_edt->clear();

	timeout_sbp->setValue(2);
	add_tb->setVisible(!one_time_conn_edit);
	update_tb->setVisible(false);
	connections_cmb->setEnabled(true);

	new_tb->setVisible(true);
	cancel_tb->setVisible(false);

	edit_tb->setEnabled(connections_cmb->count() > 0);
	remove_tb->setEnabled(connections_cmb->count() > 0);
	duplicate_tb->setEnabled(connections_cmb->count() > 0);
}

void ConnectionsConfigWidget::duplicateConnection()
{
	Connection *conn=nullptr, *new_conn=nullptr;

	try
	{
		conn=connections.at(connections_cmb->currentIndex());
		new_conn=new Connection;
		(*new_conn)=(*conn);
		connections.push_back(new_conn);

		new_conn->setConnectionParam(Connection::ParamAlias, QString("cp_%1").arg(conn->getConnectionParam(Connection::ParamAlias)));
		connections_cmb->addItem(GuiUtilsNs::getIcon("server"), new_conn->getConnectionId());
		connections_cmb->setCurrentIndex(connections_cmb->count()-1);
		setConfigurationChanged(true);
	}
	catch(Exception &e)
	{
		delete new_conn;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ConnectionsConfigWidget::handleConnection()
{
	Connection *conn=nullptr;

	try
	{
		if(!update_tb->isVisible())
		{
			conn=new Connection;
			this->configureConnection(conn, false);
			connections_cmb->addItem(GuiUtilsNs::getIcon("server"), conn->getConnectionId());
			connections.push_back(conn);
		}
		else
		{
			conn=connections.at(connections_cmb->currentIndex());
			this->configureConnection(conn, true);
			connections_cmb->setItemText(connections_cmb->currentIndex(), conn->getConnectionId());
		}

		this->newConnection();
		edit_tb->setEnabled(connections_cmb->count() > 0);
		remove_tb->setEnabled(connections_cmb->count() > 0);
		setConfigurationChanged(true);
	}
	catch(Exception &e)
	{
		if(add_tb->isVisible())
			delete conn;

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ConnectionsConfigWidget::removeConnection()
{
	if(connections_cmb->currentIndex() >= 0)
	{
		Connection *conn=nullptr;
		conn=connections.at(connections_cmb->currentIndex());
		connections.erase(connections.begin() + connections_cmb->currentIndex());
		connections_cmb->removeItem(connections_cmb->currentIndex());
		delete conn;
		this->newConnection();
		setConfigurationChanged(true);
	}
}

void ConnectionsConfigWidget::editConnection()
{
	if(connections_cmb->count() > 0)
	{
		Connection *conn=nullptr;

		conn=connections.at(connections_cmb->currentIndex());
		alias_edt->setText(conn->getConnectionParam(Connection::ParamAlias));
		auto_browse_chk->setChecked(conn->isAutoBrowseDB());

		diff_chk->setChecked(conn->isDefaultForOperation(Connection::OpDiff));
		export_chk->setChecked(conn->isDefaultForOperation(Connection::OpExport));
		import_chk->setChecked(conn->isDefaultForOperation(Connection::OpImport));
		validation_chk->setChecked(conn->isDefaultForOperation(Connection::OpValidation));

		if(!conn->getConnectionParam(Connection::ParamServerFqdn).isEmpty())
			host_edt->setText(conn->getConnectionParam(Connection::ParamServerFqdn));
		else
			host_edt->setText(conn->getConnectionParam(Connection::ParamServerIp));

		conn_db_edt->setText(conn->getConnectionParam(Connection::ParamDbName));
		user_edt->setText(conn->getConnectionParam(Connection::ParamUser));
		passwd_edt->setText(conn->getConnectionParam(Connection::ParamPassword));
		port_sbp->setValue(conn->getConnectionParam(Connection::ParamPort).toInt());
		timeout_sbp->setValue(conn->getConnectionParam(Connection::ParamConnTimeout).toInt());

		krb_server_edt->setText(conn->getConnectionParam(Connection::ParamKerberosServer));
		gssapi_auth_chk->setChecked(conn->getConnectionParam(Connection::ParamLibGssapi)=="gssapi");
		other_params_edt->setText(conn->getConnectionParam(Connection::ParamOthers));
		set_role_edt->setText(conn->getConnectionParam(Connection::ParamSetRole));

		if(conn->getConnectionParam(Connection::ParamSslMode)==Connection::SslDisable)
			ssl_mode_cmb->setCurrentIndex(0);
		else if(conn->getConnectionParam(Connection::ParamSslMode)==Connection::SslAllow)
			ssl_mode_cmb->setCurrentIndex(1);
		else if(conn->getConnectionParam(Connection::ParamSslMode)==Connection::SslRequire)
			ssl_mode_cmb->setCurrentIndex(2);
		else if(conn->getConnectionParam(Connection::ParamSslMode)==Connection::SslCaVerify)
			ssl_mode_cmb->setCurrentIndex(3);
		else
			ssl_mode_cmb->setCurrentIndex(4);

		if(ssl_mode_cmb->currentIndex() > 0)
		{
			client_cert_edt->setText(conn->getConnectionParam(Connection::ParamSslCert));
			root_cert_edt->setText(conn->getConnectionParam(Connection::ParamSslRootCert));
			client_key_edt->setText(conn->getConnectionParam(Connection::ParamSslKey));
			crl_edt->setText(conn->getConnectionParam(Connection::ParamSslCrl));
		}

		update_tb->setVisible(true);
		add_tb->setVisible(false);
		connections_cmb->setEnabled(false);

		new_tb->setVisible(false);
		duplicate_tb->setEnabled(false);
		cancel_tb->setVisible(true);
		edit_tb->setEnabled(false);
	}
}

void ConnectionsConfigWidget::configureConnection(Connection *conn, bool is_update)
{
	if(conn)
	{
		conn->setAutoBrowseDB(auto_browse_chk->isChecked());

		// Avoiding add duplicated aliases in the combo
		QString alias = alias_edt->text();
		int idx = 0, conn_idx = connections_cmb->findText(alias, Qt::MatchStartsWith);

		while(conn_idx >= 0 &&
					(!is_update || (is_update && conn_idx != connections_cmb->currentIndex())))
		{
			alias = alias_edt->text() + QString::number(++idx);
			conn_idx = connections_cmb->findText(alias, Qt::MatchStartsWith);
		}

		conn->setConnectionParam(Connection::ParamAlias, alias);
		conn->setConnectionParam(Connection::ParamServerIp, "");
		conn->setConnectionParam(Connection::ParamServerFqdn, host_edt->text());
		conn->setConnectionParam(Connection::ParamPort, QString("%1").arg(port_sbp->value()));
		conn->setConnectionParam(Connection::ParamUser, user_edt->text());
		conn->setConnectionParam(Connection::ParamPassword, passwd_edt->text());
		conn->setConnectionParam(Connection::ParamDbName, conn_db_edt->text());
		conn->setConnectionParam(Connection::ParamConnTimeout, QString("%1").arg(timeout_sbp->value()));

		conn->setDefaultForOperation(Connection::OpDiff, diff_chk->isChecked());
		conn->setDefaultForOperation(Connection::OpExport, export_chk->isChecked());
		conn->setDefaultForOperation(Connection::OpImport, import_chk->isChecked());
		conn->setDefaultForOperation(Connection::OpValidation, validation_chk->isChecked());

		switch(ssl_mode_cmb->currentIndex())
		{
			case 1:
				conn->setConnectionParam(Connection::ParamSslMode, Connection::SslAllow);
			break;
			case 2:
				conn->setConnectionParam(Connection::ParamSslMode, Connection::SslRequire);
			break;
			case 3:
				conn->setConnectionParam(Connection::ParamSslMode, Connection::SslCaVerify);
			break;
			case 4:
				conn->setConnectionParam(Connection::ParamSslMode, Connection::SslFullVerify);
			break;
			default:
			case 0:
				conn->setConnectionParam(Connection::ParamSslMode, Connection::SslDisable);
			break;
		}

		if(ssl_mode_cmb->currentIndex()!=0)
		{
			conn->setConnectionParam(Connection::ParamSslRootCert, root_cert_edt->text());
			conn->setConnectionParam(Connection::ParamSslCert, client_cert_edt->text());
			conn->setConnectionParam(Connection::ParamSslKey, client_key_edt->text());
			conn->setConnectionParam(Connection::ParamSslCrl, crl_edt->text());
		}

		if(gssapi_auth_chk->isChecked())
			conn->setConnectionParam(Connection::ParamLibGssapi, "gssapi");

		conn->setConnectionParam(Connection::ParamKerberosServer, krb_server_edt->text());
		conn->setConnectionParam(Connection::ParamOthers, other_params_edt->text());
		conn->setConnectionParam(Connection::ParamSetRole, set_role_edt->text());
	}
}

void ConnectionsConfigWidget::testConnection()
{
	Connection conn;
	attribs_map srv_info;

	try
	{
		this->configureConnection(&conn, false);
		conn.connect();
		srv_info = conn.getServerInfo();

		Messagebox::success(UtilsNs::formatMessage(tr("Connection successfully established!\n\nServer details:\n\nPID: `%1'\nProtocol: `%2'\nVersion: `%3'"))
										 .arg(srv_info[Connection::ServerPid])
										 .arg(srv_info[Connection::ServerProtocol])
										 .arg(srv_info[Connection::ServerVersion]));
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ConnectionsConfigWidget::restoreDefaults()
{
	try
	{
		//Restore the default connection config file
		BaseConfigWidget::restoreDefaults(GlobalAttributes::ConnectionsConf, false);

		//Remove all connections
		while(connections_cmb->count() > 0)
			removeConnection();

		//Reloads the configuration
		loadConfiguration();

		updateConnectionsCombo();
		setConfigurationChanged(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ConnectionsConfigWidget::saveConfiguration()
{
	try
	{
		attribs_map attribs;

		/* If add or update buttons are enabled when saving the configs indicates
	   that user forgot to click on these buttons and register the connection,
	   so in order to do not lost the data pgModeler will ask to save the connection. */
		if(add_tb->isEnabled() || update_tb->isEnabled())
		{
			if(!one_time_conn_edit)
			{
				int res = Messagebox::confirm(tr("There is a connection being configured! Do you want to save it before applying settings?"));

				if(Messagebox::isAccepted(res))
					handleConnection();
			}
			else
			{
				handleConnection();
			}
		}

		config_params[GlobalAttributes::ConnectionsConf].clear();

		/* Workaround: When there is no connection, to prevent saving an empty file, is necessary to
		 fill the attribute CONNECTIONS with white spaces */
		if(connections.empty())
			config_params[GlobalAttributes::ConnectionsConf][Attributes::Connections]="  ";
		else
		{
			for(Connection *conn : connections)
			{
				attribs=conn->getConnectionParams();

				if(attribs[Connection::ParamServerFqdn].isEmpty())
					attribs[Connection::ParamServerFqdn]=attribs[Connection::ParamServerIp];

				attribs[Attributes::Alias]=attribs[Connection::ParamAlias];
				attribs[Attributes::AutoBrowseDb]=(conn->isAutoBrowseDB() ? Attributes::True : "");
				attribs[Attributes::ConnectionTimeout]=attribs[Connection::ParamConnTimeout];

				attribs[DefaultFor.arg(Attributes::Export)]=(conn->isDefaultForOperation(Connection::OpExport) ? Attributes::True : "");
				attribs[DefaultFor.arg(Attributes::Import)]=(conn->isDefaultForOperation(Connection::OpImport) ? Attributes::True : "");
				attribs[DefaultFor.arg(Attributes::Diff)]=(conn->isDefaultForOperation(Connection::OpDiff) ? Attributes::True : "");
				attribs[DefaultFor.arg(Attributes::Validation)]=(conn->isDefaultForOperation(Connection::OpValidation) ? Attributes::True : "");

				schparser.ignoreUnkownAttributes(true);
				config_params[GlobalAttributes::ConnectionsConf][Attributes::Connections]+=
						schparser.getSourceCode(GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir,
																																											 GlobalAttributes::ConnectionsConf +
																																											 GlobalAttributes::SchemaExt), attribs);

				schparser.ignoreUnkownAttributes(false);
			}
		}

		schparser.ignoreUnkownAttributes(true);
		BaseConfigWidget::saveConfiguration(GlobalAttributes::ConnectionsConf, config_params);
		schparser.ignoreUnkownAttributes(false);
		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ConnectionsConfigWidget::getConnections(std::map<QString, Connection *> &conns, bool inc_hosts)
{
	QString alias;

	conns.clear();
	for(Connection *conn : connections)
	{
		alias=conn->getConnectionId();

		if(!inc_hosts)
			alias.remove(QRegularExpression(" \\((.)*\\)"));

		conns[alias]=conn;
	}
}

Connection *ConnectionsConfigWidget::getConnection(const QString &conn_id)
{
	for(Connection *conn : connections)
	{
		if(conn->getConnectionId() == conn_id)
			return conn;
	}

	return nullptr;
}

void ConnectionsConfigWidget::fillConnectionsComboBox(QComboBox *combo, bool incl_placeholder, Connection::ConnOperation check_def_for)
{
	std::map<QString, Connection *> connections;
	Connection *def_conn=nullptr;

	if(!combo)
		throw Exception(ErrorCode::OprNotAllocatedObject ,PGM_FUNC,PGM_FILE,PGM_LINE);

	getConnections(connections);

	combo->blockSignals(true);
	combo->clear();

	if(incl_placeholder)
	{
		if(!connections.empty())
			combo->addItem(tr("Found %1 connection(s)").arg(connections.size()));
		else
			combo->addItem(tr("No connections found"));
	}

	for(auto &itr : connections)
	{
		combo->addItem(GuiUtilsNs::getIcon("server"), itr.first, QVariant::fromValue<void *>(itr.second));

		if(!def_conn && itr.second->isDefaultForOperation(check_def_for))
			def_conn=itr.second;
	}

	if(incl_placeholder)
		combo->addItem(GuiUtilsNs::getIcon("connection"), tr("Edit connections"));

	if(def_conn)
		combo->setCurrentText(def_conn->getConnectionId());

	combo->blockSignals(false);
}

bool ConnectionsConfigWidget::openConnectionsConfiguration(bool one_time_edit,
																													 const QString &conn_alias, const QString &dbname, const QString &host,
																													 int port, const QString &username, const QString &password)
{
	BaseForm parent_form;
	ConnectionsConfigWidget conn_cfg_wgt;
	bool conns_changed = false;

	connect(parent_form.cancel_btn, &QPushButton::clicked, &parent_form, [&conn_cfg_wgt, &conns_changed]() {
		__trycatch(
			if(conn_cfg_wgt.isConfigurationChanged())
			{
				// Restore the original connection file if the user clicked "Cancel" after changing connections
				conn_cfg_wgt.loadConfiguration();
				conns_changed = true;
			}
		)
	});

	connect(parent_form.apply_ok_btn, &QPushButton::clicked, &parent_form, [&conn_cfg_wgt, &parent_form, &conns_changed](){
		__trycatch(
			conn_cfg_wgt.saveConfiguration();
			parent_form.accept();
			conns_changed = true;
		)
	});

	conn_cfg_wgt.setOneTimeEditMode(one_time_edit, conn_alias, dbname, host, port, username, password);
	
	/* Workaround to hide the border of the bg_frame when running
	 * ConnectionsConfigWidget in standalone mode */
	conn_cfg_wgt.bg_frame->setStyleSheet("QFrame { border: none; }");

	parent_form.setWindowTitle(tr("Edit database connections"));
	parent_form.setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
	parent_form.setMainWidget(&conn_cfg_wgt);
	parent_form.setButtonConfiguration(Messagebox::OkCancelButtons);
	parent_form.adjustMinimumSize();
	parent_form.exec();

	return parent_form.result() == QDialog::Accepted || conns_changed;
}

bool ConnectionsConfigWidget::openConnectionsConfiguration(QComboBox *combo, bool incl_placeholder)
{
	if(!combo)
		return false;

	bool conns_changed = openConnectionsConfiguration();

	if(conns_changed)
		fillConnectionsComboBox(combo, incl_placeholder);
	else
		combo->setCurrentIndex(0);

	return conns_changed;
}

Connection *ConnectionsConfigWidget::getDefaultConnection(Connection::ConnOperation operation)
{
	Connection *conn=nullptr;

	for(Connection *aux_conn : connections)
	{
		if(aux_conn->isDefaultForOperation(operation))
		{
			conn=aux_conn;
			break;
		}
	}

	return conn;
}
