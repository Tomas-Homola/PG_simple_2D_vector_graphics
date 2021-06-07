#pragma once
#include <QtWidgets>

class Object2D
{
private:
	int objectType = -1;
	QString objectName = "";

	QVector<QPointF> objectPoints;

	QColor penColor = QColor("#000000");
	QColor fillColor = QColor("#FFFFFF");
	
	bool shouldFill = false;

public:
	Object2D() {}
	Object2D(int objectType, QString objectName, bool shouldFill = false);

	enum ObjectType
	{
		Line = 0, Rectangle = 1, Polygon = 2, Circumference = 3, BezierCurve = 4
	};

	// set functions
	void setObjectType(int newObjectType) { objectType = newObjectType; }
	void setObjectPoints(QVector<QPointF> newObjectPoints) { objectPoints = newObjectPoints; }
	void setObjectName(QString newObjectName) { objectName = newObjectName; }
	void setShouldFill(bool shouldFill) { this->shouldFill = shouldFill; }
	void setObjectPoints(QVector<QPointF>* newObjectPoints) { objectPoints = (*newObjectPoints); }
	
	// colors
	void setPenColor(QColor newPenColor) { penColor = newPenColor; }
	void setPenColor(int red, int green, int blue) { penColor = QColor(red, green, blue); }
	void setPenColor(double red, double green, double blue) { penColor = QColor(red, green, blue); }
	
	void setFillColor(QColor newFillColor) { fillColor = newFillColor; }
	void setFillColor(int red, int green, int blue) { fillColor = QColor(red, green, blue); }
	void setFillColor(double red, double green, double blue) { fillColor = QColor(red, green, blue); }

	
	// get functions
	int getObjectType() { return objectType; }
	bool fillObject() { return shouldFill; }
	QString getObjectName() { return objectName; }

	QColor getPenColor() { return penColor; }
	QColor getFillColor() { return fillColor; }

	QVector<QPointF>* getObjectPointsPointer() { return &objectPoints; }
	QVector<QPointF>& getObjectPointsRef() { return objectPoints; }
	QVector<QPoint> getObjectPointsQPoint();
};