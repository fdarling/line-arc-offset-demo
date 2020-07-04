#ifndef LINEARCOFFSETDEMO_GEOMETRYOCCT_H
#define LINEARCOFFSETDEMO_GEOMETRYOCCT_H

#include "../Geometry.h"

#include <gp_Pnt.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>

namespace LineArcOffsetDemo {

TopoDS_Face ShapeToTopoDS_Face(const LineArcGeometry::Shape &shape);
TopoDS_Face MultiShapeToTopoDS_Face(const LineArcGeometry::MultiShape &multiShape);

LineArcGeometry::Point gp_PntToPoint(const gp_Pnt &pt);
LineArcGeometry::MultiShape TopoDS_ShapeToMultiShape(const TopoDS_Shape &shape);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOCCT_H
