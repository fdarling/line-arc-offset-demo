#include "GeometryBoost.h"
#include "../GeometryQt.h"

#include <boost/geometry.hpp>

#include <QDebug>

QT_BEGIN_NAMESPACE

static QDebug operator<<(QDebug debug, const BoostGeometry::Point &pt) __attribute__((unused));
static QDebug operator<<(QDebug debug, const BoostGeometry::Point &pt)
{
    QDebugStateSaver saver(debug);
    const LineArcGeometry::Point p = LineArcOffsetDemo::BoostPointToPoint(pt);
    debug.nospace() << p;
    return debug;
}

static QDebug operator<<(QDebug debug, const BoostGeometry::Ring &ring) __attribute__((unused));
static QDebug operator<<(QDebug debug, const BoostGeometry::Ring &ring)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Ring([";
    for (BoostGeometry::Ring::const_iterator it = ring.begin(); it != ring.end(); ++it)
    {
        const LineArcGeometry::Point p = LineArcOffsetDemo::BoostPointToPoint(*it);
        debug << (it != ring.begin() ? ", (" : "(") << p.x << ", " << p.y << ")";
    }
    debug << "])";
    return debug;
}

static QDebug operator<<(QDebug debug, const BoostGeometry::Polygon &polygon) __attribute__((unused));
static QDebug operator<<(QDebug debug, const BoostGeometry::Polygon &polygon)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Polygon(outer = " << polygon.outer() << ", inners = [";
    for (BoostGeometry::Holes::const_iterator it = polygon.inners().begin(); it != polygon.inners().end(); ++it)
    {
        debug << (it != polygon.inners().begin() ? ", (" : "(") << *it << ")";
    }
    debug << "])";
    return debug;
}

static QDebug operator<<(QDebug debug, const BoostGeometry::Polygons &polygons) __attribute__((unused));
static QDebug operator<<(QDebug debug, const BoostGeometry::Polygons &polygons)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Polygons([";
    for (BoostGeometry::Polygons::const_iterator it = polygons.begin(); it != polygons.end(); ++it)
    {
        debug << (it != polygons.begin() ? ", (" : "(") << *it << ")";
    }
    debug << "])";
    return debug;
}

QT_END_NAMESPACE

namespace LineArcOffsetDemo {

BoostGeometry::Point PointToBoostPoint(const LineArcGeometry::Point &pt)
{
    return BoostGeometry::Point(ToBoostNumber(pt.x), ToBoostNumber(pt.y));
}

LineArcGeometry::Point BoostPointToPoint(const BoostGeometry::Point &pt)
{
    return LineArcGeometry::Point(FromBoostNumber(pt.x()), FromBoostNumber(pt.y()));
}

static void AddToPath(BoostGeometry::Ring &ring, const BoostGeometry::Point &pt)
{
    if (ring.empty() || ring.back() != pt)
    {
        ring.push_back(pt);
    }
}

static BoostGeometry::Ring ContourToRing(const LineArcGeometry::Contour &contour)
{
    BoostGeometry::Ring result;
    if (!contour.segments.empty())
        AddToPath(result, PointToBoostPoint(contour.segments.front().line.p1));
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        if (it->isArc)
        {
            const double radius = it->radius();
            const double angleStepDegrees = 15.0;
            const double angleStep = angleStepDegrees*M_PI/180.0;
                  double angle1 = atan2(it->line.p1.y - it->center.y, it->line.p1.x - it->center.x);
            const double angle2 = atan2(it->line.p2.y - it->center.y, it->line.p2.x - it->center.x);
            const BoostGeometry::Ring::size_type beforeSize = result.size();
            if (it->orientation == LineArcGeometry::Segment::CounterClockwise)
            {
                if (angle1 > angle2)
                    angle1 -= 2*M_PI;
                for (double angle = angle1; angle < angle2; angle += angleStep)
                {
                    const LineArcGeometry::Point pt = it->center + LineArcGeometry::Point(cos(angle), sin(angle))*radius;
                    AddToPath(result, PointToBoostPoint(pt));
                }
            }
            else // Clockwise
            {
                if (angle1 < angle2)
                    angle1 += 2*M_PI;
                for (double angle = angle1; angle > angle2; angle -= angleStep)
                {
                    const LineArcGeometry::Point pt = it->center + LineArcGeometry::Point(cos(angle), sin(angle))*radius;
                    AddToPath(result, PointToBoostPoint(pt));
                }
            }

            // ensure at least the midpoint is present (may not be necesary)
            const BoostGeometry::Ring::size_type afterSize = result.size();
            const BoostGeometry::Point midPoint = PointToBoostPoint(it->midPoint());
            if (afterSize == beforeSize)
            {
                AddToPath(result, midPoint);
            }

            // ensure the destination is present
            const BoostGeometry::Point endPoint = PointToBoostPoint(it->line.p2);
            AddToPath(result, endPoint);
        }
        else
        {
            // add the desination
            result.push_back(PointToBoostPoint(it->line.p2));
        }
    }
    return result;
}

BoostGeometry::Polygon ShapeToPolygon(const LineArcGeometry::Shape &shape)
{
    BoostGeometry::Polygon polygon;

    // add boundary
    polygon.outer() = ContourToRing(shape.boundary);

    // add holes
    for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
    {
        polygon.inners().push_back(ContourToRing(*hole_it));
    }

    boost::geometry::correct(polygon);
    return polygon;
}

BoostGeometry::Polygons MultiShapeToPolygons(const LineArcGeometry::MultiShape &multiShape)
{
    BoostGeometry::Polygons result;
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        const LineArcGeometry::Shape &shape = *shape_it;
        result.push_back(ShapeToPolygon(shape));
    }
    return result;
}

LineArcGeometry::Contour RingToContour(const BoostGeometry::Ring &ring)
{
    LineArcGeometry::Contour result;

    if (ring.size() < 2)
        return result;

    const BoostGeometry::Point *prev = &ring.back();
    for (BoostGeometry::Ring::const_iterator it = ring.begin(); it != ring.end(); prev = &*it, ++it)
    {
        const LineArcGeometry::Point p1 = BoostPointToPoint(*prev);
        const LineArcGeometry::Point p2 = BoostPointToPoint(*it);
        const LineArcGeometry::Line line(p1, p2);
        result.segments.push_back(LineArcGeometry::Segment(line));
    }

    return result;
}

LineArcGeometry::Shape PolygonToShape(const BoostGeometry::Polygon &polygon)
{
    LineArcGeometry::Shape result;
    result.boundary = RingToContour(polygon.outer());
    for (BoostGeometry::Holes::const_iterator hole_it = polygon.inners().begin(); hole_it != polygon.inners().end(); ++hole_it)
    {
        result.holes.push_back(RingToContour(*hole_it));
    }
    return result;
}

LineArcGeometry::MultiShape PolygonsToMultiShape(const BoostGeometry::Polygons &polygons)
{
    LineArcGeometry::MultiShape result;
    for (BoostGeometry::Polygons::const_iterator it = polygons.begin(); it != polygons.end(); ++it)
    {
        result.shapes.push_back(LineArcGeometry::Shape(PolygonToShape(*it)));
    }
    return result;
}

} // namespace LineArcOffsetDemo
