#include   "ViewerWidget.h"
#include <cmath>
#include <vector>
#include <algorithm>

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete img;
	img = nullptr;
	data = nullptr;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img) {
		delete img;
		img = nullptr;
		data = nullptr;
	}
	img = new QImage(inputImg.convertToFormat(QImage::Format_ARGB32));
	if (!img || img->isNull()) {
		return false;
	}
	resizeWidget(img->size());
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img || img->isNull()) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, int r, int g, int b, int a)
{
	if (!img || !data) return;
	if (!isInside(x, y)) return;

	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(b);
	data[startbyte + 1] = static_cast<uchar>(g);
	data[startbyte + 2] = static_cast<uchar>(r);
	data[startbyte + 3] = static_cast<uchar>(a);
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	setPixel(x, y, static_cast<int>(255 * valR + 0.5), static_cast<int>(255 * valG + 0.5), static_cast<int>(255 * valB + 0.5), static_cast<int>(255 * valA + 0.5));
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		setPixel(x, y, color.red(), color.green(), color.blue(), color.alpha());
	}
}

bool ViewerWidget::isInside(int x, int y)
{
	return img && x >= 0 && y >= 0 && x < img->width() && y < img->height();
}

//Draw functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	if (!img || !data) return;

	if (storeLines) {
		lineStarts.push_back(start);
		lineEnds.push_back(end);
		lineColors.push_back(color);
		lineAlgTypes.push_back(algType);
	}

	if (algType == 0) {
		drawLineDDA(start, end, color);
	}
	else {
		drawLineBresenham(start, end, color);
	}
	update();
}

void ViewerWidget::drawCircle(QPoint center, int radius, QColor color)
{
	if (!img || !data) return;
	drawCircleBresenham(center, radius, color);
	update();
}

void ViewerWidget::addPolygonPoint(QPoint point,QColor color, int algType)
{
	if (!img || !data) return;
	if (polygonClosed) return; 


	if (PolygonPoints.isEmpty()) {
		PolygonPoints.push_back(point);
		lastPolygonPoint = point;
		
		setPixel(point.x(), point.y(), color);
	}
	else {
		drawLine(lastPolygonPoint, point, color, algType);
		PolygonPoints.push_back(point);
		lastPolygonPoint = point;
	}
	update();
}

void ViewerWidget::closePolygon(QColor color, int algType)
{
	if (!img || !data) return;
	if (polygonClosed) return;
	if (PolygonPoints.size() < 3) return; 

	drawLine(PolygonPoints.last(), PolygonPoints.first(), color, algType);

	polygonClosed = true;
	drawPolygonActivated = false;
	update();
}

void ViewerWidget::drawFilledTriangle(QColor color, int mode)
{
	if (trianglePoints.size() != 3 || triangleColors.size() != 3)
		return;

	if (trianglePoints.size() == 3)
	{
		PolygonPoints.clear();
		for (const auto& p : trianglePoints) {
			PolygonPoints.push_back(p);
		}
		fillPolygonScanline(color, mode);
	}
}

void ViewerWidget::addTrianglePoint(QPoint point, QColor color)
{
	if (trianglePoints.size() < 3)
	{
		trianglePoints.push_back(point);
		QColor tempColor;
		if (trianglePoints.size() == 1)
			tempColor = Qt::red;
		else if (trianglePoints.size() == 2)
			tempColor = Qt::green;
		else
			tempColor = Qt::blue;
		triangleColors.push_back(tempColor);
		setPixel(point.x(), point.y(), tempColor);
	}

	if (trianglePoints.size() == 2)
	{
		drawLine(trianglePoints[0], trianglePoints[1], triangleColors[1], 0);
	}
	else if (trianglePoints.size() == 3)
	{
		drawLine(trianglePoints[1], trianglePoints[2], triangleColors[2], 0);
		drawLine(trianglePoints[2], trianglePoints[0], triangleColors[0], 0);
		setDoneTriangle(true);
	}

	update();
}

