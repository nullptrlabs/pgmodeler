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

#include "modeloverviewwidget.h"
#include "modelwidget.h"
#include <QScrollBar>

ModelOverviewWidget::ModelOverviewWidget(QWidget *parent) : QWidget(parent, Qt::WindowCloseButtonHint | Qt::Tool)
{
	setupUi(this);
	model = nullptr;
	zoom_factor = 1;
	curr_resize_factor = ResizeFactor;
	moving_frame = false;
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	scene_bg_lbl->setStyleSheet("QLabel#scene_bg_lbl{ border: 0px; }");
}

void ModelOverviewWidget::show(ModelWidget *model)
{
	if(this->model)
	{
		disconnect(this->model, nullptr, this, nullptr);
		disconnect(this->model->viewport, nullptr,  this, nullptr);
		disconnect(this->model->scene, nullptr,  this, nullptr);
	}

	this->model=model;

	if(this->model)
	{
		connect(this->model, &ModelWidget::s_objectCreated, this, qOverload<>(&ModelOverviewWidget::updateOverview));
		connect(this->model, &ModelWidget::s_objectRemoved, this, qOverload<>(&ModelOverviewWidget::updateOverview));
		connect(this->model, &ModelWidget::s_objectsMoved, this, qOverload<>(&ModelOverviewWidget::updateOverview));
		connect(this->model, &ModelWidget::s_objectModified, this, qOverload<>(&ModelOverviewWidget::updateOverview));
		connect(this->model, &ModelWidget::s_zoomModified, this, &ModelOverviewWidget::updateZoomFactor);
		connect(this->model, &ModelWidget::s_modelResized, this, &ModelOverviewWidget::resizeOverview);
		connect(this->model, &ModelWidget::s_modelResized, this, &ModelOverviewWidget::resizeViewportFrame);
		connect(this->model, &ModelWidget::s_modelResized, this, qOverload<>(&ModelOverviewWidget::updateOverview));

		connect(this->model->viewport->horizontalScrollBar(), &QScrollBar::valueChanged, this, &ModelOverviewWidget::resizeViewportFrame);
		connect(this->model->viewport->verticalScrollBar(), &QScrollBar::valueChanged, this, &ModelOverviewWidget::resizeViewportFrame);

		connect(this->model->scene, &ObjectsScene::selectionChanged, this, qOverload<>(&ModelOverviewWidget::updateOverview));
		connect(this->model->scene, &ObjectsScene::sceneRectChanged,this, &ModelOverviewWidget::resizeOverview);
		connect(this->model->scene, &ObjectsScene::sceneRectChanged,this, qOverload<>(&ModelOverviewWidget::updateOverview));

		this->resizeOverview();
		this->updateZoomFactor(this->model->getCurrentZoom());
		this->updateOverview(true);

		this->move(this->model->geometry().right() - this->width(),
							 this->model->geometry().bottom() - this->height());

		frame->installEventFilter(this);
	}

	this->raise();
	QWidget::show();
}

void ModelOverviewWidget::closeEvent(QCloseEvent *event)
{
	model=nullptr;
	emit s_overviewVisible(false);
	QWidget::closeEvent(event);
}

void ModelOverviewWidget::showEvent(QShowEvent *event)
{
	emit s_overviewVisible(true);
	QWidget::showEvent(event);
}

bool ModelOverviewWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == frame && event->type()==QEvent::Wheel)
	{
		QWheelEvent *w_event = static_cast<QWheelEvent *>(event);
		QPoint dt_angle = w_event->angleDelta();

		if(dt_angle.y() < 0)
			model->applyZoom(model->getCurrentZoom() - ModelWidget::ZoomIncrement);
		else
			model->applyZoom(model->getCurrentZoom() + ModelWidget::ZoomIncrement);

		return false;
	}

	return QWidget::eventFilter(object, event);
}

void ModelOverviewWidget::updateOverview()
{
	this->updateOverview(false);
}

void ModelOverviewWidget::updateOverview(bool force_update)
{
	if(this->model && (this->isVisible() || force_update))
	{
		QPixmap pix;

		qApp->setOverrideCursor(Qt::WaitCursor);

		//Creates a pixmap with the size of the scene
		pix = QPixmap(pixmap_size);
		pix.fill(ObjectsScene::getCanvasColor());

		//Draw the scene onto the pixmap
		QPainter p(&pix);

		if(!p.isActive())
		{
			scene_bg_lbl->setPixmap(QPixmap());
			scene_bg_lbl->setText(tr("Failed to generate the overview image.\nThe requested size %1 x %2 was too large and there was not enough memory to allocate!")
										 .arg(pixmap_size.width()).arg(pixmap_size.height()));
			frame->setEnabled(false);
		}
		else
		{
			frame->setEnabled(true);

			p.setRenderHints(QPainter::Antialiasing, false);
			p.setRenderHints(QPainter::TextAntialiasing, false);
			model->scene->render(&p, QRect(), scene_rect);

			//Resizes the pixmap to the previous configured QSize
			scene_bg_lbl->setPixmap(pix);
		}

		scene_bg_lbl->resize(curr_size.toSize());
		qApp->restoreOverrideCursor();
	}
}

