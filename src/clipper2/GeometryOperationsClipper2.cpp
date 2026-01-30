#include "GeometryOperationsClipper2.h"
#include "GeometryClipper2.h"
#include "../GeometryQt.h"

#include <QDebug>

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape GeometryOperationsClipper2::identity(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsClipper2::identity()";
    const Clipper2Lib::Paths paths = MultiShapeToPaths64(multiShape);
    return Paths64ToMultiShape(paths);
}

static LineArcGeometry::MultiShape DoUnary(const LineArcGeometry::MultiShape &multiShape, const Clipper2Lib::ClipType operation)
{
    Clipper2Lib::PolyTree64 solution;
    {
        Clipper2Lib::Clipper64 c;
        const Clipper2Lib::Paths64 paths = MultiShapeToPaths64(multiShape);
        c.AddSubject(paths);
        c.Execute(operation, Clipper2Lib::FillRule::Positive, solution);
    }
    return PolyTree64ToMultiShape(solution);
}

static LineArcGeometry::MultiShape DoBoolean(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b, const Clipper2Lib::ClipType operation)
{
    Clipper2Lib::PolyTree64 solution;
    {
        Clipper2Lib::Clipper64 c;
        const Clipper2Lib::Paths64 pathsA = MultiShapeToPaths64(a);
        const Clipper2Lib::Paths64 pathsB = MultiShapeToPaths64(b);
        c.AddSubject(pathsA);
        c.AddClip(pathsB);
        c.Execute(operation, Clipper2Lib::FillRule::Positive, solution);
    }
    return PolyTree64ToMultiShape(solution);
}

LineArcGeometry::MultiShape GeometryOperationsClipper2::join(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsClipper2::join()";
    return DoUnary(multiShape, Clipper2Lib::ClipType::Union);
}

LineArcGeometry::MultiShape GeometryOperationsClipper2::join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsClipper2::join()";
    return DoBoolean(a, b, Clipper2Lib::ClipType::Union);
}

LineArcGeometry::MultiShape GeometryOperationsClipper2::intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsClipper2::intersection()";
    return DoBoolean(a, b, Clipper2Lib::ClipType::Intersection);
}

LineArcGeometry::MultiShape GeometryOperationsClipper2::difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsClipper2::difference()";
    return DoBoolean(a, b, Clipper2Lib::ClipType::Difference);
}

LineArcGeometry::MultiShape GeometryOperationsClipper2::symmetricDifference(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsClipper2::symmetricDifference()";
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

LineArcGeometry::MultiShape GeometryOperationsClipper2::symmetricDifference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsClipper2::symmetricDifference()";
    return DoBoolean(a, b, Clipper2Lib::ClipType::Xor);
}

LineArcGeometry::MultiShape GeometryOperationsClipper2::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    // qDebug() << "GeometryOperationsClipper2::offset()";
    const Clipper2CoordinateType delta = ToClipperInt64(radius);
    if (delta == 0)
        return multiShape;
    Clipper2Lib::PolyTree64 offsetPolyTree;
    {
        const Clipper2Lib::Paths paths = MultiShapeToPaths64(multiShape);
        Clipper2Lib::ClipperOffset offsetter;
        offsetter.AddPaths(paths, Clipper2Lib::JoinType::Round, Clipper2Lib::EndType::Polygon);
        offsetter.Execute(delta, offsetPolyTree);
    }
    // qDebug() << "Converting offset results, calling PolyTree64ToMultiShape()...";
    const LineArcGeometry::MultiShape offsetMultiShape = PolyTree64ToMultiShape(offsetPolyTree);
    return offsetMultiShape;
}

} // namespace LineArcOffsetDemo