QColor ViewerWidget::getBarycentricColor(int x, int y)
{
	QPoint P(x, y);
	QPoint A = trianglePoints[0];
	QPoint B = trianglePoints[1];
	QPoint C = trianglePoints[2];

	double areaABC = 0.5 * std::abs(A.x() * (B.y() - C.y()) + B.x() * (C.y() - A.y()) + C.x() * (A.y() - B.y()));
	if (areaABC == 0) return triangleColors[0];

	double w1 = 0.5 * std::abs(P.x() * (B.y() - C.y()) + B.x() * (C.y() - P.y()) + C.x() * (P.y() - B.y())) / areaABC;
	double w2 = 0.5 * std::abs(A.x() * (P.y() - C.y()) + P.x() * (C.y() - A.y()) + C.x() * (A.y() - P.y())) / areaABC;
	double w3 = 1.0 - w1 - w2;

	int r = qBound(0, (int)(w1 * triangleColors[0].red() + w2 * triangleColors[1].red() + w3 * triangleColors[2].red()), 255);
	int g = qBound(0, (int)(w1 * triangleColors[0].green() + w2 * triangleColors[1].green() + w3 * triangleColors[2].green()), 255);
	int b = qBound(0, (int)(w1 * triangleColors[0].blue() + w2 * triangleColors[1].blue() + w3 * triangleColors[2].blue()), 255);

	return QColor(r, g, b);
}

QColor ViewerWidget::getNearestColor(int x, int y)
{
	double d0 = std::pow(x - trianglePoints[0].x(), 2) + std::pow(y - trianglePoints[0].y(), 2);
	double d1 = std::pow(x - trianglePoints[1].x(), 2) + std::pow(y - trianglePoints[1].y(), 2);
	double d2 = std::pow(x - trianglePoints[2].x(), 2) + std::pow(y - trianglePoints[2].y(), 2);

	if (d0 <= d1 && d0 <= d2) return triangleColors[0];
	if (d1 <= d0 && d1 <= d2) return triangleColors[1];
	return triangleColors[2];
}

void ViewerWidget::redrawPolygon(QColor color, int algType)
{
	if (!img) return;
	img->fill(Qt::white); 
	if (PolygonPoints.size() < 2) return; 

	for (int i = 0; i < PolygonPoints.size() - 1; ++i) {
		drawLine(PolygonPoints[i], PolygonPoints[i + 1], color, algType);
	}
	if (polygonClosed) {
		drawLine(PolygonPoints.last(), PolygonPoints.first(), color, algType);
	}

	if (isFilled)
	{
		fillPolygonScanline(color, 0);
	}
	update();


}

void ViewerWidget::moveLine(int lineIndex, QPoint delta, QColor color, int algType)
{
	if (lineIndex < 0 || lineIndex >= lineStarts.size()) return;

	// Move endpoints
	lineStarts[lineIndex] += delta;
	lineEnds[lineIndex] += delta;

	// Redraw
	clearImageOnly();

	setStoreLines(false);

	for (int i = 0; i < lineStarts.size(); i++) {
		drawLine(lineStarts[i], lineEnds[i], lineColors[i], lineAlgTypes[i]);
	}

	setStoreLines(true);
	update();
}

void ViewerWidget::scalePolygon(double scaleX, double scaleY, QColor color, int algType)
{
	if (PolygonPoints.size() < 2) return;
	QPoint center = PolygonPoints.first();
	for (int i = 1; i < PolygonPoints.size(); ++i) {
		int x = PolygonPoints[i].x();
		int y = PolygonPoints[i].y();
		int newX = static_cast<int>((x - center.x()) * scaleX + center.x());
		int newY = static_cast<int>((y - center.y()) * scaleY + center.y());
		PolygonPoints[i] = QPoint(newX, newY);
	}
	redrawPolygon(color, algType);
	if (isFilled)
	{
		fillPolygonScanline(color, 0);
	}
	update();
}

