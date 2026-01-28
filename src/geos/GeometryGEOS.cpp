#include "GeometryGEOS.h"
#include "../GeometryQt.h"

#include <geos/version.h>
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

std::unique_ptr<geos::geom::LinearRing> ContourToLinearRing(const LineArcGeometry::Contour &contour, const GeometryFactoryUniquePtr &factory)
{
    const LineArcGeometry::Contour approximated = contour.approximatedArcs();

    std::unique_ptr<geos::geom::CoordinateSequence> coordinateSequence = std::make_unique<geos::geom::CoordinateSequence>();
    if (!approximated.segments.empty())
        coordinateSequence->add(PointToCoordinate(approximated.segments.front().line.p1));
    for (std::list<LineArcGeometry::Segment>::const_iterator it = approximated.segments.begin(); it != approximated.segments.end(); ++it)
    {
        coordinateSequence->add(PointToCoordinate(it->line.p2));
    }

    std::unique_ptr<geos::geom::LinearRing> result = factory->createLinearRing(std::move(coordinateSequence));

    return result;
}

std::unique_ptr<geos::geom::Polygon> ShapeToPolygon(const LineArcGeometry::Shape &shape, const GeometryFactoryUniquePtr &factory)
{
    // convert the boundary
    std::unique_ptr<geos::geom::LinearRing> boundary(ContourToLinearRing(shape.boundary, factory));

    // convert any holes
    std::vector<std::unique_ptr<geos::geom::LinearRing>> holes;
    if (!shape.holes.empty())
    {
        for (std::list<LineArcGeometry::Contour>::const_iterator it = shape.holes.begin(); it != shape.holes.end(); ++it)
        {
            holes.emplace_back(ContourToLinearRing(*it, factory));
        }
    }

    // construct the polygon
    return factory->createPolygon(std::move(boundary), std::move(holes));
}

std::unique_ptr<geos::geom::MultiPolygon> MultiShapeToMultiPolygon(const LineArcGeometry::MultiShape &multiShape, const GeometryFactoryUniquePtr &factory)
{
    // create temporary geometry for constructor argument
    typedef std::vector<std::unique_ptr<geos::geom::Geometry>> GeometryPointerVector;
    GeometryPointerVector polygons;
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        polygons.emplace_back(ShapeToPolygon(*it, factory).release());
    }

    // construct MultiPolygon
    std::unique_ptr<geos::geom::MultiPolygon> result = factory->createMultiPolygon(std::move(polygons));

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
        const geos::geom::GeometryTypeId type = geometry->getGeometryTypeId();
        if (type == geos::geom::GEOS_POLYGON)
        {
            const geos::geom::Polygon * const polygon = dynamic_cast<const geos::geom::Polygon*>(geometry);
            result.shapes.push_back(PolygonToShape(polygon));
        }
        else
        {
            qDebug() << "WARNING: MultiPolygonToMultiShape() called on unhandled type:" << geometry->getGeometryType().c_str();
        }
    }
    return result;
}

LineArcGeometry::MultiShape GeometryCollectionToMultiShape(const geos::geom::GeometryCollection *geometryCollection)
{
    LineArcGeometry::MultiShape result;
    for (std::size_t i = 0; i < geometryCollection->getNumGeometries(); i++)
    {
        const geos::geom::Geometry * const geometry = geometryCollection->getGeometryN(i);
        const geos::geom::GeometryTypeId type = geometry->getGeometryTypeId();
        qDebug() << "GeometryCollectionToMultiShape() " << geometry->getGeometryType().c_str();
        if (type == geos::geom::GEOS_POLYGON)
        {
            const geos::geom::Polygon * const polygon = dynamic_cast<const geos::geom::Polygon*>(geometry);
            result.shapes.push_back(PolygonToShape(polygon));
        }
        else
        {
            qDebug() << "WARNING: GeometryCollectionToMultiShape() called on unhandled type:" << geometry->getGeometryType().c_str();
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
    else if (type == geos::geom::GEOS_GEOMETRYCOLLECTION)
    {
        const geos::geom::GeometryCollection * const geometryCollection = dynamic_cast<const geos::geom::GeometryCollection*>(geometry);
        result = GeometryCollectionToMultiShape(geometryCollection);
    }
    else
    {
        qDebug() << "WARNING: GeometryToMultiShape() called on unhandled type:" << geometry->getGeometryType().c_str();
    }
    return result;
}

} // namespace LineArcOffsetDemo
