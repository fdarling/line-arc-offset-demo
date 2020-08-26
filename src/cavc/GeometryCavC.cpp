#include "GeometryCavC.h"
#include "CavCQt.h"

#include <cassert>

namespace LineArcOffsetDemo {

LineArcGeometry::CoordinateType FromCavC(CavC_Real value)
{
    return value;
}

CavC_Real ToCavC(LineArcGeometry::CoordinateType value)
{
    return value;
}

Vector2 PointToVector2(const LineArcGeometry::Point &point)
{
    return Vector2(ToCavC(point.x), ToCavC(point.y));
}

static CavC_Real BulgeFromSegment(const LineArcGeometry::Segment &segment)
{
    if (!segment.isArc)
        return 0.0;
    const LineArcGeometry::Point &pt1 = segment.line.p1;
    const LineArcGeometry::Point  pt2 = segment.midPoint();
    const LineArcGeometry::Point &pt3 = segment.line.p2;

    const LineArcGeometry::Point chordMid = (pt1 + pt3)/2.0;
    const LineArcGeometry::CoordinateType sagittaLength = LineArcGeometry::Line(chordMid, pt2).length();
    const LineArcGeometry::CoordinateType chordLength = LineArcGeometry::Line(pt1, pt3).length();
    const CavC_Real bulgeMagnitude = 2.0 * sagittaLength / chordLength;
    const CavC_Real bulge = (segment.orientation == LineArcGeometry::Segment::Clockwise) ? -bulgeMagnitude : bulgeMagnitude;
    return bulge;
}

PVertex SegmentToPVertex(const LineArcGeometry::Segment &segment)
{
    const CavC_Real bulge = BulgeFromSegment(segment);
    return Polyline::PVertex(PointToVector2(segment.line.p1), bulge);
}

static void AddPVertices(Polyline &polyline, const LineArcGeometry::Segment &segment)
{
    const CavC_Real bulge = BulgeFromSegment(segment);
    if (bulge >= -1.0 && bulge <= 1.0)
    {
        polyline.addVertex(Polyline::PVertex(PointToVector2(segment.line.p1), bulge));
    }
    else
    {
        // break the arc in half
        // TODO move this to a method in the Segment class
        const LineArcGeometry::Point midPoint(segment.midPoint());
        const LineArcGeometry::Segment seg1(LineArcGeometry::Line(segment.line.p1, midPoint), segment.center, segment.orientation);
        const LineArcGeometry::Segment seg2(LineArcGeometry::Line(midPoint, segment.line.p2), segment.center, segment.orientation);
        const CavC_Real bulge1 = BulgeFromSegment(seg1);
        const CavC_Real bulge2 = BulgeFromSegment(seg2);
        polyline.addVertex(Polyline::PVertex(PointToVector2(seg1.line.p1), bulge1));
        polyline.addVertex(Polyline::PVertex(PointToVector2(seg2.line.p1), bulge2));
    }
}

Polyline ContourToPolyline(const LineArcGeometry::Contour &contour)
{
    Polyline result;
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        AddPVertices(result, *it);
    }
    result.isClosed() = true;
    return result;
}

static LineArcGeometry::Contour EnsurePositive(const LineArcGeometry::Contour &contour)
{
    if (contour.orientation() == LineArcGeometry::Segment::Clockwise)
        return contour;
    return contour.reversed();
}

static LineArcGeometry::Contour EnsureNegative(const LineArcGeometry::Contour &contour)
{
    if (contour.orientation() != LineArcGeometry::Segment::Clockwise)
        return contour;
    return contour.reversed();
}

CavC_Shape ShapeToCavC_Shape(const LineArcGeometry::Shape &shape)
{
    CavC_Shape result;
    // add boundary
    result.boundary = ContourToPolyline(EnsurePositive(shape.boundary));
    // add holes
    for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
    {
        result.holes.push_back(ContourToPolyline(EnsureNegative(*hole_it)));
    }
    return result;
}

CavC_MultiShape MultiShapeToCavC_MultiShape(const LineArcGeometry::MultiShape &multiShape)
{
    CavC_MultiShape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        result.push_back(ShapeToCavC_Shape(*shape_it));
    }
    return result;
}

