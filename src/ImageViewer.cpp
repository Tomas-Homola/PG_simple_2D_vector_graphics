#include "ImageViewer.h"

// custom farby: cervena: #ED1C24; zelena: #00AD33; modra: #1F75FE

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);

	currentPenColor = QColor("#FFFFFF");
	currentFillColor = QColor("#1F75FE"); // custom modra farba
	ui->pushButton_PenColorDialog->setStyleSheet("background-color:#FFFFFF");
	ui->pushButton_FillColorDialog->setStyleSheet("background-color:#1F75FE");

	globalPenColor = QColor("#FFFFFF");
	globalFillColor = QColor("#00AD33");

	openNewTabForImg(new ViewerWidget("Default window", QSize(800, 550)));
	ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
	getCurrentViewerWidget()->clear();
}

void ImageViewer::infoMessage(QString message)
{
	msgBox.setWindowTitle("Info message");
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setText(message);
	msgBox.exec();
}
void ImageViewer::warningMessage(QString message)
{
	msgBox.setWindowTitle("Warning message");
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.setText(message);
	msgBox.exec();
}

void ImageViewer::printPoints(QVector<QPoint> geometryPoints)
{
	for (int i = 0; i < geometryPoints.size(); i++)
		qDebug() << geometryPoints.at(i);
	qDebug() << "\n";
}

void ImageViewer::drawObjects()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	currentGeometryObjects = getCurrentViewerWidget()->getGeometryObjectsPointer();

	if (currentGeometryObjects->isEmpty() || currentGeometryObjects == nullptr)
		return;
	
	vw->clear();

	for (int i = (currentGeometryObjects->size() - 1); i >= 0; i--)
	{
		if ((*currentGeometryObjects)[i]->getObjectType() == Object2D::Line)
		{
			vw->drawLine((*currentGeometryObjects)[i]);
		}
		else if ((*currentGeometryObjects)[i]->getObjectType() == Object2D::Rectangle)
		{
			vw->drawRectangle((*currentGeometryObjects)[i]);
		}
		else if ((*currentGeometryObjects)[i]->getObjectType() == Object2D::Polygon)
		{
			vw->drawPolygon((*currentGeometryObjects)[i]);
		}
		else if ((*currentGeometryObjects)[i]->getObjectType() == Object2D::Circumference)
		{
			vw->drawCircumference((*currentGeometryObjects)[i]);
		}
		else if ((*currentGeometryObjects)[i]->getObjectType() == Object2D::BezierCurve)
		{
			vw->drawBezierCurve((*currentGeometryObjects)[i]);
		}
	}
}

