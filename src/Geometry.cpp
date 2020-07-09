#include "Geometry.h"
#include "GeometryQt.h"

// TODO stop using Qt in this file!
#include <QPointF>
#include <QLineF>
// #include <QVector2D>
#include <QDebug>

namespace LineArcGeometry {

CoordinateType Line::length() const
{
    return QLineF(PointToQPointF(p1), PointToQPointF(p2)).length();
}

CoordinateType Segment::radius() const
{
    return QLineF(PointToQPointF(center), PointToQPointF(line.p2)).length();
}

Point Segment::midPoint() const
{
    if (!isArc)
    {
        return (line.p1 + line.p2)/2.0;
    }
    const QPointF qpt1(PointToQPointF(line.p1));
    const QPointF qpt2(PointToQPointF(line.p2));
    const QPointF qptc(PointToQPointF(center));
    const QPointF offsetN = (QLineF(QPointF(), qpt2 - qpt1).normalVector().unitVector()).p2()*radius();
    const double scalar = (orientation == LineArcGeometry::Segment::Clockwise) ? -1.0 : 1.0;
    const QPointF qpt3(qptc + scalar*offsetN);
    return Point(qpt3.x(), qpt3.y());
}

static QLineF LineToQLineF(const Line &line)
{
    return QLineF(PointToQPointF(line.p1), PointToQPointF(line.p2));
}

Segment::Orientation Contour::orientation() const
{
    if (segments.empty())
    {
        return Segment::Clockwise;
    }
    const Segment *prev = &segments.back();
    double totalAngle = 0.0;
    for (std::list<Segment>::const_iterator it = segments.begin(); it != segments.end(); prev = &*it, ++it)
    {
        const QLineF a = LineToQLineF(prev->line);
        const QLineF b = LineToQLineF(it->line);
        double angle = a.angleTo(b);
        if (qFuzzyCompare(angle, 180.0))
        {
            if (it->isArc)
            {
                angle = it->orientation == Segment::Clockwise ? 180.0 : -180.0;
            }
            else if (prev->isArc)
            {
                angle = prev->orientation == Segment::Clockwise ? 180.0 : -180.0;
            }
        }
        else if (angle > 180.0)
        {
            angle -= 360.0;
        }
        totalAngle += angle;
    }
    if (qFuzzyIsNull(totalAngle))
    {
        qDebug() << "WARNING: degenerate Contour without orientation:" << *this;
    }
    else if (totalAngle > 360.5 || totalAngle < -360.5)
    {
        qDebug() << "WARNING: non +/- 360 degree Contour!" << *this;
    }
    // TODO let Qt know that angleTo() gives a positive angle for clockwise not counter-clockwise when considering Y+ as upwards not downwards on a screen
    return totalAngle > 0.0 ? Segment::Clockwise : Segment::CounterClockwise;
}

template<typename T>
static bool FuzzyCompareWithNull(const T &a, const T &b)
{
    return qFuzzyCompare(a, b) || (qFuzzyIsNull(a) && qFuzzyIsNull(b));
}

static bool FuzzyComparePoints(const Point &a, const Point &b)
{
    return FuzzyCompareWithNull(a.x, b.x) && FuzzyCompareWithNull(a.y, b.y);
}

bool Contour::isValid() const
{
    orientation();
    // TODO possibly handle full circles formed by a single segment,
    // currently the segments aren't defined in that case
    if (segments.size() < 2)
    {
        return false;
    }
    const Segment *prev = &segments.back();
    for (std::list<Segment>::const_iterator it = segments.begin(); it != segments.end(); prev = &*it, ++it)
    {
        if (!FuzzyComparePoints(it->line.p1, prev->line.p2))
        {
            qDebug() << "INVALID:" << *prev << "to " << *it;
            return false;
        }
    }
    return true;
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
