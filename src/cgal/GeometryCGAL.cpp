#include "GeometryCGAL.h"
#include "CGALQt.h"
#include "../GeometryQt.h"

namespace LineArcOffsetDemo {

Point_2 PointToPoint_2(const LineArcGeometry::Point &pt)
{
    return Point_2(pt.x, pt.y);
}

template <typename T>
static LineArcGeometry::Point Point_2toPoint(const T &pt)
{
    return LineArcGeometry::Point(CGAL::to_double(pt.x()), CGAL::to_double(pt.y()));
}

LineArcGeometry::Point Point_2ToPoint(const Point_2 &pt)
{
    return Point_2toPoint(pt);
}

LineArcGeometry::Point Point_2ToPoint(const Traits_2::Point_2 &pt)
{
    return Point_2toPoint(pt);
}

Polygon_2 ContourToPolygon(const LineArcGeometry::Contour &contour, const CGAL::Orientation desiredOrientation)
{
    Traits_2 traits;
    Polygon_2 result;
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        std::list<CGAL::Object> objects;
        if (it->isArc)
        {
            const LineArcGeometry::Point midPoint = it->midPoint();
            const Point_2 pt1(PointToPoint_2(it->line.p1));
            const Point_2 pt2(PointToPoint_2(it->line.p2));
            const Point_2 pt3(PointToPoint_2(midPoint));
            const Curve_2 curve(pt1, pt3, pt2);
            if (pt1 != pt2) // NOTE: CGAL doesn't like zero-length arcs!
            {
                addCurveToPolygon(result, curve);
            }
            else
            {
                qDebug() << "WARNING: zero length arc Segment in Contour, ignoring when adding to Polygon_2!";
            }
        }
        else // it's a line
        {
            if (it->line.p1 != it->line.p2) // NOTE: CGAL doesn't like zero-length lines!
            {
                const Point_2 pt1(PointToPoint_2(it->line.p1));
                const Point_2 pt2(PointToPoint_2(it->line.p2));
                const Curve_2 curve(pt1, pt2);
                addCurveToPolygon(result, curve);
            }
            else
            {
                qDebug() << "WARNING: zero length line Segment in Contour, ignoring when adding to Polygon_2!";
            }
        }
    }
    if (result.orientation() != desiredOrientation)
    {
        result.reverse_orientation();
    }
    return result;
}

Polygon_with_holes_2 ShapeToPolygonWithHoles(const LineArcGeometry::Shape &shape)
{
    Polygon_with_holes_2 result(ContourToPolygon(shape.boundary));
    for (std::list<LineArcGeometry::Contour>::const_iterator it = shape.holes.begin(); it != shape.holes.end(); ++it)
    {
        result.add_hole(ContourToPolygon(*it, CGAL::NEGATIVE));
    }
    return result;
}

std::list<Polygon_with_holes_2> MultiShapeToPolygonWithHolesList(const LineArcGeometry::MultiShape &multiShape)
{
    std::list<Polygon_with_holes_2> result;
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        Polygon_with_holes_2 polygon = ShapeToPolygonWithHoles(*it);
        result.push_back(polygon);
    }
    return result;
}

LineArcGeometry::Contour PolygonToContour(const Polygon_2 &polygon)
{
    LineArcGeometry::Contour result;
    for (Polygon_2::Curve_const_iterator curve_it = polygon.curves_begin(); curve_it != polygon.curves_end(); ++curve_it)
    {
        const X_monotone_curve_2 &curve = *curve_it;
        if (curve.is_circular())
        {
            const LineArcGeometry::Point circleCenter(Point_2ToPoint(curve.supporting_circle().center()));
            const LineArcGeometry::Segment::Orientation orientation = (curve.orientation() == CGAL::CLOCKWISE) ? LineArcGeometry::Segment::Clockwise : LineArcGeometry::Segment::CounterClockwise;
            QPointF  startPoint(Point_2_To_QPointF(curve.source()));
            QPointF    endPoint(Point_2_To_QPointF(curve.target()));
            const double lineLength = QLineF(startPoint, endPoint).length();
            if (lineLength < 0.0001) // HACK, this is to fix some sort of rounding errors with .angle()...
            {
                const LineArcGeometry::Point p1(Point_2ToPoint(curve.source()));
                const LineArcGeometry::Point p2(Point_2ToPoint(curve.target()));
                result.segments.push_back(LineArcGeometry::Line(p1, p2));
            }
            else
            {
                const LineArcGeometry::Point p1(Point_2ToPoint(curve.source()));
                const LineArcGeometry::Point p2(Point_2ToPoint(curve.target()));
                result.segments.push_back(LineArcGeometry::Segment(LineArcGeometry::Line(p1, p2), circleCenter, orientation));
            }
        }
        else
        {
            const LineArcGeometry::Point p1(Point_2ToPoint(curve.source()));
            const LineArcGeometry::Point p2(Point_2ToPoint(curve.target()));
            result.segments.push_back(LineArcGeometry::Line(p1, p2));
        }
    }
    if (!result.isValid())
    {
        qDebug() << "ERROR: generated invalid Contour from Polygon_2!" << result;
    }
    return result;
}

LineArcGeometry::Shape PolygonWithHolesToShape(const Polygon_with_holes_2 &polygon)
{
    LineArcGeometry::Shape result(PolygonToContour(polygon.outer_boundary()));
    for (Polygon_with_holes_2::Hole_const_iterator hole_it = polygon.holes_begin(); hole_it != polygon.holes_end(); ++hole_it)
    {
        result.holes.push_back(PolygonToContour(*hole_it));
    }
    return result;
}

LineArcGeometry::MultiShape PolygonWithHolesListToMultiShape(const std::list<Polygon_with_holes_2> &polygons)
{
    LineArcGeometry::MultiShape result;
    for (std::list<Polygon_with_holes_2>::const_iterator it = polygons.begin(); it != polygons.end(); ++it)
    {
        result.shapes.push_back(PolygonWithHolesToShape(*it));
    }
    return result;
}

} // namespace LineArcOffsetDemo
