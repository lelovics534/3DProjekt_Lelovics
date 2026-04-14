#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);
	~ImageViewer() { delete ui; }
private:
	Ui::ImageViewerClass* ui;
	ViewerWidget* vW;

	QColor globalColor;
	QSettings settings;
	QMessageBox msgBox;

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
	bool openImage(QString filename);
	bool saveImage(QString filename);

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();
	void on_toolButtonDrawPolygon_clicked();
	void on_toolButtonDrawLine_clicked();
	void on_toolButtonLineMove_clicked();
	void on_toolButtonDrawCircle_clicked();	
	void on_toolButtonDrawTriangle_clicked();
	void on_toolButtonDrawCurves_clicked();
	void on_btnRotate_clicked();
	void on_btnScale_clicked();
	void on_btnReflectPolygon_clicked();
	void on_btnHorizontal_clicked();
	void on_btnVertical_clicked();
	void on_btnFillPolygon_clicked();
	void on_btnFillTriangle_clicked();
	void on_btnCreateCube_clicked();


	//Tools slots
	void on_pushButtonSetColor_clicked();
};