//ViewerWidget functions
ViewerWidget* ImageViewer::getViewerWidget(int tabId)
{
	QScrollArea* s = static_cast<QScrollArea*>(ui->tabWidget->widget(tabId));
	if (s) {
		ViewerWidget* vW = static_cast<ViewerWidget*>(s->widget());
		return vW;
	}
	return nullptr;
}
ViewerWidget* ImageViewer::getCurrentViewerWidget()
{
	return getViewerWidget(ui->tabWidget->currentIndex());
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	ViewerWidget* vw = getCurrentViewerWidget();
	Object2D* newObject = nullptr;
	QPoint p1(0, 0), p2(0, 0);
	QPointF pf1(0.0, 0.0), pf2(0.0, 0.0);
	int size = 0;
	double deltaX = 0.0, deltaY = 0.0;

	currentGeometryObjects = vw->getGeometryObjectsPointer();

	if (drawingEnabled) // ide sa kreslit
	{
		if (e->button() == Qt::LeftButton)
		{
			objectPoints.push_back((QPointF)e->pos()); // pretypovane na QPointF

			if (drawingObject == Object2D::Line)
			{
				if (objectPoints.size() == 2)
				{
					drawingEnabled = false;
					drawingObject = -1;

					newObject = new Object2D;

					newObject->setObjectType(Object2D::Line); // typ
					newObject->setObjectName(QString("line.%1").arg(objectsTally[Object2D::Line])); // nazov
					newObject->setPenColor(globalPenColor); // farba pera
					newObject->setObjectPoints(objectPoints); // body usecky

					currentGeometryObjects->push_front(newObject); // pridanie ku ostatnym objektom

					objectsTally[Object2D::Line]++;

					//ui->comboBox_SelectObject->addItem(newObject->getObjectName()); // pridanie do comboBoxu
					ui->comboBox_SelectObject->clear();
					for (int i = 0; i < currentGeometryObjects->size(); i++)
					{
						ui->comboBox_SelectObject->addItem((*currentGeometryObjects)[i]->getObjectName());
					}
					qDebug() << "count:" << ui->comboBox_SelectObject->count();

					ui->pushButton_PenColorDialog->setStyleSheet(QString("background-color:%1").arg(newObject->getPenColor().name()));

					objectPoints.clear();

					drawObjects();
				}
			}
			else if (drawingObject == Object2D::Rectangle)
			{
				if (objectPoints.size() == 2)
				{
					drawingEnabled = false;
					drawingObject = -1;

					if (objectPoints[0].x() < objectPoints[1].x())
					{
						pf1 = objectPoints[0];
						pf2 = objectPoints[1];
					}
					else if (objectPoints[0].x() > objectPoints[1].x())
					{
						pf1 = objectPoints[1];
						pf2 = objectPoints[0];
					}

					deltaX = pf2.x() - pf1.x();
					deltaY = pf2.y() - pf1.y();

					objectPoints.clear();

					if (pf1.y() < pf2.y()) // p1 hore vlavo
					{
						objectPoints.push_back(pf1);
						objectPoints.push_back(QPoint(pf1.x() + deltaX, pf1.y()));
						objectPoints.push_back(pf2);
						objectPoints.push_back(QPoint(pf1.x(), pf1.y() + deltaY));
					}
					else if (pf1.y() > pf2.y()) // p1 dole vlavo
					{
						objectPoints.push_back(pf1);
						objectPoints.push_back(QPoint(pf1.x(), pf1.y() + deltaY));
						objectPoints.push_back(pf2);
						objectPoints.push_back(QPoint(pf1.x() + deltaX, pf1.y()));
					}

					newObject = new Object2D();

					newObject->setObjectType(Object2D::Rectangle); // typ
					newObject->setObjectName(QString("rectangle.%1").arg(objectsTally[Object2D::Rectangle])); // nazov
					newObject->setPenColor(globalPenColor); // farba pera
					newObject->setFillColor(globalFillColor); // farba vyplne
					newObject->setObjectPoints(objectPoints); // body stvoruholnika

					currentGeometryObjects->push_front(newObject);

					objectsTally[Object2D::Rectangle]++;

					//ui->comboBox_SelectObject->addItem(newObject->getObjectName());
					ui->comboBox_SelectObject->clear();
					for (int i = 0; i < currentGeometryObjects->size(); i++)
					{
						ui->comboBox_SelectObject->addItem((*currentGeometryObjects)[i]->getObjectName());
					}

					ui->pushButton_PenColorDialog->setStyleSheet(QString("background-color:%1").arg(newObject->getPenColor().name()));
					ui->pushButton_FillColorDialog->setStyleSheet(QString("background-color:%1").arg(newObject->getFillColor().name()));

					objectPoints.clear();

					drawObjects();
				}
			}
			else if (drawingObject == Object2D::Circumference)
			{
				if (objectPoints.size() == 2)
				{
					drawingEnabled = false;
					drawingObject = -1;

					newObject = new Object2D();

					newObject->setObjectType(Object2D::Circumference); // typ
					newObject->setObjectName(QString("circle.%1").arg(objectsTally[Object2D::Circumference])); // nazov
					newObject->setPenColor(globalPenColor); // farba pera
					newObject->setFillColor(globalFillColor); // farba vyplne
					newObject->setObjectPoints(objectPoints);

					currentGeometryObjects->push_front(newObject);

					objectsTally[Object2D::Circumference];

					//ui->comboBox_SelectObject->addItem(newObject->getObjectName());
					ui->comboBox_SelectObject->clear();
					for (int i = 0; i < currentGeometryObjects->size(); i++)
					{
						ui->comboBox_SelectObject->addItem((*currentGeometryObjects)[i]->getObjectName());
					}

					ui->pushButton_PenColorDialog->setStyleSheet(QString("background-color:%1").arg(newObject->getPenColor().name()));
					ui->pushButton_FillColorDialog->setStyleSheet(QString("background-color:%1").arg(newObject->getFillColor().name()));

					objectPoints.clear();

					drawObjects();
				}
			}
			else if (drawingObject == Object2D::Polygon)
			{
				if (objectPoints.size() > 1)
				{
					size = objectPoints.size();

					vw->drawLine(objectPoints[size - 2], objectPoints[size - 1], globalPenColor);
				}
			}
			else if (drawingObject == Object2D::BezierCurve)
			{
				vw->drawPoint(objectPoints[objectPoints.size() - 1].toPoint(), pointColor);
			}
 		}
		else if (e->button() == Qt::RightButton) // ukoncenie kreslenia
		{
			if (drawingObject == Object2D::Polygon && objectPoints.size() > 2)
			{
				drawingEnabled = false;
				drawingObject = -1;

				newObject = new Object2D();

				newObject->setObjectType(Object2D::Polygon); // typ
				newObject->setObjectName(QString("polygon.%1").arg(objectsTally[Object2D::Polygon])); // nazov
				newObject->setPenColor(globalPenColor); // farba pera
				newObject->setFillColor(globalFillColor); // farba vyplne
				newObject->setShouldFill(true);
				newObject->setObjectPoints(objectPoints); // body polygonu

				currentGeometryObjects->push_front(newObject);

				objectsTally[Object2D::Polygon]++;

				//ui->comboBox_SelectObject->addItem(newObject->getObjectName());
				ui->comboBox_SelectObject->clear();
				for (int i = 0; i < currentGeometryObjects->size(); i++)
				{
					ui->comboBox_SelectObject->addItem((*currentGeometryObjects)[i]->getObjectName());
				}

				ui->pushButton_PenColorDialog->setStyleSheet(QString("background-color:%1").arg(newObject->getPenColor().name()));
				ui->pushButton_FillColorDialog->setStyleSheet(QString("background-color:%1").arg(newObject->getFillColor().name()));

				objectPoints.clear();

				drawObjects();
			}
			else if (drawingObject == Object2D::BezierCurve && objectPoints.size() > 2)
			{
				drawingEnabled = false;
				drawingObject = -1;

				newObject = new Object2D();

				newObject->setObjectType(Object2D::BezierCurve); // typ
				newObject->setObjectName(QString("BezierCurve.%1").arg(objectsTally[Object2D::BezierCurve])); // nazov
				newObject->setPenColor(globalPenColor); // farba pera
				newObject->setObjectPoints(objectPoints); // body krivky

				currentGeometryObjects->push_front(newObject);

				objectsTally[Object2D::BezierCurve]++;

				//ui->comboBox_SelectObject->addItem(newObject->getObjectName());
				ui->comboBox_SelectObject->clear();
				for (int i = 0; i < currentGeometryObjects->size(); i++)
				{
					ui->comboBox_SelectObject->addItem((*currentGeometryObjects)[i]->getObjectName());
				}

				ui->pushButton_PenColorDialog->setStyleSheet(QString("background-color:%1").arg(newObject->getPenColor().name()));

				objectPoints.clear();

				drawObjects();
			}
		}
	}
	else // nejde sa kreslit, ale posuvat polygon
	{
		mousePosition[0] = e->pos();
	}
		
}
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	/*ViewerWidget* vw = getCurrentViewerWidget();

	if (e->button() == Qt::LeftButton && !drawingEnabled)
	{
		if (mousePosition[1] != e->pos())
		{
			mousePosition[1] = e->pos();

			int pX = mousePosition[1].x() - mousePosition[0].x();
			int pY = mousePosition[1].y() - mousePosition[0].y();

			for (int i = 0; i < objectPoints.size(); i++) // prepocitanie suradnic polygonu
			{
				// poznamka pre autora: [i] vracia modifikovatelny objekt, .at(i) vracia const objekt
				objectPoints[i].setX(objectPoints[i].x() + pX);
				objectPoints[i].setY(objectPoints[i].y() + pY);
			}

			//getCurrentViewerWidget()->clear();

		}
	}*/
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	ViewerWidget* vw = getCurrentViewerWidget();
	if (vw == nullptr)
		return;
	double deltaX = 0.0, deltaY = 0.0;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty()) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	if ((e->buttons() == Qt::LeftButton) && !drawingEnabled)
	{
		mousePosition[1] = e->pos();

		deltaX = (double)mousePosition[1].x() - (double)mousePosition[0].x();
		deltaY = (double)mousePosition[1].y() - (double)mousePosition[0].y();

		for (int i = 0; i < tempObjPoints->size(); i++)
		{
			(*tempObjPoints)[i].setX((*tempObjPoints)[i].x() + deltaX);
			(*tempObjPoints)[i].setY((*tempObjPoints)[i].y() + deltaY);
		}

		vw->clear();

		drawObjects();

		mousePosition[0] = mousePosition[1];
	}
}
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
	ViewerWidget* vw = getCurrentViewerWidget();
	double scaleFactorXY = 0.0, sX = 0.0, sY = 0.0;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || (ui->comboBox_SelectObject->currentIndex() == -1)) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	if (!drawingEnabled)
	{
		if (tempObjPoints->size() != 0)
		{
			sX = (*tempObjPoints)[0].x();
			sY = (*tempObjPoints)[0].y();

			if (wheelEvent->angleDelta().y() > 0)
				scaleFactorXY = 1.25;
			else if (wheelEvent->angleDelta().y() < 0)
				scaleFactorXY = 0.75;

			for (int i = 0; i < tempObjPoints->size(); i++)
			{
				(*tempObjPoints)[i].setX(sX + ((*tempObjPoints)[i].x() - sX) * scaleFactorXY);
				(*tempObjPoints)[i].setY(sY + ((*tempObjPoints)[i].y() - sY) * scaleFactorXY);
			}

			vw->clear();

			drawObjects();
		}
	}
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
void ImageViewer::openNewTabForImg(ViewerWidget* vW)
{
	QScrollArea* scrollArea = new QScrollArea;
	scrollArea->setWidget(vW);

	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	QString name = vW->getName();

	ui->tabWidget->addTab(scrollArea, name);
}
bool ImageViewer::openImage(QString filename)
{
	QFileInfo fi(filename);

	QString name = fi.baseName();
	openNewTabForImg(new ViewerWidget(name, QSize(0, 0)));
	ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);

	ViewerWidget* w = getCurrentViewerWidget();

	QImage loadedImg(filename);
	return w->setImage(loadedImg);
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();
	ViewerWidget* w = getCurrentViewerWidget();

	QImage* img = w->getImage();
	return img->save(filename, extension.toStdString().c_str());
}
void ImageViewer::clearImage()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	vw->clear();
	vw->deleteObjects();

	ui->comboBox_SelectObject->clear();
}
void ImageViewer::setBackgroundColor(QColor color)
{
	ViewerWidget* w = getCurrentViewerWidget();

	if (w != nullptr)
	{
		w->setBackgroundColor(color);
		w->clear();
	}
	else
		warningMessage("No image opened");
}

