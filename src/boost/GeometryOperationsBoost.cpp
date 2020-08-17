#include "GeometryOperationsBoost.h"
#include "GeometryBoost.h"
#include "../GeometryQt.h"

#include <boost/geometry.hpp>

#include <QDebug>

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape GeometryOperationsBoost::identity(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsBoost::identity()";
    const BoostGeometry::Polygons polygons = MultiShapeToPolygons(multiShape);
    return PolygonsToMultiShape(polygons);
}

LineArcGeometry::MultiShape GeometryOperationsBoost::join(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsBoost::join()";
    const BoostGeometry::Polygons polygons = MultiShapeToPolygons(multiShape);
    BoostGeometry::Polygons joined;
    for (BoostGeometry::Polygons::const_iterator it = polygons.begin(); it != polygons.end(); ++it)
    {
        BoostGeometry::Polygons result;
        boost::geometry::union_(joined, *it, result);
        joined = result;
    }
    return PolygonsToMultiShape(joined);
}

LineArcGeometry::MultiShape GeometryOperationsBoost::join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsBoost::join()";
    const BoostGeometry::Polygons aa = MultiShapeToPolygons(a);
    const BoostGeometry::Polygons bb = MultiShapeToPolygons(b);
    BoostGeometry::Polygons result;
    boost::geometry::union_(aa, bb, result);
    return PolygonsToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsBoost::intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsBoost::intersection()";
    const BoostGeometry::Polygons aa = MultiShapeToPolygons(a);
    const BoostGeometry::Polygons bb = MultiShapeToPolygons(b);
    BoostGeometry::Polygons result;
    boost::geometry::intersection(aa, bb, result);
    return PolygonsToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsBoost::difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsBoost::difference()";
    const BoostGeometry::Polygons aa = MultiShapeToPolygons(a);
    const BoostGeometry::Polygons bb = MultiShapeToPolygons(b);
    BoostGeometry::Polygons result;
    boost::geometry::difference(aa, bb, result);
    return PolygonsToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsBoost::symmetricDifference(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsBoost::symmetricDifference()";
    const BoostGeometry::Polygons polygons = MultiShapeToPolygons(multiShape);
    BoostGeometry::Polygons xorResult;
    for (BoostGeometry::Polygons::const_iterator it = polygons.begin(); it != polygons.end(); ++it)
    {
        BoostGeometry::Polygons result;
        boost::geometry::sym_difference(xorResult, *it, result);
        xorResult = result;
    }
    return PolygonsToMultiShape(xorResult);
}

LineArcGeometry::MultiShape GeometryOperationsBoost::symmetricDifference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsBoost::symmetricDifference()";
    const BoostGeometry::Polygons aa = MultiShapeToPolygons(a);
    const BoostGeometry::Polygons bb = MultiShapeToPolygons(b);
    BoostGeometry::Polygons result;
    boost::geometry::sym_difference(aa, bb, result);
    return PolygonsToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsBoost::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    // qDebug() << "GeometryOperationsBoost::offset()";
    const BoostGeometry::Polygons polygons = MultiShapeToPolygons(multiShape);
    BoostGeometry::Polygons result;
    boost::geometry::strategy::buffer::distance_symmetric<BoostGeometry::NumberType> distance_strategy(ToBoostNumber(radius));
    const int points_per_circle = 24;  // 360 / 15
    boost::geometry::strategy::buffer::join_round join_strategy(points_per_circle);
    boost::geometry::strategy::buffer::end_round end_strategy(points_per_circle);
    boost::geometry::strategy::buffer::point_circle circle_strategy(points_per_circle);
    boost::geometry::strategy::buffer::side_straight side_strategy;
    boost::geometry::buffer(polygons, result, distance_strategy, side_strategy, join_strategy, end_strategy, circle_strategy);
    return PolygonsToMultiShape(result);
}

} // namespace LineArcOffsetDemo
