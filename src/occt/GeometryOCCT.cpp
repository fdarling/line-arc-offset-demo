#include "GeometryOCCT.h"
#include "../GeometryQt.h"

#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>

// TODO stop using Qt in this file!
#include <QLineF>
#include <QVector2D>
#include <QDebug>

QT_BEGIN_NAMESPACE

static QDebug operator<<(QDebug debug, const gp_Pnt &pt)
{
    QDebugStateSaver saver(debug);
    //QString::number(pt.Y(), 'f', 4)
    // debug.nospace() << "gp_Pnt(" << pt.X() << ", " << pt.Y() << ")";
    debug.nospace() << "gp_Pnt(" << (qFuzzyIsNull(pt.X()) ? 0.0 : pt.X()) << ", " << (qFuzzyIsNull(pt.Y()) ? 0.0 : pt.Y()) << ")";
    return debug;
}

/*static QDebug operator<<(QDebug debug, const LineArcGeometry::Point &pt)
{
    QDebugStateSaver saver(debug);
    // debug.nospace() << "Point(" << pt.x << ", " << pt.y << ")";
    debug.nospace() << "Point(" << (qFuzzyIsNull(pt.x) ? 0.0 : pt.x) << ", " << (qFuzzyIsNull(pt.y) ? 0.0 : pt.y) << ")";
    return debug;
}*/

QT_END_NAMESPACE

namespace LineArcOffsetDemo {

template<typename T>
static bool FuzzyCompareWithNull(const T &a, const T &b)
{
    return qFuzzyCompare(a, b) || (qFuzzyIsNull(a) && qFuzzyIsNull(b));
}

static bool FuzzyCompare_gp_Pnt(const gp_Pnt &a, const gp_Pnt &b)
{
    return FuzzyCompareWithNull(a.X(), b.X()) && FuzzyCompareWithNull(a.Y(), b.Y());
}

static TopoDS_Wire ContourToTopoDS_Wire(const LineArcGeometry::Contour &contour)
{
    // qDebug() << "ContourToTopoDS_Wire";
    BRepBuilderAPI_MakeWire wire_builder;
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        TopoDS_Edge edge;
        if (it->isArc)
        {
            // TODO handle "full" circles which means the start/end points are coincident, currently LineArcGeometry::Segment isn't well defined for this though...
            // qDebug() << "ContourToTopoDS_Wire: center" << it->center.x << "," << it->center.y << " --- from" << it->line.p1.x << "," << it->line.p1.y << "to" << it->line.p2.x << "," << it->line.p2.y << "ORIENT:" << it->orientation;
            const LineArcGeometry::Point midPoint = it->midPoint();
            const gp_Pnt p1(it->line.p1.x, it->line.p1.y, 0);
            const gp_Pnt p2(it->line.p2.x, it->line.p2.y, 0);
            const gp_Pnt p3(midPoint.x, midPoint.y, 0);
            const Handle(Geom_TrimmedCurve) arc = GC_MakeArcOfCircle(p1, p3, p2);
            BRepBuilderAPI_MakeEdge edge_builder(arc);
            if (!edge_builder.IsDone())
            {
                qDebug() << "ERROR: BRepBuilderAPI_MakeEdge::IsDone() is false!";
                return TopoDS_Wire();
            }
            if (edge_builder.Error() != BRepBuilderAPI_EdgeDone) // might be redundant with above check
            {
                qDebug() << "ERROR: BRepBuilderAPI_MakeEdge::Error() gives some error status!";
                return TopoDS_Wire();
            }
            edge = edge_builder.Edge();
            // for debug output
            {
                // const Handle(Geom_Curve) curve = arc->BasisCurve();
                // const Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);
                // const LineArcGeometry::Point center(gp_PntToPoint(circle->Circ().Location()));
                // qDebug() << "ToOCCT: ARC center" << center.x << "," << center.y << "from" << p1.X() << "," << p1.Y() << "to" << p2.X() << "," << p2.Y() << "through" << p3.X() << "," << p3.Y();
            }
            // verifying the created edge matches the source segment
            {
                Standard_Real t_start, t_end;
                const Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, t_start, t_end);
                gp_Pnt op1, op2, op3;
                curve->D0(t_start, op1);
                curve->D0((t_end - t_start)/2.0, op3);
                curve->D0(t_end, op2);
                if (edge.Orientation() == TopAbs_REVERSED)
                {
                    std::swap(op1, op2);
                }

                if (!FuzzyCompare_gp_Pnt(p1, op1) ||
                    !FuzzyCompare_gp_Pnt(p2, op2) ||
                    !FuzzyCompare_gp_Pnt(p3, op3))
                {
                    qDebug() << "ERROR: generated OCCT TopoDS_Edge didn't match source LineArcGeometry::Segment";
                    qDebug() << "Input: " <<  p1 <<  p2 <<  p3;
                    qDebug() << "Output:" << op1 << op2 << op3;
                }
            }
        }
        else
        {
            TopoDS_Vertex p1(BRepBuilderAPI_MakeVertex(gp_Pnt(it->line.p1.x, it->line.p1.y, 0)));
            TopoDS_Vertex p2(BRepBuilderAPI_MakeVertex(gp_Pnt(it->line.p2.x, it->line.p2.y, 0)));
            BRepBuilderAPI_MakeEdge edge_builder(p1, p2);
            if (!edge_builder.IsDone())
            {
                qDebug() << "ERROR: BRepBuilderAPI_MakeEdge::IsDone() is false!";
                return TopoDS_Wire();
            }
            if (edge_builder.Error() != BRepBuilderAPI_EdgeDone) // might be redundant with above check
            {
                qDebug() << "ERROR: BRepBuilderAPI_MakeEdge::Error() gives some error status!";
                return TopoDS_Wire();
            }
            edge = edge_builder.Edge();
        }
        wire_builder.Add(edge);
    }
    if (!wire_builder.IsDone())
    {
        qDebug() << "ERROR: BRepBuilderAPI_MakeWire::IsDone() is false!";
        return TopoDS_Wire();
    }
    if (wire_builder.Error() != BRepBuilderAPI_WireDone) // might be redundant with above check
    {
             if (wire_builder.Error() == BRepBuilderAPI_EmptyWire)
            qDebug() << "ERROR: BRepBuilderAPI_MakeWire::Error() gives BRepBuilderAPI_EmptyWire";
        else if (wire_builder.Error() == BRepBuilderAPI_DisconnectedWire)
            qDebug() << "ERROR: BRepBuilderAPI_MakeWire::Error() gives BRepBuilderAPI_DisconnectedWire";
        else if (wire_builder.Error() == BRepBuilderAPI_NonManifoldWire)
            qDebug() << "ERROR: BRepBuilderAPI_MakeWire::Error() gives BRepBuilderAPI_NonManifoldWire";
        else
            qDebug() << "ERROR: BRepBuilderAPI_MakeWire::Error() gives some unrecognized error!";
        return TopoDS_Wire();
    }
    return wire_builder.Wire();
}