//Slots

//Tabs slots
void ImageViewer::on_tabWidget_tabCloseRequested(int tabId)
{
	ViewerWidget* vw = getViewerWidget(tabId);
	
	vw->deleteObjects();

	delete vw; //vW->~ViewerWidget();
	ui->tabWidget->removeTab(tabId);
	ui->comboBox_SelectObject->clear();
}
void ImageViewer::on_actionRename_triggered()
{
	if (!isImgOpened()) {
		msgBox.setText("No image is opened.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	ViewerWidget* w = getCurrentViewerWidget();
	bool ok;
	QString text = QInputDialog::getText(this, QString("Rename image"), tr("Image name:"), QLineEdit::Normal, w->getName(), &ok);
	if (ok && !text.trimmed().isEmpty())
	{
		w->setName(text);
		ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), text);
	}
}

void ImageViewer::on_tabWidget_currentChanged(int x)
{
	ViewerWidget* vw = getCurrentViewerWidget();
}

//Image slots
void ImageViewer::on_actionNew_triggered()
{
	newImgDialog = new NewImageDialog(this);
	connect(newImgDialog, SIGNAL(accepted()), this, SLOT(newImageAccepted()));
	newImgDialog->exec();
}
void ImageViewer::newImageAccepted()
{
	NewImageDialog* newImgDialog = static_cast<NewImageDialog*>(sender());

	int width = newImgDialog->getWidth();
	int height = newImgDialog->getHeight();
	QString name = newImgDialog->getName();
	openNewTabForImg(new ViewerWidget(name, QSize(width, height)));
	ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
}
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	if (!isImgOpened()) {
		msgBox.setText("No image to save.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	QString folder = settings.value("folder_img_save_path", "").toString();

	ViewerWidget* w = getCurrentViewerWidget();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder + "/" + w->getName(), fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

	if (!saveImage(fileName)) {
		msgBox.setText("Unable to save image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
	else {
		msgBox.setText(QString("File %1 saved.").arg(fileName));
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	if (!isImgOpened()) {
		msgBox.setText("No image is opened.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	clearImage();
}
void ImageViewer::on_actionSet_background_color_triggered()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	if (vw == nullptr)
		return;

	QColor backgroundColor = QColorDialog::getColor(vw->getBackgroundColor(), this, "Select color of background");
	if (backgroundColor.isValid()) {
		setBackgroundColor(backgroundColor);
	}
}

// Colors

// "local" colors
void ImageViewer::on_pushButton_PenColorDialog_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();

	if (currentGeometryObjects->isEmpty() || (ui->comboBox_SelectObject->currentIndex() == -1))
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];

	QColor chosenColor = QColorDialog::getColor(currentPenColor.name(), this, "Select pen color");

	if (chosenColor.isValid())
	{
		currentPenColor = chosenColor;

		selectedObject->setPenColor(chosenColor);
		ui->pushButton_PenColorDialog->setStyleSheet(QString("background-color:%1").arg(chosenColor.name()));

		vw->clear();

		drawObjects();
	}
}
void ImageViewer::on_pushButton_FillColorDialog_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();

	if (currentGeometryObjects->isEmpty() || (ui->comboBox_SelectObject->currentIndex() == -1))
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];

	QColor chosenColor = QColorDialog::getColor(currentPenColor.name(), this, "Select fill color");

	if (chosenColor.isValid())
	{
		currentFillColor = chosenColor;

		selectedObject->setFillColor(chosenColor);
		ui->pushButton_FillColorDialog->setStyleSheet(QString("background-color:%1").arg(chosenColor.name()));

		vw->clear();

		drawObjects();
	}
}

// global colors
void ImageViewer::on_actionSet_Global_Pen_Color_triggered()
{
	QColor chosenColor = QColorDialog::getColor(globalPenColor.name(), this, "Select global pen color");

	if (chosenColor.isValid())
		globalPenColor = chosenColor;
}
void ImageViewer::on_actionSet_Global_Fill_Color_triggered()
{
	QColor chosenColor = QColorDialog::getColor(globalFillColor.name(), this, "Select global fill color");

	if (chosenColor.isValid())
		globalFillColor = chosenColor;
}

// select shape to draw
void ImageViewer::on_actionLine_triggered()
{
	if (getCurrentViewerWidget() != nullptr)
	{
		drawingEnabled = true;
		drawingObject = Object2D::Line;

		if (!objectPoints.isEmpty())
			objectPoints.clear();
	}
}
void ImageViewer::on_actionRectangle_triggered()
{
	if (getCurrentViewerWidget() != nullptr)
	{
		drawingEnabled = true;
		drawingObject = Object2D::Rectangle;

		if (!objectPoints.isEmpty())
			objectPoints.clear();
	}
	
}
void ImageViewer::on_actionPolygon_triggered()
{
	if (getCurrentViewerWidget() != nullptr)
	{
		drawingEnabled = true;
		drawingObject = Object2D::Polygon;

		if (!objectPoints.isEmpty())
			objectPoints.clear();
	}
}
void ImageViewer::on_actionCircumference_triggered()
{
	if (getCurrentViewerWidget() != nullptr)
	{
		drawingEnabled = true;
		drawingObject = Object2D::Circumference;

		if (!objectPoints.isEmpty())
			objectPoints.clear();
	}
}
void ImageViewer::on_actionBezier_curve_triggered()
{
	if (getCurrentViewerWidget() != nullptr)
	{
		drawingEnabled = true;
		drawingObject = Object2D::BezierCurve;

		if (!objectPoints.isEmpty())
			objectPoints.clear();
	}
}

// Transformations
void ImageViewer::on_pushButton_Rotate_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || (ui->comboBox_SelectObject->currentIndex() == -1))
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];

	if (selectedObject->getObjectType() == Object2D::Circumference)
		return;

	tempObjPoints = selectedObject->getObjectPointsPointer();

	if (tempObjPoints->size() != 0)
	{
		double angle = ((double)ui->spinBox_Angle->value() / 180.0) * M_PI;
		double sX = (*tempObjPoints)[0].x();
		double sY = (*tempObjPoints)[0].y();
		double x = 0.0, y = 0.0;

		if (ui->spinBox_Angle->value() < 0) // clockwise
		{
			//qDebug() << "clockwise";
			for (int i = 1; i < tempObjPoints->size(); i++)
			{
				x = (*tempObjPoints)[i].x();
				y = (*tempObjPoints)[i].y();

				(*tempObjPoints)[i].setX((x - sX) * qCos(angle) + (y - sY) * qSin(angle) + sX);
				(*tempObjPoints)[i].setY(-(x - sX) * qSin(angle) + (y - sY) * qCos(angle) + sY);
			}
		}
		else if (ui->spinBox_Angle->value() > 0) // anti-clockwise
		{
			//qDebug() << "anti-clockwise";
			angle = 2 * M_PI - angle;
			for (int i = 1; i < tempObjPoints->size(); i++)
			{
				x = (*tempObjPoints)[i].x();
				y = (*tempObjPoints)[i].y();

				(*tempObjPoints)[i].setX((x - sX) * qCos(angle) - (y - sY) * qSin(angle) + sX);
				(*tempObjPoints)[i].setY((x - sX) * qSin(angle) + (y - sY) * qCos(angle) + sY);
			}
		}

		vw->clear();
		drawObjects();
	}
}
void ImageViewer::on_pushButton_Symmetry_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || (ui->comboBox_SelectObject->currentIndex() == -1))
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];

	if (selectedObject->getObjectType() == Object2D::Circumference || selectedObject->getObjectType() == Object2D::BezierCurve)
		return;

	tempObjPoints = selectedObject->getObjectPointsPointer();

	if (tempObjPoints->size() != 0)
	{
		// symetria polygonu cez usecku medzi prvym a druhym bodom
		// symetria usecky cez horizontalnu priamku prechadzajucu stredom usecky

		double u = static_cast<double>((*tempObjPoints)[1].x()) - (*tempObjPoints)[0].x();
		double v = static_cast<double>((*tempObjPoints)[1].y()) - (*tempObjPoints)[0].y();
		double a = v;
		double b = -u;
		double c = -a * (*tempObjPoints)[0].x() - b * (*tempObjPoints)[0].y();
		double x = 0.0, y = 0.0;
		int midPointX = qAbs((*tempObjPoints)[1].x() + (*tempObjPoints)[0].x()) / 2;
		int midPointY = qAbs((*tempObjPoints)[1].y() + (*tempObjPoints)[0].y()) / 2;
		int deltaY = 0;

		if (tempObjPoints->size() == 2) // usecka
		{
			deltaY = qAbs((*tempObjPoints)[0].y() - midPointY);

			if ((*tempObjPoints)[0].y() < midPointY)
			{
				(*tempObjPoints)[0].setY((*tempObjPoints)[0].y() + 2.0 * deltaY);
				(*tempObjPoints)[1].setY((*tempObjPoints)[1].y() - 2.0 * deltaY);
			}
			else if ((*tempObjPoints)[0].y() > midPointY)
			{
				(*tempObjPoints)[0].setY((*tempObjPoints)[0].y() - 2.0 * deltaY);
				(*tempObjPoints)[1].setY((*tempObjPoints)[1].y() + 2.0 * deltaY);
			}
		}
		else if (tempObjPoints->size() > 2) // polygon
		{
			for (int i = 2; i < tempObjPoints->size(); i++)
			{
				x = (*tempObjPoints)[i].x();
				y = (*tempObjPoints)[i].y();

				(*tempObjPoints)[i].setX(x - 2 * a * ((a * x + b * y + c) / (a * a + b * b)));
				(*tempObjPoints)[i].setY(y - 2 * b * ((a * x + b * y + c) / (a * a + b * b)));
			}
		}

		vw->clear();
		drawObjects();
	}
}

