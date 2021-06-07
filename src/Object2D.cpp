#include "Object2D.h"

Object2D::Object2D(int objectType, QString objectName, bool shouldFill)
{
	this->objectType = objectType;
	this->shouldFill = shouldFill;
	this->objectName = objectName;
}

QVector<QPoint> Object2D::getObjectPointsQPoint()
{
	QVector<QPoint> QPointPolygonPoints;
	QPoint tempQPoint(-1, -1);

	if (!objectPoints.isEmpty())
	{
		for (int i = 0; i < objectPoints.size(); i++)
		{
			tempQPoint.setX(static_cast<int>(objectPoints[i].x() + 0.5));
			tempQPoint.setY(static_cast<int>(objectPoints[i].y() + 0.5));

			QPointPolygonPoints.push_back(tempQPoint);
		}
	}

	return QPointPolygonPoints;
}
