#include "GeometryOperationsClipper.h"
#include "GeometryClipper.h"
#include "../GeometryQt.h"

#include <QDebug>

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape GeometryOperationsClipper::identity(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsClipper::identity()";
    const ClipperLib::Paths paths = MultiShapeToPaths(multiShape);
    return PathsToMultiShape(paths);
}

LineArcGeometry::MultiShape GeometryOperationsClipper::join(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsClipper::join()";
    ClipperLib::PolyTree solution;
    {
        ClipperLib::Clipper c;
        const ClipperLib::Paths paths = MultiShapeToPaths(multiShape);
        c.AddPaths(paths, ClipperLib::ptSubject, true);
        c.Execute(ClipperLib::ctUnion, solution, ClipperLib::pftPositive, ClipperLib::pftPositive);
    }
    return PolyTreeToMultiShape(solution);
}

static LineArcGeometry::MultiShape DoBoolean(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b, const ClipperLib::ClipType operation, const ClipperLib::PolyType bType = ClipperLib::ptSubject)
{
    // qDebug() << "GeometryOperationsClipper::join()";
    ClipperLib::PolyTree solution;
    {
        ClipperLib::Clipper c;
        const ClipperLib::Paths pathsA = MultiShapeToPaths(a);
        const ClipperLib::Paths pathsB = MultiShapeToPaths(b);
        c.AddPaths(pathsA, ClipperLib::ptSubject, true);
        c.AddPaths(pathsB, bType, true);
        c.Execute(operation, solution, ClipperLib::pftPositive, ClipperLib::pftPositive);
    }
    return PolyTreeToMultiShape(solution);
}

LineArcGeometry::MultiShape GeometryOperationsClipper::join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsClipper::join()";
    return DoBoolean(a, b, ClipperLib::ctUnion);
}

LineArcGeometry::MultiShape GeometryOperationsClipper::intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsClipper::intersection()";
    return DoBoolean(a, b, ClipperLib::ctIntersection, ClipperLib::ptClip);
}

LineArcGeometry::MultiShape GeometryOperationsClipper::difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsClipper::difference()";
    return DoBoolean(a, b, ClipperLib::ctDifference, ClipperLib::ptClip);
}

LineArcGeometry::MultiShape GeometryOperationsClipper::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    // qDebug() << "GeometryOperationsClipper::offset()";
    const ClipperLib::cInt delta = ToClipperInt(radius);
    if (delta == 0)
        return multiShape;
    ClipperLib::PolyTree solution;
    {
        const ClipperLib::Paths paths = MultiShapeToPaths(multiShape);
        ClipperLib::ClipperOffset c;
        c.AddPaths(paths, ClipperLib::jtRound, ClipperLib::etOpenRound);
        c.Execute(solution, std::abs(delta));
    }
    // qDebug() << "Converting offset results...";
    // return PolyTreeToMultiShape(solution);

    // outset
    if (delta > 0)
        return DoBoolean(PolyTreeToMultiShape(solution), multiShape, ClipperLib::ctUnion);

    // inset
    const LineArcGeometry::MultiShape insideHalf = DoBoolean(PolyTreeToMultiShape(solution), multiShape, ClipperLib::ctIntersection, ClipperLib::ptClip);
    const LineArcGeometry::MultiShape holesOnly = DoBoolean(insideHalf, multiShape, ClipperLib::ctXor, ClipperLib::ptClip);
    return holesOnly;
}

} // namespace LineArcOffsetDemo
