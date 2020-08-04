#include "GeometryGEOS.h"
#include "../GeometryQt.h"

#include <geos/version.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Geometry.h>

#include <cmath>

#include <QDebug>

namespace LineArcOffsetDemo {

#if (GEOS_VERSION_MAJOR <= 3) && (GEOS_VERSION_MINOR <= 7)
typedef std::vector<geos::geom::Geometry*> HoleVector;
#else
typedef std::vector<geos::geom::LinearRing*> HoleVector;
// typedef const geos::geom::Geometry* GeometryPointer;
#endif

geos::geom::Coordinate PointToCoordinate(const LineArcGeometry::Point &pt)
{
    return geos::geom::Coordinate(pt.x, pt.y);
}

LineArcGeometry::Point CoordinateToPoint(const geos::geom::Coordinate &pt)
{
    return LineArcGeometry::Point(pt.x, pt.y);
}

std::unique_ptr<geos::geom::LinearRing> ContourToLinearRing(const LineArcGeometry::Contour &contour, const geos::geom::GeometryFactory *factory)
{
    const LineArcGeometry::Contour approximated = contour.approximatedArcs();
    
    std::vector<geos::geom::Coordinate> * const points = new std::vector<geos::geom::Coordinate>();
    if (!approximated.segments.empty())
        points->push_back(PointToCoordinate(approximated.segments.front().line.p1));
    for (std::list<LineArcGeometry::Segment>::const_iterator it = approximated.segments.begin(); it != approximated.segments.end(); ++it)
    {
        points->push_back(PointToCoordinate(it->line.p2));
    }

    std::unique_ptr<geos::geom::CoordinateSequence> coordinateSequence(factory->getCoordinateSequenceFactory()->create(points, std::size_t(0)));
    std::unique_ptr<geos::geom::LinearRing> result(factory->createLinearRing(coordinateSequence.release()));
    
    return result;
}

std::unique_ptr<geos::geom::Polygon> ShapeToPolygon(const LineArcGeometry::Shape &shape, const geos::geom::GeometryFactory *factory)
{
    // convert the boundary
    std::unique_ptr<geos::geom::LinearRing> boundary(ContourToLinearRing(shape.boundary, factory));

    // possibly convert the holes
    HoleVector *holes = nullptr;
    if (!shape.holes.empty())
    {
        holes = new HoleVector();
        for (std::list<LineArcGeometry::Contour>::const_iterator it = shape.holes.begin(); it != shape.holes.end(); ++it)
        {
            holes->push_back(ContourToLinearRing(*it, factory).release());
        }
    }

    // construct the polygon
    return std::unique_ptr<geos::geom::Polygon>(factory->createPolygon(boundary.release(), holes));
}

std::unique_ptr<geos::geom::MultiPolygon> MultiShapeToMultiPolygon(const LineArcGeometry::MultiShape &multiShape, const geos::geom::GeometryFactory *factory)
{
    // create temporary geometry for constructor argument
    typedef std::vector<geos::geom::Geometry*> GeometryPointerVector;
    GeometryPointerVector * const polygons = new GeometryPointerVector(); // TODO make it nullptr if it's going to be empty?
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        polygons->push_back(ShapeToPolygon(*it, factory).release());
    }

    // construct MultiPolygon
    std::unique_ptr<geos::geom::MultiPolygon> result(factory->createMultiPolygon(polygons));

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