TopoDS_Face ShapeToTopoDS_Face(const LineArcGeometry::Shape &shape)
{
    // qDebug() << "ShapeToTopoDS_Face";
    BRepBuilderAPI_MakeFace builder(gp_Pln(), ContourToTopoDS_Wire(shape.boundary), true);
    for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
    {
        builder.Add(ContourToTopoDS_Wire(hole_it->reversed())); // TODO does reversed() even help?
    }
    if (!builder.IsDone())
    {
        qDebug() << "ERROR: BRepBuilderAPI_MakeFace::IsDone() is false!";
    }
    return builder.Face();
}

TopoDS_Face MultiShapeToTopoDS_Face(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "MultiShapeToTopoDS_Face";
    TopoDS_Compound result;
    TopoDS_Builder topods_builder;
    topods_builder.MakeCompound(result);
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        TopoDS_Face face = ShapeToTopoDS_Face(*it);
        topods_builder.Add(result, face);
    }

    // HACK force assigning TopoDS_Shape to a TopoDS_Face by down-casting the target through a reference. How is that even working?
    TopoDS_Face face_result;
    TopoDS_Shape &face_ref = face_result;
    face_ref = result;
    return face_result;
}

LineArcGeometry::Point gp_PntToPoint(const gp_Pnt &pt)
{
    return LineArcGeometry::Point(pt.X(), pt.Y());
}

