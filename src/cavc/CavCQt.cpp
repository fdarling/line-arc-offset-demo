#include "GeometryCavC.h"
#include "CavCQt.h"

QT_BEGIN_NAMESPACE

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::PVertex &v)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "PVertex(" << LineArcOffsetDemo::FromCavC(v.x()) << ", " << LineArcOffsetDemo::FromCavC(v.y());
    if (!v.bulgeIsZero())
        debug << ", " << v.bulge();
    debug << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Polyline &polyline)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Polyline([";
    for (std::vector<LineArcOffsetDemo::PVertex>::const_iterator it = polyline.vertexes().begin(); it != polyline.vertexes().end(); ++it)
    {
        if (it != polyline.vertexes().begin())
            debug << ", ";
        debug << *it;
    }
    debug << "])";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Polylines &polylines)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Polylines([";
    for (LineArcOffsetDemo::Polylines::const_iterator it = polylines.begin(); it != polylines.end(); ++it)
    {
        if (it != polylines.begin())
            debug << ", ";
        debug << *it;
    }
    debug << "])";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::CavC_Shape &shape)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "CavC_Shape(boundary = " << shape.boundary << ", holes = " << shape.holes;
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::CavC_MultiShape &multiShape)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "CavC_MultiShape(";
    for (LineArcOffsetDemo::CavC_MultiShape::const_iterator it = multiShape.begin(); it != multiShape.end(); ++it)
    {
        if (it != multiShape.begin())
            debug << ", ";
        debug << *it;
    }
    debug << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::OffsetLoop &loop)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "OffsetLoop(parentLoopIndex = " << loop.parentLoopIndex << ", polyline = " << loop.polyline << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::OffsetLoops &loops)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "OffsetLoops([";
    for (LineArcOffsetDemo::OffsetLoops::const_iterator it = loops.begin(); it != loops.end(); ++it)
    {
        if (it != loops.begin())
            debug << ", ";
        debug << *it;
    }
    debug << "])";
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::OffsetLoopSet &loopSet)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "OffsetLoopSet(ccwLoops = " << loopSet.ccwLoops << ", cwLoops = " << loopSet.cwLoops << ")";
    return debug;
}

QT_END_NAMESPACE
