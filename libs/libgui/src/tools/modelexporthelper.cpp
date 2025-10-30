#include "modelexporthelper.h"
#include "utilsns.h"
#include <QSvgGenerator>
#include "pgsqlversions.h"

ModelExportHelper::ModelExportHelper(QObject *parent) : QObject(parent)
{
	resetExportParams();
}

void ModelExportHelper::resetExportParams()
{
	sql_gen_progress = progress = 0;
	db_created = ignore_dup = drop_db = drop_objs = export_canceled = false;
	simulate = use_tmp_names = db_sql_reenabled = override_bg_color = false;
	force_db_drop = gen_drop_file = md_format = false;
	show_grid = show_delim = page_by_page = split = browsable = false;
	transactional = false;
	created_objs[ObjectType::Role] = created_objs[ObjectType::Tablespace] = -1;
	db_model = nullptr;
	connection = nullptr;
	scene = nullptr;
	zoom = 100;
	viewp = nullptr;
	code_gen_mode = DatabaseModel::OriginalSql;
}

void ModelExportHelper::abortExport(Exception &e)
{
	resetExportParams();

	//When running in a separated thread (other than the main application thread) redirects the error in form of signal
	if(this->thread() && this->thread()!=qApp->thread())
		emit s_exportAborted(Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e));
	else
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
}

void ModelExportHelper::handleSQLError(Exception &e, const QString &sql_cmd, bool ignore_dup)
{
	//Ignoring the error if it is in the ignored list
	if(ignored_errors.indexOf(e.getExtraInfo()) >= 0 ||
			(ignore_dup && isDuplicationError(e.getExtraInfo())))
		emit s_errorIgnored(e.getExtraInfo(), e.getErrorMessage(), sql_cmd);
	//Raises an excpetion if the error returned by the database is not listed in the ignored list of errors
	else if(ignored_errors.indexOf(e.getExtraInfo()) < 0)
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e, sql_cmd);
	else
		errors.push_back(e);
}

void ModelExportHelper::setIgnoredErrors(const QStringList &err_codes)
{
	QRegularExpression valid_code = QRegularExpression(QRegularExpression::anchoredPattern("([a-z]|[A-Z]|[0-9])+"));

	ignored_errors.clear();

	for(auto &code : err_codes)
	{
		if(valid_code.match(code).hasMatch())
			ignored_errors.push_back(code);
	}

	ignored_errors.removeDuplicates();
}

void ModelExportHelper::exportToSQL(DatabaseModel *db_model, const QString &filename, const QString &pgsql_ver, bool split, DatabaseModel::CodeGenMode code_gen_mode, bool gen_drop_file)
{
	if(!db_model)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	connect(db_model, &DatabaseModel::s_objectLoaded, this, &ModelExportHelper::updateProgress);

	try
	{
		progress=sql_gen_progress=0;
		BaseObject::setPgSQLVersion(pgsql_ver);
		emit s_progressUpdated(progress,
													 tr("Generating SQL code for PostgreSQL `%1'").arg(BaseObject::getPgSQLVersion()),
													 ObjectType::BaseObject);
		progress=1;

		if(!split)
		{
			db_model->saveModel(filename, SchemaParser::SqlCode);
			emit s_progressUpdated(100, tr("SQL file `%1' successfully written.").arg(filename), ObjectType::BaseObject);
		}
		else
		{
			db_model->saveSplitSQLDefinition(filename, code_gen_mode, gen_drop_file);
			emit s_progressUpdated(100, tr("SQL files successfully written in `%1'.").arg(filename), ObjectType::BaseObject);
		}

		if(export_canceled)
			emit s_exportCanceled();
		else
			emit s_exportFinished();
	}
	catch(Exception &e)
	{
		disconnect(db_model, nullptr, this, nullptr);
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}

	disconnect(db_model, nullptr, this, nullptr);
}

