#ifndef LINEARCOFFSETDEMO_GEOMETRYOPERATIONSCGAL_H
#define LINEARCOFFSETDEMO_GEOMETRYOPERATIONSCGAL_H

#include "../GeometryOperations.h"

namespace LineArcOffsetDemo {

class GeometryOperationsCGAL : public GeometryOperations
{
public:
    LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &multiShape);
    // MultiShape intersection(const MultiShape &a, const MultiShape &b);
    // MultiShape difference(const MultiShape &a, const MultiShape &b);
    // MultiShape offset(const MultiShape &multiShape);
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOPERATIONSCGAL_H
