#include "GeometryQt.h"

namespace LineArcGeometry {

QPointF PointToQPointF(const Point &pt)
{
    return QPointF(pt.x, pt.y);
}

} // namespace LineArcGeometry