void ModelExportHelper::exportToPNG(ObjectsScene *scene, const QString &filename, double zoom, bool show_grid, bool show_delim, bool page_by_page, bool override_bg_color, QGraphicsView *viewp)
{
	if(!scene)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		bool prev_show_grd = false, prev_show_dlm = false;
		QGraphicsView *view = nullptr;
		QColor bg_color;

		/* If an external view is specified it will be used instead of creating a local one,
		 * this is a workaround to the error below when running the helper in a separated thread
		 * QCoreApplication::sendPostedEvents: Cannot send posted events for objects in another thread */
		if(viewp)
			view = viewp;
		else
			view = new QGraphicsView(scene);

		//Clear the object scene selection to avoid drawing the selectoin rectangle of the objects
		scene->clearSelection();

		//Make a backup of the current scene options
		prev_show_grd = ObjectsScene::isShowGrid();
		prev_show_dlm = ObjectsScene::isShowPageDelimiters();
		bg_color = ObjectsScene::getCanvasColor();

		//Sets the options passed by the user
		if(override_bg_color)
			ObjectsScene::setCanvasColor(QColor(255,255,255));

		ObjectsScene::setShowGrid(show_grid);
		ObjectsScene::setShowPageDelimiters(show_delim);
		scene->setShowSceneLimits(false);

		QPixmap pix;
		QList<QRectF> pages;
		unsigned v_cnt=0, h_cnt=0, page_idx=1;
		QString tmpl_filename, file;

		if(page_by_page)
		{
			QFileInfo fi(filename);
			QString suffix = fi.completeSuffix();

			if(!suffix.isEmpty())
				suffix.prepend('.');

			//Calculates the page dimensions and the page count (horizontally and vertically) to be exported
			pages = scene->getPagesForPrinting(h_cnt, v_cnt);

			//Configures the template filename for pages pixmaps
			tmpl_filename = fi.absolutePath() + GlobalAttributes::DirSeparator + fi.baseName() + QString("_p%1") + suffix;
		}
		else
		{
			QRectF rect = scene->itemsBoundingRect(true, false, true);

			//Give some margin to the resulting image
			QSizeF margin = QSizeF(5 * BaseObjectView::HorizSpacing, 5 * BaseObjectView::VertSpacing);
			rect.setTopLeft(rect.topLeft() - QPointF(margin.width(), margin.height()));
			rect.setSize(rect.size() + margin);

			pages.push_back(rect);
			file = filename;
		}

		//Updates the scene to apply the change on grid and delimiter
		scene->update();

		QPainter painter;
		QRect rect;

		view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		view->resetTransform();
		view->centerOn(0,0);

		/* We consider the device pixel ration when applying scale to the viewport
		 * so the resulting pixmap can have a compatible size in hi-dpi screens */
		view->scale(zoom * qApp->devicePixelRatio(), zoom * qApp->devicePixelRatio());

		for(auto &pg_rect : pages)
		{
			if(export_canceled) break;

			rect = view->mapFromScene(pg_rect).boundingRect();
			pix = QPixmap(rect.size());
			pix.fill(ObjectsScene::getCanvasColor());

			//Setting optimizations on the painter
			painter.begin(&pix);
			painter.setRenderHint(QPainter::Antialiasing, true);
			painter.setRenderHint(QPainter::TextAntialiasing, true);
			painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

			emit s_progressUpdated((page_idx/static_cast<double>(pages.size())) * 90,
														 tr("Rendering objects to page %1/%2.").arg(page_idx).arg(pages.size()), ObjectType::BaseObject);

			view->scene()->render(&painter, QRect(), pg_rect);
			painter.end();

			if(page_by_page)
				file = tmpl_filename.arg(page_idx++);

			//If the pixmap is not saved raises an error
			if(!pix.save(file))
			{
				//Restoring the scene settings before throw error
				if(override_bg_color)
					ObjectsScene::setCanvasColor(bg_color);

				ObjectsScene::setShowGrid(prev_show_grd);
				ObjectsScene::setShowPageDelimiters(prev_show_dlm);
				scene->update();

				throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(file),
												ErrorCode::FileDirectoryNotWritten,PGM_FUNC,PGM_FILE,PGM_LINE);
			}
		}

		//Restoring the scene settings
		if(override_bg_color)
			ObjectsScene::setCanvasColor(bg_color);

		ObjectsScene::setShowGrid(prev_show_grd);
		ObjectsScene::setShowPageDelimiters(prev_show_dlm);
		scene->setShowSceneLimits(true);
		scene->update();

		if(!export_canceled)
		{
			emit s_progressUpdated(100, tr("Output image `%1' successfully written.").arg(filename), ObjectType::BaseObject);
			emit s_exportFinished();
		}
		else
			emit s_exportCanceled();

		if(view != viewp)
			delete view;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ModelExportHelper::exportToSVG(ObjectsScene *scene, const QString &filename, bool show_grid, bool show_delim)
{
	if(!scene)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	bool prev_show_dlm=false, prev_show_grd=false;
	QSvgGenerator svg_gen;
	QRectF scene_rect=scene->itemsBoundingRect(true, false, true), svg_rect;
	QFileInfo fi(filename);

	emit s_progressUpdated(0, tr("Exporting model to SVG file."));

	//Give some margin to the resulting image
	QSizeF margin = QSizeF(5 * BaseObjectView::HorizSpacing, 5 * BaseObjectView::VertSpacing);
	scene_rect.setTopLeft(scene_rect.topLeft() - QPointF(margin.width(), margin.height()));
	scene_rect.setSize(scene_rect.size() + margin);

	svg_rect = QRectF(0, 0, scene_rect.size().width(), scene_rect.size().height());
	svg_gen.setFileName(filename);
	svg_gen.setTitle(tr("SVG representation of database model"));
	svg_gen.setDescription(tr("SVG file generated by pgModeler"));
	svg_gen.setSize(svg_rect.size().toSize());
	svg_gen.setViewBox(svg_rect);

	// The SVG's dpi is calculated from the logical DPI of the viewport associated to the scene
	QGraphicsView *view = new QGraphicsView(scene);
	int dpi = (view->logicalDpiX() + view->logicalDpiY())/2;
	svg_gen.setResolution(dpi);

	//Making a backup of the current scene options
	prev_show_grd = ObjectsScene::isShowGrid();
	prev_show_dlm = ObjectsScene::isShowPageDelimiters();

	ObjectsScene::setShowGrid(show_grid);
	ObjectsScene::setShowPageDelimiters(show_delim);
	scene->setShowSceneLimits(false);
	scene->update();

	QPainter *svg_painter = new QPainter(&svg_gen);

	/* Workaround: In order to force the QSvgGenerator to render
	 * the grid and the delimiter lines we have to write the scene
	 * background to a separated pixmap and then render it in the
	 * painter responsible to draw elements in the SVG output.
	 * It seems that the QSvgGenerator doesn't render for itself the
	 * scene's brackground brush when this one is composed by a
	 * texture image (grid + delimiter lines). */
	if(show_grid || show_delim)
	{
		QPixmap bg_img = QPixmap(svg_rect.size().toSize());
		QPainter *bg_painter = new QPainter(&bg_img);
		QStringList act_layers = scene->getActiveLayers();

		/* In order to make a background clip we first hide all objects
		 * by deactivating the layers the draw the background in the
		 * bg painter in the dimensions defined by the scene rect,
		 * this will clip the background in the right position and size.
		 * After getting the bg pixmap we restore the active layers. */
		scene->blockSignals(true);
		scene->setActiveLayers(QStringList());
		scene->render(bg_painter, svg_rect, scene_rect);
		scene->setActiveLayers(act_layers);
		scene->blockSignals(false);

		// Drawing the bg pixmap in the svg painter
		svg_painter->drawPixmap(svg_rect, bg_img, svg_rect);
		delete bg_painter;
	}

	// Rendering the objects in the svg painter
	scene->render(svg_painter, svg_rect, scene_rect);
	delete svg_painter;
	delete view;

	//Restoring the scene settings
	ObjectsScene::setShowGrid(prev_show_grd);
	ObjectsScene::setShowPageDelimiters(prev_show_dlm);
	scene->setShowSceneLimits(true);
	scene->update();

	if(!fi.exists() || !fi.isWritable() || !fi.isReadable())
	{
			throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(filename),
											ErrorCode::FileDirectoryNotWritten,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	emit s_progressUpdated(100, tr("Output file `%1' successfully written.").arg(filename), ObjectType::BaseObject);
	emit s_exportFinished();
}

void ModelExportHelper::exportToDBMS(DatabaseModel *db_model, Connection conn, const QString &pgsql_ver, bool ignore_dup, bool drop_db, bool drop_objs, bool simulate,
																		 bool use_tmp_names, bool forced_db_drop, bool transactional)
{
	int type_id = 0, pos = -1;
	QString  version, sql_cmd, buf, sql_cmd_comment;
	Connection new_db_conn;
	unsigned i, count;
	ObjectType types[]={ObjectType::Role, ObjectType::Tablespace};
	BaseObject *object=nullptr;
	QString tmpl_comm_regexp = QString("(COMMENT)( )+(ON)( )+(%1)(.)+(\n)(") + Attributes::DdlEndToken + ")";
	QRegularExpression comm_regexp;
	QRegularExpressionMatch match;

	try
	{
		if(!db_model)
			throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		/* If the export is called using ignore duplications or drop database and simulation mode at same time
		an error is raised because the simulate mode (mainly used as SQL validation) cannot
		undo column addition (this can be changed in the future) */
		if(simulate && (ignore_dup || drop_db || drop_objs || transactional))
			throw Exception(ErrorCode::MixingIncompExportOptions,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(drop_db && drop_objs)
			throw Exception(ErrorCode::MixingIncompDropOptions,PGM_FUNC,PGM_FILE,PGM_LINE);

		connect(db_model, &DatabaseModel::s_objectLoaded, this, &ModelExportHelper::updateProgress, Qt::DirectConnection);

		export_canceled=false;
		db_created=false;
		progress=sql_gen_progress=0;
		created_objs[ObjectType::Role]=created_objs[ObjectType::Tablespace]=-1;
		errors.clear();

		//Retrive the DBMS version in order to generate the correct code
		conn.connect();
		version=conn.getPgSQLVersion(true);

		emit s_progressUpdated(progress, tr("Starting export to DBMS."));

		//Overriding the DBMS version case the version is informed on parameter
		if(!pgsql_ver.isEmpty())
		{
			BaseObject::setPgSQLVersion(pgsql_ver);
			emit s_progressUpdated(progress, tr("PostgreSQL version detection overridden. Using version `%1'.").arg(pgsql_ver));
		}
		else
		{
			BaseObject::setPgSQLVersion(version);
			emit s_progressUpdated(progress, tr("PostgreSQL `%1' server detected.").arg(version));
		}

		if(simulate && use_tmp_names)
		{
			emit s_progressUpdated(progress, tr("Generating temporary names for database, roles and tablespaces."));
			generateTempObjectNames(db_model);
		}
		else if(use_tmp_names)
			throw Exception(ErrorCode::InvUsageTempNamesExportOption,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(db_model->isSQLDisabled())
		{
			if(simulate)
			{
				db_model->setSQLDisabled(false);
				db_sql_reenabled=true;
				emit s_progressUpdated(progress, tr("Enabling the SQL code for database `%1' to avoid errors.").arg(db_model->getName()));
			}
			else
				throw Exception(Exception::getErrorMessage(ErrorCode::ExportFailureDbSQLDisabled).arg(db_model->getName()),
												ErrorCode::ExportFailureDbSQLDisabled, PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		if(ignore_dup)
		{
			emit s_progressUpdated(progress, tr("Ignoring object duplication errors."));

			//Save the current status for ALTER command generation for table columns/constraints
			saveGenAtlerCmdsStatus(db_model);
		}

		if(!ignored_errors.isEmpty())
			emit s_progressUpdated(progress, tr("Ignoring the following error code(s): `%1'.").arg(ignored_errors.join(", ")));

		if(drop_db)
		{
			emit s_progressUpdated(progress, tr("Trying to drop database `%1'.").arg(db_model->getName()));

			try
			{
				QString extra_drop_opt;

				if(version >= PgSqlVersions::PgSqlVersion130 && forced_db_drop)
					extra_drop_opt = "WITH (FORCE)";

				sql_cmd = QString("DROP DATABASE IF EXISTS %1 %2;").arg(db_model->getName(true), extra_drop_opt);
				conn.executeDDLCommand(sql_cmd);
			}
			catch(Exception &e)
			{
				handleSQLError(e, sql_cmd, ignore_dup);
			}

			sql_cmd.clear();
		}

		if(simulate)
			emit s_progressUpdated(progress, tr("Simulation mode activated."));

		//Creates the roles and tablespaces separately from the other objects
		for(type_id=0; type_id < 2 && !export_canceled; type_id++)
		{
			count=db_model->getObjectCount(types[type_id]);

			for(i=0; i < count && !export_canceled; i++)
			{
				object=db_model->getObject(i, types[type_id]);
				progress=((10 * (type_id+1)) + ((i/static_cast<double>(count)) * 10));

				try
				{
					if(!object->isSQLDisabled())
					{
						sql_cmd=object->getSourceCode(SchemaParser::SqlCode);

						//Emits a signal indicating that the object is being exported
						emit s_progressUpdated(progress,
																	 tr("Creating object `%1' (%2)").arg(object->getName()).arg(object->getTypeName()),
																	 object->getObjectType(), sql_cmd);

						if(types[type_id] == ObjectType::Tablespace)
						{
							comm_regexp = QRegularExpression(tmpl_comm_regexp.arg(object->getSQLName()));
							match = comm_regexp.match(sql_cmd);
							pos = match.capturedStart();

							/* If we find a comment on statement we should strip it from the tablespace definition in
							 * order to execute it after creating the db */
							if(pos >= 0)
							{
								sql_cmd_comment = sql_cmd.mid(pos, match.capturedLength());
								sql_cmd.remove(pos, match.capturedLength());
								//pos = -1;
							}
						}

						conn.executeDDLCommand(sql_cmd);

						if(!sql_cmd_comment.isEmpty())
							conn.executeDDLCommand(sql_cmd_comment);
					}
				}
				catch(Exception &e)
				{
					handleSQLError(e, sql_cmd, ignore_dup);
				}

				created_objs[types[type_id]]++;
			}
		}

		try
		{
			if(!db_model->isSQLDisabled() && !export_canceled)
			{
				comm_regexp = QRegularExpression(tmpl_comm_regexp.arg(db_model->getSQLName()));
				sql_cmd = db_model->__getSourceCode(SchemaParser::SqlCode);
				match = comm_regexp.match(sql_cmd);
				pos = match.capturedStart();

				/* If we find a comment on statment we should strip it from the DB definition in
				 * order to execute it after creating the db */
				if(pos >= 0)
				{
					sql_cmd_comment = sql_cmd.mid(pos, match.capturedLength());
					sql_cmd.remove(pos, match.capturedLength());
				}

				//Creating the database on the DBMS
				emit s_progressUpdated(progress,
															 tr("Creating database `%1'").arg(db_model->getName()),
															 ObjectType::Database, sql_cmd);

				conn.executeDDLCommand(sql_cmd);
				db_created=true;

				if(!sql_cmd_comment.isEmpty())
					conn.executeDDLCommand(sql_cmd_comment);
			}
		}
		catch(Exception &e)
		{
			handleSQLError(e, sql_cmd, ignore_dup);
		}

		if(!export_canceled)
		{
			//Connects to the new created database to create the other objects
			progress=20;
			new_db_conn=conn;
			new_db_conn.setConnectionParam(Connection::ParamDbName, db_model->getName());
			emit s_progressUpdated(progress,
								   tr("Connecting to database `%1'")
								   .arg(db_model->getName()));

			new_db_conn.connect();
			progress=30;

			//Creating the other object types
			emit s_progressUpdated(progress, tr("Generating SQL for `%1' objects...").arg(db_model->getObjectCount()));

			//Exporting the database model definition using the opened connection
			buf=db_model->getSourceCode(SchemaParser::SqlCode, false);
			progress=40;
			exportBufferToDBMS(buf, new_db_conn, drop_objs, transactional);
		}

		disconnect(db_model, nullptr, this, nullptr);

		if(ignore_dup)
			restoreGenAtlerCmdsStatus();

		//Closes the new opened connection
		new_db_conn.close();

		/* If the process was a simulation or even canceled undo the export
		removing the created objects */
		if(simulate || export_canceled)
			undoDBMSExport(db_model, conn, use_tmp_names);

		conn.close();

		if(!export_canceled)
			emit s_exportFinished();
		else
			emit s_exportCanceled();
	}
	catch(Exception &e)
	{
		disconnect(db_model, nullptr, this, nullptr);

		if(ignore_dup)
			restoreGenAtlerCmdsStatus();

		try
		{
			//Closes the new opened connection
			new_db_conn.close();

			//Undo the export removing the created objects
			undoDBMSExport(db_model, conn, use_tmp_names);
		}
		catch(Exception &){}

		conn.close();

		/* When running in a separated thread (other than the main application thread)
		 * redirects the error in form of signal */
		if(this->thread() && this->thread()!=qApp->thread())
		{
			errors.push_back(e);
			emit s_exportAborted(Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, errors));
		}
		else
		{
			if(errors.empty())
				throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);

			errors.push_back(e);
			throw Exception(e.getErrorMessage(),PGM_FUNC,PGM_FILE,PGM_LINE, errors);
		}
	}
}

void ModelExportHelper::exportToDataDict(DatabaseModel *db_model, const QString &path, bool browsable, bool split, bool md_format)
{
	if(!db_model)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	connect(db_model, &DatabaseModel::s_objectLoaded, this, &ModelExportHelper::updateProgress);

	try
	{
		progress=0;
		emit s_progressUpdated(progress,
													 tr("Starting data dictionary generation..."),
													 ObjectType::BaseObject);
		progress=1;
		db_model->saveDataDictionary(path, browsable, split, md_format);

		emit s_progressUpdated(100, tr("Data dictionary successfully saved into `%1'.").arg(path), ObjectType::BaseObject);
		emit s_exportFinished();
	}
	catch(Exception &e)
	{
		disconnect(db_model, nullptr, this, nullptr);
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}

	disconnect(db_model, nullptr, this, nullptr);
}

void ModelExportHelper::saveGenAtlerCmdsStatus(DatabaseModel *db_model)
{
	std::vector<BaseObject *> objects;
	PhysicalTable *tab=nullptr;
	Relationship *rel=nullptr;

	objects.insert(objects.end(), db_model->getObjectList(ObjectType::Table)->begin(),
								 db_model->getObjectList(ObjectType::Table)->end());

	objects.insert(objects.end(), db_model->getObjectList(ObjectType::ForeignTable)->begin(),
								 db_model->getObjectList(ObjectType::ForeignTable)->end());

	//Store the relationship on the auxiliary vector but only many-to-many are considered
	objects.insert(objects.end(), db_model->getObjectList(ObjectType::Relationship)->begin(),
				   db_model->getObjectList(ObjectType::Relationship)->end());

	alter_cmds_status.clear();

	while(!objects.empty())
	{
		rel=dynamic_cast<Relationship *>(objects.back());

		/* If the current object is a relationship try to get the
		generated table (if the relationship is many-to-many only) */
		if(rel)
			tab=rel->getGeneratedTable();
		else
			tab=dynamic_cast<PhysicalTable *>(objects.back());

		if(tab)
		{
			//Store the current alter state
			alter_cmds_status[tab]=tab->isGenerateAlterCmds();
			//Forcing columns and constraints to be generated via ALTER command
			tab->setGenerateAlterCmds(true);
		}

		objects.pop_back();
	}
}

void ModelExportHelper::restoreGenAtlerCmdsStatus()
{
	for(auto &itr : alter_cmds_status)
		itr.first->setGenerateAlterCmds(itr.second);

	alter_cmds_status.clear();
}

void ModelExportHelper::undoDBMSExport(DatabaseModel *db_model, Connection &conn, bool use_tmp_names)
{
	QString drop_cmd=QString("DROP %1 %2;");
	ObjectType types[]={ObjectType::Role, ObjectType::Tablespace};
	int type_id;
	BaseObject *object=nullptr;

	//In case of error during the export all created object are removed
	if(db_created || created_objs[ObjectType::Role] >= 0 || created_objs[ObjectType::Tablespace] >= 0)
	{
		emit s_progressUpdated(99, tr("Destroying objects created on the server."));

		//Dropping the database
		if(db_created)
			conn.executeDDLCommand(drop_cmd.arg(db_model->getSQLName()).arg(db_model->getName(true)));

		//Drop the roles / tablespaces created
		for(type_id=1; type_id >=0; type_id--)
		{
			while(created_objs[types[type_id]] >= 0)
			{
				object=db_model->getObject(created_objs[types[type_id]], types[type_id]);

				try
				{
					if(!object->isSQLDisabled())
						conn.executeDDLCommand(drop_cmd.arg(object->getSQLName()).arg(object->getName(true)));
				}
				catch(Exception &){}

				created_objs[types[type_id]]--;
			}
		}

		if(use_tmp_names)
		{
			emit s_progressUpdated(100, tr("Restoring original names of database, roles and tablespaces."));
			restoreObjectNames();
		}
	}

	if(db_sql_reenabled)
	{
		db_model->setSQLDisabled(true);
		db_sql_reenabled=false;
	}
}

void ModelExportHelper::generateTempObjectNames(DatabaseModel *db_model)
{
	QString tmp_name, old_name;
	QTextStream stream(&tmp_name);
	QDateTime dt=QDateTime::currentDateTime();
	std::map<ObjectType, QString> obj_suffixes={ { ObjectType::Database, "db_" },
											{ ObjectType::Role, "rl_"},
											{ ObjectType::Tablespace, "tb_"} };

	orig_obj_names.clear();
	orig_obj_names[db_model]=db_model->getName();

	for(auto &role : *db_model->getObjectList(ObjectType::Role))
	{
		if(!role->isSystemObject())
			orig_obj_names[role]=role->getName();
	}

	for(auto &tabspc : *db_model->getObjectList(ObjectType::Tablespace))
	{
		if(!tabspc->isSystemObject())
			orig_obj_names[tabspc]=tabspc->getName();
	}

	for(auto &obj : orig_obj_names)
	{
		stream << reinterpret_cast<unsigned *>(obj.first) << "_" << dt.toMSecsSinceEpoch();

		//Generates an unique name for the object through md5 hash
		tmp_name = obj_suffixes[obj.first->getObjectType()] + UtilsNs::getStringHash(tmp_name);

		old_name=obj.first->getName();
		obj.first->setName(tmp_name.mid(0,15));
		tmp_name.clear();

		emit s_progressUpdated(progress, tr("Renaming `%1' (%2) to `%3'")
							   .arg(old_name)
							   .arg(obj.first->getTypeName())
							   .arg(obj.first->getName()));
	}

	/* Invalidates the codes of all objects on database model in order to generate the SQL referencing the
		 renamed object correctly */
	db_model->setCodesInvalidated();
}

void ModelExportHelper::restoreObjectNames()
{
	for(auto &obj : orig_obj_names)
		obj.first->setName(obj.second);

	/* Invalidates the codes of all objects on database model in order to generate the SQL referencing the
		 object's with their original names */
	if(db_model)
		db_model->setCodesInvalidated();
}

bool ModelExportHelper::isDuplicationError(const QString &error_code)
{
	/* Error codes treated in this method
	  42P04 	duplicate_database
	  42723 	duplicate_function
	  42P06 	duplicate_schema
	  42P07 	duplicate_table
	  42710 	duplicate_object
	  42701   duplicate_column
	  42P16   invalid_table_definition

	 Reference:
	  http://www.postgresql.org/docs/current/static/errcodes-appendix.html*/
	static QStringList err_codes = {"42P04", "42723", "42P06",
																	"42P07", "42710", "42701",
																	"42P16"};

	return err_codes.contains(error_code);
}

void ModelExportHelper::exportBufferToDBMS(const QString &buffer, Connection &conn, bool drop_objs, bool transactional)
{
	Connection aux_conn;
	QString sql_buf=buffer, sql_cmd, aux_cmd, lin, msg,
			obj_name, obj_tp_name, tab_name, orig_conn_db_name,
			alter_tab="ALTER TABLE";
	std::vector<QString> db_sql_cmds;
	QTextStream ts;
	ObjectType obj_type=ObjectType::BaseObject;
	bool ddl_tk_found=false, is_create=false, is_drop=false, in_transaction = false;
	unsigned aux_prog=0, curr_size=0, buf_size=sql_buf.size(),
			factor=(db_name.isEmpty() ? 70 : 90);
	int pos=0, pos1=0, comm_cnt=0;

	//Regexp used to extract the object being created
	QRegularExpression obj_reg("(CREATE|DROP|ALTER)(.)+(\n)", QRegularExpression::DontCaptureOption),

			tab_obj_reg(QString("^(%1)(.)+(ADD|DROP)( )(COLUMN|CONSTRAINT)( )*").arg(alter_tab),
									QRegularExpression::DontCaptureOption),

			drop_reg("^((\\-\\-)+( )*)+(DROP)(.)+",
							 QRegularExpression::DontCaptureOption),

			drop_tab_obj_reg(QString("^((\\-\\-)+( )*)+(%1)(.)+(DROP)(.)+").arg(alter_tab),
											 QRegularExpression::DontCaptureOption),
			reg_aux,

			name_rx("^((\".+\")|(\\w|_|\\d)+)(\\.((\".+\")|(\\w|_|\\d)+))*"),

			comm_regexp = QRegularExpression(QString("^(%1)|((--\\s)(%2|object|ALTER|DROP)(.)+)$").arg(Attributes::DdlEndToken, "\\*"),
																			 QRegularExpression::DontCaptureOption);
	QRegularExpressionMatch match;

	std::vector<ObjectType> obj_types={ ObjectType::Role, ObjectType::Function, ObjectType::Trigger, ObjectType::Index,
																 ObjectType::Policy, ObjectType::Rule,	ObjectType::Table, ObjectType::View, ObjectType::Domain,
																 ObjectType::Schema,	ObjectType::Aggregate, ObjectType::OpFamily,
																 ObjectType::OpClass, ObjectType::Operator,  ObjectType::Sequence,
																 ObjectType::Conversion, ObjectType::Cast,	ObjectType::Language,
																 ObjectType::Collation, ObjectType::Extension, ObjectType::Type,
																 ObjectType::EventTrigger, ObjectType::ForeignDataWrapper, ObjectType::ForeignServer,
																 ObjectType::UserMapping, ObjectType::ForeignTable, ObjectType::Transform,
																 ObjectType::Procedure, ObjectType::Database, ObjectType::BaseObject };

	/* Extract each SQL command from the buffer and execute them separately. This is done
   to permit the user, in case of error, identify what object is wrongly configured. */
	ts.setString(&sql_buf);

	if(!conn.isStablished())
	{
		orig_conn_db_name = conn.getConnectionParam(Connection::ParamDbName);

		if(!db_name.isEmpty())
			conn.setConnectionParam(Connection::ParamDbName, db_name);

		conn.connect();
	}

	while(!ts.atEnd() && !export_canceled)
	{
		try
		{
			//Cleanup single line comments
			lin=ts.readLine();
			curr_size+=lin.size();
			aux_prog=progress + ((curr_size/static_cast<double>(buf_size)) * factor);

			/* If the simulation mode is off and the drop objects option is checked,
			 * check if the current line matches one of the accepted drop commands
			 * (DROP [OBJECT] or ALTER TABLE...DROP) */
			if(drop_objs && (drop_reg.match(lin).hasMatch() || drop_tab_obj_reg.match(lin).hasMatch()))
			{
				comm_cnt = lin.count("--");
				lin = lin.remove("--").trimmed();

				/* If the count of comment indicators (--) is 1 indicates that the DDL of the
				 * object related to the DROP is enabled, so the DROP is executed otherwise ignored */
				if(comm_cnt == 1)
				{
					sql_cmd = lin + "\n";
					ddl_tk_found = true;
				}
			}
			else
			{
				ddl_tk_found = lin.contains(Attributes::DdlEndToken);
				lin.remove(comm_regexp);

				//If the line isn't empty after cleanup it will be included on sql command
				if(!lin.isEmpty())
					sql_cmd += lin + "\n";
			}

			//If the ddl end token is found
			if(ddl_tk_found || (!sql_cmd.isEmpty() && ts.atEnd()))
			{
				//Checking if the command is a column or constraint creation via ALTER TABLE
				aux_cmd = sql_cmd;

				if(tab_obj_reg.match(aux_cmd).hasMatch())
				{
					aux_cmd.remove("IF EXISTS ");
					obj_type=(aux_cmd.contains("COLUMN") ? ObjectType::Column : ObjectType::Constraint);
					reg_aux.setPattern("(COLUMN|CONSTRAINT)( )+");

					//Extracting the table name
					pos = aux_cmd.indexOf(alter_tab) + alter_tab.size();
					pos1 = aux_cmd.indexOf("ADD");

					if(pos1 < 0)
					{
						pos1=aux_cmd.indexOf("DROP");
						is_drop=true;
					}

					tab_name=aux_cmd.mid(pos, pos1 - pos).simplified();

					//Extracting the child object name (column | constraint) the one between
					match = reg_aux.match(aux_cmd, pos1);
					pos = match.capturedStart();
					pos += match.capturedLength();

					pos1=aux_cmd.indexOf(" ", pos);
					obj_name=aux_cmd.mid(pos, pos1 - pos).simplified();

					//Creating a fully qualified name for the object (schema.table.name)
					obj_name=tab_name + "." + obj_name;

					if(is_drop)
						msg=tr("Dropping object `%1' (%2)").arg(obj_name).arg(BaseObject::getTypeName(obj_type));
					else
						msg=tr("Creating object `%1' (%2)").arg(obj_name).arg(BaseObject::getTypeName(obj_type));

					emit s_progressUpdated(aux_prog, msg, obj_type, sql_cmd);
					is_drop=false;
				}
				//Check if the regex matches the sql command
				else if(obj_reg.match(sql_cmd).hasMatch())
				{
					//Get the fisrt line of the sql command, that contains the CREATE/DROP/ALTER ... statement
					lin=sql_cmd.mid(0, sql_cmd.indexOf('\n'));

					for(ObjectType obj_tp : obj_types)
					{
						if(export_canceled) break;

						obj_type=obj_tp;

						//Removing/replacing noisy keywords in order to extract more easily the object's name
						if(lin.startsWith("CREATE") || lin.startsWith("ALTER"))
						{
							if(lin.contains("OR REPLACE"))
							{
								lin.remove("OR REPLACE");
							}
							else if(obj_tp == ObjectType::Index)
							{
								lin.remove("UNIQUE");
								lin.remove("CONCURRENTLY");
							}
							else if(obj_tp == ObjectType::View)
							{
								lin.remove("MATERIALIZED");
								lin.remove("RECURSIVE");
							}
							else if(obj_tp == ObjectType::Table)
							{
								lin.remove("UNLOGGED");
							}
							else if(obj_tp == ObjectType::Transform)
							{
								lin.remove(" FOR");
								lin.replace(" LANGUAGE ", "_");
								lin.replace(QRegularExpression("(TRANSFORM)(.)+(\\.)"), "TRANSFORM ");
							}
						}
						else if(lin.startsWith("DROP"))
						{
							lin.remove("IF EXISTS");
							lin.remove("MATERIALIZED");
						}

						lin = lin.simplified();

						//Check if the keyword for the current object exists on string
						reg_aux.setPattern(QString("(CREATE|DROP|ALTER)( )(%1)").arg(BaseObject::getSQLName(obj_tp)));
						match = reg_aux.match(lin);

						if(match.hasMatch())
						{
							is_create = lin.startsWith("CREATE");
							is_drop = (!is_create && lin.startsWith("DROP"));

							//Extracts from the line the string starting with the object's name
							lin = lin.mid(match.capturedLength(), sql_cmd.indexOf('\n')).simplified();

							if(obj_tp != ObjectType::BaseObject)
							{
								if(obj_tp != ObjectType::Cast && obj_tp != ObjectType::UserMapping)
								{
									match = name_rx.match(lin);
									obj_name = lin.mid(match.capturedStart(), match.capturedLength());

									if(obj_tp != ObjectType::Function && obj_tp != ObjectType::Procedure)
									{
										obj_name = obj_name.remove('(').simplified();
										obj_name = obj_name.remove(')').simplified();
									}

									if(obj_tp == ObjectType::Trigger || obj_tp == ObjectType::Index)
									{
										int on_idx = sql_cmd.indexOf("ON ") + 3,
												lb_idx = sql_cmd.indexOf('\n', on_idx);

										// Extracting the table name from command so it'll be prepended to object name
										lin = sql_cmd.mid(on_idx, lb_idx - on_idx);
										match = name_rx.match(lin);
										obj_name.prepend(lin.mid(match.capturedStart(), match.capturedLength()) + '.');
									}
								}
								else if(obj_tp == ObjectType::UserMapping)
								{
									obj_name.prepend(lin.remove("FOR").trimmed() + "@");
								}
								else
								{
									obj_name="cast" + lin.replace(" AS ",",");
								}

								//Stores the object type name
								obj_tp_name = BaseObject::getTypeName(obj_tp);
								obj_name.remove(';');

								if(is_create)
									msg = tr("Creating object `%1' (%2)").arg(obj_name).arg(obj_tp_name);
								else if(is_drop)
									msg = tr("Dropping object `%1' (%2)").arg(obj_name).arg(obj_tp_name);
								else
									msg = tr("Changing object `%1' (%2)").arg(obj_name).arg(obj_tp_name);
							}
							// If the type of the object being create can't be identified
							else
							{
								QString aux_cmd_type;

								if(is_create)
									aux_cmd_type = "CREATE";
								else if(is_drop)
									aux_cmd_type = "DROP";
								else
									aux_cmd_type = "ALTER";

								msg=tr("Running auxiliary `%1' command...").arg(aux_cmd_type);
							}

							break;
						}
					}

					emit s_progressUpdated(aux_prog, msg, obj_type, sql_cmd);
					/* is_create = */ is_drop = false;
					msg.clear();
				}
				else if(!sql_cmd.trimmed().isEmpty())
				{
					//General commands like grant, revoke or set aren't explicitly shown
					emit s_progressUpdated(aux_prog, tr("Running auxiliary command."), ObjectType::BaseObject, sql_cmd);
				}

				//Executes the extracted SQL command
				if(!sql_cmd.isEmpty() && !export_canceled)
				{
					if(obj_type != ObjectType::Database)
					{
						if(transactional && !in_transaction)
						{
							emit s_progressUpdated(aux_prog, tr("Starting transaction."), ObjectType::BaseObject, "");
							conn.executeDDLCommand("BEGIN");
							in_transaction = true;
						}

						conn.executeDDLCommand(sql_cmd);
					}
					else
						//If it's a database level command (e.g. ALTER DATABASE ... RENAME TO ...)
						db_sql_cmds.push_back(sql_cmd);
				}

				sql_cmd.clear();
				ddl_tk_found=false;
			}

			if(ts.atEnd() && in_transaction)
			{
				if(export_canceled)
				{
					emit s_progressUpdated(aux_prog, tr("Rolling back changes."), ObjectType::BaseObject, "");
					conn.executeDDLCommand("ROLLBACK");
				}
				else
				{
					emit s_progressUpdated(aux_prog, tr("Committing changes."), ObjectType::BaseObject, "");
					conn.executeDDLCommand("COMMIT");
				}
			}

			//Executing the pending database level commands
			if(ts.atEnd() && !db_sql_cmds.empty() && !export_canceled)
			{
				conn.close();
				aux_conn=conn;

				if(!orig_conn_db_name.isEmpty())
					aux_conn.setConnectionParam(Connection::ParamDbName, orig_conn_db_name);

				aux_conn.connect();
				for(QString cmd : db_sql_cmds)
					aux_conn.executeDDLCommand(cmd);
			}
		}
		catch(Exception &e)
		{
			if(conn.isStablished() && in_transaction)
			{
				try
				{
					emit s_progressUpdated(aux_prog, tr("Rolling back changes."), ObjectType::BaseObject, "");
					conn.executeDDLCommand("ROLLBACK");
				}
				catch(Exception &)
				{
					emit s_progressUpdated(aux_prog, tr("Failed to roll back changes!"), ObjectType::BaseObject, "");
				}
			}

			if(ddl_tk_found) ddl_tk_found=false;
			handleSQLError(e, sql_cmd, ignore_dup);
			sql_cmd.clear();
		}
	}
}

void ModelExportHelper::updateProgress(int prog, QString object_id, unsigned obj_type)
{
	int aux_prog = progress + (prog/progress);

	sql_gen_progress = prog;

	if(aux_prog > 100)
		aux_prog=100;

	emit s_progressUpdated(aux_prog, object_id, static_cast<ObjectType>(obj_type), "", sender() == db_model);
}

void ModelExportHelper::setExportToDBMSParams(DatabaseModel *db_model, Connection *conn, const QString &pgsql_ver,
																							bool ignore_dup, bool drop_db, bool drop_objs, bool simulate, bool use_rand_names,
																							bool force_db_drop, bool transactional)
{
	this->db_model = db_model;
	this->connection = conn;
	this->pgsql_ver = pgsql_ver;
	this->ignore_dup = ignore_dup;
	this->simulate = simulate;
	this->drop_db = drop_db && !drop_objs;
	this->drop_objs = drop_objs && !drop_db;
	this->use_tmp_names = use_rand_names;
	this->force_db_drop = drop_db && force_db_drop;
	this->transactional = transactional;
	this->sql_buffer.clear();
	this->db_name.clear();
	this->errors.clear();
}

void ModelExportHelper::setExportToDBMSParams(const QString &sql_buffer, Connection *conn, const QString &db_name, bool ignore_dup, bool transactional)
{
	this->sql_buffer = sql_buffer;
	this->connection = conn;
	this->db_name = db_name;
	this->ignore_dup = ignore_dup;
	this->simulate = false;
	this->drop_db = false;
	this->transactional = transactional;
	this->use_tmp_names = false;
	this->errors.clear();
}

void ModelExportHelper::setExportToSQLParams(DatabaseModel *db_model, const QString &filename, const QString &pgsql_ver, bool split, DatabaseModel::CodeGenMode code_gen_mode, bool gen_drop_file)
{
	this->db_model = db_model;
	this->filename = filename;
	this->pgsql_ver = pgsql_ver;
	this->split = split;
	this->code_gen_mode = code_gen_mode;
	this->gen_drop_file = gen_drop_file;
}

void ModelExportHelper::setExportToPNGParams(ObjectsScene *scene, QGraphicsView *viewp, const QString &filename,
																						 double zoom, bool show_grid, bool show_delim, bool page_by_page,
																						 bool override_bg_color)
{
	this->scene = scene;
	this->viewp = viewp;
	this->filename = filename;
	this->zoom = zoom;
	this->show_grid = show_grid;
	this->show_delim = show_delim;
	this->page_by_page = page_by_page;
	this->override_bg_color = override_bg_color;
}

void ModelExportHelper::setExportToSVGParams(ObjectsScene *scene, const QString &filename, bool show_grid, bool show_delim)
{
	this->scene = scene;
	this->filename = filename;
	this->show_grid = show_grid;
	this->show_delim = show_delim;
}

void ModelExportHelper::setExportToDataDictParams(DatabaseModel *db_model, const QString &path, bool browsable, bool split, bool md_format)
{
	this->db_model = db_model;
	this->filename = path;
	this->browsable = browsable;
	this->split = split;
	this->md_format = md_format;
}

void ModelExportHelper::exportToDBMS()
{
	if(connection)
	{
		if(sql_buffer.isEmpty())
		{
			exportToDBMS(db_model, *connection, pgsql_ver, ignore_dup, drop_db,
									 drop_objs, simulate, use_tmp_names, force_db_drop, transactional);
		}
		else
		{
			try
			{
				exportBufferToDBMS(sql_buffer, *connection, false, transactional);

				if(export_canceled)
					emit s_exportCanceled();
				else
					emit s_exportFinished();
			}
			catch(Exception &e)
			{
				abortExport(e);
			}
		}
		resetExportParams();
	}
}

void ModelExportHelper::exportToPNG()
{
	try
	{
		exportToPNG(scene, filename, zoom, show_grid, show_delim, page_by_page, override_bg_color, viewp);
		resetExportParams();
	}
	catch(Exception &e)
	{
		abortExport(e);
	}
}

void ModelExportHelper::exportToSVG()
{
	try
	{
		exportToSVG(scene, filename, show_grid, show_delim);
		resetExportParams();
	}
	catch(Exception &e)
	{
		abortExport(e);
	}
}

void ModelExportHelper::exportToSQL()
{
	try
	{
		exportToSQL(db_model, filename, pgsql_ver, split, code_gen_mode, gen_drop_file);
		resetExportParams();
	}
	catch(Exception &e)
	{
		abortExport(e);
	}
}

void ModelExportHelper::exportToDataDict()
{
	try
	{
		exportToDataDict(db_model, filename, browsable, split, md_format);
		resetExportParams();
	}
	catch(Exception &e)
	{
		abortExport(e);
	}
}

void ModelExportHelper::cancelExport()
{
	if(db_model)
		db_model->setCancelSaving(true);

	export_canceled = true;
}
