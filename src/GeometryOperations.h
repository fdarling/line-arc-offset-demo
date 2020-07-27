#ifndef LINEARCOFFSETDEMO_GEOMETRYOPERATIONS_H
#define LINEARCOFFSETDEMO_GEOMETRYOPERATIONS_H

#include "Geometry.h"

namespace LineArcOffsetDemo {

class GeometryOperations
{
public:
    GeometryOperations();
    virtual ~GeometryOperations();
    virtual LineArcGeometry::MultiShape identity(const LineArcGeometry::MultiShape &multiShape) = 0;
    virtual LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &multiShape) = 0;
    virtual LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b) = 0;
    virtual LineArcGeometry::MultiShape intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b) = 0;
    virtual LineArcGeometry::MultiShape difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b) = 0;
    virtual LineArcGeometry::MultiShape offset(const LineArcGeometry::MultiShape &multiShape, double radius) = 0;
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOPERATIONS_H
