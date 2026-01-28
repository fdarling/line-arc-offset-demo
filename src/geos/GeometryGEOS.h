#ifndef LINEARCOFFSETDEMO_GEOMETRYGEOS_H
#define LINEARCOFFSETDEMO_GEOMETRYGEOS_H

#include "../Geometry.h"

#ifndef USE_UNSTABLE_GEOS_CPP_API
#define USE_UNSTABLE_GEOS_CPP_API
#endif // USE_UNSTABLE_GEOS_CPP_API

#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/GeometryFactory.h>

#include <memory>

namespace LineArcOffsetDemo {

typedef geos::geom::GeometryFactory::Ptr GeometryFactoryUniquePtr;

geos::geom::Coordinate PointToCoordinate(const LineArcGeometry::Point &pt);
std::unique_ptr<geos::geom::LinearRing> ContourToLinearRing(const LineArcGeometry::Contour &contour, const GeometryFactoryUniquePtr &factory);
std::unique_ptr<geos::geom::Polygon> ShapeToPolygon(const LineArcGeometry::Shape &shape, const GeometryFactoryUniquePtr &factory);
std::unique_ptr<geos::geom::MultiPolygon> MultiShapeToMultiPolygon(const LineArcGeometry::MultiShape &multiShape, const GeometryFactoryUniquePtr &factory);
LineArcGeometry::Point CoordinateToPoint(const geos::geom::Coordinate &pt);
LineArcGeometry::Contour LinearRingToContour(const geos::geom::LineString *ring);
LineArcGeometry::Shape PolygonToShape(const geos::geom::Polygon *polygon);
LineArcGeometry::MultiShape MultiPolygonToMultiShape(const geos::geom::MultiPolygon *multiPolygon);
LineArcGeometry::MultiShape GeometryCollectionToMultiShape(const geos::geom::GeometryCollection *geometryCollection);
LineArcGeometry::MultiShape GeometryToMultiShape(const geos::geom::Geometry *geometry);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYGEOS_H
