#include "CGALWrapper.h"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Polygon_2.h> // for transform()
// #include <CGAL/Qt/Converter.h> // for CGAL::Qt::Converter<Kernel>

#include <type_traits>

#include <QtMath>
#include <QLineF>
#include <QRectF>
// #include <QPainterPath>
#include <QDebug>

namespace LineArcOffsetDemo {

// forward declarations
static Polygon_2 construct_circle_polygon(const QPointF &center, double radius);
// static Polygon_with_holes_2 construct_annulus_polygon(const QPointF &center, double radius_inner, double radius_outer);
// static Polygon_2 construct_rect_polygon(const QRectF &rect);
static Polygon_2 construct_wire(const QPointF &startPoint, const QPointF &endPoint, double width);
// static std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_2 &polygon, double radius);
static std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_with_holes_2 &polygon, double radius);
// static std::list<Polygon_with_holes_2> construct_polygon_offset(const Polygon_with_holes_2 &polygon, double radius);
// static std::list<Polygon_with_holes_2> construct_thermal(const QPointF &center, double radius_inner, double isolation, double width);
// static std::list<Polygon_with_holes_2> construct_thermal_negative(const QPointF &center, double radius_inner, double isolation, double width);

template <typename T>
static void DumpCurve(const T &curve)
{
    if (curve.is_circular())
    {
        const Circle_2 circle = curve.supporting_circle();
        const double radius = qSqrt(CGAL::to_double(circle.squared_radius()));
        qDebug() << "CURVE: center @ " << QString::number(CGAL::to_double(circle.center().x()), 'f', 6) << "," << QString::number(CGAL::to_double(circle.center().y()), 'f', 6) << " radius " << QString::number(radius, 'f', 6);
        qDebug() << "       source @ " << QString::number(CGAL::to_double(curve.source().x()), 'f', 6) << "," << QString::number(CGAL::to_double(curve.source().y()), 'f', 6);
        qDebug() << "       target @ " << QString::number(CGAL::to_double(curve.target().x()), 'f', 6) << "," << QString::number(CGAL::to_double(curve.target().y()), 'f', 6);
    }
}

static Point_2 Point_2_from_QPointF(const QPointF &pt)
{
    return Point_2(pt.x(), pt.y());
}

