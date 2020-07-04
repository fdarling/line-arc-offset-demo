#include "GeometryQt.h"

#include <QDebug>

QT_BEGIN_NAMESPACE

QDebug operator<<(QDebug debug, const LineArcGeometry::Point &pt)
{
    QDebugStateSaver saver(debug);
    // debug.nospace() << "Point(" << pt.x << ", " << pt.y << ")";
    debug.nospace() << "Point(" << (qFuzzyIsNull(pt.x) ? 0.0 : pt.x) << ", " << (qFuzzyIsNull(pt.y) ? 0.0 : pt.y) << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcGeometry::Line &line)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "Line("<< line.p1 << line.p2 << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcGeometry::Segment &segment)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "Segment("<< segment.line << ")"; // TODO dump other properties
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcGeometry::Contour &contour)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Contour(segments = [";
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        if (it != contour.segments.begin())
        {
            debug << ", ";
        }
        debug << *it;
    }
    debug << "])";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcGeometry::Shape &shape)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Shape(boundary = ";
    debug << shape.boundary;
    debug << ", holes = [";
    for (std::list<LineArcGeometry::Contour>::const_iterator it = shape.holes.begin(); it != shape.holes.end(); ++it)
    {
        if (it != shape.holes.begin())
        {
            debug << ", ";
        }
        debug << *it;
    }
    debug << "])";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcGeometry::MultiShape &multiShape)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "MultiShape(shapes = [";
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        if (it != multiShape.shapes.begin())
        {
            debug << ", ";
        }
        debug << *it;
    }
    debug << "])";
    return debug;
}

QT_END_NAMESPACE

namespace LineArcGeometry {

QPointF PointToQPointF(const Point &pt)
{
    return QPointF(pt.x, pt.y);
}

} // namespace LineArcGeometry
