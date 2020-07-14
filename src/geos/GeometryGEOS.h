#ifndef LINEARCOFFSETDEMO_GEOMETRYGEOS_H
#define LINEARCOFFSETDEMO_GEOMETRYGEOS_H

#include "../Geometry.h"

#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/GeometryFactory.h>

namespace LineArcOffsetDemo {

geos::geom::Coordinate PointToCoordinate(const LineArcGeometry::Point &pt);
geos::geom::LinearRing * ContourToLinearRing(const LineArcGeometry::Contour &contour, const geos::geom::GeometryFactory *factory);
geos::geom::Polygon * ShapeToPolygon(const LineArcGeometry::Shape &shape, const geos::geom::GeometryFactory *factory);
geos::geom::MultiPolygon * MultiShapeToMultiPolygon(const LineArcGeometry::MultiShape &multiShape, const geos::geom::GeometryFactory *factory);
LineArcGeometry::Point CoordinateToPoint(const geos::geom::Coordinate &pt);
LineArcGeometry::Contour LinearRingToContour(const geos::geom::LineString *ring);
LineArcGeometry::Shape PolygonToShape(const geos::geom::Polygon *polygon);
LineArcGeometry::MultiShape MultiPolygonToMultiShape(const geos::geom::MultiPolygon *multiPolygon);
LineArcGeometry::MultiShape GeometryToMultiShape(const geos::geom::Geometry *geometry);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYGEOS_H
