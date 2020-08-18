#ifndef LINEARCOFFSETDEMO_GEOMETRYCGAL_H
#define LINEARCOFFSETDEMO_GEOMETRYCGAL_H

#include "../Geometry.h"
#include "CGALWrapper.h"

namespace LineArcOffsetDemo {

Point_2 PointToPoint_2(const LineArcGeometry::Point &pt);
Polygon_2 ContourToPolygon(const LineArcGeometry::Contour &contour, const CGAL::Orientation desiredOrientation = CGAL::POSITIVE);
Polygon_with_holes_2 ShapeToPolygonWithHoles(const LineArcGeometry::Shape &shape);
std::list<Polygon_with_holes_2> MultiShapeToPolygonWithHolesList(const LineArcGeometry::MultiShape &multiShape);
LineArcGeometry::Point Point_2ToPoint(const Point_2 &pt);
LineArcGeometry::Point Point_2ToPoint(const Traits_2::Point_2 &pt);
LineArcGeometry::Contour PolygonToContour(const Polygon_2 &polygon);
LineArcGeometry::Shape PolygonWithHolesToShape(const Polygon_with_holes_2 &polygon);
LineArcGeometry::MultiShape PolygonWithHolesListToMultiShape(const std::list<Polygon_with_holes_2> &polygons);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYCGAL_H