void ImageViewer::on_pushButton_MoveUp_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	double deltaY = 7.0;

	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || ui->comboBox_SelectObject->currentIndex() == -1) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	for (int i = 0; i < tempObjPoints->size(); i++)
	{
		(*tempObjPoints)[i].setY((*tempObjPoints)[i].y() - deltaY);
	}

	vw->clear();
	drawObjects();
}
void ImageViewer::on_pushButton_MoveDown_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	double deltaY = 7.0;

	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || ui->comboBox_SelectObject->currentIndex() == -1) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	for (int i = 0; i < tempObjPoints->size(); i++)
	{
		(*tempObjPoints)[i].setY((*tempObjPoints)[i].y() + deltaY);
	}

	vw->clear();
	drawObjects();
}
void ImageViewer::on_pushButton_MoveLeft_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	double deltaX = 7.0;

	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || ui->comboBox_SelectObject->currentIndex() == -1) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	for (int i = 0; i < tempObjPoints->size(); i++)
	{
		(*tempObjPoints)[i].setX((*tempObjPoints)[i].x() - deltaX);
	}

	vw->clear();
	drawObjects();
}
void ImageViewer::on_pushButton_MoveRight_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	double deltaX = 7.0;

	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || ui->comboBox_SelectObject->currentIndex() == -1) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	for (int i = 0; i < tempObjPoints->size(); i++)
	{
		(*tempObjPoints)[i].setX((*tempObjPoints)[i].x() + deltaX);
	}

	vw->clear();
	drawObjects();
}

