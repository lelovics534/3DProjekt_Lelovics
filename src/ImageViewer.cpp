#include <cmath>
#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);

	ui->toolButtonDrawCircle->setCheckable(true);
	ui->toolButtonDrawPolygon->setCheckable(true);
	ui->toolButtonDrawLine->setCheckable(true);
	ui->toolButtonLineMove->setCheckable(true);
	ui->toolButtonDrawTriangle->setCheckable(true);
	ui->toolButtonDrawCurves->setCheckable(true);

	vW = new ViewerWidget(QSize(500, 500), ui->scrollArea);
	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(false);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::blue;
	QString style_sheet = QString("background-color: %1;").arg(globalColor.name(QColor::HexRgb));
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return QMainWindow::eventFilter(obj, event);
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
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawPolygon->isChecked() && w->isPolygonClosed()) {
		w->setIsDragging(true);
		w->setLastMousePos(e->pos());
		return; 
	}

	else if (e->button() == Qt::LeftButton && ui->toolButtonLineMove->isChecked())
	{
		int index = ui->spinLineIndex->value();

		if (index >= 0 && index < w->getLineCount()) {
			w->startDraggingLine(index, e->pos());
			return;
		}
	}

	else if (e->button() == Qt::LeftButton && ui->toolButtonDrawTriangle->isChecked())
	{
		w->addTrianglePoint(e->pos(), globalColor);

		if (w->getDoneTriangle()) {
			int mode = ui->comboBoxInterpolation->currentIndex();
			w->drawFilledTriangle(globalColor,mode);
		}
	}

	else if (e->button() == Qt::LeftButton && ui->toolButtonDrawLine->isChecked())
	{
		if (w->getDrawLineActivated()) {
			w->drawLine(w->getDrawLineBegin(), e->pos(), globalColor, ui->comboBoxLineAlg->currentIndex());
			ui->spinLineIndex->setMaximum(w->getLineCount() - 1);
			w->setDrawLineActivated(false);
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
 			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	else if (e->button() == Qt::LeftButton && ui->toolButtonDrawCircle->isChecked())
	{
		if(w->getDrawCircleActivated())
		{QPoint center = w->getDrawCircleCenter();
		int radius = static_cast<int>(std::sqrt(std::pow(e->pos().x() - center.x(), 2) + std::pow(e->pos().y() - center.y(), 2)));
			w->drawCircle(center, radius, globalColor);
			w->setDrawCircleActivated(false);
		}
		else {
			w->setDrawCircleCenter(e->pos());
			w->setDrawCircleActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	else if (e->button() == Qt::LeftButton && ui->toolButtonDrawPolygon->isChecked())
	{
		
		w->addPolygonPoint(e->pos(), globalColor, ui->comboBoxLineAlg->currentIndex());
	}
	else if (e->button() == Qt::RightButton && ui->toolButtonDrawPolygon->isChecked())
	{
		w->closePolygon(globalColor, ui->comboBoxLineAlg->currentIndex());
		ui->spinEdgeIndex->setMaximum(w->getPolygonPoints().size());
	}
	else if ( ui->toolButtonDrawCurves->isChecked() ) {
		if (e->button() == Qt::LeftButton) {
			if (!w->getIsCurveDone()) {
				w->addControlPoints(e->pos(), globalColor);
			}
		}
		else if (e->button() == Qt::RightButton) {
			w->setIsCurveDone(true);
		}
			w->clearImageOnly();
			w->drawCurves(globalColor,ui-> comboBoxCurves->currentIndex());
			w->update();
		
		return;
	}
}

void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	
	if (e->button() == Qt::LeftButton) {
		w->setIsDragging(false); // polygon
		w->stopDraggingLine(); // line
	}
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (w->getIsDraggingLine() && ui->toolButtonLineMove->isChecked()) {
		QPoint currentPos = e->pos();
		QPoint delta = currentPos - w->getLastMousePos();
		w->setLastMousePos(currentPos);

		w->moveLine(w->getDraggedLineIndex(), delta, globalColor, ui->comboBoxLineAlg->currentIndex());

		return;
	}

	if (w->getIsDragging()) {
		QPoint currentPos = e->pos();
		QPoint delta = currentPos - w->getLastMousePos();
		w->setLastMousePos(currentPos);

		QVector<QPoint>& points = w->getPolygonPointsRef();

		// Move polygon
		for (int i = 0; i < points.size(); i++) {
			points[i] += delta;
		}

		int xMin = 0;
		int yMin = 0;
		int xMax = w->width() - 1;
		int yMax = w->height() - 1;

		QVector<QPoint> polygonCopy = points;

		QVector<QPoint> clipped = w->getSutherlandHodgmanClipped(
			polygonCopy, xMin, xMax, yMin, yMax
		);

		QVector<QPoint> toDraw;

		if (clipped.size() >= 2) {
			toDraw = clipped;
		}
		else {
			toDraw = polygonCopy;
		}

		QVector<QPoint> original = w->getPolygonPointsRef();

		// Clear image
		w->clearImageOnly();

		w->getPolygonPointsRef() = toDraw;

		w->redrawPolygon(globalColor, ui->comboBoxLineAlg->currentIndex());
		w->getPolygonPointsRef() = original;
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

	double factor = 1.0;
	if (wheelEvent->angleDelta().y() > 0) {
		factor = 1.25; // Zoom in
	}
	else if (wheelEvent->angleDelta().y() < 0) {
		factor = 0.75; // Zoom out
	}
	if (factor == 1.0) {
		return;
	}

	if (ui->radioPolygon->isChecked()) {
		if (w->isPolygonClosed()) {
			w->scalePolygon(factor, factor, globalColor, ui->comboBoxLineAlg->currentIndex());
		}
	}
	else {
		int index = ui->spinLineIndex->value();

		if (index >= 0 && index < w->getLineCount()) {
			w->scaleLine(index, factor, factor);
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
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//Slots
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
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
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	vW->clear();
}
void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

void ImageViewer::on_toolButtonDrawPolygon_clicked()
{
	ui->toolButtonDrawLine->setChecked(false);
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawCurves->setChecked(false);
	ui->toolButtonDrawTriangle->setChecked(false);

	// Activate polygon 
	vW->setDrawPolygonActivated(true);
}

void ImageViewer::on_toolButtonDrawLine_clicked()
{
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonDrawCurves->setChecked(false);
	ui->toolButtonDrawTriangle->setChecked(false);
}

void ImageViewer::on_toolButtonLineMove_clicked()
{
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonDrawCurves->setChecked(false);
	ui->toolButtonDrawTriangle->setChecked(false);
}

void ImageViewer::on_toolButtonDrawCircle_clicked()
{
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonDrawCurves->setChecked(false);
	ui->toolButtonDrawTriangle->setChecked(false);
	ui->toolButtonDrawLine->setChecked(false);
}

void ImageViewer::on_toolButtonDrawTriangle_clicked()
{
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonDrawCurves->setChecked(false);
	ui->toolButtonDrawLine->setChecked(false);
}

void ImageViewer::on_toolButtonDrawCurves_clicked()
{
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonDrawLine->setChecked(false);
	ui->toolButtonDrawTriangle->setChecked(false);

}

void ImageViewer::on_btnRotate_clicked()
{
	double angle = ui->spinAngle->value();

	if (ui->radioPolygon->isChecked()) {
		vW->rotatePolygon(angle, globalColor, ui->comboBoxLineAlg->currentIndex());
	}
	else if (ui->radioLine->isChecked()){
		int i = ui->spinLineIndex->value();
		vW->rotateLine(i, angle);
	}
	else return;
}

void ImageViewer::on_btnScale_clicked()
{
	double scaleX = ui->spinScaleX->value();
	double scaleY = ui->spinScaleY->value();
	if (ui->radioPolygon->isChecked()) {
		vW->scalePolygon(scaleX, scaleY, globalColor, ui->comboBoxLineAlg->currentIndex());
	}
	else if (ui->radioLine->isChecked()) {
		int i = ui->spinLineIndex->value();
		vW->scaleLine(i, scaleX, scaleY);
	}
	else
		return;
}

void ImageViewer::on_btnReflectPolygon_clicked()
{
	if (!ui->radioPolygon->isChecked()) {
		return;
	}
	int edgeIndex = ui->spinEdgeIndex->value(); 

	vW->reflectPolygon(
		edgeIndex,
		globalColor,
		ui->comboBoxLineAlg->currentIndex()
	);
}

void ImageViewer::on_btnHorizontal_clicked()
{
	if (ui->radioPolygon->isChecked()) {
		return;
	}
	else if (ui->radioLine->isChecked()) {
		vW->reflectLine(ui->spinLineIndex->value(), true);
	}
	else
		return;
}

void ImageViewer::on_btnVertical_clicked()
{
	if (ui->radioPolygon->isChecked()) {
		return;
	}
	else if (ui->radioLine->isChecked()) {
		vW->reflectLine(ui->spinLineIndex->value(), false);
	}
	else
		return;
}



void ImageViewer::on_btnFillPolygon_clicked()
{
	vW->fillPolygonScanline(globalColor,0);
}

void ImageViewer::on_btnFillTriangle_clicked()
{
	if (vW->getDoneTriangle())
	{
		vW->fillPolygonScanline(globalColor, ui->comboBoxInterpolation->currentIndex());
	}
}

void ImageViewer::on_btnCreateCube_clicked()
{
	double size = ui->doubleSpinBoxCubeLen->value();
	QString fileName = ui->lineEditVtkName->text();
	if (fileName.isEmpty())
	{
		QMessageBox::warning(this, "Error", "Please enter a file name for the cube.");
		return;
	}

	vW-> writeVTK(fileName.toStdString(), size);
	QMessageBox::information(this,"Success",QString("Cube created and saved"));

}


void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: %1;").arg(newColor.name(QColor::HexRgb));
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
	}
}