void ViewerWidget::scaleLine(int index, double scaleX, double scaleY)
{
	if (index < 0 || index >= lineStarts.size()) return;

	QPoint center = lineStarts[index];
	QPoint p = lineEnds[index];

	int newX = static_cast<int>((p.x() - center.x()) * scaleX + center.x());
	int newY = static_cast<int>((p.y() - center.y()) * scaleY + center.y());

	lineEnds[index] = QPoint(newX, newY);

	// redraw
	clearImageOnly();
	setStoreLines(false);

	for (int i = 0; i < lineStarts.size(); i++) {
		drawLine(lineStarts[i], lineEnds[i], lineColors[i], lineAlgTypes[i]);
	}

	setStoreLines(true);
	update();
}

void ViewerWidget::rotatePolygon(double degrees, QColor color, int algType)
{
	if (PolygonPoints.size() < 2) return;

	QPoint center = PolygonPoints.first();
	double rad = degrees * (M_PI / 180.0);
	double cosA = std::cos(rad);
	double sinA = std::sin(rad);

	for (int i = 1; i < PolygonPoints.size(); ++i) {
		int x = PolygonPoints[i].x();
		int y = PolygonPoints[i].y();

		int newX = static_cast<int>((x - center.x()) * cosA - (y - center.y()) * sinA + center.x());
		int newY = static_cast<int>((x - center.x()) * sinA + (y - center.y()) * cosA + center.y());

		PolygonPoints[i] = QPoint(newX, newY);
	}
	redrawPolygon(color, algType);

	if (isFilled) {
		fillPolygonScanline(color, 0);
	}

	update();
}

void ViewerWidget::rotateLine(int index, double degrees)
{
	if (index < 0 || index >= lineStarts.size()) return;

	QPoint center = lineStarts[index];
	QPoint p = lineEnds[index];

	double rad = degrees * (M_PI / 180.0);
	double cosA = std::cos(rad);
	double sinA = std::sin(rad);

	int newX = static_cast<int>((p.x() - center.x()) * cosA - (p.y() - center.y()) * sinA + center.x());
	int newY = static_cast<int>((p.x() - center.x()) * sinA + (p.y() - center.y()) * cosA + center.y());

	lineEnds[index] = QPoint(newX, newY);

	// redraw
	clearImageOnly();
	setStoreLines(false);

	for (int i = 0; i < lineStarts.size(); i++) {
		drawLine(lineStarts[i], lineEnds[i], lineColors[i], lineAlgTypes[i]);
	}

	setStoreLines(true);
	update();
}

void ViewerWidget::reflectPolygon(int edgeIndex, QColor color, int algType)
{
	if (PolygonPoints.size() < 3) return;

	QPoint A = PolygonPoints[edgeIndex % PolygonPoints.size()];
	QPoint B = PolygonPoints[(edgeIndex + 1) % PolygonPoints.size()];

	int u = B.x() - A.x();
	int v = B.y() - A.y();

	double a = v;
	double b = -u;
	double c = -a * A.x() - b * A.y();

	double denom = a * a + b * b;
	if (denom == 0) return;

	for (int i = 0; i < PolygonPoints.size(); ++i) {
		int x = PolygonPoints[i].x();
		int y = PolygonPoints[i].y();
		double commonFactor  = (a * x + b * y + c)/denom;
		int newX = static_cast<int>(x - 2 * a * commonFactor);
		int newY = static_cast<int>(y - 2 * b * commonFactor);

		PolygonPoints[i] = QPoint(newX, newY);
	}
	redrawPolygon(color, algType);
	if (isFilled) {
		fillPolygonScanline(color, 0);
	}

	update();
}

bool ViewerWidget::isInside(QPoint p, int edge, int limit)
{
	switch (edge) {
	case 0: return p.x() >= limit; // LEFT
	case 1: return p.x() <= limit; // RIGHT
	case 2: return p.y() >= limit; // TOP
	case 3: return p.y() <= limit; // BOTTOM
	}
	return false;
}