static LineArcGeometry::Contour TopoDS_WireToContour(const TopoDS_Wire &wire)
{
    LineArcGeometry::Contour result;
    // TODO is there a difference between TopExp_Explorer::Current() and TopExp_Explorer::Value()?
    for (TopExp_Explorer edge_it(wire, TopAbs_EDGE); edge_it.More(); edge_it.Next())
    {
        // get edge
        const TopoDS_Edge edge = TopoDS::Edge(edge_it.Current());

        // get curve and it's start/end points
        Standard_Real t_start, t_end;
        const Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, t_start, t_end);
        gp_Pnt p1, p2, p3;
        curve->D0(t_start, p1);
        curve->D0((t_end - t_start)/2.0, p3);
        curve->D0(t_end, p2);
        // const bool isReversed = edge.Orientation() == TopAbs_REVERSED;

        const LineArcGeometry::Line line(gp_PntToPoint(p1), gp_PntToPoint(p2));
        if(curve->DynamicType() == STANDARD_TYPE(Geom_Line))
        {
            result.segments.push_back(line);
        }
        else if(curve->DynamicType() == STANDARD_TYPE(Geom_Circle))
        {
            const Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);
            const LineArcGeometry::Point center(gp_PntToPoint(circle->Circ().Location()));

            const QVector2D chordLineNormal(QLineF(QPointF(), QPointF(line.p2.x, line.p2.y) - QPointF(line.p1.x, line.p1.y)).normalVector().p2());
            const QVector2D midptVec(QLineF(QPointF(), QPointF(p3.X(), p3.Y()) - QPointF(line.p1.x, line.p1.y)).p2());
            const bool veersLeft = (QVector2D::dotProduct(chordLineNormal, midptVec) < 0.0);
            LineArcGeometry::Segment::Orientation orientation = veersLeft ? LineArcGeometry::Segment::Clockwise : LineArcGeometry::Segment::CounterClockwise;
            // LineArcGeometry::Segment::Orientation orientation = LineArcGeometry::Segment::Clockwise;
            // LineArcGeometry::Segment::Orientation orientation = LineArcGeometry::Segment::CounterClockwise;
            // qDebug() << "OUT: ARC center" << center << "from" << p1 << "to" << p2 << "through" << p3 << "Orientation():" << edge.Orientation() << "veersLeft" << veersLeft << "calc orientation" << orientation;
            // check for "full circles"
            if (line.p1 == line.p2)
            {
                // treat concident start/end points as full circles and not as zero length arcs
                const LineArcGeometry::Point pa(line.p1);
                const LineArcGeometry::Point pb(2*center - line.p1); // got the opposite side
                const LineArcGeometry::Line l1(pa, pb);
                const LineArcGeometry::Line l2(pb, pa);
                result.segments.push_back(LineArcGeometry::Segment(l1, center, orientation));
                result.segments.push_back(LineArcGeometry::Segment(l2, center, orientation));
            }
            else
            {
                result.segments.push_back(LineArcGeometry::Segment(line, center, orientation));
            }
        }
        else if (curve->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
        {
            const Handle(Geom_TrimmedCurve) trimmed = Handle(Geom_TrimmedCurve)::DownCast(curve);

            if (trimmed->BasisCurve()->DynamicType() == STANDARD_TYPE(Geom_Line))
            {
                result.segments.push_back(line);
            }
            else if (trimmed->BasisCurve()->DynamicType() == STANDARD_TYPE(Geom_Circle))
            {
                Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(trimmed->BasisCurve());
                const LineArcGeometry::Point center(gp_PntToPoint(circle->Circ().Location()));
                LineArcGeometry::Segment::Orientation orientation = edge.Orientation() ? LineArcGeometry::Segment::Clockwise : LineArcGeometry::Segment::CounterClockwise;
                result.segments.push_back(LineArcGeometry::Segment(line, center, orientation));
            }
            else
            {
                qDebug() << "ERROR: unsupported Geom_TrimmedCurve basis curve type:" << trimmed->BasisCurve()->DynamicType()->Name();
                return LineArcGeometry::Contour();
            }
        }
        else
        {
            qDebug() << "ERROR: unsupported Geom_Curve type:" << curve->DynamicType()->Name();
            return LineArcGeometry::Contour();
        }
    }
    return result;
}

static LineArcGeometry::Shape TopoDS_FaceToShape(const TopoDS_Face &face)
{
    LineArcGeometry::Shape result;
    // idea to determine the outer boundary taken from
    // https://www.opencascade.com/content/how-get-external-and-internal-boundary-holes-edges-mesh-face
    double largestArea = -1.0;
    for (TopExp_Explorer wire_it(face, TopAbs_WIRE); wire_it.More(); wire_it.Next())
    {
        const TopoDS_Wire wire = TopoDS::Wire(wire_it.Current());
        const LineArcGeometry::Contour contour = TopoDS_WireToContour(wire);

        Bnd_Box box;
        BRepBndLib::Add(wire, box);
        const double area = box.SquareExtent();
        if (area > largestArea)
        {
            // save what we thought was the boundary as a hole
            result.holes.push_back(result.boundary);

            // update the boundary with the newly found largest contour
            result.boundary = contour;
        }
        else
        {
            // we definitely have a hole since it's smaller than something we've seen before
            result.holes.push_back(contour);
        }
    }
    return result;
}

LineArcGeometry::MultiShape TopoDS_ShapeToMultiShape(const TopoDS_Shape &shape)
{
    LineArcGeometry::MultiShape result;
    for (TopExp_Explorer face_it(shape, TopAbs_FACE); face_it.More(); face_it.Next())
    {
        const TopoDS_Face face = TopoDS::Face(face_it.Current());
        const LineArcGeometry::Shape shape = TopoDS_FaceToShape(face);
        result.shapes.push_back(shape);
    }
    return result;
}

} // namespace LineArcOffsetDemo
