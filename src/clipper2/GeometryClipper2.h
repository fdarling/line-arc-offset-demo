#ifndef LINEARCOFFSETDEMO_GEOMETRYCLIPPER2_H
#define LINEARCOFFSETDEMO_GEOMETRYCLIPPER2_H

#include "../Geometry.h"

#include <clipper2/clipper.h>

namespace LineArcOffsetDemo {

typedef int64_t Clipper2CoordinateType;

LineArcGeometry::CoordinateType FromClipperInt64(Clipper2CoordinateType value);
Clipper2CoordinateType ToClipperInt64(LineArcGeometry::CoordinateType value);
Clipper2Lib::Point64 PointToPoint64(const LineArcGeometry::Point &pt);
Clipper2Lib::Paths64 MultiShapeToPaths64(const LineArcGeometry::MultiShape &multiShape);
LineArcGeometry::Point Point64ToPoint(const Clipper2Lib::Point64 &pt);
LineArcGeometry::Contour Path64ToContour(const Clipper2Lib::Path64 &path);
LineArcGeometry::MultiShape Paths64ToMultiShape(const Clipper2Lib::Paths64 &paths);
LineArcGeometry::MultiShape PolyTree64ToMultiShape(const Clipper2Lib::PolyTree64 &tree);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYCLIPPER_H