QPoint ViewerWidget::getIntersect(QPoint S, QPoint V, int edge, int limit)
{
	double t;
	if (edge == 0 || edge == 1) { // Vertikálne hrany
		t = double(limit - S.x()) / (V.x() - S.x());
		return QPoint(limit, std::round(S.y() + t * (V.y() - S.y())));
	}
	else { // Horizontálne hrany
		t = double(limit - S.y()) / (V.y() - S.y());
		return QPoint(std::round(S.x() + t * (V.x() - S.x())), limit);
	}
}



void ViewerWidget::reflectLine(int index, bool horizontal)
{
	if (index < 0 || index >= lineStarts.size()) return;

	QPoint p1 = lineStarts[index];
	QPoint p2 = lineEnds[index];

	if (horizontal) {
		// horizontal reflection
		int newY = 2 * p1.y() - p2.y();
		p2.setY(newY);
	}
	else {
		// vertical reflection
		int newX = 2 * p1.x() - p2.x();
		p2.setX(newX);
	}

	lineEnds[index] = p2;
	clearImageOnly();
	setStoreLines(false);

	for (int i = 0; i < lineStarts.size(); i++) {
		drawLine(lineStarts[i], lineEnds[i], lineColors[i], lineAlgTypes[i]);
	}

	setStoreLines(true);
	update();
}

void ViewerWidget::clipLineCyrusBeck(int index, int xMin, int xMax, int yMin, int yMax)
{
	if (index < 0 || index >= lineStarts.size()) return;

	QPoint p1 = lineStarts[index];
	QPoint p2 = lineEnds[index];

	double tE = 0.0;
	double tL = 1.0;

	double dx = p2.x() - p1.x();
	double dy = p2.y() - p1.y();

	int nx[] = { -1, 1, 0, 0 };
	int ny[] = { 0, 0, -1, 1 };

	int px[] = { xMin, xMax, 0, 0 };
	int py[] = { 0, 0, yMin, yMax };

	for (int i = 0; i < 4; i++) {
		double DdotN = dx * nx[i] + dy * ny[i];
		double WdotN = (p1.x() - px[i]) * nx[i] + (p1.y() - py[i]) * ny[i];

		if (DdotN != 0) {
			double t = -WdotN / DdotN;

			if (DdotN > 0)
				tE = std::max(tE, t);
			else
				tL = std::min(tL, t);
		}
		else if (WdotN > 0) {
			return;
		}
	}

	if (tE <= tL) {
		QPoint newStart(
			p1.x() + tE * dx,
			p1.y() + tE * dy
		);

		QPoint newEnd(
			p1.x() + tL * dx,
			p1.y() + tL * dy
		);

		lineStarts[index] = newStart;
		lineEnds[index] = newEnd;
	}
	else {
		lineStarts.remove(index);
		lineEnds.remove(index);
		lineColors.remove(index);
		lineAlgTypes.remove(index);
	}

	// redraw
	clearImageOnly();
	setStoreLines(false);

	for (int i = 0; i < lineStarts.size(); i++) {
		drawLine(lineStarts[i], lineEnds[i], lineColors[i], lineAlgTypes[i]);
	}

	setStoreLines(true);
	update();
}



void ViewerWidget::drawPolygon(QVector<QPoint>&points, QColor color, int algType)
{
	if (points.size() < 2) return;
	for (int i = 0; i < points.size(); i++) {
		this->drawLine(points[i], points[(i + 1) % points.size()], color, algType);
	}
}

