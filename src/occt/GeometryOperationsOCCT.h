#ifndef LINEARCOFFSETDEMO_GEOMETRYOPERATIONSOCCT_H
#define LINEARCOFFSETDEMO_GEOMETRYOPERATIONSOCCT_H

#include "../GeometryOperations.h"

namespace LineArcOffsetDemo {

class GeometryOperationsOCCT : public GeometryOperations
{
public:
    LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &multiShape);
    // MultiShape intersection(const MultiShape &a, const MultiShape &b);
    // MultiShape difference(const MultiShape &a, const MultiShape &b);
    // MultiShape offset(const MultiShape &multiShape);
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOPERATIONSOCCT_H
