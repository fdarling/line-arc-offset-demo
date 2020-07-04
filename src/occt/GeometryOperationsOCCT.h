#ifndef LINEARCOFFSETDEMO_GEOMETRYOPERATIONSOCCT_H
#define LINEARCOFFSETDEMO_GEOMETRYOPERATIONSOCCT_H

#include "../GeometryOperations.h"

namespace LineArcOffsetDemo {

class GeometryOperationsOCCT : public GeometryOperations
{
public:
    LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &multiShape);
    LineArcGeometry::MultiShape intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    // LineArcGeometry::MultiShape offset(const LineArcGeometry::MultiShape &multiShape);
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOPERATIONSOCCT_H