void ViewerWidget::fillPolygonScanline(QColor color, int mode)
{
	if (PolygonPoints.size() < 3) return;

	int n = PolygonPoints.size();

	int yMin = PolygonPoints[0].y();
	int yMax = PolygonPoints[0].y();

	for (const QPoint& p : PolygonPoints) {
		yMin = std::min(yMin, p.y());
		yMax = std::max(yMax, p.y());
	}

	for (int y = yMin; y <= yMax; y++) {
		std::vector<double> intersections;
		for (int i = 0; i < n; i++) {
			QPoint p1 = PolygonPoints[i];
			QPoint p2 = PolygonPoints[(i + 1) % n];

			if (p1.y() == p2.y()) continue;

			if (p1.y() > p2.y()) std::swap(p1, p2);

			if (y >= p1.y() && y < p2.y()) {
				double x = p1.x() + (double)(y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y());
				intersections.push_back(x);
			}
		}

		std::sort(intersections.begin(), intersections.end());

		for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
			int xStart = (int)std::ceil(intersections[i]);
			int xEnd = (int)std::floor(intersections[i + 1]);

			for (int x = xStart; x <= xEnd; x++) {
				if (getDoneTriangle() == true && trianglePoints.size() == 3)
				{
					if (mode == 0) {
						setPixel(x, y, getNearestColor(x, y));
					}
					else {
						setPixel(x, y, getBarycentricColor(x, y));
					}
				}
				else {
					setPixel(x, y, color);
				}
			}
		}
	}
	setIsFilled(true);
	update();
}

void ViewerWidget::drawControlPoints(QColor color)
{
	for (const QPoint& p : controlPoints) {
				setPixel(p.x(), p.y(), color);
	}
}

void ViewerWidget::addControlPoints(QPoint point, QColor color)
{
	if (!img || !data) return;
	if (isCurveDone) return;
	controlPoints.push_back(point);

	setPixel(point.x(), point.y(), color);
	update();
}

void ViewerWidget::drawHermite(int segments,QColor color)
{
	if (controlPoints.size() < 2) return;

	double dt = 1.0 / segments;

	for (int i = 1; i < controlPoints.size(); i++) {

		QPoint P0 = controlPoints[i - 1];
		QPoint P1 = controlPoints[i];

		QPoint T0(150, 0);
		QPoint T1(150, 0);

		QPoint Q0 = P0;

		for (double t = dt; t <= 1.0; t += dt) {

			double h1 = 2 * t * t * t - 3 * t * t + 1;
			double h2 = -2 * t * t * t + 3 * t * t;
			double h3 = t * t * t - 2 * t * t + t;
			double h4 = t * t * t - t * t;

			QPoint Q1(
				std::round(P0.x() * h1 + P1.x() * h2 + T0.x() * h3 + T1.x() * h4),
				std::round(P0.y() * h1 + P1.y() * h2 + T0.y() * h3 + T1.y() * h4)
			);

			drawLine(Q0, Q1, color, 1);
			Q0 = Q1;
		}

		//tangents
		drawLine(P0, P0 + T0, Qt::red, 1);
		drawLine(P1, P1 + T1, Qt::red, 1);
	}
}

QPoint ViewerWidget::deCasteljau(QVector<QPoint> pts, double t)
{
	for (int k = 1; k < pts.size(); k++) {
		for (int i = 0; i < pts.size() - k; i++) {
			pts[i].setX((1 - t) * pts[i].x() + t * pts[i + 1].x());
			pts[i].setY((1 - t) * pts[i].y() + t * pts[i + 1].y());
		}
	}
	return pts[0];
}

void ViewerWidget::drawBezier(int segments, QColor color)
{
	if (controlPoints.size() < 2) return;

	double dt = 1.0 / segments;
	QPoint Q0 = controlPoints[0];

	for (double t = dt; t <= 1.0; t += dt) {
		QPoint Q1 = deCasteljau(controlPoints, t);
		drawLine(Q0, Q1, color, 1);
		Q0 = Q1;
	}
}

double ViewerWidget::B0(double t)
{
	 return  (1.0 - t) * (1.0 - t) * (1.0 - t) / 6.0;
}

double ViewerWidget::B1(double t)
{
	return (3.0 * t * t * t - 6.0 * t * t + 4.0) / 6.0;
}

