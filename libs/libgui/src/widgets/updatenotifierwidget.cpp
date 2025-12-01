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

#include "updatenotifierwidget.h"
#include "guiutilsns.h"
#include "messagebox.h"
#include <QDesktopServices>

UpdateNotifierWidget::UpdateNotifierWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);

	show_no_upd_msg = false;
	update_chk_reply = nullptr;
	frame->installEventFilter(this);

	GuiUtilsNs::createDropShadow(this, 5, 5, 30);
	GuiUtilsNs::configureWidgetsFont({ ver_num_lbl, ver_date_lbl }, GuiUtilsNs::BigFontFactor);

	connect(&update_chk_manager, &QNetworkAccessManager::finished, this, &UpdateNotifierWidget::handleUpdateChecked);

	connect(get_source_btn, &QPushButton::clicked, this, [this](){
		activateLink(GlobalAttributes::PgModelerSourceURL);
	});

	connect(get_binary_btn, &QPushButton::clicked, this, [this](){
		activateLink(GlobalAttributes::PgModelerDownloadURL);
	});

	connect(blog_post_btn, &QPushButton::clicked, this, [this](){
		activateLink(blogpost);
	});

	connect(hide_tb, &QToolButton::clicked, this,	[this](){
		hide();
		emit s_hideRequested();
	});

	this->adjustSize();
}

void UpdateNotifierWidget::setCheckVersions(const QString &chk_versions)
{
	check_versions = chk_versions;
}

bool UpdateNotifierWidget::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == frame && event->type() == QEvent::MouseMove)
	{
		static GuiUtilsNs::WidgetCornerId corner_id;
		QMouseEvent *m_event = dynamic_cast<QMouseEvent *>(event);

		if(m_event->buttons() == Qt::NoButton)
		{
			corner_id = GuiUtilsNs::getWidgetHoveredCorner(this, frame, m_event,
																										 GuiUtilsNs::WidgetCornerId::BottomRightCorner |
																										 GuiUtilsNs::WidgetCornerId::BottomLeftCorner);
		}

		GuiUtilsNs::resizeFloatingWidget(this, m_event, corner_id);
	}

	return QWidget::eventFilter(obj, event);
}

void UpdateNotifierWidget::activateLink(const QString &link)
{
	QDesktopServices::openUrl(QUrl(link));
	hide();
	emit s_hideRequested();
}

void UpdateNotifierWidget::checkForUpdate()
{
	QUrl url(GlobalAttributes::PgModelerUpdateCheckURL.arg(GlobalAttributes::PgModelerVersion, check_versions));
	QNetworkRequest req(url);

	req.setRawHeader("User-Agent", "pgModelerUpdateCheck");
	show_no_upd_msg=(dynamic_cast<QAction *>(sender())!=nullptr);
	update_chk_reply=update_chk_manager.get(req);
}

void UpdateNotifierWidget::handleUpdateChecked(QNetworkReply *reply)
{
	Messagebox msg_box;
	unsigned http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();

	if(reply->error() != QNetworkReply::NoError)
	{
		msg_box.show(tr("Failed to check updates"),
								 tr("The update notifier failed to check for new versions! Please, verify your internet connectivity and try again! Connection error returned: <em>%1</em> - <strong>%2</strong>.").arg(http_status).arg(reply->errorString()),
								 Messagebox::Error, Messagebox::OkButton);
	}
	else
	{
		/* In case of [301 - Move permanently] there is the need to make a new request to
			 reach the final destination */
		if(http_status == 301 || http_status == 302)
		{
			QString url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

			if(http_status == 302 && !url.startsWith(GlobalAttributes::PgModelerSite))
				url.prepend(GlobalAttributes::PgModelerSite);

			QNetworkRequest req(url);
			update_chk_reply = update_chk_manager.get(req);
		}
		else
		{
			//In case of [200 - OK] updates the contents with the retrieved json
			if(http_status == 200)
			{
				QJsonDocument json_doc = QJsonDocument::fromJson(reply->readAll());
				QJsonObject json_obj = json_doc.object();
				QString version = json_obj.value(Attributes::NewVersion).toString(),
						changelog = json_obj.value(Attributes::Changelog).toString(),
						date = json_obj.value(Attributes::Date).toString();
				bool upd_found = (!version.isEmpty() && version!=Attributes::False);

				blogpost.clear();

				if(upd_found)
				{
					blogpost = json_obj.value(Attributes::BlogPost).toString();
					blog_post_btn->setVisible(!blogpost.isEmpty());
					ver_num_lbl->setText(version);
					changelog_txt->setText(changelog);
					ver_date_lbl->setText(date);
				}
				else if(show_no_upd_msg)
				{
					msg_box.show(tr("No updates found"),
											 tr("You are running the most recent pgModeler version! No update needed."),
											 Messagebox::Info, Messagebox::OkButton);
				}

				emit s_updateAvailable(upd_found);
			}
			else
			{
				msg_box.show(tr("Failed to check updates"),
										 tr("The update notifier failed to check for new versions! A HTTP status code was returned: <strong>%1</strong>").arg(http_status),
										 Messagebox::Error, Messagebox::OkButton);
			}

			update_chk_reply->deleteLater();
			update_chk_reply=nullptr;
		}
	}
}


