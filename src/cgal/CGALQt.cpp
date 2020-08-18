#include "CGALQt.h"
#include "../GeometryQt.h"

QT_BEGIN_NAMESPACE

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Point_2 &pt)
{
    const LineArcGeometry::Point point(LineArcOffsetDemo::Point_2ToPoint(pt));
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << point;
    return debug;
}

template <typename T>
static QDebug WriteCircularArcCurveToDebug(QDebug debug, const T &curve)

{
    QDebugStateSaver saver(debug);
    debug.nospace();
    const LineArcGeometry::Point p1(LineArcOffsetDemo::Point_2ToPoint(curve.source()));
    const LineArcGeometry::Point p2(LineArcOffsetDemo::Point_2ToPoint(curve.target()));
    const LineArcGeometry::Line line(p1, p2);
    if (curve.is_circular())
    {
        const LineArcGeometry::Point circleCenter(LineArcOffsetDemo::Point_2ToPoint(curve.supporting_circle().center()));
        debug << "Curve_2<Arc>(" << line << ", center = " << circleCenter << ", " << ((curve.orientation() == CGAL::CLOCKWISE) ? "CW" : "CCW") << ")";
    }
    else
    {
        debug << "Curve_2<Line>(" << line << ")";
    }
    return debug;
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::X_monotone_curve_2 &curve)
{
    return WriteCircularArcCurveToDebug(debug, curve);
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Curve_2 &curve)
{
    return WriteCircularArcCurveToDebug(debug, curve);
}

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Polygon_2 &polygon)
{
    QDebugStateSaver saver(debug);
    debug.nospace();
    debug << "Polygon_2(";
    for (LineArcOffsetDemo::Polygon_2::Curve_const_iterator curve_it = polygon.curves_begin(); curve_it != polygon.curves_end(); ++curve_it)
    {
        if (curve_it != polygon.curves_begin())
            debug.nospace() << ", "; // TODO determine if redundant .nospace() call is necessary
        debug << *curve_it;
    }
    debug << ")";
    return debug;
}

QT_END_NAMESPACE

namespace LineArcOffsetDemo {

Point_2 QPointF_To_Point_2(const QPointF &pt)
{
    const LineArcGeometry::Point point(LineArcGeometry::QPointFToPoint(pt));
    return PointToPoint_2(point);
}

template <typename T>
static QPointF Point_2_to_QPointF(const T &pt)
{
    const LineArcGeometry::Point point(Point_2ToPoint(pt));
    return LineArcGeometry::PointToQPointF(point);
}

QPointF Point_2_To_QPointF(const Point_2 &pt)
{
    return Point_2_to_QPointF(pt);
}

QPointF Point_2_To_QPointF(const Traits_2::Point_2 &pt)
{
    return Point_2_to_QPointF(pt);
}

} // namespace LineArcOffsetDemo