double ViewerWidget::B2(double t)
{
	return ( - 3.0 * t * t * t + 3.0 * t * t + 3.0 * t + 1.0) / 6.0;
}

double ViewerWidget::B3(double t)
{
	return t * t * t / 6.0;
}

void ViewerWidget::drawBSpline(int segments, QColor color)
{
	if (controlPoints.size() < 4) return;

	double dt = 1.0 / segments;

	for (int i = 3; i < controlPoints.size(); i++) {

		double t_start = 0.0;
		double startX = B0(t_start) * controlPoints[i - 3].x() +
			B1(t_start) * controlPoints[i - 2].x() +
			B2(t_start) * controlPoints[i - 1].x() +
			B3(t_start) * controlPoints[i].x();
		double startY = B0(t_start) * controlPoints[i - 3].y() +
			B1(t_start) * controlPoints[i - 2].y() +
			B2(t_start) * controlPoints[i - 1].y() +
			B3(t_start) * controlPoints[i].y();

		QPoint Q0(std::round(startX), std::round(startY));

		for (double t = dt; t <= 1.0; t += dt) {

			QPoint Q1(
				std::round(
					B0(t) * controlPoints[i - 3].x() +
					B1(t) * controlPoints[i - 2].x() +
					B2(t) * controlPoints[i - 1].x() +
					B3(t) * controlPoints[i].x()
				),
				std::round(
					B0(t) * controlPoints[i - 3].y() +
					B1(t) * controlPoints[i - 2].y() +
					B2(t) * controlPoints[i - 1].y() +
					B3(t) * controlPoints[i].y()
				)
			);

			drawLine(Q0, Q1, color, 1);
			Q0 = Q1;
		}
	}
}

void ViewerWidget::drawCurves(QColor color, int CurveType)
{
	drawControlPoints(color);

	if (!isCurveDone) return;

	if (CurveType == 0) {
		drawHermite(100,color);
	}
	else if (CurveType == 1) {
		drawBezier(100,color);
	}
	else if (CurveType == 2) {
		drawBSpline(100,color);
	}
}

std::vector<QVector3D> ViewerWidget::createCubeVerticles(double size)
{
	std::vector<QVector3D> V;

	V.push_back(QVector3D(0, 0, 0));
	V.push_back(QVector3D(size, 0, 0));
	V.push_back(QVector3D(size, size, 0));
	V.push_back(QVector3D(0, size, 0));
	V.push_back(QVector3D(0, 0, size));	
	V.push_back(QVector3D(size, 0, size));
	V.push_back(QVector3D(size, size, size));
	V.push_back(QVector3D(0, size, size));
	
	return V ;
}

QVector<QPoint> ViewerWidget::getSutherlandHodgmanClipped(
	const QVector<QPoint>& input,
	int xMin, int xMax, int yMin, int yMax)
{
	QVector<QPoint> out = input;

	int limits[] = { xMin, xMax, yMin, yMax };

	for (int edge = 0; edge < 4; edge++) {
		if (out.isEmpty()) break;

		QVector<QPoint> nextIn = out;
		out.clear();

		QPoint S = nextIn.last();

		for (const QPoint& V : nextIn) {
			bool Sin = isInside(S, edge, limits[edge]);
			bool Vin = isInside(V, edge, limits[edge]);

			if (Vin) {
				if (!Sin) {
					// Case 2: S je von, V je dnu 
					out.append(getIntersect(S, V, edge, limits[edge]));
				}
				// Case 1 & 2: V je dnu 
				out.append(V);
			}
			else if (Sin) {
				// Case 3: S je dnu, V je von
				out.append(getIntersect(S, V, edge, limits[edge]));
			}
			// Case 4: Obaja sú von 

			S = V;
		}
	}

	// Cleanup
	QVector<QPoint> clean;
	for (int i = 0; i < out.size(); i++) {
		if (i == 0 || out[i] != out[i - 1]) {
			clean.append(out[i]);
		}
	}
	if (clean.size() > 1 && clean.first() == clean.last()) {
		clean.pop_back();
	}

	return clean;
}

