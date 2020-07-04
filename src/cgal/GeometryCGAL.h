#ifndef LINEARCOFFSETDEMO_GEOMETRYCGAL_H
#define LINEARCOFFSETDEMO_GEOMETRYCGAL_H

#include "../Geometry.h"
#include "CGALWrapper.h"

namespace LineArcOffsetDemo {

Polygon_2 ContourToPolygon(const LineArcGeometry::Contour &contour);
Polygon_with_holes_2 ShapeToPolygonWithHoles(const LineArcGeometry::Shape &shape);
std::list<Polygon_with_holes_2> MultiShapeToPolygonWithHolesList(const LineArcGeometry::MultiShape &multiShape);
LineArcGeometry::Contour PolygonToContour(const Polygon_2 &polygon);
LineArcGeometry::Shape PolygonWithHolesToShape(const Polygon_with_holes_2 &polygon);
LineArcGeometry::MultiShape PolygonWithHolesListToMultiShape(const std::list<Polygon_with_holes_2> &polygons);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYCGAL_H
