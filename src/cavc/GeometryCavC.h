#ifndef LINEARCOFFSETDEMO_GEOMETRYCAVC_H
#define LINEARCOFFSETDEMO_GEOMETRYCAVC_H

#include "../Geometry.h"

#include <cavc/polyline.hpp>
#include <cavc/polylineoffsetislands.hpp>

#include <list>

namespace LineArcOffsetDemo {

typedef double CavC_Real;
typedef cavc::Vector2<CavC_Real> Vector2;
typedef cavc::PlineVertex<CavC_Real> PVertex;
typedef cavc::Polyline<CavC_Real> Polyline;
typedef std::vector<Polyline> Polylines;

struct CavC_Shape
{
    Polyline boundary;
    Polylines holes;
};

typedef std::list<CavC_Shape> CavC_MultiShape;
typedef cavc::StaticSpatialIndex<CavC_Real> StaticSpatialIndex;
typedef cavc::OffsetLoop<CavC_Real> OffsetLoop;
typedef std::vector<OffsetLoop> OffsetLoops;
typedef cavc::OffsetLoopSet<CavC_Real> OffsetLoopSet;

LineArcGeometry::CoordinateType FromCavC(CavC_Real value);
CavC_Real ToCavC(LineArcGeometry::CoordinateType value);
Vector2 PointToVector2(const LineArcGeometry::Point &point);
PVertex SegmentToPVertex(const LineArcGeometry::Segment &segment);
Polyline ContourToPolyline(const LineArcGeometry::Contour &contour);
CavC_Shape ShapeToCavC_Shape(const LineArcGeometry::Shape &shape);
CavC_MultiShape MultiShapeToCavC_MultiShape(const LineArcGeometry::MultiShape &multiShape);
OffsetLoopSet MultiShapeToOffsetLoopSet(const LineArcGeometry::MultiShape &multiShape);
LineArcGeometry::Point Vector2ToPoint(const Vector2 &vertex);
LineArcGeometry::Segment PVerticesToSegment(const PVertex &v1, const PVertex &v2);
LineArcGeometry::Contour PolylineToContour(const Polyline &polyline);
LineArcGeometry::Shape CavC_ShapeToShape(const CavC_Shape &cavcShape);
LineArcGeometry::MultiShape CavC_MultiShapeToMultiShape(const CavC_MultiShape &cavcMultiShape);
LineArcGeometry::MultiShape OffsetLoopSetToMultiShape(const OffsetLoopSet &loopSet);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYCAVC_H