void ViewerWidget::drawClippedLine(QPoint p1, QPoint p2, int xMin, int xMax, int yMin, int yMax, QColor color, int alg)
{
	double tIn = 0.0, tOut = 1.0;
	double dx = p2.x() - p1.x();
	double dy = p2.y() - p1.y();

	int nx[] = { -1, 1, 0, 0 };
	int ny[] = { 0, 0, -1, 1 };
	int bVals[] = { xMin, xMax, yMin, yMax };

	for (int i = 0; i < 4; i++) {
		double DdotN = dx * nx[i] + dy * ny[i];
		double WdotN = (p1.x() - (i < 2 ? bVals[i] : 0)) * nx[i] + (p1.y() - (i >= 2 ? bVals[i] : 0)) * ny[i];

		if (DdotN != 0) {
			double t = -WdotN / DdotN;
			if (DdotN > 0) tIn = std::max(tIn, t);
			else tOut = std::min(tOut, t);
		}
		else if (WdotN > 0) return;
	}

	if (tIn <= tOut) {
		QPoint start(p1.x() + tIn * dx, p1.y() + tIn * dy);
		QPoint end(p1.x() + tOut * dx, p1.y() + tOut * dy);
		this->drawLine(start, end, color, alg);
	}
}



void ViewerWidget::clear()
{
	if (!img) return;
	img->fill(Qt::white);
	PolygonPoints.clear();
	lastPolygonPoint = QPoint(0, 0);

	lineStarts.clear();
	lineEnds.clear();
	lineColors.clear();
	lineAlgTypes.clear();

	polygonClosed = false;
	drawPolygonActivated = false;
	isFilled = false;
	trianglePoints.clear();
	triangleColors.clear();
	doneTriangle = false;

	controlPoints.clear();  
	isCurveDone = false;

	update();
}

void ViewerWidget::drawLineDDA(QPoint start, QPoint end, QColor color)
{
	double dx = end.x() - start.x();
	double dy = end.y() - start.y();

	double steps = std::max(std::abs(dx), std::abs(dy));

	if (steps == 0) {
		setPixel(start.x(), start.y(), color);
		return;
	}

	double xInc = dx / steps;
	double yInc = dy / steps;

	double x = start.x();
	double y = start.y();

	for (int i = 0; i <= steps; i++) {
		setPixel(std::round(x), std::round(y), color);
		x += xInc;
		y += yInc;
	}
}

void ViewerWidget::drawLineBresenham(QPoint start, QPoint end, QColor color)
{
	int x1 = start.x();
	int y1 = start.y();
	int x2 = end.x();
	int y2 = end.y();

	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);

	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;

	int err = dx - dy;

	while (true)
	{
		setPixel(x1, y1, color);

		if (x1 == x2 && y1 == y2)
			break;

		int e2 = 2 * err;

		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}

		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}
	}
}


void ViewerWidget::drawCircleBresenham(QPoint center, int radius, QColor color)
{
	int xc = center.x();
	int yc = center.y();

	int x = 0;
	int y = radius;
	int p = 1 - radius;
	int dvaX = 3;
	int dvaY = 2 * radius - 2;

	while (x <= y)
	{
		setPixel(xc - y, yc + x, color);
		setPixel(xc + x, yc - y, color);
		setPixel(xc + x, yc + y, color);
		setPixel(xc + y, yc + x, color);
		setPixel(xc + y, yc - x, color);
		setPixel(xc - x, yc + y, color);
		setPixel(xc - x, yc - y, color);
		setPixel(xc - y, yc - x, color);

		if (p > 0)
		{
			p = p - dvaY;
			y--;
			dvaY = dvaY - 2;
		}
		else
		{
			p = p + dvaX;
			dvaX = dvaX + 2;
			x++;
		}

		
	}
}
//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}