static OffsetLoop ContourToBoundaryLoop(const LineArcGeometry::Contour &contour, bool isBoundary)
{
    // TODO why does the orientation have to be backwards from expected?
    Polyline polyline(ContourToPolyline((!isBoundary ? EnsurePositive : EnsureNegative)(contour)));
    assert(!polyline.vertexes().empty());
    StaticSpatialIndex spatialIndex(cavc::createApproxSpatialIndex(polyline));
    OffsetLoop boundaryLoop = {0, std::move(polyline), std::move(spatialIndex)};
    return std::move(boundaryLoop);
}

OffsetLoopSet MultiShapeToOffsetLoopSet(const LineArcGeometry::MultiShape &multiShape)
{
    OffsetLoopSet result;
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        const LineArcGeometry::Shape &shape = *shape_it;
        // add boundary
        result.ccwLoops.push_back(ContourToBoundaryLoop(shape_it->boundary, true));
        // add holes
        for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
        {
            result.cwLoops.push_back(ContourToBoundaryLoop(*hole_it, false));
        }
    }
    return result;
}

LineArcGeometry::Point Vector2ToPoint(const Vector2 &vertex)
{
    return LineArcGeometry::Point(FromCavC(vertex.x()), FromCavC(vertex.y()));
}

LineArcGeometry::Segment PVerticesToSegment(const PVertex &v1, const PVertex &v2)
{
    const LineArcGeometry::Line line(Vector2ToPoint(v1.pos()), Vector2ToPoint(v2.pos()));
    if (v1.bulgeIsZero())
        return LineArcGeometry::Segment(line);
    const cavc::ArcRadiusAndCenter<CavC_Real> radiusAndCenter = cavc::arcRadiusAndCenter(v1, v2);
    const LineArcGeometry::Segment::Orientation orientation = (v1.bulge() < 0.0) ? LineArcGeometry::Segment::Clockwise : LineArcGeometry::Segment::CounterClockwise;
    const LineArcGeometry::Segment segment(line, Vector2ToPoint(radiusAndCenter.center), orientation);
    return segment;
}

LineArcGeometry::Contour PolylineToContour(const Polyline &polyline)
{
    LineArcGeometry::Contour result;
    const std::vector<PVertex> vertices = polyline.vertexes();
    if (vertices.empty())
        return result;
    const PVertex *prev = &vertices.back();
    for (std::vector<PVertex>::const_iterator it = vertices.begin(); it != vertices.end(); prev = &(*it), ++it)
    {
        const LineArcGeometry::Segment segment(PVerticesToSegment(*prev, *it));
        result.segments.push_back(segment);
    }
    return result;
}

LineArcGeometry::Shape CavC_ShapeToShape(const CavC_Shape &cavcShape)
{
    LineArcGeometry::Shape result(PolylineToContour(cavcShape.boundary));
    for (Polylines::const_iterator hole_it = cavcShape.holes.begin(); hole_it != cavcShape.holes.end(); ++hole_it)
    {
        result.holes.push_back(PolylineToContour(*hole_it));
    }
    return result;
}

LineArcGeometry::MultiShape CavC_MultiShapeToMultiShape(const CavC_MultiShape &cavcMultiShape)
{
    LineArcGeometry::MultiShape result;
    for (CavC_MultiShape::const_iterator shape_it = cavcMultiShape.begin(); shape_it != cavcMultiShape.end(); ++shape_it)
    {
        result.shapes.push_back(CavC_ShapeToShape(*shape_it));
    }
    return result;
}

LineArcGeometry::MultiShape OffsetLoopSetToMultiShape(const OffsetLoopSet &loopSet)
{
    LineArcGeometry::MultiShape result;
    for (OffsetLoops::const_iterator it = loopSet.ccwLoops.begin(); it != loopSet.ccwLoops.end(); ++it)
    {
        result.shapes.push_back(LineArcGeometry::Shape(PolylineToContour(it->polyline)));
    }
    // TODO assign the holes to the shapes correctly!
    for (OffsetLoops::const_iterator it = loopSet.cwLoops.begin(); it != loopSet.cwLoops.end(); ++it)
    {
        result.shapes.push_back(LineArcGeometry::Shape(PolylineToContour(it->polyline)));
    }
    return result;
}

} // namespace LineArcOffsetDemo
