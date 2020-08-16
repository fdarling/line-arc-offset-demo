#include "GeometryScene.h"
#include "GeometryQt.h"

namespace LineArcOffsetDemo {

static void AddContourToPath(QPainterPath &path, const LineArcGeometry::Contour &contour)
{
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        // const bool doMoveTo = true;
        const bool doMoveTo = (it == contour.segments.begin());
        QPointF pt1(PointToQPointF(it->line.p1));
        QPointF pt2(PointToQPointF(it->line.p2));
        if (it->isArc)
        {
            const QPointF circleCenter(PointToQPointF(it->center));
            const double radius = QLineF(circleCenter, pt2).length();
            const QRectF  circleRect(it->center.x - radius, it->center.y - radius, radius*2.0, radius*2.0);
            QPointF  startPoint = pt1;
            QPointF    endPoint = pt2;
            const bool isCCW = (it->orientation != LineArcGeometry::Segment::Clockwise);
            const double startAngle = QLineF(circleCenter, startPoint).angle();
            // const double   endAngle = QLineF(circleCenter,   endPoint).angle();
            const double sweepAngle = QLineF(circleCenter, startPoint).angleTo(QLineF(circleCenter, endPoint));
            const double lineLength = QLineF(startPoint, endPoint).length();
            if (lineLength < 0.0001) // HACK, this is to fix some sort of rounding errors with .angle()...
            {
                if (doMoveTo) path.moveTo(startPoint);
                path.lineTo(endPoint);
            }
            else
            {
                if (doMoveTo) path.arcMoveTo(circleRect, startAngle);
                path.arcTo(circleRect, startAngle, isCCW ? (sweepAngle - 360.0) : sweepAngle);
            }
        }
        else
        {
            if (doMoveTo) path.moveTo(pt1);
            path.lineTo(pt2);
        }
    }
}

GeometryScene::GeometryScene(QObject *parent) : QGraphicsScene(parent)
{
}

void GeometryScene::addShape(const LineArcGeometry::Shape &shape, const QPen &pen, const QBrush &brush)
{
    QPainterPath path;
    AddContourToPath(path, shape.boundary);
    for (std::list<LineArcGeometry::Contour>::const_iterator it = shape.holes.begin(); it != shape.holes.end(); ++it)
    {
        AddContourToPath(path, *it);
    }
    addPath(path, pen, brush);
}

void GeometryScene::addMultiShape(const LineArcGeometry::MultiShape &multiShape, const QPen &pen, const QBrush &brush)
{
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        addShape(*it, pen, brush);
    }
}

} // namespace LineArcOffsetDemo