void ImageViewer::on_pushButton_MoveUpLeft_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	double deltaX = 7.0, deltaY = 7.0;

	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || ui->comboBox_SelectObject->currentIndex() == -1) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	for (int i = 0; i < tempObjPoints->size(); i++)
	{
		(*tempObjPoints)[i].setX((*tempObjPoints)[i].x() - deltaX);
		(*tempObjPoints)[i].setY((*tempObjPoints)[i].y() - deltaY);
	}

	vw->clear();
	drawObjects();
}
void ImageViewer::on_pushButton_MoveUpRight_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	double deltaX = 7.0, deltaY = 7.0;

	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || ui->comboBox_SelectObject->currentIndex() == -1) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	for (int i = 0; i < tempObjPoints->size(); i++)
	{
		(*tempObjPoints)[i].setX((*tempObjPoints)[i].x() + deltaX);
		(*tempObjPoints)[i].setY((*tempObjPoints)[i].y() - deltaY);
	}

	vw->clear();
	drawObjects();
}
void ImageViewer::on_pushButton_MoveDownLeft_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	double deltaX = 7.0, deltaY = 7.0;

	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || ui->comboBox_SelectObject->currentIndex() == -1) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	for (int i = 0; i < tempObjPoints->size(); i++)
	{
		(*tempObjPoints)[i].setX((*tempObjPoints)[i].x() - deltaX);
		(*tempObjPoints)[i].setY((*tempObjPoints)[i].y() + deltaY);
	}

	vw->clear();
	drawObjects();
}
void ImageViewer::on_pushButton_MoveDownRight_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	double deltaX = 7.0, deltaY = 7.0;

	if (vw == nullptr)
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || ui->comboBox_SelectObject->currentIndex() == -1) // ak nie je nic nakreslene
		return;

	selectedObject = (*currentGeometryObjects)[ui->comboBox_SelectObject->currentIndex()];
	tempObjPoints = selectedObject->getObjectPointsPointer();

	for (int i = 0; i < tempObjPoints->size(); i++)
	{
		(*tempObjPoints)[i].setX((*tempObjPoints)[i].x() + deltaX);
		(*tempObjPoints)[i].setY((*tempObjPoints)[i].y() + deltaY);
	}

	vw->clear();
	drawObjects();
}

