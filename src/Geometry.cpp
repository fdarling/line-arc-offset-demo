#include "Geometry.h"
#include "GeometryQt.h"

// TODO stop using Qt in this file!
#include <QPointF>
#include <QLineF>
// #include <QVector2D>
#include <QDebug>

#include <algorithm>
#include <utility>

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

typedef std::pair<bool, bool> BoolPair;
typedef std::pair<LineArcGeometry::CoordinateType, BoolPair> RatedCombination;

static RatedCombination MakeRatedCombination(const Line &prev, const Line &curr, bool reversePrev, bool reverseCurr)
{
    const Line gapLine(reversePrev ? prev.p1 : prev.p2, reverseCurr ? curr.p2 : curr.p1);
    return RatedCombination(gapLine.length(), BoolPair(reversePrev, reverseCurr));
}

static bool RatedCombinationCompare(const RatedCombination &a, RatedCombination &b)
{
    return a.first < b.first;
}

void Contour::fixSegmentOrientations()
{
    if (segments.size() < 2)
        return;
    RatedCombination combinations[4];
    const int combinationLen = sizeof(combinations)/sizeof(combinations[0]);
    Segment *prev = &segments.back();
    for (std::list<Segment>::iterator it = segments.begin(); it != segments.end(); prev = &*it, ++it)
    {
        // weigh the possibilities for reversing the segments
        for (int i = 0; i < combinationLen; i++)
        {
            combinations[i] = MakeRatedCombination(prev->line, it->line, i & 1,i & 2);
        }

        // pick the combination of reversals resulting in the least gap between the end of "prev" and the start of "curr"
        RatedCombination * const best = std::min_element(combinations, combinations + combinationLen, RatedCombinationCompare);

        // perform the appropriate reversals
        if (best->second.first)
        {
            *prev = prev->reversed();
        }
        if (best->second.second)
        {
            *it = it->reversed();
        }
    }
    if (!isValid())
    {
        qDebug() << "WARNING: Contour::fixSegmentOrientation() wasn't completely successful";
    }
}

void Contour::fixSegmentEndpoints()
{
    if (segments.size() < 2)
        return;
    Segment *prev = &segments.back();
    for (std::list<Segment>::iterator it = segments.begin(); it != segments.end(); prev = &*it, ++it)
    {
        if (Line(prev->line.p2, it->line.p1).length() < 0.0001)
        {
            if (it->isArc)
            {
                it->line.p1 = prev->line.p2;
            }
            else
            {
                prev->line.p2 = it->line.p1;
            }
        }
    }
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

Segment::Orientation LinePointOrientation(const Line &line, const Point &pt)
{
    const CoordinateType ldx = line.p2.x - line.p1.x;
    const CoordinateType ldy = line.p2.y - line.p1.y;
    const CoordinateType pdx = pt.x - line.p1.x;
    const CoordinateType pdy = pt.y - line.p1.y;
    const CoordinateType det = pdx*ldy - pdy*ldx;
    return det > 0.0 ? Segment::Clockwise : Segment::CounterClockwise;
}

} // namespace LineArcGeometry
