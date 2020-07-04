#ifndef LINEARCOFFSETDEMO_GEOMETRYOPERATIONS_H
#define LINEARCOFFSETDEMO_GEOMETRYOPERATIONS_H

#include "Geometry.h"

namespace LineArcOffsetDemo {

class GeometryOperations
{
public:
    virtual LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &multiShape) = 0;
    // virtual MultiShape intersection(const MultiShape &a, const MultiShape &b) = 0;
    // virtual MultiShape difference(const MultiShape &a, const MultiShape &b) = 0;
    // virtual MultiShape offset(const MultiShape &multiShape) = 0;
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOPERATIONS_H