template <typename T>
std::list<Polygon_with_holes_2> construct_grown_curve(const T &curve, double radius_cap)
{
    const QPointF pt1(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
    const QPointF pt2(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
    if (qFuzzyIsNull(QLineF(pt1, pt2).length()))
    {
        std::list<Polygon_with_holes_2> res;
        res.push_back(Polygon_with_holes_2(construct_circle_polygon(pt1, radius_cap)));
        return res;
    }
    else if (curve.is_circular())
    {
        const Circle_2 circle = curve.supporting_circle();
        if constexpr (std::is_same<T, Curve_2>::value)
        if (curve.is_full())
        {
            const double radius = qSqrt(CGAL::to_double(circle.squared_radius()));
            const double radius_outer = radius + radius_cap;
            const double radius_inner = radius - radius_cap;
            const Circle_2 circle_outer(circle.center(), radius_outer*radius_outer);
            const Circle_2 circle_inner(circle.center(), radius_inner*radius_inner);
            Traits_2 traits;
            Polygon_2 outer;
            {
                std::list<CGAL::Object> objects;
                traits.make_x_monotone_2_object() (Curve_2(circle_outer), std::back_inserter(objects));
                X_monotone_curve_2 arc;
                std::list<CGAL::Object>::iterator iter;
                for (iter = objects.begin(); iter != objects.end(); ++iter)
                {
                    CGAL::assign (arc, *iter);
                    outer.push_back (arc);
                }
            }
            std::list<Polygon_with_holes_2> result;
            if (radius_inner < 0.0) // TODO: make sure 0.0 radius circles are okay! This should allow for endmill "drilling"
            {
                result.push_back(Polygon_with_holes_2(outer));
                return result;
            }
            
            Polygon_2 inner;
            {
                std::list<CGAL::Object> objects;
                traits.make_x_monotone_2_object() (Curve_2(circle_inner), std::back_inserter(objects));
                X_monotone_curve_2 arc;
                std::list<CGAL::Object>::iterator iter;
                for (iter = objects.begin(); iter != objects.end(); ++iter)
                {
                    CGAL::assign (arc, *iter);
                    inner.push_back (arc);
                }
            }
            // Polygon_with_holes_2 joined;
            CGAL::difference(outer, inner, std::back_inserter(result));
            // result.push_back(joined);
            return result;
        }
        //else
        {
            const QPointF startPoint(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
            const QPointF   endPoint(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
            const QPointF center(CGAL::to_double(circle.center().x()), CGAL::to_double(circle.center().y()));
            const double radius = qSqrt(CGAL::to_double(circle.squared_radius()));
            const double startAngle = qAtan2(CGAL::to_double(curve.source().y()), CGAL::to_double(curve.source().x()));
                  double   endAngle = qAtan2(CGAL::to_double(curve.target().y()), CGAL::to_double(curve.target().x()));
            if (endAngle < startAngle)
                endAngle += 2.0*M_PI;
            const bool debugging = false;//QLineF(center, QPointF(0.5, 0.5)).length() < 0.01;
            if (debugging)
            {
                qDebug() << "ORIGINAL CURVE:";
                DumpCurve(curve);
                qDebug() << "Constructing rainbow...";
            }
            std::list<Polygon_with_holes_2> pieces;
            {
                const QPointF middle_vec = QLineF(QPointF(0.0, 0.0), (endPoint - startPoint)).normalVector().unitVector().p2();
                const QPointF source_offset = QLineF(QPointF(0.0, 0.0), startPoint - center).unitVector().p2()*radius_cap;
                const QPointF target_offset = QLineF(QPointF(0.0, 0.0),   endPoint - center).unitVector().p2()*radius_cap;
                const Point_2 source_out = Point_2_from_QPointF(startPoint + source_offset);
                const Point_2 source_in  = Point_2_from_QPointF(startPoint - source_offset);
                const Point_2 target_out = Point_2_from_QPointF(  endPoint + target_offset);
                const Point_2 target_in  = Point_2_from_QPointF(  endPoint - target_offset);
                const double reverse_factor = (curve.orientation() == CGAL::COUNTERCLOCKWISE) ? 1.0 : -1.0; // TODO this seems backwards...
                const Point_2 middle_out = Point_2_from_QPointF(center + middle_vec*(radius + radius_cap)*reverse_factor);
                const Point_2 middle_in  = Point_2_from_QPointF(center + middle_vec*(radius - radius_cap)*reverse_factor);
                if (debugging)
                {
                    qDebug() << "middle_vec:" << middle_vec;
                    qDebug() << "middle_in: " << QString::number(CGAL::to_double(middle_in.x()), 'f', 6) << "," << QString::number(CGAL::to_double(middle_in.y()), 'f', 6);
                    qDebug() << "middle_out:" << QString::number(CGAL::to_double(middle_out.x()), 'f', 6) << "," << QString::number(CGAL::to_double(middle_out.y()), 'f', 6);
                }
                Traits_2 traits;
                std::list<CGAL::Object> objects;
                if (curve.orientation() == CGAL::COUNTERCLOCKWISE)
                {
                    if (debugging)
                    {
                        qDebug() << "GENERATED OUTER CCW:";
                        DumpCurve(Curve_2(source_out, middle_out, target_out));
                    }
                    traits.make_x_monotone_2_object() (Curve_2(source_out, middle_out, target_out), std::back_inserter(objects));
                    // traits.make_x_monotone_2_object() (Curve_2(source_out, target_out), std::back_inserter(objects));
                    if (radius > radius_cap && target_in != source_in)
                    {
                        traits.make_x_monotone_2_object() (Curve_2(target_out, target_in), std::back_inserter(objects));
                        if (debugging)
                        {
                            qDebug() << "GENERATED INNER CCW:" << radius << radius_cap;
                            //CGAL_precondition(Kernel().compare_xy_2_object() (p1, p3) != EQUAL);
                            qDebug() << "p1:" << CGAL::to_double(target_in.x()) << "," << CGAL::to_double(target_in.y()) << "p2:" << CGAL::to_double(middle_in.x()) << "," << CGAL::to_double(middle_in.y()) << "p3:" << CGAL::to_double(source_in.x()) << "," << CGAL::to_double(source_in.y());
                            DumpCurve(Curve_2(target_in, middle_in, source_in));
                        }
                        traits.make_x_monotone_2_object() (Curve_2(target_in, middle_in, source_in), std::back_inserter(objects));
                        traits.make_x_monotone_2_object() (Curve_2(source_in, source_out), std::back_inserter(objects));
                    }
                    else
                    {
                        traits.make_x_monotone_2_object() (Curve_2(target_out, circle.center()), std::back_inserter(objects));
                        traits.make_x_monotone_2_object() (Curve_2(circle.center(), source_out), std::back_inserter(objects));
                    }
                }
                else
                {
                    if (debugging)
                    {
                        qDebug() << "GENERATED OUTER CW:";
                        DumpCurve(Curve_2(target_out, middle_out, source_out));
                    }
                    traits.make_x_monotone_2_object() (Curve_2(target_out, middle_out, source_out), std::back_inserter(objects));
                    // traits.make_x_monotone_2_object() (Curve_2(target_out, source_out), std::back_inserter(objects));
                    if (radius > radius_cap)
                    {
                        traits.make_x_monotone_2_object() (Curve_2(source_out, source_in), std::back_inserter(objects));
                        if (debugging)
                        {
                            qDebug() << "GENERATED INNER CW:";
                            DumpCurve(Curve_2(source_in, middle_in, target_in));
                        }
                        traits.make_x_monotone_2_object() (Curve_2(source_in, middle_in, target_in), std::back_inserter(objects));
                        traits.make_x_monotone_2_object() (Curve_2(target_in, target_out), std::back_inserter(objects));
                    }
                    else
                    {
                        traits.make_x_monotone_2_object() (Curve_2(source_out, circle.center()), std::back_inserter(objects));
                        traits.make_x_monotone_2_object() (Curve_2(circle.center(), target_out), std::back_inserter(objects));
                    }
                }
                Polygon_2 pgn;
                {
                    X_monotone_curve_2 arc;
                    std::list<CGAL::Object>::iterator iter;
                    for (iter = objects.begin(); iter != objects.end(); ++iter)
                    {
                        CGAL::assign (arc, *iter);
                        pgn.push_back (arc);
                    }
                }
                pieces.push_back(Polygon_with_holes_2(pgn));
            }
            if (debugging)
                qDebug() << "Constructing source circle...";
            pieces.push_back(Polygon_with_holes_2(construct_circle_polygon(startPoint, radius_cap)));
            if (debugging)
                qDebug() << "Constructing end circle...";
            pieces.push_back(Polygon_with_holes_2(construct_circle_polygon(  endPoint, radius_cap)));
            // return res;
            // return pieces;
            std::list<Polygon_with_holes_2> result;
            CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(result));
            return result;
        }
    }
    else // if (curve.is_linear())
    {
        std::list<Polygon_with_holes_2> res;
        res.push_back(Polygon_with_holes_2(construct_wire(pt1, pt2, radius_cap*2.0)));
        return res;
    }

    // std::list<Polygon_with_holes_2> pieces;
    /*pieces.push_back(Polygon_with_holes_2(construct_rainbow(centerPoint, startAngle, endAngle, radius, width)));
    {
        const QPointF pt1(centerPoint.x() +   endCos*radius_outer                      , centerPoint.y() +   endSin*radius_outer);
        const QPointF pt2(centerPoint.x() +   endCos*radius       -   endSin*radius_cap, centerPoint.y() +   endSin*radius       + endCos*radius_cap);
        const QPointF pt3(centerPoint.x() +   endCos*radius_inner                      , centerPoint.y() +   endSin*radius_inner);
        const QPointF pt4(centerPoint.x() +   endCos*radius       +   endSin*radius_cap, centerPoint.y() +   endSin*radius       - endCos*radius_cap);
        pieces.push_back(Polygon_with_holes_2(construct_four_point_circle(pt1, pt2, pt3, pt4)));
    }
    {
        const QPointF pt1(centerPoint.x() + startCos*radius_outer                      , centerPoint.y() + startSin*radius_outer);
        const QPointF pt2(centerPoint.x() + startCos*radius       - startSin*radius_cap, centerPoint.y() + startSin*radius       + startCos*radius_cap);
        const QPointF pt3(centerPoint.x() + startCos*radius_inner                      , centerPoint.y() + startSin*radius_inner);
        const QPointF pt4(centerPoint.x() + startCos*radius       + startSin*radius_cap, centerPoint.y() + startSin*radius       - startCos*radius_cap);
        pieces.push_back(Polygon_with_holes_2(construct_four_point_circle(pt1, pt2, pt3, pt4)));
    }*/
    // return pieces;
    // std::list<Polygon_with_holes_2> result;
    // CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(result));
    // return result;
}

/*template <typename T>
void AddCurvesToPath(QPainterPath &path, const T &poly)
{
    for (auto curve_it = poly.curves_begin(); curve_it != poly.curves_end(); ++curve_it)
    {
        auto curve = *curve_it;
        if (curve.is_circular())
        {
#if 0
            // TODO use <CGAL/Qt/PainterOstream.h> class instead...
            // NOTE: code ripped from https://github.com/CGAL/cgal/blob/master/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/ArrangementGraphicsItem.h#L558
            const auto &c = curve;
            QPointF source( CGAL::to_double(c.source().x()), CGAL::to_double(c.source().y()) );
            QPointF target( CGAL::to_double(c.target().x()), CGAL::to_double(c.target().y()) );
            // if ( ! this->isProperOrientation( cc ) )
            // {
              // std::swap( source, target );
            // }

            QPointF circleCenter( CGAL::to_double(c.supporting_circle().center().x()),
                                  CGAL::to_double(c.supporting_circle().center().y()) );

            std::swap( source, target );
            double asource = atan2( -(source - circleCenter).y(),
                                    (source - circleCenter).x() );
            double atarget = atan2( -(target - circleCenter).y(),
                                    (target - circleCenter).x() );
            double aspan = atarget - asource;
            std::swap( source, target );

            // CGAL::Qt::Converter< Kernel > convert; // not worth importing this class... bounding box is easy enough to calculate
            const double radius = sqrt(CGAL::to_double(c.supporting_circle().squared_radius()));
            const QRectF  circleRect(circleCenter.x() - radius, circleCenter.y() - radius, radius*2.0, radius*2.0);
            // path.moveTo( source );
            path.arcMoveTo( circleRect, asource * 180/M_PI);
            path.arcTo( circleRect, asource * 180/M_PI, // path.arcTo( convert(c.supporting_circle().bbox()), asource * 180/M_PI,
                        aspan * 180/M_PI );
#else
            auto circle = curve.supporting_circle();
            const double radius = sqrt(CGAL::to_double(circle.squared_radius()));
            const QPointF circleCenter(CGAL::to_double(circle.center().x()), CGAL::to_double(circle.center().y()));
            const QRectF  circleRect(circleCenter.x() - radius, circleCenter.y() - radius, radius*2.0, radius*2.0);
            QPointF  startPoint(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
            QPointF    endPoint(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
            if (curve.orientation() != CGAL::CLOCKWISE)
            {
                qSwap(startPoint, endPoint);
            }
            const double startAngle = QLineF(circleCenter, startPoint).angle();
            const double   endAngle = QLineF(circleCenter,   endPoint).angle();
            // const double sweepAngle = endAngle - startAngle;
            const double sweepAngle = QLineF(circleCenter, startPoint).angleTo(QLineF(circleCenter, endPoint));
            // if (path.elementCount() == 0)
            const double lineLength = QLineF(startPoint, endPoint).length();
            // if (false)
            if (lineLength < 0.0001) // HACK, this is to fix some sort of rounding errors with .angle()...
            {
                path.moveTo(startPoint);
                path.lineTo(endPoint);
            }
            else
            {
                // if (lineLength < 0.0001)
                // {
                // path.arcMoveTo(circleRect.translated(QPointF(0.01, 0.01)), startAngle);
                // path.arcTo(circleRect.translated(QPointF(0.01, 0.01)), startAngle, sweepAngle);
                // }
                // else
                // {
                path.arcMoveTo(circleRect, startAngle);
                path.arcTo(circleRect, startAngle, sweepAngle);
                // }
            }
            const bool debugging = false;//QLineF(circleCenter, QPointF(0.5, 0.5)).length() <= 0.1;
            if (debugging)
            {
                qDebug() << "------------" << "swapped" << (curve.orientation() != CGAL::CLOCKWISE) << "Xeq" << (startPoint.x() == endPoint.x()) << "Yeq" << (startPoint.y() == endPoint.y()) << "Peq" << (startPoint == endPoint) << "length:" << QLineF(startPoint, endPoint).length();
                qDebug() << "C" << circleCenter << "S" << startPoint << "T" << endPoint << "R" << radius;
                qDebug() << "path.arcTo():" << circleRect << startAngle << sweepAngle << "end" << endAngle;
                qDebug() << "S qAtan2():" << qAtan2(startPoint.y() - circleCenter.y(), startPoint.x() - circleCenter.x());
                qDebug() << "T qAtan2():" << qAtan2(endPoint.y() - circleCenter.y(), endPoint.x() - circleCenter.x());
            }
#endif
        }
        else
        {
            const QPointF  startPoint(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
            const QPointF    endPoint(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
            // if (path.elementCount() == 0)
                path.moveTo(startPoint);
            path.lineTo(endPoint);
        }
    }
}*/

std::list<Polygon_with_holes_2> subtractPolygonLists(const std::list<Polygon_with_holes_2> &a, const std::list<Polygon_with_holes_2> &b)
{
    std::list<Polygon_with_holes_2> tmp = a;
    std::list<Polygon_with_holes_2> result;
    for (std::list<Polygon_with_holes_2>::const_iterator poly = b.begin(); poly != b.end(); ++poly)
    {
        result.clear();
        for (std::list<Polygon_with_holes_2>::const_iterator poly_with_holes = tmp.begin(); poly_with_holes != tmp.end(); ++poly_with_holes)
        {
            CGAL::difference(*poly_with_holes, *poly, std::back_inserter(result));
        }
        tmp = result;
    }
    return result;
}

std::list<Polygon_with_holes_2> intersectPolygonLists(const std::list<Polygon_with_holes_2> &a, const std::list<Polygon_with_holes_2> &b)
{
    std::list<Polygon_with_holes_2> all_combinations_of_a_and_b_intersected;
    for (std::list<Polygon_with_holes_2>::const_iterator a_it = a.begin(); a_it != a.end(); ++a_it)
    {
        for (std::list<Polygon_with_holes_2>::const_iterator b_it = b.begin(); b_it != b.end(); ++b_it)
        {
            CGAL::intersection(*a_it, *b_it, std::back_inserter(all_combinations_of_a_and_b_intersected));
        }
    }
    std::list<Polygon_with_holes_2> joined;
    CGAL::join(all_combinations_of_a_and_b_intersected.begin(), all_combinations_of_a_and_b_intersected.end(), std::back_inserter(joined));
    return joined;
}

std::list<Polygon_with_holes_2> xorPolygonList(const std::list<Polygon_with_holes_2> &polygons)
{
    std::list<Polygon_with_holes_2> result;
    CGAL::symmetric_difference(polygons.begin(), polygons.end(), std::back_inserter(result));
    return result;
}

std::list<Polygon_with_holes_2> xorPolygonLists(const std::list<Polygon_with_holes_2> &a, const std::list<Polygon_with_holes_2> &b)
{
    std::list<Polygon_with_holes_2> result;
    // TODO make this more efficient, this XOR's the operands against themselves (not just the other set) which does nothing when they are planar
    CGAL::symmetric_difference(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(result));
    return result;
}

// Construct a polygon from a circle.
static Polygon_2 construct_circle_polygon(const QPointF &center, double radius)
{
    // Subdivide the circle into two x-monotone arcs.
    Traits_2 traits;
    Curve_2 curve(Circle_2(Point_2(center.x(), center.y()), CGAL::square(radius)));
    std::list<CGAL::Object> objects;
    traits.make_x_monotone_2_object()(curve, std::back_inserter(objects));
    CGAL_assertion(objects.size() == 2);
    // Construct the polygon.
    Polygon_2 pgn;
    X_monotone_curve_2 arc;
    for (std::list<CGAL::Object>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
    {
        CGAL::assign(arc, *iter);
        pgn.push_back(arc);
    }
    return pgn;
}

/*static Polygon_with_holes_2 construct_annulus_polygon(const QPointF &center, double radius_inner, double radius_outer)
{
    const Polygon_2 outerCircle = construct_circle_polygon(center, radius_outer);
    const Polygon_2 innerCircle = construct_circle_polygon(center, radius_inner);

    std::list<Polygon_with_holes_2> res;
    CGAL::difference(outerCircle, innerCircle, std::back_inserter(res));
    return res.front();
}*/

// Construct a polygon from a circle.
/*static Polygon_2 construct_rect_polygon(const QRectF &rect)
{
    const Point_2 p1(rect.left() , rect.top());
    const Point_2 p2(rect.right(), rect.top());
    const Point_2 p3(rect.right(), rect.bottom());
    const Point_2 p4(rect.left() , rect.bottom());

    Polygon_2 pgn;
    pgn.push_back(X_monotone_curve_2(p1, p2));
    pgn.push_back(X_monotone_curve_2(p2, p3));
    pgn.push_back(X_monotone_curve_2(p3, p4));
    pgn.push_back(X_monotone_curve_2(p4, p1));
    return pgn;
}*/

// Construct a polygon from a circle.
static Polygon_2 construct_wire(const QPointF &startPoint, const QPointF &endPoint, double width)
{
    const double radius = width/2.0;
    const QLineF line(startPoint, endPoint);
    const QPointF offsetN = (QLineF(QPointF(), endPoint - startPoint).normalVector().unitVector()).p2()*radius;
    const QPointF offsetP = (QLineF(QPointF(), endPoint - startPoint).unitVector()).p2()*radius;

    // Subdivide the circle into two x-monotone arcs.
    const bool debugging = false;
    Traits_2 traits;
    std::list<CGAL::Object> objects;
    if (debugging)
    {
        qDebug() << "---- GENERATING WIRE STROKE ----";
    }
    {
      Point_2 pt1(startPoint.x()-offsetN.x(), startPoint.y()-offsetN.y());
      Point_2 pt2(startPoint.x()-offsetP.x(), startPoint.y()-offsetP.y());
      Point_2 pt3(startPoint.x()+offsetN.x(), startPoint.y()+offsetN.y());
      Curve_2 curve(pt1, pt2, pt3);
      if (debugging)
      {
          DumpCurve(curve);
      }
      traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    {
      Point_2 pt1(startPoint.x()+offsetN.x(), startPoint.y()+offsetN.y());
      Point_2 pt2(  endPoint.x()+offsetN.x(),   endPoint.y()+offsetN.y());
      Curve_2 curve(pt1, pt2);
      if (debugging)
      {
          DumpCurve(curve);
      }
      traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    {
      Point_2 pt1(  endPoint.x()+offsetN.x(),   endPoint.y()+offsetN.y());
      Point_2 pt2(  endPoint.x()+offsetP.x(),   endPoint.y()+offsetP.y());
      Point_2 pt3(  endPoint.x()-offsetN.x(),   endPoint.y()-offsetN.y());
      Curve_2 curve(pt1, pt2, pt3);
      if (debugging)
      {
          DumpCurve(curve);
      }
      traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    {
      Point_2 pt1(  endPoint.x()-offsetN.x(),   endPoint.y()-offsetN.y());
      Point_2 pt2(startPoint.x()-offsetN.x(), startPoint.y()-offsetN.y());
      Curve_2 curve(pt1, pt2);
      if (debugging)
      {
          DumpCurve(curve);
      }
      traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    // Construct the polygon.
    Polygon_2 pgn;
    X_monotone_curve_2 arc;
    std::list<CGAL::Object>::iterator iter;
    for (iter = objects.begin(); iter != objects.end(); ++iter)
    {
        CGAL::assign (arc, *iter);
        pgn.push_back (arc);
    }
    return pgn;
}

/*

// WORKING CODE, but was just for proof of concept, uses awkward parameters

Polygon_2 construct_rainbow(const QPointF &centerPoint, double startAngle, double endAngle, double radius, double width)
{
    const double avgAngle = (startAngle + endAngle)/2.0;
    const double radius_outer = radius + width/2.0;
    const double radius_inner = std::max(0.0, radius - width/2.0);

    const double startCos = qCos(startAngle);
    const double startSin = qSin(startAngle);
    const double avgCos = qCos(avgAngle);
    const double avgSin = qSin(avgAngle);
    const double endCos = qCos(endAngle);
    const double endSin = qSin(endAngle);

    // Subdivide the circle into two x-monotone arcs.
    Traits_2 traits;
    std::list<CGAL::Object> objects;
    {
        Point_2 pt1(centerPoint.x() + startCos*radius_outer, centerPoint.y() + startSin*radius_outer);
        Point_2 pt2(centerPoint.x() +   avgCos*radius_outer, centerPoint.y() +   avgSin*radius_outer);
        Point_2 pt3(centerPoint.x() +   endCos*radius_outer, centerPoint.y() +   endSin*radius_outer);
        Curve_2 curve(pt1, pt2, pt3);
        traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    {
      Point_2 pt1(centerPoint.x() +   endCos*radius_outer, centerPoint.y() +   endSin*radius_outer);
      Point_2 pt2(centerPoint.x() +   endCos*radius_inner, centerPoint.y() +   endSin*radius_inner);
      Curve_2 curve(pt1, pt2);
      traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    if (radius_inner != 0.0)
    {
        Point_2 pt1(centerPoint.x() +   endCos*radius_inner, centerPoint.y() +   endSin*radius_inner);
        Point_2 pt2(centerPoint.x() +   avgCos*radius_inner, centerPoint.y() +   avgSin*radius_inner);
        Point_2 pt3(centerPoint.x() + startCos*radius_inner, centerPoint.y() + startSin*radius_inner);
        Curve_2 curve(pt1, pt2, pt3);
        traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    {
      Point_2 pt1(centerPoint.x() + startCos*radius_inner, centerPoint.y() + startSin*radius_inner);
      Point_2 pt2(centerPoint.x() + startCos*radius_outer, centerPoint.y() + startSin*radius_outer);
      Curve_2 curve(pt1, pt2);
      traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    // Construct the polygon.
    Polygon_2 pgn;
    X_monotone_curve_2 arc;
    std::list<CGAL::Object>::iterator iter;
    for (iter = objects.begin(); iter != objects.end(); ++iter)
    {
        CGAL::assign (arc, *iter);
        pgn.push_back (arc);
    }
    return pgn;
}

Polygon_2 construct_four_point_circle(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d)
{
    // Subdivide the circle into two x-monotone arcs.
    Traits_2 traits;
    std::list<CGAL::Object> objects;
    {
        Point_2 pt1(a.x(), a.y());
        Point_2 pt2(b.x(), b.y());
        Point_2 pt3(c.x(), c.y());
        Curve_2 curve(pt1, pt2, pt3);
        traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    {
        Point_2 pt1(c.x(), c.y());
        Point_2 pt2(d.x(), d.y());
        Point_2 pt3(a.x(), a.y());
        Curve_2 curve(pt1, pt2, pt3);
        traits.make_x_monotone_2_object() (curve, std::back_inserter(objects));
    }
    // Construct the polygon.
    Polygon_2 pgn;
    X_monotone_curve_2 arc;
    std::list<CGAL::Object>::iterator iter;
    for (iter = objects.begin(); iter != objects.end(); ++iter)
    {
        CGAL::assign (arc, *iter);
        pgn.push_back (arc);
    }
    return pgn;
}

std::list<Polygon_with_holes_2> construct_capped_rainbow(const QPointF &centerPoint, double startAngle, double endAngle, double radius, double width)
{
    const double radius_cap   = width/2.0;
    const double radius_outer = radius + radius_cap;
    const double radius_inner = radius - radius_cap; // NO MAX here!!! NOT std::max(0.0, radius - radius_cap);

    const double startCos = qCos(startAngle);
    const double startSin = qSin(startAngle);
    const double endCos = qCos(endAngle);
    const double endSin = qSin(endAngle);

    std::list<Polygon_with_holes_2> pieces;
    // pieces.push_back(Polygon_with_holes_2(construct_rainbow(centerPoint, startAngle, endAngle, radius, width)));
    {
        const QPointF pt1(centerPoint.x() +   endCos*radius_outer                      , centerPoint.y() +   endSin*radius_outer);
        const QPointF pt2(centerPoint.x() +   endCos*radius       -   endSin*radius_cap, centerPoint.y() +   endSin*radius       + endCos*radius_cap);
        const QPointF pt3(centerPoint.x() +   endCos*radius_inner                      , centerPoint.y() +   endSin*radius_inner);
        const QPointF pt4(centerPoint.x() +   endCos*radius       +   endSin*radius_cap, centerPoint.y() +   endSin*radius       - endCos*radius_cap);
        pieces.push_back(Polygon_with_holes_2(construct_four_point_circle(pt1, pt2, pt3, pt4)));
    }
    {
        const QPointF pt1(centerPoint.x() + startCos*radius_outer                      , centerPoint.y() + startSin*radius_outer);
        const QPointF pt2(centerPoint.x() + startCos*radius       - startSin*radius_cap, centerPoint.y() + startSin*radius       + startCos*radius_cap);
        const QPointF pt3(centerPoint.x() + startCos*radius_inner                      , centerPoint.y() + startSin*radius_inner);
        const QPointF pt4(centerPoint.x() + startCos*radius       + startSin*radius_cap, centerPoint.y() + startSin*radius       - startCos*radius_cap);
        pieces.push_back(Polygon_with_holes_2(construct_four_point_circle(pt1, pt2, pt3, pt4)));
    }
    // return pieces;
    std::list<Polygon_with_holes_2> result;
    CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(result));

    return result;
}*/

/*static std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_2 &polygon, double radius)
{
    std::list<Polygon_with_holes_2> pieces;
    for (auto curve_it = polygon.curves_begin(); curve_it != polygon.curves_end(); ++curve_it)
    {
        pieces.splice(pieces.end(), construct_grown_curve(*curve_it, radius));
    }
    std::list<Polygon_with_holes_2> result;
    CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(result));
    return result;
}*/

static std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_with_holes_2 &polygon, double radius)
{
    std::list<Polygon_with_holes_2> pieces;
    for (auto curve_it = polygon.outer_boundary().curves_begin(); curve_it != polygon.outer_boundary().curves_end(); ++curve_it)
    {
        pieces.splice(pieces.end(), construct_grown_curve(*curve_it, radius));
    }
    for (Polygon_with_holes_2::Hole_const_iterator hole_it = polygon.holes_begin(); hole_it != polygon.holes_end(); ++hole_it)
    {
        for (auto curve_it = hole_it->curves_begin(); curve_it != hole_it->curves_end(); ++curve_it)
        {
            pieces.splice(pieces.end(), construct_grown_curve(*curve_it, radius));
        }
    }
    // return pieces; // for debugging
    std::list<Polygon_with_holes_2> result;
    CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(result));
    return result;
}

/*static std::list<Polygon_with_holes_2> construct_polygon_offset(const Polygon_with_holes_2 &polygon, double radius)
{
    std::list<Polygon_with_holes_2> stroke = construct_polygon_stroke(polygon, radius);
    stroke.push_back(polygon);
    std::list<Polygon_with_holes_2> result;
    CGAL::join(stroke.begin(), stroke.end(), std::back_inserter(result));
    return result;
}*/

std::list<Polygon_with_holes_2> construct_polygon_list_offset(const std::list<Polygon_with_holes_2> &polygons, double radius)
{
    std::list<Polygon_with_holes_2> pieces;

    // union the original polygon to eliminate the holes if doing an outset
    if (radius > 0.0)
    {
        pieces = polygons;
    }

    // make the individual overlapping strokes
    for (std::list<Polygon_with_holes_2>::const_iterator polygon_it = polygons.begin(); polygon_it != polygons.end(); ++polygon_it)
    {
        pieces.splice(pieces.end(), construct_polygon_stroke(*polygon_it, std::abs(radius)));
    }

    // combine all the strokes
    std::list<Polygon_with_holes_2> strokes;
    CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(strokes));
    // return strokes;

    // outset
    if (radius > 0.0)
        return strokes;

    // inset
    const std::list<Polygon_with_holes_2> innerHalf = intersectPolygonLists(strokes, polygons);
    // return innerHalf;
    const std::list<Polygon_with_holes_2> holesOnly = xorPolygonLists(innerHalf, polygons);
    return holesOnly;
}

/*static std::list<Polygon_with_holes_2> construct_thermal(const QPointF &center, double radius_inner, double isolation, double width)
{
    const double radius_outer = radius_inner + isolation;

    std::list<Polygon_with_holes_2> polygons;
    polygons.push_back(construct_annulus_polygon(center, radius_outer, radius_outer*2.0));
    polygons.push_back(Polygon_with_holes_2(construct_circle_polygon(center, radius_inner)));
    polygons.push_back(Polygon_with_holes_2(construct_rect_polygon(QRectF(center.x() - radius_outer*1.5, center.y() - width/2.0, radius_outer*3.0, width))));
    polygons.push_back(Polygon_with_holes_2(construct_rect_polygon(QRectF(center.x() - width/2.0, center.y() - radius_outer*1.5, width, radius_outer*3.0))));

    std::list<Polygon_with_holes_2> result;
    CGAL::join(polygons.begin(), polygons.end(), std::back_inserter(result));

    return result;
}*/

/*static std::list<Polygon_with_holes_2> construct_thermal_negative(const QPointF &center, double radius_inner, double isolation, double width)
{
    const double radius_outer = radius_inner + isolation;
    std::list<Polygon_with_holes_2> beingCut;
    beingCut.push_back(Polygon_with_holes_2(construct_circle_polygon(center, radius_outer)));

    std::list<Polygon_2> cutters;
    cutters.push_back(construct_circle_polygon(center, radius_inner));
    cutters.push_back(construct_rect_polygon(QRectF(center.x() - radius_outer*2.0, center.y() - width/2.0, radius_outer*4.0, width)));
    cutters.push_back(construct_rect_polygon(QRectF(center.x() - width/2.0, center.y() - radius_outer*2.0, width, radius_outer*4.0)));

    std::list<Polygon_with_holes_2> result;
    for (std::list<Polygon_2>::const_iterator poly = cutters.begin(); poly != cutters.end(); ++poly)
    {
        result.clear();
        for (std::list<Polygon_with_holes_2>::const_iterator poly_with_holes = beingCut.begin(); poly_with_holes != beingCut.end(); ++poly_with_holes)
        {
            CGAL::difference(*poly_with_holes, *poly, std::back_inserter(result));
        }
        beingCut = result;
    }

    return result;
}*/

////////////////////////////////////////////////////////////////////

/*static void AddWire(QGraphicsScene *scene, const QLineF &line, double width)
{
    const double radius = width/2.0;
    const QPointF offset = (QLineF(QPointF(), line.p2() - line.p1()).normalVector().unitVector()).p2()*radius;

    // circle centers
    // Point_2 start(startPoint.x(), startPoint.y());
    // Point_2 end(endPoint.x(), endPoint.y());

    // circles
    // Circle_2 circle1(start, radius);
    // Circle_2 circle2(end, radius);

    // lines
    const QLineF line1(line.p1() + offset, line.p2() + offset);
    const QLineF line2(line.p1() - offset, line.p2() - offset);
    QPainterPath path;
    path.moveTo(line1.p1());
    path.lineTo(line1.p2());
    path.arcTo(QRectF(line.p2().x() - radius, line.p2().y() - radius, radius*2.0, radius*2.0), QLineF(QPointF(), offset).angle(), -180.0);
    path.lineTo(line2.p1());
    path.arcTo(QRectF(line.p1().x() - radius, line.p1().y() - radius, radius*2.0, radius*2.0), QLineF(QPointF(), -offset).angle(), -180.0);
    path.closeSubpath();
    scene->addPath(path, QPen(Qt::black, 0.0));
    // Line_2 line1(Point_2(startPoint.x() + offset.x(), startPoint.y() + offset.y()), Point_2(endPoint.x() + offset.x(), endPoint.y() + offset.y()));
    // Line_2 line2(Point_2(startPoint.x() - offset.x(), startPoint.y() - offset.y()), Point_2(endPoint.x() - offset.x(), endPoint.y() - offset.y()));
}*/

} // namespace LineArcOffsetDemo
