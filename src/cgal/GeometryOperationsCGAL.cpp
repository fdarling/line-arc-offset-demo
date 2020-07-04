#include "GeometryOperationsCGAL.h"
#include "GeometryCGAL.h"
#include "CGALWrapper.h"

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape GeometryOperationsCGAL::join(const LineArcGeometry::MultiShape &multiShape)
{
    std::list<Polygon_with_holes_2> joined;
    {
        const std::list<Polygon_with_holes_2> pieces(MultiShapeToPolygonWithHolesList(multiShape));
        CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(joined));
    }
    return PolygonWithHolesListToMultiShape(joined);
}

} // namespace LineArcOffsetDemo
