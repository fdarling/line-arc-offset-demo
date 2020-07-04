#include "Geometry.h"
#include "GeometryQt.h"

// TODO stop using Qt in this file!
#include <QPointF>
#include <QLineF>
// #include <QVector2D>

namespace LineArcGeometry {

Point Segment::midPoint() const
{
    if (!isArc)
    {
        return (line.p1 + line.p2)/2.0;
    }
    const QPointF qpt1(PointToQPointF(line.p1));
    const QPointF qpt2(PointToQPointF(line.p2));
    const QPointF qptc(PointToQPointF(center));
    const double radius = QLineF(qptc, qpt2).length();
    const QPointF offsetN = (QLineF(QPointF(), qpt2 - qpt1).normalVector().unitVector()).p2()*radius;
    const double scalar = (orientation == LineArcGeometry::Segment::Clockwise) ? -1.0 : 1.0;
    const QPointF qpt3(qptc + scalar*offsetN);
    return Point(qpt3.x(), qpt3.y());
}

Contour ContourFromLineAndRadius(const Line &line, double radius)
{
    Contour contour;

    const QPointF startPoint(PointToQPointF(line.p1));
    const QPointF   endPoint(PointToQPointF(line.p2));
    const QPointF offsetN = (QLineF(QPointF(), endPoint - startPoint).normalVector().unitVector()).p2()*radius;

    {
        const Point pt1(startPoint.x()-offsetN.x(), startPoint.y()-offsetN.y());
        const Point pt2(startPoint.x()+offsetN.x(), startPoint.y()+offsetN.y());
        contour.segments.push_back(Segment(Line(pt1, pt2), line.p1, Segment::CounterClockwise));
    }
    {
        const Point pt1(startPoint.x()+offsetN.x(), startPoint.y()+offsetN.y());
        const Point pt2(  endPoint.x()+offsetN.x(),   endPoint.y()+offsetN.y());
        contour.segments.push_back(Line(pt1, pt2));
    }
    {
        const Point pt1(  endPoint.x()+offsetN.x(),   endPoint.y()+offsetN.y());
        const Point pt2(  endPoint.x()-offsetN.x(),   endPoint.y()-offsetN.y());
        contour.segments.push_back(Segment(Line(pt1, pt2), line.p2, Segment::CounterClockwise));
    }
    {
        const Point pt1(  endPoint.x()-offsetN.x(),   endPoint.y()-offsetN.y());
        const Point pt2(startPoint.x()-offsetN.x(), startPoint.y()-offsetN.y());
        contour.segments.push_back(Line(pt1, pt2));
    }

    return contour;
}

} // namespace LineArcGeometry
