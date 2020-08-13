#ifndef LINEARCOFFSETDEMO_CGALWRAPPER_H
#define LINEARCOFFSETDEMO_CGALWRAPPER_H

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

// #include <CGAL/Simple_cartesian.h>
// #include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/General_polygon_set_2.h>

#include <list>

namespace LineArcOffsetDemo {

// typedef CGAL::Simple_cartesian<double> Kernel;
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

std::list<Polygon_with_holes_2> subtractPolygonLists(const std::list<Polygon_with_holes_2> &a, const std::list<Polygon_with_holes_2> &b);
std::list<Polygon_with_holes_2> intersectPolygonLists(const std::list<Polygon_with_holes_2> &a, const std::list<Polygon_with_holes_2> &b);
std::list<Polygon_with_holes_2> xorPolygonLists(const std::list<Polygon_with_holes_2> &a, const std::list<Polygon_with_holes_2> &b);
std::list<Polygon_with_holes_2> construct_polygon_list_offset(const std::list<Polygon_with_holes_2> &polygons, double radius);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_CGALWRAPPER_H
