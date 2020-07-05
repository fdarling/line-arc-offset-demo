#ifndef LINEARCOFFSETDEMO_CGALWRAPPER_H
#define LINEARCOFFSETDEMO_CGALWRAPPER_H

#include <type_traits>

#include <QtMath>
#include <QLineF>
#include <QRectF>
#include <QPainterPath>
#include <QDebug>

////////////////////////////////////////////////////////////////////

/*#include <CGAL/Cartesian.h>
#include <CGAL/Exact_rational.h>
#include <CGAL/Arr_circle_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
typedef CGAL::Cartesian<CGAL::Exact_rational>         Kernel;
typedef Kernel::Circle_2                              Circle_2;
typedef Kernel::Segment_2                             Segment_2;
typedef CGAL::Arr_circle_segment_traits_2<Kernel>     Traits_2;
typedef Traits_2::CoordNT                             CoordNT;
typedef Traits_2::Point_2                             Point_2;
typedef Traits_2::Curve_2                             Curve_2;
typedef CGAL::Arrangement_2<Traits_2>                 Arrangement_2;*/

////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////

// #include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Polygon_2.h> // for transform()
// #include <CGAL/Qt/Converter.h> // for CGAL::Qt::Converter<Kernel>

// typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_2                                   Point_2;
typedef Kernel::Circle_2                                  Circle_2;
typedef Kernel::Line_2                                    Line_2;
typedef CGAL::Gps_circle_segment_traits_2<Kernel>         Traits_2;
typedef CGAL::General_polygon_set_2<Traits_2>             Polygon_set_2;
typedef Traits_2::Polygon_2                               Polygon_2;
typedef Traits_2::Polygon_with_holes_2                    Polygon_with_holes_2;
typedef Traits_2::Curve_2                                 Curve_2;
typedef Traits_2::X_monotone_curve_2                      X_monotone_curve_2;

/*#include <CGAL/approximated_offset_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/General_polygon_set_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Gps_circle_segment_traits_2<Kernel>         Traits;

typedef CGAL::General_polygon_set_2<Traits>               Polygon_set_2;
typedef Traits::Polygon_2                                 Polygon_2;
typedef Traits::Polygon_with_holes_2 Polygon_with_holes_2;*/

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

Polygon_2 construct_circle_polygon(const QPointF &center, double radius);
Polygon_with_holes_2 construct_annulus_polygon(const QPointF &center, double radius_inner, double radius_outer);
Polygon_2 construct_rect_polygon(const QRectF &rect);
Polygon_2 construct_wire(const QPointF &startPoint, const QPointF &endPoint, double width);

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

std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_2 &polygon, double radius);
std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_with_holes_2 &polygon, double radius);
std::list<Polygon_with_holes_2> construct_polygon_offset(const Polygon_with_holes_2 &polygon, double radius);
std::list<Polygon_with_holes_2> construct_polygon_list_offset(const std::list<Polygon_with_holes_2> &polygons, double radius);
std::list<Polygon_with_holes_2> construct_thermal(const QPointF &center, double radius_inner, double isolation, double width);
std::list<Polygon_with_holes_2> construct_thermal_negative(const QPointF &center, double radius_inner, double isolation, double width);

template <typename T>
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
            /*if ( ! this->isProperOrientation( cc ) )
            {
              std::swap( source, target );
            }*/

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
}

#endif // LINEARCOFFSETDEMO_CGALWRAPPER_H