void ModelOverviewWidget::resizeViewportFrame()
{
	if(!model || moving_frame)
		return;

	QSizeF size;
	double factor = curr_resize_factor / zoom_factor;
	QScrollBar *h_scroll = model->viewport->horizontalScrollBar(),
			*v_scroll = model->viewport->verticalScrollBar();

	//Resizes the window frame based upon the model's viewport dimensions
	size = model->viewport->geometry().size();
	size.setWidth(size.width() * factor);
	size.setHeight(size.height() * factor);
	viewport_frm->resize(size.toSize());

	//Set the frame position based upon the viewport scroll bar values
	QPoint pnt(h_scroll->value(), v_scroll->value()),
			p_aux = QPoint(abs(h_scroll->minimum()), abs(v_scroll->minimum())) + pnt;

	viewport_frm->move(p_aux.x() * factor, p_aux.y() * factor);
}

void ModelOverviewWidget::resizeOverview()
{
	if(!model)
		return;

	QRect screen_rect = this->screen()->geometry();

	//Make an initial calculation of the overview window size
	scene_rect = model->scene->sceneRect();
	curr_size = scene_rect.size();
	curr_size.setWidth(curr_size.width() * ResizeFactor);
	curr_size.setHeight(curr_size.height() * ResizeFactor);

	//If the size exceeds the screen's width or height in 50%
	if(curr_size.height() > screen_rect.height() * ScreenSzThreshold ||
		 curr_size.width() > screen_rect.width() * ScreenSzThreshold)
	{
		double h_ratio = (screen_rect.height() * ScreenSzThreshold) / scene_rect.height(),
				w_ratio = (screen_rect.width() * ScreenSzThreshold) / scene_rect.width();

		curr_resize_factor = std::min(w_ratio, h_ratio);
		curr_size.setHeight(scene_rect.height() * curr_resize_factor);
		curr_size.setWidth(scene_rect.width() * curr_resize_factor);
		pixmap_size = scene_rect.size().toSize();
	}
	else
	{
		curr_resize_factor = ResizeFactor;
		pixmap_size = scene_rect.size().toSize();
	}

	viewport_frm->setVisible(true);
	resize(curr_size.toSize());
	setMaximumSize(curr_size.toSize());
	setMinimumSize(curr_size.toSize());
}

void ModelOverviewWidget::updateZoomFactor(double zoom)
{
	zoom_factor = zoom;
	resizeViewportFrame();
}

void ModelOverviewWidget::mouseMoveEvent(QMouseEvent *event)
{
	if(!frame->isEnabled())
		return;

	if(event->buttons() == Qt::LeftButton)
	{
		QRect bg_rect = scene_bg_lbl->geometry(), rect = viewport_frm->geometry();
		QScrollBar *h_scroll = model->viewport->horizontalScrollBar(),
				*v_scroll = model->viewport->verticalScrollBar();

		int width = rect.width() / 2,
				height = rect.height() / 2,
				x = event->pos().x(),
				y = event->pos().y();

		//Configures a rectangle having as central point the event position
		rect.setLeft(x - width);
		rect.setTop(y - height);
		rect.setRight(x + width);
		rect.setBottom(y + height);

		// Do not allow the viewport rectangle extrapolates the bg_rect limits
		if(rect.left() < 0)
			rect.moveLeft(0);

		if(rect.top() < 0)
			rect.moveTop(0);

		if(rect.right() > bg_rect.right())
			rect.moveRight(bg_rect.right());

		if(rect.bottom() > bg_rect.bottom())
			rect.moveBottom(bg_rect.bottom());

		viewport_frm->move(rect.topLeft());

		/* Determine the horizontal and vertical factors by divinding the rect left/top
		 * by the background rect width / height. These factors are used to determine
		 * the new position of the viewport by changing the scroll bars values */
		double h_factor = (rect.left() / static_cast<double>(bg_rect.width())),
				v_factor = (rect.top() / static_cast<double>(bg_rect.height()));

		// Calculates the new scene position based upon the viewport rectangle.
		QPointF pnt;
		pnt.setX((scene_rect.left() + (scene_rect.width() * h_factor)) * zoom_factor);
		pnt.setY((scene_rect.top() + (scene_rect.height() * v_factor)) * zoom_factor);

		h_scroll->setValue(pnt.x());
		v_scroll->setValue(pnt.y());
	}
}

void ModelOverviewWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
		moving_frame = true;
		viewport_frm->setCursor(QCursor(Qt::OpenHandCursor));
		setCursor(QCursor(Qt::OpenHandCursor));
		model->startSceneMove();
	}
}

void ModelOverviewWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
		viewport_frm->setCursor(QCursor(Qt::ArrowCursor));
		setCursor(QCursor(Qt::ArrowCursor));
		model->finishSceneMove();
		moving_frame = false;
	}
}
