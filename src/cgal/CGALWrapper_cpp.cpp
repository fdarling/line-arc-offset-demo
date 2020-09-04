#include "CGALWrapper.h"
#include "CGALQt.h"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Polygon_2.h> // for transform()

#include <type_traits>

#include <QtMath>
#include <QLineF>
#include <QDebug>

namespace LineArcOffsetDemo {

// forward declarations
static Polygon_2 construct_circle_polygon(const QPointF &center, double radius);
static Polygon_2 construct_wire(const QPointF &startPoint, const QPointF &endPoint, double width);
// static std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_2 &polygon, double radius);
static std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_with_holes_2 &polygon, double radius);

void addCurveToPolygon(Polygon_2 &polygon, const Curve_2 &curve)
{
    Traits_2 traits;
    std::list<CGAL::Object> objects;
    traits.make_x_monotone_2_object()(curve, std::back_inserter(objects));
    X_monotone_curve_2 arc;
    for (std::list<CGAL::Object>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
    {
        CGAL::assign(arc, *iter);
        polygon.push_back(arc);
    }
}

template <typename T>
std::list<Polygon_with_holes_2> construct_grown_curve(const T &curve, double radius_cap)
{
    const QPointF pt1(Point_2_To_QPointF(curve.source()));
    const QPointF pt2(Point_2_To_QPointF(curve.target()));
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
            const double radius = CGAL::sqrt(CGAL::to_double(CGAL::sqrt(circle.squared_radius())));
            const double radius_outer = radius + radius_cap;
            const double radius_inner = radius - radius_cap;
            const Circle_2 circle_outer(circle.center(), radius_outer*radius_outer);
            const Circle_2 circle_inner(circle.center(), radius_inner*radius_inner);
            Polygon_2 outer;
            addCurveToPolygon(outer, Curve_2(circle_outer));

            std::list<Polygon_with_holes_2> result;
            if (radius_inner < 0.0) // TODO: make sure 0.0 radius circles are okay! This should allow for endmill "drilling"
            {
                result.push_back(Polygon_with_holes_2(outer));
                return result;
            }

            Polygon_2 inner;
            addCurveToPolygon(inner, Curve_2(circle_inner));
            CGAL::difference(outer, inner, std::back_inserter(result)); // TODO avoid use of expensive boolean operation
            return result;
        }
        //else
        {
            const QPointF startPoint(Point_2_To_QPointF(curve.source()));
            const QPointF   endPoint(Point_2_To_QPointF(curve.target()));
            const QPointF center(Point_2_To_QPointF(circle.center()));
            const double radius = CGAL::sqrt(CGAL::to_double(circle.squared_radius()));
            const double startAngle = qAtan2(CGAL::to_double(curve.source().y()), CGAL::to_double(curve.source().x()));
                  double   endAngle = qAtan2(CGAL::to_double(curve.target().y()), CGAL::to_double(curve.target().x()));
            if (endAngle < startAngle)
                endAngle += 2.0*M_PI;
            const bool debugging = false;//QLineF(center, QPointF(0.5, 0.5)).length() < 0.01;
            if (debugging)
            {
                qDebug() << "ORIGINAL CURVE:";
                qDebug() << curve;
                qDebug() << "Constructing rainbow...";
            }
            std::list<Polygon_with_holes_2> pieces;
            {
                const QPointF middle_vec = QLineF(QPointF(0.0, 0.0), (endPoint - startPoint)).normalVector().unitVector().p2();
                const QPointF source_offset = QLineF(QPointF(0.0, 0.0), startPoint - center).unitVector().p2()*radius_cap;
                const QPointF target_offset = QLineF(QPointF(0.0, 0.0),   endPoint - center).unitVector().p2()*radius_cap;
                const Point_2 source_out = QPointF_To_Point_2(startPoint + source_offset);
                const Point_2 source_in  = QPointF_To_Point_2(startPoint - source_offset);
                const Point_2 target_out = QPointF_To_Point_2(  endPoint + target_offset);
                const Point_2 target_in  = QPointF_To_Point_2(  endPoint - target_offset);
                const double reverse_factor = (curve.orientation() == CGAL::COUNTERCLOCKWISE) ? 1.0 : -1.0; // TODO this seems backwards...
                const Point_2 middle_out = QPointF_To_Point_2(center + middle_vec*(radius + radius_cap)*reverse_factor);
                const Point_2 middle_in  = QPointF_To_Point_2(center + middle_vec*(radius - radius_cap)*reverse_factor);
                if (debugging)
                {
                    qDebug() << "middle_vec:" << middle_vec;
                    qDebug() << "middle_in: " << middle_in;
                    qDebug() << "middle_out:" << middle_out;
                }
                Polygon_2 pgn;
                if (curve.orientation() == CGAL::COUNTERCLOCKWISE)
                {
                    if (debugging)
                    {
                        qDebug() << "GENERATED OUTER CCW:";
                        qDebug() << Curve_2(source_out, middle_out, target_out);
                    }
                    addCurveToPolygon(pgn, Curve_2(source_out, middle_out, target_out));
                    if (radius > radius_cap && target_in != source_in)
                    {
                        addCurveToPolygon(pgn, Curve_2(target_out, target_in));
                        if (debugging)
                        {
                            qDebug() << "GENERATED INNER CCW:" << radius << radius_cap;
                            //CGAL_precondition(Kernel().compare_xy_2_object() (p1, p3) != EQUAL);
                            qDebug() << "p1:" << target_in << "p2:" << middle_in << "p3:" << source_in;
                            qDebug() << Curve_2(target_in, middle_in, source_in);
                        }
                        addCurveToPolygon(pgn, Curve_2(target_in, middle_in, source_in));
                        addCurveToPolygon(pgn, Curve_2(source_in, source_out));
                    }
                    else
                    {
                        addCurveToPolygon(pgn, Curve_2(target_out, circle.center()));
                        addCurveToPolygon(pgn, Curve_2(circle.center(), source_out));
                    }
                }
                else
                {
                    if (debugging)
                    {
                        qDebug() << "GENERATED OUTER CW:";
                        qDebug() << Curve_2(target_out, middle_out, source_out);
                    }
                    addCurveToPolygon(pgn, Curve_2(target_out, middle_out, source_out));
                    if (radius > radius_cap)
                    {
                        addCurveToPolygon(pgn, Curve_2(source_out, source_in));
                        if (debugging)
                        {
                            qDebug() << "GENERATED INNER CW:";
                            qDebug() << Curve_2(source_in, middle_in, target_in);
                        }
                        addCurveToPolygon(pgn, Curve_2(source_in, middle_in, target_in));
                        addCurveToPolygon(pgn, Curve_2(target_in, target_out));
                    }
                    else
                    {
                        addCurveToPolygon(pgn, Curve_2(source_out, circle.center()));
                        addCurveToPolygon(pgn, Curve_2(circle.center(), target_out));
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
}

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
    const Curve_2 curve(Circle_2(Point_2(center.x(), center.y()), CGAL::square(radius)));
    Polygon_2 pgn;
    addCurveToPolygon(pgn, curve);
    return pgn;
}

// Construct a polygon from a circle.
static Polygon_2 construct_wire(const QPointF &startPoint, const QPointF &endPoint, double width)
{
    const double radius = width/2.0;
    const QLineF line(startPoint, endPoint);
    const QPointF offsetN = (QLineF(QPointF(), endPoint - startPoint).normalVector().unitVector()).p2()*radius;
    const QPointF offsetP = (QLineF(QPointF(), endPoint - startPoint).unitVector()).p2()*radius;

    // Subdivide the circle into two x-monotone arcs.
    const bool debugging = false;
    Polygon_2 pgn;
    if (debugging)
    {
        qDebug() << "---- GENERATING WIRE STROKE ----";
    }
    {
      const Point_2 pt1(startPoint.x()-offsetN.x(), startPoint.y()-offsetN.y());
      const Point_2 pt2(startPoint.x()-offsetP.x(), startPoint.y()-offsetP.y());
      const Point_2 pt3(startPoint.x()+offsetN.x(), startPoint.y()+offsetN.y());
      const Curve_2 curve(pt1, pt2, pt3);
      if (debugging)
      {
          qDebug() << curve;
      }
      addCurveToPolygon(pgn, curve);
    }
    {
      const Point_2 pt1(startPoint.x()+offsetN.x(), startPoint.y()+offsetN.y());
      const Point_2 pt2(  endPoint.x()+offsetN.x(),   endPoint.y()+offsetN.y());
      const Curve_2 curve(pt1, pt2);
      if (debugging)
      {
          qDebug() << curve;
      }
      addCurveToPolygon(pgn, curve);
    }
    {
      const Point_2 pt1(  endPoint.x()+offsetN.x(),   endPoint.y()+offsetN.y());
      const Point_2 pt2(  endPoint.x()+offsetP.x(),   endPoint.y()+offsetP.y());
      const Point_2 pt3(  endPoint.x()-offsetN.x(),   endPoint.y()-offsetN.y());
      const Curve_2 curve(pt1, pt2, pt3);
      if (debugging)
      {
          qDebug() << curve;
      }
      addCurveToPolygon(pgn, curve);
    }
    {
      const Point_2 pt1(  endPoint.x()-offsetN.x(),   endPoint.y()-offsetN.y());
      const Point_2 pt2(startPoint.x()-offsetN.x(), startPoint.y()-offsetN.y());
      const Curve_2 curve(pt1, pt2);
      if (debugging)
      {
          qDebug() << curve;
      }
      addCurveToPolygon(pgn, curve);
    }
    return pgn;
}

static std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_with_holes_2 &polygon, double radius)
{
    std::list<Polygon_with_holes_2> pieces;
    for (Polygon_with_holes_2::General_polygon_2::Curve_const_iterator curve_it = polygon.outer_boundary().curves_begin(); curve_it != polygon.outer_boundary().curves_end(); ++curve_it)
    {
        pieces.splice(pieces.end(), construct_grown_curve(*curve_it, radius));
    }
    for (Polygon_with_holes_2::Hole_const_iterator hole_it = polygon.holes_begin(); hole_it != polygon.holes_end(); ++hole_it)
    {
        for (Polygon_with_holes_2::General_polygon_2::Curve_const_iterator curve_it = hole_it->curves_begin(); curve_it != hole_it->curves_end(); ++curve_it)
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

} // namespace LineArcOffsetDemo
