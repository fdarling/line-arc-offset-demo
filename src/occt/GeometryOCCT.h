#ifndef LINEARCOFFSETDEMO_GEOMETRYOCCT_H
#define LINEARCOFFSETDEMO_GEOMETRYOCCT_H

#include "../Geometry.h"

#include <gp_Pnt.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>

namespace LineArcOffsetDemo {

TopoDS_Wire ContourToTopoDS_Wire(const LineArcGeometry::Contour &contour);
TopoDS_Face ShapeToTopoDS_Face(const LineArcGeometry::Shape &shape);
TopoDS_Face MultiShapeToTopoDS_Face(const LineArcGeometry::MultiShape &multiShape);

LineArcGeometry::Point gp_PntToPoint(const gp_Pnt &pt);
LineArcGeometry::Contour TopoDS_WireToContour(const TopoDS_Wire &wire);
LineArcGeometry::Shape TopoDS_FaceToShape(const TopoDS_Face &face);
LineArcGeometry::MultiShape TopoDS_ShapeToMultiShape(const TopoDS_Shape &shape, bool useFaces = true);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOCCT_H
