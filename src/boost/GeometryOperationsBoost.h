#ifndef LINEARCOFFSETDEMO_GEOMETRYOPERATIONSBOOST_H
#define LINEARCOFFSETDEMO_GEOMETRYOPERATIONSBOOST_H

#include "../GeometryOperations.h"

namespace LineArcOffsetDemo {

class GeometryOperationsBoost : public GeometryOperations
{
public:
    LineArcGeometry::MultiShape identity(const LineArcGeometry::MultiShape &multiShape);
    LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &multiShape);
    LineArcGeometry::MultiShape join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b);
    LineArcGeometry::MultiShape offset(const LineArcGeometry::MultiShape &multiShape, double radius);
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYOPERATIONSBOOST_H
