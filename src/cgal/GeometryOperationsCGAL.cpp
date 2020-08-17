#include "GeometryOperationsCGAL.h"
#include "GeometryCGAL.h"
#include "CGALWrapper.h"

#include <CGAL/Boolean_set_operations_2.h>

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape GeometryOperationsCGAL::identity(const LineArcGeometry::MultiShape &multiShape)
{
    const std::list<Polygon_with_holes_2> polygons = MultiShapeToPolygonWithHolesList(multiShape);
    const LineArcGeometry::MultiShape reconverted = PolygonWithHolesListToMultiShape(polygons);
    return reconverted;
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

LineArcGeometry::MultiShape GeometryOperationsCGAL::join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    std::list<Polygon_with_holes_2> joined;
    {
        const std::list<Polygon_with_holes_2> aa(MultiShapeToPolygonWithHolesList(a));
        const std::list<Polygon_with_holes_2> bb(MultiShapeToPolygonWithHolesList(b));
        CGAL::join(aa.begin(), aa.end(), bb.begin(), bb.end(), std::back_inserter(joined));
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

LineArcGeometry::MultiShape GeometryOperationsCGAL::symmetricDifference(const LineArcGeometry::MultiShape &multiShape)
{
    const std::list<Polygon_with_holes_2> polygons = MultiShapeToPolygonWithHolesList(multiShape);
    const std::list<Polygon_with_holes_2> result = xorPolygonList(polygons);
    return PolygonWithHolesListToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsCGAL::symmetricDifference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    const std::list<Polygon_with_holes_2> aa = MultiShapeToPolygonWithHolesList(a);
    const std::list<Polygon_with_holes_2> bb = MultiShapeToPolygonWithHolesList(b);
    const std::list<Polygon_with_holes_2> result = xorPolygonLists(aa, bb);
    return PolygonWithHolesListToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsCGAL::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    const std::list<Polygon_with_holes_2> polygons = MultiShapeToPolygonWithHolesList(multiShape);
    std::list<Polygon_with_holes_2> offset_polygons = construct_polygon_list_offset(polygons, radius);
    return PolygonWithHolesListToMultiShape(offset_polygons);
}

} // namespace LineArcOffsetDemo