// Objects
void ImageViewer::on_comboBox_SelectObject_currentIndexChanged(int newIndex)
{
	if (newIndex == -1 || getCurrentViewerWidget() == nullptr)
		return;
	
	selectedObject = getCurrentViewerWidget()->getGeometryObjectsRef()[ui->comboBox_SelectObject->currentIndex()];

	if (selectedObject->getObjectType() == Object2D::Line || selectedObject->getObjectType() == Object2D::BezierCurve)
	{
		ui->label_FillColor->setVisible(false);
		ui->pushButton_FillColorDialog->setVisible(false);
		
		ui->label_FillObject->setVisible(false);
		ui->checkBox_FillObject->setVisible(false);

		ui->pushButton_PenColorDialog->setStyleSheet(QString("background-color:%1").arg(selectedObject->getPenColor().name()));
	}
	else
	{
		ui->label_FillColor->setVisible(true);
		ui->pushButton_FillColorDialog->setVisible(true);

		ui->label_FillObject->setVisible(true);
		ui->checkBox_FillObject->setVisible(true);

		if (selectedObject->fillObject())
			ui->checkBox_FillObject->setChecked(true);
		else
			ui->checkBox_FillObject->setChecked(false);

		ui->pushButton_PenColorDialog->setStyleSheet(QString("background-color:%1").arg(selectedObject->getPenColor().name()));
		ui->pushButton_FillColorDialog->setStyleSheet(QString("background-color:%1").arg(selectedObject->getFillColor().name()));
	}
}
void ImageViewer::on_pushButton_BringObjectUp_clicked()
{
	int currentIndex = ui->comboBox_SelectObject->currentIndex();
	Object2D* temp = nullptr;
	ViewerWidget* vw = getCurrentViewerWidget();
	if (vw == nullptr)
		return;

	if (currentIndex <= 0) // ak je mimo alebo je najvyssie
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || currentGeometryObjects == nullptr)
		return;

	temp = (*currentGeometryObjects)[currentIndex];
	(*currentGeometryObjects)[currentIndex] = (*currentGeometryObjects)[currentIndex - 1];
	(*currentGeometryObjects)[currentIndex - 1] = temp;

	ui->comboBox_SelectObject->clear();
	for (int i = 0; i < currentGeometryObjects->size(); i++)
	{
		ui->comboBox_SelectObject->addItem((*currentGeometryObjects)[i]->getObjectName());
	}
	ui->comboBox_SelectObject->setCurrentIndex(currentIndex - 1);

	vw->clear();
	drawObjects();
}
void ImageViewer::on_pushButton_BringObjectDown_clicked()
{
	int currentIndex = ui->comboBox_SelectObject->currentIndex();
	Object2D* temp = nullptr;
	ViewerWidget* vw = getCurrentViewerWidget();
	if (vw == nullptr)
		return;
	
	if (currentIndex == -1 || currentIndex == (ui->comboBox_SelectObject->count() - 1)) // ak je mimo alebo je najnizsie
		return;

	currentGeometryObjects = vw->getGeometryObjectsPointer();
	if (currentGeometryObjects->isEmpty() || currentGeometryObjects == nullptr)
		return;

	temp = (*currentGeometryObjects)[currentIndex];
	(*currentGeometryObjects)[currentIndex] = (*currentGeometryObjects)[currentIndex + 1];
	(*currentGeometryObjects)[currentIndex + 1] = temp;

	ui->comboBox_SelectObject->clear();
	for (int i = 0; i < currentGeometryObjects->size(); i++)
	{
		ui->comboBox_SelectObject->addItem((*currentGeometryObjects)[i]->getObjectName());
	}
	ui->comboBox_SelectObject->setCurrentIndex(currentIndex + 1);

	vw->clear();
	drawObjects();
}
void ImageViewer::on_checkBox_FillObject_clicked()
{
	ViewerWidget* vw = getCurrentViewerWidget();

	if (ui->comboBox_SelectObject->currentIndex() != -1)
	{
		selectedObject = vw->getGeometryObjectsRef()[ui->comboBox_SelectObject->currentIndex()];
	}
	else
		return;

	if (selectedObject != nullptr)
	{
		if (ui->checkBox_FillObject->isChecked())
		{
			selectedObject->setShouldFill(true);
		}
		else if (!ui->checkBox_FillObject->isChecked())
		{
			selectedObject->setShouldFill(false);
		}

		getCurrentViewerWidget()->clear();
		drawObjects();
	}
}

