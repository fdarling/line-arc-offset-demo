#include "GeometryClipper.h"
#include "../GeometryQt.h"

#include <cmath>
#include <iostream>

#include <QDebug>

QT_BEGIN_NAMESPACE

static QDebug operator<<(QDebug debug, const ClipperLib::IntPoint &pt) __attribute__((unused));
static QDebug operator<<(QDebug debug, const ClipperLib::IntPoint &pt)
{
    QDebugStateSaver saver(debug);
    const LineArcGeometry::Point p = LineArcOffsetDemo::IntPointToPoint(pt);
    debug.nospace() << "IntPoint(" << p.x << ", " << p.y << ")";
    return debug;
}

static QDebug operator<<(QDebug debug, const ClipperLib::Path &path) __attribute__((unused));
static QDebug operator<<(QDebug debug, const ClipperLib::Path &path)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Path([";
    for (ClipperLib::Path::const_iterator it = path.begin(); it != path.end(); ++it)
    {
        const LineArcGeometry::Point p = LineArcOffsetDemo::IntPointToPoint(*it);
        debug << (it != path.begin() ? ", (" : "(") << p.x << ", " << p.y << ")";
    }
    debug << "])";
    return debug;
}

static QDebug operator<<(QDebug debug, const ClipperLib::Paths &paths) __attribute__((unused));
static QDebug operator<<(QDebug debug, const ClipperLib::Paths &paths)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Paths([";
    for (ClipperLib::Paths::const_iterator it = paths.begin(); it != paths.end(); ++it)
    {
        debug << (it != paths.begin() ? ", (" : "(") << *it << ")";
    }
    debug << "])";
    return debug;
}

QT_END_NAMESPACE

namespace LineArcOffsetDemo {

static const int FIXED_POINT_SCALAR = 10000;

LineArcGeometry::CoordinateType FromClipperInt(ClipperLib::cInt value)
{
    return static_cast<LineArcGeometry::CoordinateType>(value)/FIXED_POINT_SCALAR;
}

ClipperLib::cInt ToClipperInt(LineArcGeometry::CoordinateType value)
{
    return value*FIXED_POINT_SCALAR;
}

ClipperLib::IntPoint PointToIntPoint(const LineArcGeometry::Point &pt)
{
    return ClipperLib::IntPoint(ToClipperInt(pt.x), ToClipperInt(pt.y));
}

LineArcGeometry::Point IntPointToPoint(const ClipperLib::IntPoint &pt)
{
    return LineArcGeometry::Point(FromClipperInt(pt.X), FromClipperInt(pt.Y));
}

static void AddToPath(ClipperLib::Path &path, const ClipperLib::IntPoint &pt)
{
    if (path.empty() || path.back() != pt)
    {
        path.push_back(pt);
    }
}

static ClipperLib::Path ContourToPath(const LineArcGeometry::Contour &contour)
{
    ClipperLib::Path result;
    if (!contour.segments.empty())
        AddToPath(result, PointToIntPoint(contour.segments.front().line.p1));
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        if (it->isArc)
        {
            const double radius = it->radius();
            const double angleStepDegrees = 15.0;
            const double angleStep = angleStepDegrees*M_PI/180.0;
                  double angle1 = atan2(it->line.p1.y - it->center.y, it->line.p1.x - it->center.x);
            const double angle2 = atan2(it->line.p2.y - it->center.y, it->line.p2.x - it->center.x);
            const ClipperLib::Path::size_type beforeSize = result.size();
            if (it->orientation == LineArcGeometry::Segment::CounterClockwise)
            {
                if (angle1 > angle2)
                    angle1 -= 2*M_PI;
                for (double angle = angle1; angle < angle2; angle += angleStep)
                {
                    const LineArcGeometry::Point pt = it->center + LineArcGeometry::Point(cos(angle), sin(angle))*radius;
                    AddToPath(result, PointToIntPoint(pt));
                }
            }
            else // Clockwise
            {
                if (angle1 < angle2)
                    angle1 += 2*M_PI;
                for (double angle = angle1; angle > angle2; angle -= angleStep)
                {
                    const LineArcGeometry::Point pt = it->center + LineArcGeometry::Point(cos(angle), sin(angle))*radius;
                    AddToPath(result, PointToIntPoint(pt));
                }
            }

            // ensure at least the midpoint is present (may not be necesary)
            const ClipperLib::Path::size_type afterSize = result.size();
            const ClipperLib::IntPoint midPoint = PointToIntPoint(it->midPoint());
            if (afterSize == beforeSize)
            {
                AddToPath(result, midPoint);
            }

            // ensure the destination is present
            const ClipperLib::IntPoint endPoint = PointToIntPoint(it->line.p2);
            AddToPath(result, endPoint);
        }
        else
        {
            // add the desination
            result.push_back(PointToIntPoint(it->line.p2));
        }
    }
    return result;
}

static bool PathIsHole(const ClipperLib::Path &path) __attribute__((unused));
static bool PathIsHole(const ClipperLib::Path &path)
{
    return !ClipperLib::Orientation(path);
}

static void EnsurePositive(ClipperLib::Path &path)
{
    (void)path;
    if (PathIsHole(path))
        ClipperLib::ReversePath(path);
}

static void EnsureNegative(ClipperLib::Path &path)
{
    (void)path;
    if (!PathIsHole(path))
        ClipperLib::ReversePath(path);
}

ClipperLib::Paths MultiShapeToPaths(const LineArcGeometry::MultiShape &multiShape)
{
    ClipperLib::Paths result;
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

LineArcGeometry::Contour PathToContour(const ClipperLib::Path &path)
{
    LineArcGeometry::Contour result;

    if (path.size() < 2)
        return result;

    const ClipperLib::IntPoint *prev = &path.back();
    for (ClipperLib::Path::const_iterator it = path.begin(); it != path.end(); prev = &*it, ++it)
    {
        const LineArcGeometry::Point p1 = IntPointToPoint(*prev);
        const LineArcGeometry::Point p2 = IntPointToPoint(*it);
        const LineArcGeometry::Line line(p1, p2);
        result.segments.push_back(LineArcGeometry::Segment(line));
    }

    return result;
}

LineArcGeometry::MultiShape PathsToMultiShape(const ClipperLib::Paths &paths)
{
    LineArcGeometry::MultiShape result;
    for (ClipperLib::Paths::const_iterator path_it = paths.begin(); path_it != paths.end(); ++path_it)
    {
        result.shapes.push_back(LineArcGeometry::Shape(PathToContour(*path_it)));
    }
    return result;
}

// #define PRINTING_TREE

void AddShapesFromPolyNodeList(LineArcGeometry::MultiShape &result, const ClipperLib::PolyNode *node, LineArcGeometry::Shape *parentShape)
{
    if (!node)
        return;
#ifdef PRINTING_TREE
    static int tabLevel = -1;
    tabLevel++;
    for (int i = 0; i < tabLevel; i++) std::cout << "\t";
    std::cout << "node " << (node->IsHole() ? "hole" : "boundary") << std::endl;
#endif // PRINTING_TREE
    const LineArcGeometry::Contour contour = PathToContour(node->Contour);

    LineArcGeometry::Shape *boundary = nullptr;
    if (!node->Contour.empty())
    {
        if (!node->IsHole()) // boundary
        {
            result.shapes.push_back(LineArcGeometry::Shape(contour));
            boundary = &result.shapes.back();
        }
        else // hole
        {
            if (parentShape)
            {
                parentShape->holes.push_back(contour);
            }
            else
            {
                qDebug() << "WARNING: attempting to add ClipperLib::PolyNode hole without destination LineArcGeometry::Shape as a parent!";
            }
        }
    }

    // traverse children
    for (ClipperLib::PolyNodes::const_iterator it = node->Childs.begin(); it != node->Childs.end(); ++it)
    {
        AddShapesFromPolyNodeList(result, *it, boundary);
    }
#ifdef PRINTING_TREE
    tabLevel--;
#endif // PRINTING_TREE
}

LineArcGeometry::MultiShape PolyTreeToMultiShape(const ClipperLib::PolyTree &tree)
{
    LineArcGeometry::MultiShape result;
    // LineArcGeometry::Shape *parentShape = nullptr;
#ifdef PRINTING_TREE
    std::cout << "===============" << std::endl;
#endif // PRINTING_TREE
    AddShapesFromPolyNodeList(result, &tree, nullptr);
#ifdef PRINTING_TREE
    std::cout << "---------------" << std::endl;
#endif // PRINTING_TREE

    return result;
}

} // namespace LineArcOffsetDemo
