#include "Geometry.h"
#include "GeometryQt.h"

// TODO stop using Qt in this file!
#include <QPointF>
#include <QLineF>
// #include <QVector2D>
#include <QDebug>

#include <cmath>
#include <algorithm>
#include <utility>
#include <cassert>

#define USE_AREA_METHOD

namespace LineArcGeometry {

static const CoordinateType TOLERANCE = 0.0001;

CoordinateType Line::length() const
{
    return QLineF(PointToQPointF(p1), PointToQPointF(p2)).length();
}

// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
// TODO test this function
CoordinateType Line::distanceTo(const Point &pt) const
{
    const CoordinateType len = length();
    if (len < TOLERANCE)
        return Line(p1, pt).length();
    const Point delta = p2 - p1;
    const CoordinateType distance = qAbs(delta.y*pt.x - delta.x*pt.y + p2.x*p1.y - p2.y*p1.x)/len;
    return distance;
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

#ifndef USE_AREA_METHOD
// https://stackoverflow.com/questions/14066933/direct-way-of-computing-clockwise-angle-between-2-vectors/16544330#16544330
static double GetAngleBetween(const Line &a, const Line &b)
{
    const Point da = (a.p2 - a.p1);
    const Point db = (b.p2 - b.p1);
    const double x1 = da.x;
    const double y1 = da.y;
    const double x2 = db.x;
    const double y2 = db.y;
    const double dot = x1*x2 + y1*y2;
    const double det = x1*y2 - y1*x2;
    const double angle = atan2(det, dot);
    return angle*180.0/M_PI;
}

static Line GetNormalVector(const Line &line)
{
    const Point delta = line.p2 - line.p1;
    return Line(line.p1, Point(delta.y, -delta.x));
}

// NOTE: not normalized (doesn't need to be for angle measurement)
static Line GetTangent(const Segment &segment, bool atEnd = true)
{
    if (qFuzzyIsNull(segment.line.length()))
    {
        return Line(segment.line.p2, segment.line.p2 + Point(1.0, 0.0));
    }
    if (segment.isArc)
    {
        const Point &pt1 = segment.center;
        const Point &pt2 = atEnd ? segment.line.p2 : segment.line.p1;
        const Line normalVector = GetNormalVector(Line(Point(), segment.orientation != Segment::Clockwise ? (pt1 - pt2) : (pt2 - pt1)));
        const Line tangent(pt2, pt2 + normalVector.p2);
        return tangent;
    }
    else
    {
        const Point &pt1 = segment.line.p1;
        const Point &pt2 = segment.line.p2;
        const Line tangent(pt2, pt2 + (pt2 - pt1));
        return tangent;
    }
}
#endif // USE_AREA_METHOD

#ifdef USE_AREA_METHOD
// https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order/1165943#1165943
static double AreaUnderneath(const Line &line)
{
    return (line.p2.x - line.p1.x)*(line.p2.y + line.p1.y);
}

static double AreaUnderneath(const Segment &segment)
{
    if (segment.isArc)
    {
        // TODO calculate area under arcs correctly, taking into X reversals
        const Point midPoint = segment.midPoint();
        return AreaUnderneath(Line(segment.line.p1, midPoint)) + AreaUnderneath(Line(midPoint, segment.line.p2));
    }
    return AreaUnderneath(segment.line);
}

static Segment::Orientation OrientationByArea(const Contour &contour)
{
    double totalArea = 0.0;
    for (std::list<Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        const double area = AreaUnderneath(*it);
        totalArea += area;
    }
    if (qFuzzyIsNull(totalArea))
    {
        qDebug() << "WARNING: zero area Contour, can't take orientation! Total area:" << totalArea;
    }
    return totalArea > 0.0 ? Segment::CounterClockwise : Segment::Clockwise;
}
#endif // USE_AREA_METHOD

Segment::Orientation Contour::orientation() const
{
#ifdef USE_AREA_METHOD
    const Segment::Orientation answer1 = OrientationByArea(*this);
    const Segment::Orientation answer2 = OrientationByArea(this->approximatedArcs());
    assert(answer1 == answer2);
    return answer1;
#else
    if (segments.empty())
    {
        return Segment::Clockwise;
    }
    const Segment *prev = &segments.back();
    double totalAngle = 0.0;
    for (std::list<Segment>::const_iterator it = segments.begin(); it != segments.end(); prev = &*it, ++it)
    {
        const Line a = GetTangent(*prev);
        const Line b = it->isArc ? GetTangent(*it, false) : a;
        const Line c = GetTangent(*it);
        double angle1 = GetAngleBetween(a, b);
        double angle2 = GetAngleBetween(b, c);
        if (qFuzzyCompare(qAbs(angle1), 180.0))
        {
            qDebug() << "WARNING: U-turn detected between adjacent Segments in Contour!";
            // assert(false);
        }
        if (it->isArc)
        {
            if (angle2 > 0.0 && it->orientation == Segment::Clockwise)
                angle2 -= 360.0;
            else if (angle2 < 0.0 && it->orientation != Segment::Clockwise)
                angle2 += 360.0;
        }
        const double angle = angle1 + angle2;
        totalAngle += angle;
    }
    if (qFuzzyIsNull(totalAngle))
    {
        qDebug() << "WARNING: degenerate Contour without orientation! Total angle:" << totalAngle << "Contour:" << *this;
    }
    else if (qAbs(qAbs(totalAngle) - 360.0) > 0.1)
    {
        qDebug() << "WARNING: non +/- 360 degree Contour! Total angle:" << totalAngle << "Contour:" << *this;
    }
    // TODO let Qt know that angleTo() gives a positive angle for clockwise not counter-clockwise when considering Y+ as upwards not downwards on a screen
    return totalAngle > 0.0 ? Segment::Clockwise : Segment::CounterClockwise;
#endif // USE_AREA_METHOD
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
        // if (it->line.p1 != prev->line.p2)
        {
            qDebug() << "INVALID:" << *prev << "to " << *it;
            return false;
        }
    }
    return true;
}

bool Contour::isCircle() const
{
    for (std::list<Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it)
    {
        if (!it->isArc || !FuzzyComparePoints(it->center, segments.front().center))
        {
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
        if (Line(prev->line.p2, it->line.p1).length() < TOLERANCE)
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

static void AddLineToContour(Contour &contour, const Point &pt1, const Point &pt2)
{
    if (pt1 != pt2)
    {
        contour.segments.push_back(Segment(Line(pt1, pt2)));
    }
}

Contour Contour::approximatedArcs() const
{
    Contour result;
    Point oldPoint;
    if (!segments.empty())
    {
        oldPoint = segments.front().line.p1;
    }
    for (std::list<Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it)
    {
        if (it->isArc)
        {
            const double radius = it->radius();
            const double angleStepDegrees = (360.0/24.0);
            const double angleStep = angleStepDegrees*M_PI/180.0;
                  double angle1 = atan2(it->line.p1.y - it->center.y, it->line.p1.x - it->center.x);
            const double angle2 = atan2(it->line.p2.y - it->center.y, it->line.p2.x - it->center.x);
            const std::list<Segment>::size_type beforeSize = result.segments.size();
            if (it->orientation == Segment::CounterClockwise)
            {
                if (angle1 > angle2)
                    angle1 -= 2*M_PI;
                for (double angle = angle1; angle < angle2; angle += angleStep)
                {
                    const Point pt = it->center + Point(cos(angle), sin(angle))*radius;
                    AddLineToContour(result, oldPoint, pt);
                    oldPoint = pt;
                }
            }
            else // Clockwise
            {
                if (angle1 < angle2)
                    angle1 += 2*M_PI;
                for (double angle = angle1; angle > angle2; angle -= angleStep)
                {
                    const Point pt = it->center + Point(cos(angle), sin(angle))*radius;
                    AddLineToContour(result, oldPoint, pt);
                    oldPoint = pt;
                }
            }

            // ensure at least the midpoint is present (may not be necesary)
            const std::list<Segment>::size_type afterSize = result.segments.size();
            const Point &midPoint = it->midPoint();
            if (afterSize == beforeSize)
            {
                AddLineToContour(result, oldPoint, midPoint);
                oldPoint = midPoint;
            }

            // ensure the destination is present
            const Point &endPoint = it->line.p2;
            AddLineToContour(result, oldPoint, endPoint);
            oldPoint = endPoint;
        }
        else
        {
            // add the desination
            AddLineToContour(result, oldPoint, it->line.p2);
            oldPoint = it->line.p2;
        }
    }
    return result;
}

Contour Contour::arcsRecovered() const
{
    return *this;
}

bool Shape::isAnnulus() const
{
    return boundary.isCircle() && holes.size() == 1 && holes.front().isCircle() && FuzzyComparePoints(boundary.segments.front().center, holes.front().segments.front().center);
}

Shape Shape::approximatedArcs() const
{
    Shape result;
    result.boundary = boundary.approximatedArcs();
    for (std::list<Contour>::const_iterator it = holes.begin(); it != holes.end(); ++it)
    {
        result.holes.push_back(it->approximatedArcs());
    }
    return result;
}

Shape Shape::arcsRecovered() const
{
    Shape result;
    result.boundary = boundary.arcsRecovered();
    for (std::list<Contour>::const_iterator it = holes.begin(); it != holes.end(); ++it)
    {
        result.holes.push_back(it->arcsRecovered());
    }
    return result;
}

MultiShape MultiShape::approximatedArcs() const
{
    MultiShape result;
    for (std::list<Shape>::const_iterator it = shapes.begin(); it != shapes.end(); ++it)
    {
        result.shapes.push_back(it->approximatedArcs());
    }
    return result;
}

MultiShape MultiShape::arcsRecovered() const
{
    MultiShape result;
    for (std::list<Shape>::const_iterator it = shapes.begin(); it != shapes.end(); ++it)
    {
        result.shapes.push_back(it->arcsRecovered());
    }
    return result;
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
