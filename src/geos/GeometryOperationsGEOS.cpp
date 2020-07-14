#include "GeometryOperationsGEOS.h"
#include "GeometryGEOS.h"
#include "../GeometryQt.h"

#include <geos/geom/PrecisionModel.h>
#include <geos/operation/union/UnaryUnionOp.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/buffer/BufferOp.h>

#include <memory>

#include <QDebug>

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape GeometryOperationsGEOS::identity(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsGEOS::identity()";
    std::unique_ptr<geos::geom::PrecisionModel> pm(new geos::geom::PrecisionModel());
    geos::geom::GeometryFactory::unique_ptr factory = geos::geom::GeometryFactory::create(pm.get(), -1);
    std::unique_ptr<geos::geom::MultiPolygon> multiPolygon(MultiShapeToMultiPolygon(multiShape, factory.get()));
    return MultiPolygonToMultiShape(multiPolygon.get());
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::join(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsGEOS::join()";
    std::unique_ptr<geos::geom::PrecisionModel> pm(new geos::geom::PrecisionModel());
    geos::geom::GeometryFactory::unique_ptr factory = geos::geom::GeometryFactory::create(pm.get(), -1);
    std::unique_ptr<geos::geom::MultiPolygon> multiPolygon(MultiShapeToMultiPolygon(multiShape, factory.get()));
    std::unique_ptr<geos::geom::Geometry> joined(geos::operation::geounion::UnaryUnionOp::Union(*multiPolygon));
    return GeometryToMultiShape(joined.get());
}

static LineArcGeometry::MultiShape DoBoolean(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b, geos::operation::overlay::OverlayOp::OpCode type)
{
    std::unique_ptr<geos::geom::PrecisionModel> pm(new geos::geom::PrecisionModel());
    geos::geom::GeometryFactory::unique_ptr factory = geos::geom::GeometryFactory::create(pm.get(), -1);
    std::unique_ptr<geos::geom::MultiPolygon> aa(MultiShapeToMultiPolygon(a, factory.get()));
    std::unique_ptr<geos::geom::MultiPolygon> bb(MultiShapeToMultiPolygon(b, factory.get()));
    geos::operation::overlay::OverlayOp op(aa.get(), bb.get());
    std::unique_ptr<geos::geom::Geometry> result(op.getResultGeometry(type));
    return GeometryToMultiShape(result.get());
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsGEOS::join()";
    return DoBoolean(a, b, geos::operation::overlay::OverlayOp::OpCode::opUNION);
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsGEOS::intersection()";
    return DoBoolean(a, b, geos::operation::overlay::OverlayOp::OpCode::opINTERSECTION);
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsGEOS::difference()";
    return DoBoolean(a, b, geos::operation::overlay::OverlayOp::OpCode::opDIFFERENCE);
}

LineArcGeometry::MultiShape GeometryOperationsGEOS::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    // qDebug() << "GeometryOperationsGEOS::offset()";
    std::unique_ptr<geos::geom::PrecisionModel> pm(new geos::geom::PrecisionModel());
    geos::geom::GeometryFactory::unique_ptr factory = geos::geom::GeometryFactory::create(pm.get(), -1);
    std::unique_ptr<geos::geom::MultiPolygon> multiPolygon(MultiShapeToMultiPolygon(multiShape, factory.get()));
    std::unique_ptr<geos::geom::Geometry> result(geos::operation::buffer::BufferOp::bufferOp(multiPolygon.get(), radius));
    return GeometryToMultiShape(result.get());
}

} // namespace LineArcOffsetDemo
