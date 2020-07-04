#include "CGALWrapper.h"

// Construct a polygon from a circle.
Polygon_2 construct_circle_polygon(const QPointF &center, double radius)
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

Polygon_with_holes_2 construct_annulus_polygon(const QPointF &center, double radius_inner, double radius_outer)
{
    const Polygon_2 outerCircle = construct_circle_polygon(center, radius_outer);
    const Polygon_2 innerCircle = construct_circle_polygon(center, radius_inner);

    std::list<Polygon_with_holes_2> res;
    CGAL::difference(outerCircle, innerCircle, std::back_inserter(res));
    return res.front();
}

// Construct a polygon from a circle.
Polygon_2 construct_rect_polygon(const QRectF &rect)
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
}

// Construct a polygon from a circle.
Polygon_2 construct_wire(const QPointF &startPoint, const QPointF &endPoint, double width)
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

std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_2 &polygon, double radius)
{
    std::list<Polygon_with_holes_2> pieces;
    for (auto curve_it = polygon.curves_begin(); curve_it != polygon.curves_end(); ++curve_it)
    {
        pieces.splice(pieces.end(), construct_grown_curve(*curve_it, radius));
    }
    std::list<Polygon_with_holes_2> result;
    CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(result));
    return result;
}

std::list<Polygon_with_holes_2> construct_polygon_stroke(const Polygon_with_holes_2 &polygon, double radius)
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

std::list<Polygon_with_holes_2> construct_polygon_offset(const Polygon_with_holes_2 &polygon, double radius)
{
    std::list<Polygon_with_holes_2> stroke = construct_polygon_stroke(polygon, radius);
    stroke.push_back(polygon);
    std::list<Polygon_with_holes_2> result;
    CGAL::join(stroke.begin(), stroke.end(), std::back_inserter(result));
    return result;
}

std::list<Polygon_with_holes_2> construct_polygon_list_offset(const std::list<Polygon_with_holes_2> &polygons, double radius)
{
    std::list<Polygon_with_holes_2> pieces = polygons;
    for (std::list<Polygon_with_holes_2>::const_iterator polygon_it = polygons.begin(); polygon_it != polygons.end(); ++polygon_it)
    {
        pieces.splice(pieces.end(), construct_polygon_stroke(*polygon_it, radius));
    }
    std::list<Polygon_with_holes_2> result;
    CGAL::join(pieces.begin(), pieces.end(), std::back_inserter(result));
    return result;
}

std::list<Polygon_with_holes_2> construct_thermal(const QPointF &center, double radius_inner, double isolation, double width)
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
}

std::list<Polygon_with_holes_2> construct_thermal_negative(const QPointF &center, double radius_inner, double isolation, double width)
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
}

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
