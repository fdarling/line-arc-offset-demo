#include "GeometryClipper2.h"
#include "../GeometryQt.h"

#include <cmath>
#include <iostream>

#include <QDebug>

QT_BEGIN_NAMESPACE

static QDebug operator<<(QDebug debug, const Clipper2Lib::Point64 &pt) __attribute__((unused));
static QDebug operator<<(QDebug debug, const Clipper2Lib::Point64 &pt)
{
    QDebugStateSaver saver(debug);
    const LineArcGeometry::Point p = LineArcOffsetDemo::Point64ToPoint(pt);
    debug.nospace() << "Point64(" << p.x << ", " << p.y << ")";
    return debug;
}

static QDebug operator<<(QDebug debug, const Clipper2Lib::Path64 &path) __attribute__((unused));
static QDebug operator<<(QDebug debug, const Clipper2Lib::Path64 &path)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Path64([";
    for (Clipper2Lib::Path64::const_iterator it = path.begin(); it != path.end(); ++it)
    {
        const LineArcGeometry::Point p = LineArcOffsetDemo::Point64ToPoint(*it);
        debug << (it != path.begin() ? ", (" : "(") << p.x << ", " << p.y << ")";
    }
    debug << "])";
    return debug;
}

static QDebug operator<<(QDebug debug, const Clipper2Lib::Paths64 &paths) __attribute__((unused));
static QDebug operator<<(QDebug debug, const Clipper2Lib::Paths64 &paths)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Paths64([";
    for (Clipper2Lib::Paths64::const_iterator it = paths.begin(); it != paths.end(); ++it)
    {
        debug << (it != paths.begin() ? ", (" : "(") << *it << ")";
    }
    debug << "])";
    return debug;
}

QT_END_NAMESPACE

namespace LineArcOffsetDemo {

static const Clipper2CoordinateType FIXED_POINT_SCALAR = 1000000;

LineArcGeometry::CoordinateType FromClipperInt64(Clipper2CoordinateType value)
{
    return static_cast<LineArcGeometry::CoordinateType>(value)/FIXED_POINT_SCALAR;
}

Clipper2CoordinateType ToClipperInt64(LineArcGeometry::CoordinateType value)
{
    return value*FIXED_POINT_SCALAR;
}

Clipper2Lib::Point64 PointToPoint64(const LineArcGeometry::Point &pt)
{
    return Clipper2Lib::Point64(ToClipperInt64(pt.x), ToClipperInt64(pt.y));
}

LineArcGeometry::Point Point64ToPoint(const Clipper2Lib::Point64 &pt)
{
    return LineArcGeometry::Point(FromClipperInt64(pt.x), FromClipperInt64(pt.y));
}

static Clipper2Lib::Path64 ContourToPath(const LineArcGeometry::Contour &contour)
{
    const LineArcGeometry::Contour approximated = contour.approximatedArcs();
    Clipper2Lib::Path64 result;
    if (!approximated.segments.empty())
        result.push_back(PointToPoint64(approximated.segments.front().line.p1));
    for (std::list<LineArcGeometry::Segment>::const_iterator it = approximated.segments.begin(); it != approximated.segments.end(); ++it)
    {
        result.push_back(PointToPoint64(it->line.p2));
    }
    return result;
}

static bool PathIsHole(const Clipper2Lib::Path64 &path) __attribute__((unused));
static bool PathIsHole(const Clipper2Lib::Path64 &path)
{
    return !Clipper2Lib::IsPositive(path);
}

static void EnsurePositive(Clipper2Lib::Path64 &path)
{
    if (PathIsHole(path))
        std::reverse(path.begin(), path.end());
}

static void EnsureNegative(Clipper2Lib::Path64 &path)
{
    if (!PathIsHole(path))
        std::reverse(path.begin(), path.end());
}

Clipper2Lib::Paths64 MultiShapeToPaths64(const LineArcGeometry::MultiShape &multiShape)
{
    Clipper2Lib::Paths64 result;
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        const LineArcGeometry::Shape &shape = *shape_it;

        // add boundary
        result.push_back(ContourToPath(shape.boundary));
        EnsurePositive(result.back());

        // add holes
        for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
        {
            result.push_back(ContourToPath(*hole_it));
            EnsureNegative(result.back());
        }
    }
    return result;
}

LineArcGeometry::Contour Path64ToContour(const Clipper2Lib::Path64 &path)
{
    LineArcGeometry::Contour result;

    if (path.size() < 2)
        return result;

    const Clipper2Lib::Point64 *prev = &path.back();
    for (Clipper2Lib::Path64::const_iterator it = path.begin(); it != path.end(); prev = &*it, ++it)
    {
        const LineArcGeometry::Point p1 = Point64ToPoint(*prev);
        const LineArcGeometry::Point p2 = Point64ToPoint(*it);
        const LineArcGeometry::Line line(p1, p2);
        result.segments.push_back(LineArcGeometry::Segment(line));
    }

    return result;
}

LineArcGeometry::MultiShape Paths64ToMultiShape(const Clipper2Lib::Paths64 &paths)
{
    LineArcGeometry::MultiShape result;
    for (Clipper2Lib::Paths64::const_iterator path_it = paths.begin(); path_it != paths.end(); ++path_it)
    {
        result.shapes.push_back(LineArcGeometry::Shape(Path64ToContour(*path_it)));
    }
    return result;
}

// #define PRINTING_TREE

void AddShapesFromPolyPath64List(LineArcGeometry::MultiShape &result, const Clipper2Lib::PolyPath64 &node, LineArcGeometry::Shape *parentShape)
{
    // grab the path for the current level
    const Clipper2Lib::Path64 &outerPath = node.Polygon();

#ifdef PRINTING_TREE
    static int tabLevel = -1;
    tabLevel++;
    for (int i = 0; i < tabLevel; i++) std::cout << "\t";
    if (outerPath.empty())
        std::cout << "node (container only)" << std::endl;
    else
        std::cout << "node " << (node.IsHole() ? "hole" : "boundary") << std::endl;
#endif // PRINTING_TREE

    // boundary to which we will add holes later
    LineArcGeometry::Shape *boundary = nullptr;

    // make sure we don't try to generate an empty contour!
    if (!outerPath.empty())
    {
        // convert Path64 to Contour
        const LineArcGeometry::Contour contour = Path64ToContour(outerPath);
        if (!node.IsHole()) // boundary
        {
            // add outer boundaries directly to the main MultiShape
            result.shapes.push_back(LineArcGeometry::Shape(contour));

            // remember it for adding the children as holes later
            boundary = &result.shapes.back();
        }
        else // hole
        {
            if (parentShape)
            {
                // add the contour as a hole to the converted parent shape
                parentShape->holes.push_back(contour);
            }
            else
            {
                qDebug() << "WARNING: attempting to add Clipper2Lib::PolyNode hole without destination LineArcGeometry::Shape as a parent!";
            }
        }
    }

    // traverse children (if present, won't be if there isn't a boundary either)
    for (Clipper2Lib::PolyPath64List::const_iterator it = node.begin(); it != node.end(); ++it)
    {
        const Clipper2Lib::PolyPath64List::value_type &entry = *it; // an std::unique_ptr<>
        AddShapesFromPolyPath64List(result, *entry, boundary); // dereference above unique_ptr
    }
#ifdef PRINTING_TREE
    tabLevel--;
#endif // PRINTING_TREE
}

LineArcGeometry::MultiShape PolyTree64ToMultiShape(const Clipper2Lib::PolyTree64 &tree)
{
    LineArcGeometry::MultiShape result;
    // LineArcGeometry::Shape *parentShape = nullptr;
#ifdef PRINTING_TREE
    std::cout << "===============" << std::endl;
#endif // PRINTING_TREE
    AddShapesFromPolyPath64List(result, tree, nullptr);
#ifdef PRINTING_TREE
    std::cout << "---------------" << std::endl;
#endif // PRINTING_TREE

    return result;
}

} // namespace LineArcOffsetDemo
