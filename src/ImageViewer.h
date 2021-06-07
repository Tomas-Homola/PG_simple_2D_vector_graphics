#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include <ui_ImageViewer.h>
#include "ViewerWidget.h"
#include "NewImageDialog.h"

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);

private:
	Ui::ImageViewerClass* ui;
	NewImageDialog* newImgDialog;

	QSettings settings;
	QMessageBox msgBox;

	// zadanie stuff
	QColor globalPenColor;
	QColor globalFillColor;
	QColor currentPenColor;
	QColor currentFillColor;
	QColor pointColor = QColor("#1F75FE");
	QColor currentBackgroundColor;

	QVector<QPointF> objectPoints;
	QVector<QPointF>* tempObjPoints = nullptr;
	QVector<Object2D*>* currentGeometryObjects = nullptr;
	Object2D* selectedObject = nullptr;

	QPoint mousePosition[2];

	bool drawingEnabled = false;
	int drawingObject = -1;
	int objectsTally[5] = { 0,0,0,0,0 };

	void infoMessage(QString message);
	void warningMessage(QString message);
	void printPoints(QVector<QPoint> geometryPoints);

	void drawObjects();


	//ViewerWidget functions
	ViewerWidget* getViewerWidget(int tabId);
	ViewerWidget* getCurrentViewerWidget();

	//Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	//ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event);

	//ImageViewer Events
	void closeEvent(QCloseEvent* event);

	//Image functions
	void openNewTabForImg(ViewerWidget* vW);
	bool openImage(QString filename);
	bool saveImage(QString filename);
	void clearImage();
	void setBackgroundColor(QColor color);

	//Inline functions
	inline bool isImgOpened() { return ui->tabWidget->count() == 0 ? false : true; }

private slots:
	//Tabs slots
	void on_tabWidget_tabCloseRequested(int tabId);
	void on_actionRename_triggered();
	void on_tabWidget_currentChanged(int x);

	//Image slots
	void on_actionNew_triggered();
	void newImageAccepted();
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionSet_background_color_triggered();

	// Colors
	void on_pushButton_PenColorDialog_clicked();
	void on_pushButton_FillColorDialog_clicked();
	void on_actionSet_Global_Pen_Color_triggered();
	void on_actionSet_Global_Fill_Color_triggered();

	// Drawing objects
	void on_actionLine_triggered();
	void on_actionRectangle_triggered();
	void on_actionPolygon_triggered();
	void on_actionCircumference_triggered();
	void on_actionBezier_curve_triggered();

	// Transformations
	void on_pushButton_Rotate_clicked();
	void on_pushButton_Symmetry_clicked();

	void on_pushButton_MoveUp_clicked();
	void on_pushButton_MoveDown_clicked();
	void on_pushButton_MoveLeft_clicked();
	void on_pushButton_MoveRight_clicked();

	void on_pushButton_MoveUpLeft_clicked();
	void on_pushButton_MoveUpRight_clicked();
	void on_pushButton_MoveDownLeft_clicked();
	void on_pushButton_MoveDownRight_clicked();

	// Objects
	void on_comboBox_SelectObject_currentIndexChanged(int newIndex);
	void on_pushButton_BringObjectUp_clicked();
	void on_pushButton_BringObjectDown_clicked();
	void on_checkBox_FillObject_clicked();

	// Export/Import
	void on_actionExport_triggered();
	void on_actionImport_triggered();
};
