#include "GeometryOperationsCGAL.h"
#include "GeometryCGAL.h"
#include "CGALWrapper.h"

namespace LineArcOffsetDemo {

static std::list<Polygon_with_holes_2> subtractPolygonLists(const std::list<Polygon_with_holes_2> &a, const std::list<Polygon_with_holes_2> &b)
{
    std::list<Polygon_with_holes_2> tmp = a;
    std::list<Polygon_with_holes_2> result;
    for (std::list<Polygon_with_holes_2>::const_iterator poly = b.begin(); poly != b.end(); ++poly)
    {
        result.clear();
        for (std::list<Polygon_with_holes_2>::const_iterator poly_with_holes = tmp.begin(); poly_with_holes != tmp.end(); ++poly_with_holes)
        {
            CGAL::difference(*poly_with_holes, *poly, std::back_inserter(result));
        }
        tmp = result;
    }
    return result;
}

static std::list<Polygon_with_holes_2> intersectPolygonLists(const std::list<Polygon_with_holes_2> &a, const std::list<Polygon_with_holes_2> &b)
{
    std::list<Polygon_with_holes_2> tmp = a;
    std::list<Polygon_with_holes_2> result;
    for (std::list<Polygon_with_holes_2>::const_iterator poly = b.begin(); poly != b.end(); ++poly)
    {
        result.clear();
        for (std::list<Polygon_with_holes_2>::const_iterator poly_with_holes = tmp.begin(); poly_with_holes != tmp.end(); ++poly_with_holes)
        {
            CGAL::intersection(*poly_with_holes, *poly, std::back_inserter(result));
        }
        tmp = result;
    }
    return result;
}

LineArcGeometry::MultiShape GeometryOperationsCGAL::join(const LineArcGeometry::MultiShape &multiShape)
{
    std::list<Polygon_with_holes_2> joined;
    {
        const std::list<Polygon_with_holes_2> pieces(MultiShapeToPolygonWithHolesList(multiShape));
        CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(joined));
    }
    return PolygonWithHolesListToMultiShape(joined);
}

LineArcGeometry::MultiShape GeometryOperationsCGAL::intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    const std::list<Polygon_with_holes_2> aa = MultiShapeToPolygonWithHolesList(a);
    const std::list<Polygon_with_holes_2> bb = MultiShapeToPolygonWithHolesList(b);
    const std::list<Polygon_with_holes_2> intersected = intersectPolygonLists(aa, bb);
    return PolygonWithHolesListToMultiShape(intersected);
}

LineArcGeometry::MultiShape GeometryOperationsCGAL::difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    const std::list<Polygon_with_holes_2> minuend    = MultiShapeToPolygonWithHolesList(a);
    const std::list<Polygon_with_holes_2> subtrahend = MultiShapeToPolygonWithHolesList(b);
    const std::list<Polygon_with_holes_2> difference = subtractPolygonLists(minuend, subtrahend);
    return PolygonWithHolesListToMultiShape(difference);
}

} // namespace LineArcOffsetDemo
