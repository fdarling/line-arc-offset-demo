#ifndef LINEARCOFFSETDEMO_GEOMETRYCLIPPER_H
#define LINEARCOFFSETDEMO_GEOMETRYCLIPPER_H

#include "../Geometry.h"

#include <clipper.hpp>

namespace LineArcOffsetDemo {

LineArcGeometry::CoordinateType FromClipperInt(ClipperLib::cInt value);
ClipperLib::cInt ToClipperInt(LineArcGeometry::CoordinateType value);
ClipperLib::IntPoint PointToIntPoint(const LineArcGeometry::Point &pt);
ClipperLib::Paths MultiShapeToPaths(const LineArcGeometry::MultiShape &multiShape);
LineArcGeometry::Point IntPointToPoint(const ClipperLib::IntPoint &pt);
LineArcGeometry::Contour PathToContour(const ClipperLib::Paths &paths);
LineArcGeometry::MultiShape PathsToMultiShape(const ClipperLib::Paths &paths);
LineArcGeometry::MultiShape PolyTreeToMultiShape(const ClipperLib::PolyTree &tree);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYCLIPPER_H
