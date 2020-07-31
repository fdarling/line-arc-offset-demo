#ifndef LINEARCOFFSETDEMO_GEOMETRYQT_H
#define LINEARCOFFSETDEMO_GEOMETRYQT_H

#include "Geometry.h"

#include <QPointF>
#include <QLineF>

QT_BEGIN_NAMESPACE

class QDebug;

QDebug operator<<(QDebug debug, const LineArcGeometry::Point &pt);
QDebug operator<<(QDebug debug, const LineArcGeometry::Line &line);
QDebug operator<<(QDebug debug, const LineArcGeometry::Segment &segment);
QDebug operator<<(QDebug debug, const LineArcGeometry::Contour &contour);
QDebug operator<<(QDebug debug, const LineArcGeometry::Shape &shape);
QDebug operator<<(QDebug debug, const LineArcGeometry::MultiShape &multiShape);

QT_END_NAMESPACE

namespace LineArcGeometry {

QPointF PointToQPointF(const Point &pt);
QLineF LineToQLineF(const Line &line);
Point QPointFToPoint(const QPointF &pt);

} // namespace LineArcGeometry

#endif // LINEARCOFFSETDEMO_GEOMETRYQT_H