void ImageViewer::on_actionExport_triggered()
{
	ViewerWidget* vw = getCurrentViewerWidget();
	QVector<Object2D*>* tempVect = nullptr;
	QVector<QPointF>* tempVectPoints = nullptr;
	QString type = "", name = "", pen = "", fill = "", shouldFill = "", numOfPoints = "", points = "";
	QStringList pointsList;

	if (vw == nullptr)
		return;

	tempVect = vw->getGeometryObjectsPointer();

	QString fileName = QFileDialog::getSaveFileName(this, "Export File", vw->getName(), tr("th File (*.th);;All files (*.)"));

	if (fileName.isEmpty())
		return;

	QFile exportFile(fileName);

	if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		warningMessage("Error with opening file");
		return;
	}

	QTextStream toFile(&exportFile);

	toFile << "#TH File Format\n"; // hlavicka
	toFile << vw->getName() << "\n"; // nazov
	toFile << QString::number(vw->getImgWidth()) << "\n"; // rozmery
	toFile << QString::number(vw->getImgHeight()) << "\n";
	toFile << vw->getBackgroundColor().name() << "\n"; // farba pozadia

	if (tempVect->isEmpty())
	{
		toFile << 0;
		exportFile.close();
		infoMessage("File exported");
	}
	else
	{
		toFile << tempVect->size() << "\n"; // pocet objektov

		for (int i = 0; i < tempVect->size(); i++)
		{
			type = QString::number((*tempVect)[i]->getObjectType()); // typ objektu
			toFile << type << "|";
			name = (*tempVect)[i]->getObjectName(); // nazov objektu
			toFile << name << "|";
			pen = (*tempVect)[i]->getPenColor().name(); // farba pera
			toFile << pen << "|";
			fill = (*tempVect)[i]->getFillColor().name(); // farba vyplne
			toFile << fill << "|";
			if ((*tempVect)[i]->fillObject()) // ci sa ma objekt vyplnat
				shouldFill = QString::number(1); // ano
			else
				shouldFill = QString::number(0); // nie
			toFile << shouldFill << "|";

			tempVectPoints = (*tempVect)[i]->getObjectPointsPointer();
			numOfPoints = QString::number(tempVectPoints->size()); // pocet vrcholov
			toFile << numOfPoints << "|";

			pointsList.clear();
			for (int j = 0; j < tempVectPoints->size(); j++) // body sa daju do QString listu
			{
				//pointsList.push_back(QString("%1,%2").arg(QString::number((*tempVectPoints)[j].x(), 'f', 3)).arg(QString::number((*tempVectPoints)[j].y(), 'f', 3)));
				
				toFile << QString("%1,%2").arg(QString::number((*tempVectPoints)[j].x(), 'f', 3)).arg(QString::number((*tempVectPoints)[j].y(), 'f', 3)) << ";";
			}

			toFile << "\n";

			//points = pointsList.join(";"); // toto spravi z listu jeden string, kde budu body oddelene cez ";"

			//toFile << type << "|" << name << "|" << pen << "|" << fill << "|" << shouldFill << "|" << numOfPoints << "|" << points << "\n";
		}

		exportFile.close();
		infoMessage("File exported");
	}
}
void ImageViewer::on_actionImport_triggered()
{
	ViewerWidget* vw = nullptr;
	Object2D* newObject = nullptr;

	QVector<Object2D*> tempObjects;
	QVector<QPointF> tempObjectPoints;

	QPointF tempPointF(0.0, 0.0);
	QString name = "", penColor = "", fillColor = "", backgroundColor = "", tempString = "", objName = "";
	int height = 0, width = 0, type = 0, tempInt = 0, numOfObj = 0, numOfPoints = 0;
	double x = 0.0, y = 0.0;
	bool shouldFill = false;

	QString fileName = QFileDialog::getOpenFileName(this, "Import File", "", tr("th File (*.th)"));
	if (fileName.isEmpty())
	{
		warningMessage("Empty fileName");
		return;
	}

	QFile importFile(fileName);
	if (!importFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		warningMessage("Error with opening file");
		return;
	}

	QTextStream fromFile(&importFile);

	if (fromFile.readLine() != QString("#TH File Format"))
	{
		warningMessage("Incorrect file");
		importFile.close();
		return;
	}

	name = fromFile.readLine(); // nazov obrazka
	width = fromFile.readLine().toInt(); // rozmery
	height = fromFile.readLine().toInt();
	backgroundColor = fromFile.readLine(); // farba pozadia
	numOfObj = fromFile.readLine().toInt(); // pocet objektov

	qDebug() << name << width << height << backgroundColor << numOfObj;

	for (int i = 0; i < numOfObj; i++) // postupne precitanie objektov
	{
		tempString = fromFile.readLine();
		qDebug() << "tempString->" << tempString;

		type = tempString.split("|").at(0).toInt(); // typ objektu
		objName = tempString.split("|").at(1);
		penColor = tempString.split("|").at(2);
		fillColor = tempString.split("|").at(3);
		tempInt = tempString.split("|").at(4).toInt();
		
		if (tempInt == 0)
			shouldFill = false;
		else if (tempInt == 1)
			shouldFill = true;

		numOfPoints = tempString.split("|").at(5).toInt();

		tempString = tempString.split("|").at(6); // tu uz su v tempString iba body
		qDebug() << "iba body->" << tempString;

		for (int j = 0; j < numOfPoints; j++)
		{
			x = tempString.split(";").at(j).split(",").at(0).toDouble();
			y = tempString.split(";").at(j).split(",").at(1).toDouble();

			qDebug() << "\nx:" << x << "\ny:" << y;

			tempObjectPoints.push_back(QPointF(x, y));
		}

		newObject = new Object2D;
		newObject->setObjectType(type); // typ objektu
		newObject->setObjectName(objName); // nazov
		newObject->setPenColor(QColor(penColor)); // farba pera
		newObject->setFillColor(QColor(fillColor)); // farba vyplne
		newObject->setShouldFill(shouldFill); // ci treba vyplnat
		newObject->setObjectPoints(&tempObjectPoints); // body objektu

		tempObjects.push_back(newObject);
	}

	importFile.close();
	infoMessage("import done");

	openNewTabForImg(new ViewerWidget(name, QSize(width, height)));
	ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
	vw = getCurrentViewerWidget();

	vw->setBackgroundColor(QColor(backgroundColor)); // farba pozadia
	vw->setGeometryObjects(tempObjects);

	currentGeometryObjects = vw->getGeometryObjectsPointer();

	ui->comboBox_SelectObject->clear();
	for (int i = 0; i < currentGeometryObjects->size(); i++)
	{
		ui->comboBox_SelectObject->addItem((*currentGeometryObjects)[i]->getObjectName());
	}

	vw->clear();
	drawObjects();
}
