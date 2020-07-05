#ifndef LINEARCOFFSETDEMO_GEOMETRYOPERATIONSCGAL_H
#define LINEARCOFFSETDEMO_GEOMETRYOPERATIONSCGAL_H

#include "../GeometryOperations.h"

namespace LineArcOffsetDemo {

class GeometryOperationsCGAL : public GeometryOperations
{
public:
    LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &multiShape);
    LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    // LineArcGeometry::MultiShape offset(const LineArcGeometry::MultiShape &multiShape);
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOPERATIONSCGAL_H
