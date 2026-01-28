#include "GeometryOperationsGEOS.h"
#include "GeometryGEOS.h"
#include "../GeometryQt.h"

#include <geos/version.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/operation/union/UnaryUnionOp.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/buffer/BufferOp.h>

#include <memory>

#include <QDebug>

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape GeometryOperationsGEOS::identity(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsGEOS::identity()";
    const GeometryFactoryUniquePtr factory(geos::geom::GeometryFactory::create());
    const std::unique_ptr<geos::geom::MultiPolygon> multiPolygon(MultiShapeToMultiPolygon(multiShape, factory));
    return MultiPolygonToMultiShape(multiPolygon.get());
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::join(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsGEOS::join()";
    const GeometryFactoryUniquePtr factory(geos::geom::GeometryFactory::create());
    const std::unique_ptr<geos::geom::Geometry> multiPolygon(MultiShapeToMultiPolygon(multiShape, factory));
    const std::unique_ptr<geos::geom::Geometry> joined = multiPolygon->Union();
    return GeometryToMultiShape(joined.get());
}

static LineArcGeometry::MultiShape DoBoolean(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b, const int type)
{
    const GeometryFactoryUniquePtr factory(geos::geom::GeometryFactory::create());
    const std::unique_ptr<geos::geom::MultiPolygon> aa(MultiShapeToMultiPolygon(a, factory));
    const std::unique_ptr<geos::geom::MultiPolygon> bb(MultiShapeToMultiPolygon(b, factory));
    geos::operation::overlayng::OverlayNG op(aa.get(), bb.get(), type);
    const std::unique_ptr<geos::geom::Geometry> result(op.getResult());
    return GeometryToMultiShape(result.get());
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsGEOS::join()";
    return DoBoolean(a, b, geos::operation::overlayng::OverlayNG::UNION);
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsGEOS::intersection()";
    return DoBoolean(a, b, geos::operation::overlayng::OverlayNG::INTERSECTION);
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsGEOS::difference()";
    return DoBoolean(a, b, geos::operation::overlayng::OverlayNG::DIFFERENCE);
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::symmetricDifference(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsGEOS::symmetricDifference()";
    LineArcGeometry::MultiShape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        if (result.shapes.empty())
        {
            result.shapes.push_back(*it);
        }
        else
        {
            LineArcGeometry::MultiShape cutter;
            cutter.shapes.push_back(*it);
            result = symmetricDifference(result, cutter);
        }
    }
    return result;
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::symmetricDifference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsGEOS::symmetricDifference()";
    return DoBoolean(a, b, geos::operation::overlayng::OverlayNG::SYMDIFFERENCE);
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    // qDebug() << "GeometryOperationsGEOS::offset()";
    const GeometryFactoryUniquePtr factory(geos::geom::GeometryFactory::create());
    const std::unique_ptr<geos::geom::MultiPolygon> multiPolygon(MultiShapeToMultiPolygon(multiShape, factory));
    const std::unique_ptr<geos::geom::Geometry> result(geos::operation::buffer::BufferOp::bufferOp(multiPolygon.get(), radius));
    return GeometryToMultiShape(result.get());
}

} // namespace LineArcOffsetDemo
