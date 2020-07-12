#ifndef LINEARCOFFSETDEMO_GEOMETRYBOOST_H
#define LINEARCOFFSETDEMO_GEOMETRYBOOST_H

#include "../Geometry.h"

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>
#include <boost/geometry/algorithms/correct.hpp>

#include <list>

namespace BoostGeometry {

typedef double NumberType;
typedef boost::geometry::model::d2::point_xy<NumberType> Point;
typedef boost::geometry::model::polygon<Point> Polygon;
typedef boost::geometry::model::multi_polygon<Polygon> Polygons;
typedef Polygon::ring_type Ring;
typedef Polygon::inner_container_type Holes;

} // namespace BoostGeometry

// TODO possibly file a bug report with boost about boost::geometry::model::d2::point_xy<double> not having operator==() nor operator!=()
inline bool operator==(const BoostGeometry::Point &a, const BoostGeometry::Point &b)
{
    return a.x() == b.x() && a.y() == b.y();
}

inline bool operator!=(const BoostGeometry::Point &a, const BoostGeometry::Point &b)
{
    return a.x() != b.x() || a.y() != b.y();
}

namespace LineArcOffsetDemo {

inline LineArcGeometry::CoordinateType FromBoostNumber(BoostGeometry::NumberType value)
{
    return static_cast<LineArcGeometry::CoordinateType>(value);
}

inline BoostGeometry::NumberType ToBoostNumber(LineArcGeometry::CoordinateType value)
{
    return static_cast<BoostGeometry::NumberType>(value);
}

BoostGeometry::Point PointToBoostPoint(const LineArcGeometry::Point &pt);
BoostGeometry::Polygon ShapeToPolygon(const LineArcGeometry::Shape &shape);
BoostGeometry::Polygons MultiShapeToPolygons(const LineArcGeometry::MultiShape &multiShape);
LineArcGeometry::Point BoostPointToPoint(const BoostGeometry::Point &pt);
LineArcGeometry::Contour RingToContour(const BoostGeometry::Ring &ring);
LineArcGeometry::MultiShape PolygonsToMultiShape(const BoostGeometry::Polygons &polygons);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYBOOST_H
