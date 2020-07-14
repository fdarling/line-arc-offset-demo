#include "GeometryGEOS.h"
#include "../GeometryQt.h"

#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Geometry.h>

#include <cmath>

#include <QDebug>

namespace LineArcOffsetDemo {

geos::geom::Coordinate PointToCoordinate(const LineArcGeometry::Point &pt)
{
    return geos::geom::Coordinate(pt.x, pt.y);
}

LineArcGeometry::Point CoordinateToPoint(const geos::geom::Coordinate &pt)
{
    return LineArcGeometry::Point(pt.x, pt.y);
}

static void AddToSequence(geos::geom::CoordinateSequence *sequence, const geos::geom::Coordinate &pt)
{
    if (sequence->isEmpty() || sequence->back() != pt)
    {
        sequence->add(pt);
    }
}

geos::geom::LinearRing * ContourToLinearRing(const LineArcGeometry::Contour &contour, const geos::geom::GeometryFactory *factory)
{
    geos::geom::CoordinateSequence * const points = factory->getCoordinateSequenceFactory()->create(static_cast<std::size_t>(0), 0);

    if (!contour.segments.empty())
    {
        AddToSequence(points, PointToCoordinate(contour.segments.front().line.p1));
    }
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        if (it->isArc)
        {
            const double radius = it->radius();
            const double angleStepDegrees = 15.0;
            const double angleStep = angleStepDegrees*M_PI/180.0;
                  double angle1 = atan2(it->line.p1.y - it->center.y, it->line.p1.x - it->center.x);
            const double angle2 = atan2(it->line.p2.y - it->center.y, it->line.p2.x - it->center.x);
            const std::size_t beforeSize = points->size();
            if (it->orientation == LineArcGeometry::Segment::CounterClockwise)
            {
                if (angle1 > angle2)
                    angle1 -= 2*M_PI;
                for (double angle = angle1; angle < angle2; angle += angleStep)
                {
                    const LineArcGeometry::Point pt = it->center + LineArcGeometry::Point(cos(angle), sin(angle))*radius;
                    AddToSequence(points, PointToCoordinate(pt));
                }
            }
            else // Clockwise
            {
                if (angle1 < angle2)
                    angle1 += 2*M_PI;
                for (double angle = angle1; angle > angle2; angle -= angleStep)
                {
                    const LineArcGeometry::Point pt = it->center + LineArcGeometry::Point(cos(angle), sin(angle))*radius;
                    AddToSequence(points, PointToCoordinate(pt));
                }
            }

            // ensure at least the midpoint is present (may not be necesary)
            const std::size_t afterSize = points->size();
            const geos::geom::Coordinate midPoint = PointToCoordinate(it->midPoint());
            if (afterSize == beforeSize)
            {
                AddToSequence(points, midPoint);
            }

            // ensure the destination is present
            const geos::geom::Coordinate endPoint = PointToCoordinate(it->line.p2);
            AddToSequence(points, endPoint);
        }
        else
        {
            // add the desination
            AddToSequence(points, PointToCoordinate(it->line.p2));
        }
    }

    geos::geom::LinearRing * const result = factory->createLinearRing(points);
    return result;
}

geos::geom::Polygon * ShapeToPolygon(const LineArcGeometry::Shape &shape, const geos::geom::GeometryFactory *factory)
{
    // convert the boundary
    geos::geom::LinearRing * const boundary = ContourToLinearRing(shape.boundary, factory);

    // possibly convert the holes
    std::vector<geos::geom::Geometry*> *holes = nullptr;
    if (!shape.holes.empty())
    {
        holes = new std::vector<geos::geom::Geometry*>();
        for (std::list<LineArcGeometry::Contour>::const_iterator it = shape.holes.begin(); it != shape.holes.end(); ++it)
        {
            holes->push_back(ContourToLinearRing(*it, factory));
        }
    }

    // construct the polygon
    return factory->createPolygon(boundary, holes);
}

geos::geom::MultiPolygon * MultiShapeToMultiPolygon(const LineArcGeometry::MultiShape &multiShape, const geos::geom::GeometryFactory *factory)
{
    // create temporary geometry for constructor argument
    std::vector<geos::geom::Geometry*> polygons;
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        polygons.push_back(ShapeToPolygon(*it, factory));
    }

    // construct MultiPolygon
    geos::geom::MultiPolygon * const result = factory->createMultiPolygon(polygons);

    // NOTE: unnecessary since the MultiPolygon took ownership
    // clean up temporary geometry
    /*for (std::vector<geos::geom::Geometry*>::iterator it = polygons.begin(); it != polygons.end(); ++it)
    {
        delete *it;
    }*/

    // return constructed object
    return result;
}

LineArcGeometry::Contour LinearRingToContour(const geos::geom::LineString *ring)
{
    LineArcGeometry::Contour result;

    if (ring->getNumPoints() < 2)
        return result;

    const geos::geom::Coordinate *prev = &ring->getCoordinateN(ring->getNumPoints() - 1);
    for (std::size_t i = 0; i < ring->getNumPoints(); i++)
    {
        const geos::geom::Coordinate &curr = ring->getCoordinateN(i);
        const LineArcGeometry::Point p1 = CoordinateToPoint(*prev);
        const LineArcGeometry::Point p2 = CoordinateToPoint(curr);
        const LineArcGeometry::Line line(p1, p2);
        result.segments.push_back(LineArcGeometry::Segment(line));
        prev = &curr;
    }

    return result;
}

LineArcGeometry::Shape PolygonToShape(const geos::geom::Polygon *polygon)
{
    LineArcGeometry::Shape result;
    result.boundary = LinearRingToContour(polygon->getExteriorRing());
    for (std::size_t i = 0; i < polygon->getNumInteriorRing(); i++)
    {
        const geos::geom::LineString * const ring = polygon->getInteriorRingN(i);
        result.holes.push_back(LinearRingToContour(ring));
    }
    return result;
}

LineArcGeometry::MultiShape MultiPolygonToMultiShape(const geos::geom::MultiPolygon *multiPolygon)
{
    LineArcGeometry::MultiShape result;
    for (std::size_t i = 0; i < multiPolygon->getNumGeometries(); i++)
    {
        const geos::geom::Geometry * const geometry = multiPolygon->getGeometryN(i);
        const geos::geom::Polygon * const polygon = dynamic_cast<const geos::geom::Polygon*>(geometry);
        if (polygon)
        {
            result.shapes.push_back(LineArcGeometry::Shape(PolygonToShape(polygon)));
        }
    }
    return result;
}

LineArcGeometry::MultiShape GeometryToMultiShape(const geos::geom::Geometry *geometry)
{
    LineArcGeometry::MultiShape result;
    const geos::geom::GeometryTypeId type = geometry->getGeometryTypeId();
    if (type == geos::geom::GEOS_POLYGON)
    {
        const geos::geom::Polygon * const polygon = dynamic_cast<const geos::geom::Polygon*>(geometry);
        result.shapes.push_back(PolygonToShape(polygon));
    }
    else if (type == geos::geom::GEOS_MULTIPOLYGON)
    {
        const geos::geom::MultiPolygon * const multiPolygon = dynamic_cast<const geos::geom::MultiPolygon*>(geometry);
        result = MultiPolygonToMultiShape(multiPolygon);
    }
    else
    {
        qDebug() << "WARNING: GeometryToMultiShape() called on unhandled type:" << geometry->getGeometryType().c_str();
    }
    return result;
}

} // namespace LineArcOffsetDemo
