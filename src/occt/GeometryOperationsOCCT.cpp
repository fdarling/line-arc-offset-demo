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

static void FuseShapeInto(TopoDS_Shape &result, const TopoDS_Shape &shape)
{
    if (result.IsNull())
        result = shape;
    else
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

template <typename T>
const T & GetNth(const std::list<T> &l, int index)
{
    typename std::list<T>::const_iterator it = l.begin();
    for (int i = 0; i < index; i++, ++it)
    {
    }
    return *it;
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    // qDebug() << "GeometryOperationsOCCT::offset()";
    TopoDS_Shape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        // convert the Shape to a TopoDS_Face
        const TopoDS_Face face = ShapeToTopoDS_Face(*shape_it);

        // offset the TopoDS_Face (resulting in wires, no longer a face)
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

        // take the TopoDS_Shape containing free wires (no faces) and turn it into a shape (the outer boundary is determined, the rest are holes)
        const LineArcGeometry::Shape convertedOffsetShape = TopoDS_ShapeToShape(offsetShape);

        // convert it back to "Face", this time with the boundary/holes
        const TopoDS_Face reconvertedOffsetShape = ShapeToTopoDS_Face(convertedOffsetShape);

        // apply boolean union, which works because the holes are respected
        FuseShapeInto(result, reconvertedOffsetShape);
    }
    if (0)
    {
        qDebug() << "Exporting offset() result to IGES...";
        IGES_Save("testcases/output.igs", result);
    }
    // qDebug() << "...converting results to MultiShape...";
    return TopoDS_ShapeToMultiShape(result);
}

} // namespace LineArcOffsetDemo
