#pragma once
#include <QtWidgets>
#include <QVector>
class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr;
	uchar* data = nullptr;

	//line storage
	QVector<QPoint> lineStarts;
	QVector<QPoint> lineEnds;
	QVector<QColor> lineColors;
	QVector<int> lineAlgTypes;
	bool storeLines = true;

	bool drawLineActivated = false;
	QPoint drawLineBegin = QPoint(0, 0);

	bool drawCircleActivated = false;
	QPoint drawCircleCenter = QPoint(0, 0);

	//Polygon storage
	bool drawPolygonActivated = false;
	QVector<QPoint> PolygonPoints;
	QPoint lastPolygonPoint = QPoint(0, 0);
	bool polygonClosed = false; 

	//Moving 
	bool isDraggingPolygon = false;
	QPoint lastMousePos = QPoint(0, 0);	

	bool isDraggingLine = false;
	int draggedLineIndex = -1;

	bool isFilled = false;

	//triangle storage
	QVector<QPoint> trianglePoints;
	QVector<QColor> triangleColors;
	bool doneTriangle = false;
	
	//Curves
	QVector<QPoint> controlPoints;
	int CurveType = 0;
	bool isCurveDone = false;

	//Cube
	std::vector<QVector3D> vertexList;


public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);


	void setPixel(int x, int y, int r, int g, int b, int a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y);

	//Draw functions
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }
	void setDrawLineActivated(bool state) { drawLineActivated = state; }
	bool getDrawLineActivated() { return drawLineActivated; }
	void setStoreLines(bool val) { storeLines = val; }

	//Circle functions 
	void drawCircle(QPoint center, int radius, QColor color);
	void setDrawCircleCenter(QPoint center) { drawCircleCenter = center; }
	QPoint getDrawCircleCenter() { return drawCircleCenter; }
	void setDrawCircleActivated(bool state) { drawCircleActivated = state; }
	bool getDrawCircleActivated() { return drawCircleActivated; }

	//Polygon functions
	void setDrawPolygonActivated(bool state) { drawPolygonActivated = state; }
	bool getDrawPolygonActivated() { return drawPolygonActivated; }
	void addPolygonPoint(QPoint point, QColor color, int algType);
	void drawPolygon(QVector<QPoint>& points, QColor color, int algType);
	void closePolygon(QColor color,int algType);
	const QVector<QPoint>& getPolygonPoints() const { return PolygonPoints; }
	QVector<QPoint>& getPolygonPointsRef() { return PolygonPoints; }
	bool isPolygonClosed() { return polygonClosed; }
	void redrawPolygon(QColor color, int algType);
	bool getIsFilled() { return isFilled; };
	void setIsFilled(bool state) { isFilled = state; }

	//triangle functions
	void drawFilledTriangle(QColor color,int mode);
	void addTrianglePoint(QPoint point, QColor color);
	bool getDoneTriangle() { return doneTriangle; }
	void setDoneTriangle(bool state) { doneTriangle = state; };
	QColor getBarycentricColor(int x, int y);
	QColor getNearestColor(int x, int y);

	//moving objects functions 
	
	bool getIsDragging() { return isDraggingPolygon; }
	void setIsDragging(bool state) { isDraggingPolygon = state; }
	QPoint getLastMousePos() { return lastMousePos; }
	void setLastMousePos(QPoint pos) { lastMousePos = pos; }

	//line functions
	void moveLine(int lineIndex, QPoint delta, QColor color, int algType);
	int getLineCount() const { return lineStarts.size(); }
	void startDraggingLine(int index, QPoint pos) {isDraggingLine = true; draggedLineIndex = index; lastMousePos = pos;}
	void stopDraggingLine() {isDraggingLine = false; draggedLineIndex = -1; }
	bool getIsDraggingLine() { return isDraggingLine; }
	int getDraggedLineIndex() { return draggedLineIndex; }
	QPoint getLineStart(int i) const { return lineStarts[i]; }
	QPoint getLineEnd(int i) const { return lineEnds[i]; }

	//rotation/scaling/reflection/clip
	//polygon
	void scalePolygon(double scaleX,double scaleY, QColor color, int algType);	
	void rotatePolygon(double degrees, QColor color, int algType);
	void reflectPolygon(int edgeIndex, QColor color, int algType);	
	bool isInside(QPoint p, int edge, int limit);
	QPoint getIntersect(QPoint S, QPoint V, int edge, int limit);
	QVector<QPoint> getSutherlandHodgmanClipped(const QVector<QPoint>& points, int xMin, int xMax, int yMin, int yMax);

	//line
	void scaleLine(int index, double scaleX, double scaleY);
	void rotateLine(int index, double degrees);
	void reflectLine(int index, bool horizontal);
	void clipLineCyrusBeck(int index, int xMin, int xMax, int yMin, int yMax);
	void drawClippedLine(QPoint p1, QPoint p2, int xMin, int xMax, int yMin, int yMax, QColor color, int alg);

	// fill functions
	void fillPolygonScanline(QColor color, int mode);

	// Curves
	
	void drawControlPoints(QColor color);
	void setCurveType(int type) { CurveType = type; }
	void setIsCurveDone(bool state) { isCurveDone = state; }
	bool getIsCurveDone() { return isCurveDone; }
	void addControlPoints(QPoint point, QColor color);
	void drawHermite(int segments,QColor color);
	QPoint deCasteljau(QVector<QPoint> pts, double t);
	void drawBezier(int segments,QColor color);
	double B0(double t);
	double B1(double t);
	double B2(double t);
	double B3(double t);
	void drawBSpline(int segments, QColor color);
	void drawCurves(QColor color, int CurveType);

	// Cube
	
	std::vector<QVector3D> createCubeVerticles(double size);

	///////

	uchar* getData() { return data; }
	void setDataPtr() { data = img ? img->bits() : nullptr; }

	int getImgWidth() { return img ? img->width() : 0; };
	int getImgHeight() { return img ? img->height() : 0; };

	void clear();
	void clearImageOnly() { if (img) img->fill(Qt::white); update(); }

	//Algorithms
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);
	void drawCircleBresenham(QPoint center, int radius, QColor color);

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};