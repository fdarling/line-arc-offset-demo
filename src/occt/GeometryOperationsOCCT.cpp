#include "GeometryOperationsOCCT.h"
#include "GeometryOCCT.h"
#include "../GeometryQt.h"

#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <ShapeFix_Face.hxx>
#include <gp_Pln.hxx>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>

#include <IGESControl_Writer.hxx>

#include <QDebug>

// https://www.opencascade.com/content/remove-seams-after-fuse
// https://tracker.dev.opencascade.org/view.php?id=25462

namespace LineArcOffsetDemo {

static void IGES_Save(const char *filePath, const TopoDS_Shape &shape)
{
    IGESControl_Writer writer;
    writer.AddShape(shape);
    std::ofstream outfile(filePath, std::ofstream::out);
    writer.Write(outfile);
    outfile.close();
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::identity(const LineArcGeometry::MultiShape &multiShape)
{
    const TopoDS_Face face = MultiShapeToTopoDS_Face(multiShape);
    const LineArcGeometry::MultiShape reconverted = TopoDS_ShapeToMultiShape(face);
    return reconverted;
}

template <typename BooleanOperationClass>
static TopoDS_Shape DoBoolean(const TopoDS_Shape &shapeA, const TopoDS_Shape &shapeB)
{
    BooleanOperationClass fuser(shapeA, shapeB);
    fuser.SetRunParallel(true);
    fuser.SetFuzzyValue(1.e-5);
    fuser.SetNonDestructive(true);
    fuser.SetGlue(BOPAlgo_GlueShift);
    fuser.SetCheckInverted(true);

    fuser.Build();

    if (fuser.HasErrors())
    {
        qDebug() << "BRepAlgoAPI_Fuse::HasErrors()";
        fuser.GetReport()->Dump(std::cout);
        return TopoDS_Shape();
    }

    if (fuser.HasWarnings())
    {
        qDebug() << "BRepAlgoAPI_Fuse::HasWarnings()";
        fuser.GetReport()->Dump(std::cout);
    }

    // clean up output
    const TopoDS_Shape &r = fuser.Shape();
    ShapeUpgrade_UnifySameDomain su(r);
    su.Build();
    return su.Shape();
}

static TopoDS_Shape DoUnion(const TopoDS_Shape &shapeA, const TopoDS_Shape &shapeB)
{
    // TODO more reliable way of determining if they are empty to suppress warnings!
    if (shapeA.IsNull())
        return shapeB;
    else if (shapeB.IsNull())
        return shapeA;
    return DoBoolean<BRepAlgoAPI_Fuse>(shapeA, shapeB);
}

static TopoDS_Shape DoDifference(const TopoDS_Shape &shapeA, const TopoDS_Shape &shapeB)
{
    // TODO more reliable way of determining if they are empty to suppress warnings!
    if (shapeA.IsNull())
        return TopoDS_Shape();
    else if (shapeB.IsNull())
        return shapeA;
    return DoBoolean<BRepAlgoAPI_Cut>(shapeA, shapeB);
}

static void FuseShapeInto(TopoDS_Shape &result, const TopoDS_Shape &shape)
{
    // TODO more reliable way of determining if they are empty to suppress warnings!
    if (result.IsNull())
        result = shape;
    else if (!shape.IsNull())
        result = DoBoolean<BRepAlgoAPI_Fuse>(result, shape);
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::join(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsOCCT::join()";
    TopoDS_Shape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        FuseShapeInto(result, ShapeToTopoDS_Face(*it));
    }
    if (0)
    {
        qDebug() << "Exporting join() result to IGES...";
        IGES_Save("testcases/output.igs", result);
    }
    return TopoDS_ShapeToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsOCCT::join()";
    const TopoDS_Face aa = MultiShapeToTopoDS_Face(a);
    const TopoDS_Face bb = MultiShapeToTopoDS_Face(b);
    const TopoDS_Shape result = DoBoolean<BRepAlgoAPI_Fuse>(aa, bb);
    if (0)
    {
        qDebug() << "Exporting join() result to IGES...";
        IGES_Save("testcases/output.igs", result);
    }
    return TopoDS_ShapeToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsOCCT::intersection()";
    const TopoDS_Face aa = MultiShapeToTopoDS_Face(a);
    const TopoDS_Face bb = MultiShapeToTopoDS_Face(b);
    const TopoDS_Shape result = DoBoolean<BRepAlgoAPI_Common>(aa, bb);
    if (0)
    {
        qDebug() << "Exporting intersection() result to IGES...";
        IGES_Save("testcases/output.igs", result);
    }
    // qDebug() << "...converting results to MultiShape...";
    return TopoDS_ShapeToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsOCCT::difference()";
    const TopoDS_Face aa = MultiShapeToTopoDS_Face(a);
    const TopoDS_Face bb = MultiShapeToTopoDS_Face(b);
    const TopoDS_Shape result = DoBoolean<BRepAlgoAPI_Cut>(aa, bb);
    if (0)
    {
        qDebug() << "Exporting difference() result to IGES...";
        IGES_Save("testcases/output.igs", result);
    }
    // qDebug() << "...converting results to MultiShape...";
    return TopoDS_ShapeToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::symmetricDifference(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsOCCT::symmetricDifference()";
    LineArcGeometry::MultiShape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        if (result.shapes.empty())
        {
            result.shapes.push_back(*it);
        }
        else
        {
            LineArcGeometry::MultiShape cutter;
            cutter.shapes.push_back(*it);
            result = symmetricDifference(result, cutter);
        }
    }
    return result;
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::symmetricDifference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsOCCT::symmetricDifference()";
#if 1
    const TopoDS_Face aa = MultiShapeToTopoDS_Face(a);
    const TopoDS_Face bb = MultiShapeToTopoDS_Face(b);
    const TopoDS_Shape a_minus_b = DoDifference(aa, bb);
    const TopoDS_Shape b_minus_a = DoDifference(bb, aa);
    const TopoDS_Shape result = DoUnion(a_minus_b, b_minus_a);
    return TopoDS_ShapeToMultiShape(result);
#else
    const LineArcGeometry::MultiShape a_minus_b = difference(a, b);
    const LineArcGeometry::MultiShape b_minus_a = difference(b, a);
    const LineArcGeometry::MultiShape joined = join(a_minus_b, b_minus_a);
    return joined;
#endif
}

static TopoDS_Shape MakeOffsetFromContour(const LineArcGeometry::Contour &contour, double radius)
{
    // qDebug() << "MakeOffsetFromContour()";
    const bool boundary_needs_reversal = (contour.orientation() == LineArcGeometry::Segment::Clockwise);
    const LineArcGeometry::Contour fixed_boundary = boundary_needs_reversal ? contour.reversed() : contour;
    TopoDS_Wire boundary = ContourToTopoDS_Wire(fixed_boundary);
    BRepOffsetAPI_MakeOffset off(boundary);
    off.Init(GeomAbs_Arc);
    off.Perform(radius);
    if (!off.IsDone())
    {
        qDebug() << "ERROR: BRepOffsetAPI_MakeOffset::IsDone() is false!";
    }
    const TopoDS_Shape& offsetShape = off.Shape();
    if (offsetShape.IsNull())
    {
        qDebug() << "WARNING: TopoDS_Shape::IsNull() is true (might be okay)";
    }
    return offsetShape;
}

static TopoDS_Shape MakeOffsetFromHoles(const LineArcGeometry::Shape &shape, double radius)
{
    // qDebug() << "MakeOffsetFromHoles()";
    if (shape.holes.empty())
        return TopoDS_Shape();
    BRepOffsetAPI_MakeOffset off;
    for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
    {
        const bool hole_needs_reversal = (hole_it->orientation() != LineArcGeometry::Segment::Clockwise);
        const LineArcGeometry::Contour fixed_hole = hole_needs_reversal ? hole_it->reversed() : *hole_it;
        off.AddWire(ContourToTopoDS_Wire(fixed_hole));
    }
    off.Init(GeomAbs_Arc);
    off.Perform(-radius); // NOTE: negative on purpose!
    if (!off.IsDone())
    {
        qDebug() << "ERROR: BRepOffsetAPI_MakeOffset::IsDone() is false!";
    }
    const TopoDS_Shape& offsetShape = off.Shape();
    if (offsetShape.IsNull())
    {
        qDebug() << "WARNING: TopoDS_Shape::IsNull() is true (might be okay)";
    }
    return offsetShape;
}

static TopoDS_Shape CombineOuterAndInnerOffsets(const TopoDS_Shape &outerShape, const TopoDS_Shape &innerShape)
{
    // qDebug() << "CombineOuterAndInnerOffsets()";
    if (innerShape.IsNull())
        return outerShape;
    else if (outerShape.IsNull())
        return TopoDS_Shape();
    // qDebug() << "...converting boundary to Shape...";
    const LineArcGeometry::Shape outerShapeConv = TopoDS_ShapeToShape(outerShape);
    // qDebug() << "...converting boundary to TopoDS_Face...";
    const TopoDS_Shape outerFace = ShapeToTopoDS_Face(outerShapeConv);
    // qDebug() << "...converting holes to Shape..."; // TODO this needs to be a MultiShape or something!
    const LineArcGeometry::Shape innerShapeConv = TopoDS_ShapeToShape(innerShape); // TODO this needs to be a MultiShape or something!
    if (innerShapeConv.boundary.segments.empty())
    {
        // qDebug() << "...no holes, skipping subtraction...";
        return outerShape;
    }
    // qDebug() << "...converting holes to TopoDS_Face...";
    const TopoDS_Shape innerFace = ShapeToTopoDS_Face(innerShapeConv);
    // qDebug() << "...subtracting holes from boundary...";
    const TopoDS_Shape offsetShape = DoBoolean<BRepAlgoAPI_Cut>(outerFace, innerFace);
    return offsetShape;
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    qDebug() << "GeometryOperationsOCCT::offset()";
    TopoDS_Shape result;
    qDebug() << "processing LineArcGeometry::Shape's...";
if (radius < 0.0)
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        qDebug() << "current LineArcGeometry::Shape:" << *shape_it;
        // convert the Shape to a TopoDS_Face
        qDebug() << "...converting LineArcGeometry::Shape to TopoDS_Face...";
        const TopoDS_Face face = ShapeToTopoDS_Face(*shape_it);

        // generate outer boundary and inner holes offsets independently
        qDebug() << "...offsetting outer boundary wire...";
        const TopoDS_Shape outerShape = MakeOffsetFromContour(shape_it->boundary, radius);
        qDebug() << "...offsetting hole wires...";
        const TopoDS_Shape innerShape = MakeOffsetFromHoles(*shape_it, radius);
        
        // then combine them using boolean difference
        qDebug() << "...combining offset boundary and interior offset holes...";
        const TopoDS_Shape offsetShape = CombineOuterAndInnerOffsets(outerShape, innerShape);

        // take the TopoDS_Shape containing free wires (no faces) and turn it into a shape (the outer boundary is determined, the rest are holes)
        qDebug() << "...converting TopoDS_Shape to LineArcGeometry::Shape...";
        const LineArcGeometry::Shape convertedOffsetShape = TopoDS_ShapeToShape(offsetShape);

        // convert it back to "Face", this time with the boundary/holes
        qDebug() << "...converting LineArcGeometry::Shape to TopoDS_Face...";
        const TopoDS_Face reconvertedOffsetShape = ShapeToTopoDS_Face(convertedOffsetShape);

        // apply boolean union, which works because the holes are respected
        qDebug() << "...fusing TopoDS_Face into cumulative TopoDS_Shape...";
        FuseShapeInto(result, reconvertedOffsetShape);
    }
else
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        qDebug() << "current LineArcGeometry::Shape:" << *shape_it;
        // convert the Shape to a TopoDS_Face
        qDebug() << "...converting LineArcGeometry::Shape to TopoDS_Face...";
        const TopoDS_Face face = ShapeToTopoDS_Face(*shape_it);

        // offset the TopoDS_Face (resulting in wires, no longer a face)
        qDebug() << "...offsetting TopoDS_Face into TopoDS_Shape containing wires...";
        BRepOffsetAPI_MakeOffset off(face);
        off.Init(GeomAbs_Arc);
        off.Perform(radius);
        if (!off.IsDone())
        {
            qDebug() << "ERROR: BRepOffsetAPI_MakeOffset::IsDone() is false!";
        }
        const TopoDS_Shape& offsetShape = off.Shape();
        if (offsetShape.IsNull())
        {
            qDebug() << "WARNING: TopoDS_Shape::IsNull() is true (might be okay)";
        }

        /*qDebug() << "...fixing TopoDS_Shape...";
        ShapeUpgrade_UnifySameDomain su(offsetShape);
        su.Build();
        const TopoDS_Shape fixedShape = su.Shape();*/

        // take the TopoDS_Shape containing free wires (no faces) and turn it into a shape (the outer boundary is determined, the rest are holes)
        qDebug() << "...converting TopoDS_Shape to LineArcGeometry::Shape...";
        const LineArcGeometry::Shape convertedOffsetShape = TopoDS_ShapeToShape(offsetShape); // or fixedShape

        // convert it back to "Face", this time with the boundary/holes
        qDebug() << "...converting LineArcGeometry::Shape to TopoDS_Face...";
        const TopoDS_Face reconvertedOffsetShape = ShapeToTopoDS_Face(convertedOffsetShape);

        // apply boolean union, which works because the holes are respected
        qDebug() << "...fusing TopoDS_Face into cumulative TopoDS_Shape...";
        FuseShapeInto(result, reconvertedOffsetShape);
    }
    qDebug() << "...done processing LineArcGeometry::Shape's";
    if (0)
    {
        qDebug() << "Exporting offset() result to IGES...";
        IGES_Save("testcases/output.igs", result);
    }
    qDebug() << "...converting results to MultiShape...";
    return TopoDS_ShapeToMultiShape(result);
}

} // namespace LineArcOffsetDemo
