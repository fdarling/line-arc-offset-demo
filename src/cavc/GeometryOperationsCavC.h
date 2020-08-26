#ifndef LINEARCOFFSETDEMO_GEOMETRYOPERATIONSCAVC_H
#define LINEARCOFFSETDEMO_GEOMETRYOPERATIONSCAVC_H

#include "../GeometryOperations.h"

namespace LineArcOffsetDemo {

class GeometryOperationsCavC : public GeometryOperations
{
public:
    LineArcGeometry::MultiShape identity(const LineArcGeometry::MultiShape &multiShape);
    LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &multiShape);
    LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape symmetricDifference(const LineArcGeometry::MultiShape &multiShape);
    LineArcGeometry::MultiShape symmetricDifference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape offset(const LineArcGeometry::MultiShape &multiShape, double radius);
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOPERATIONSCAVC_H
