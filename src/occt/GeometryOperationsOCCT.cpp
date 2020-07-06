#include "GeometryOperationsOCCT.h"
#include "GeometryOCCT.h"
#include "../GeometryQt.h"

#include <TopoDS_Face.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>

#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>

#include <IGESControl_Writer.hxx>

#include <QDebug>

// https://www.opencascade.com/content/remove-seams-after-fuse
// https://tracker.dev.opencascade.org/view.php?id=25462

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape GeometryOperationsOCCT::identity(const LineArcGeometry::MultiShape &multiShape)
{
    const TopoDS_Face face = MultiShapeToTopoDS_Face(multiShape);
    const LineArcGeometry::MultiShape reconverted = TopoDS_ShapeToMultiShape(face);
    return reconverted;
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::join(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsOCCT::join()";
    TopoDS_Shape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        if (it == multiShape.shapes.begin())
        {
            result = ShapeToTopoDS_Face(*it);
            continue;
        }
        BRepAlgoAPI_Fuse fuser(result, ShapeToTopoDS_Face(*it));
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
            return LineArcGeometry::MultiShape();
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
        result = su.Shape();
    }

    return TopoDS_ShapeToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsOCCT::join()";
    TopoDS_Shape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator it = a.shapes.begin(); it != a.shapes.end(); ++it)
    {
        if (it == a.shapes.begin())
        {
            result = ShapeToTopoDS_Face(*it);
            continue;
        }
        BRepAlgoAPI_Fuse fuser(result, ShapeToTopoDS_Face(*it));
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
            return LineArcGeometry::MultiShape();
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
        result = su.Shape();
    }
    for (std::list<LineArcGeometry::Shape>::const_iterator it = b.shapes.begin(); it != b.shapes.end(); ++it)
    {
        if (it == a.shapes.begin())
        {
            result = ShapeToTopoDS_Face(*it);
            continue;
        }
        BRepAlgoAPI_Fuse fuser(result, ShapeToTopoDS_Face(*it));
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
            return LineArcGeometry::MultiShape();
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
        result = su.Shape();
    }

    return TopoDS_ShapeToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
#if 1
    const TopoDS_Face bb = MultiShapeToTopoDS_Face(b);
    LineArcGeometry::MultiShape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = a.shapes.begin(); shape_it != a.shapes.end(); ++shape_it)
    {
        const TopoDS_Face aa = ShapeToTopoDS_Face(*shape_it);

        BRepAlgoAPI_Common common(aa, bb);
        common.SetRunParallel(true);
        common.SetFuzzyValue(1.e-5);
        common.SetNonDestructive(true);
        common.SetGlue(BOPAlgo_GlueShift);
        common.SetCheckInverted(true);

        common.Build();

        if (common.HasErrors())
        {
            qDebug() << "BRepAlgoAPI_Common::HasErrors()";
            common.GetReport()->Dump(std::cout);
            return LineArcGeometry::MultiShape();
        }

        if (common.HasWarnings())
        {
            qDebug() << "BRepAlgoAPI_Common::HasWarnings()";
            common.GetReport()->Dump(std::cout);
        }

        // clean up output
        const TopoDS_Shape &r = common.Shape();
        ShapeUpgrade_UnifySameDomain su(r);
        su.Build();
        const TopoDS_Shape diff = su.Shape();
        qDebug() << "...converting results to MultiShape...";
        const LineArcGeometry::MultiShape difference = TopoDS_ShapeToMultiShape(diff);
        for (std::list<LineArcGeometry::Shape>::const_iterator it = a.shapes.begin(); it != a.shapes.end(); ++it)
        {
            result.shapes.push_back(*it);
        }
    }
    // qDebug() << "...returning MultiShape!";
    return result;
#else
    // qDebug() << "GeometryOperationsOCCT::intersection()";
    const TopoDS_Face aa = MultiShapeToTopoDS_Face(a);
    const TopoDS_Face bb = MultiShapeToTopoDS_Face(b);

    BRepAlgoAPI_Common common(aa, bb);
    common.SetRunParallel(true);
    common.SetFuzzyValue(1.e-5);
    common.SetNonDestructive(true);
    common.SetGlue(BOPAlgo_GlueShift);
    common.SetCheckInverted(true);

    common.Build();

    if (common.HasErrors())
    {
        qDebug() << "BRepAlgoAPI_Common::HasErrors()";
        common.GetReport()->Dump(std::cout);
        return LineArcGeometry::MultiShape();
    }

    if (common.HasWarnings())
    {
        qDebug() << "BRepAlgoAPI_Common::HasWarnings()";
        common.GetReport()->Dump(std::cout);
    }

    // clean up output
    const TopoDS_Shape &r = common.Shape();
    ShapeUpgrade_UnifySameDomain su(r);
    su.Build();
    const TopoDS_Shape result = su.Shape();
    // qDebug() << "...converting results to MultiShape...";
    return TopoDS_ShapeToMultiShape(result);
#endif
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
#if 1
    // qDebug() << "GeometryOperationsOCCT::difference()";
    const TopoDS_Face subtrahend = MultiShapeToTopoDS_Face(b);
    // const TopoDS_Face subtrahend = ShapeToTopoDS_Face(b.shapes.front());
    LineArcGeometry::MultiShape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = a.shapes.begin(); shape_it != a.shapes.end(); ++shape_it)
    {
        // qDebug() << "constructing minuend...";
        // qDebug() << "minuend:" << LineArcGeometry::Shape(shape_it->boundary);
        // const TopoDS_Face minuend    = ShapeToTopoDS_Face(LineArcGeometry::Shape(shape_it->boundary));
        const TopoDS_Face minuend    = ShapeToTopoDS_Face(*shape_it);
        // qDebug() << "...done constructing minuend!";

        BRepAlgoAPI_Cut cutter(minuend, subtrahend);
        cutter.SetRunParallel(true);
        cutter.SetFuzzyValue(1.e-5);
        cutter.SetNonDestructive(true);
        cutter.SetGlue(BOPAlgo_GlueShift);
        cutter.SetCheckInverted(true);

        cutter.Build();

        if (cutter.HasErrors())
        {
            qDebug() << "BRepAlgoAPI_Cut::HasErrors()";
            cutter.GetReport()->Dump(std::cout);
            return LineArcGeometry::MultiShape();
        }

        if (cutter.HasWarnings())
        {
            qDebug() << "BRepAlgoAPI_Cut::HasWarnings()";
            cutter.GetReport()->Dump(std::cout);
        }

        // clean up output
        const TopoDS_Shape &r = cutter.Shape();
        ShapeUpgrade_UnifySameDomain su(r);
        su.Build();
        const TopoDS_Shape diff = su.Shape();
        // qDebug() << "...converting results to MultiShape...";
        const LineArcGeometry::MultiShape difference = TopoDS_ShapeToMultiShape(diff);
        for (std::list<LineArcGeometry::Shape>::const_iterator it = a.shapes.begin(); it != a.shapes.end(); ++it)
        {
            result.shapes.push_back(*it);
        }
    }
    // qDebug() << "...returning MultiShape!";
    return result;
#else
    // qDebug() << "GeometryOperationsOCCT::difference()";
    // qDebug() << "constructing minuend...";
    const TopoDS_Face minuend    = MultiShapeToTopoDS_Face(a);
    // qDebug() << "constructing subtrahend...";
    const TopoDS_Face subtrahend = MultiShapeToTopoDS_Face(b);
    // qDebug() << "...minuend and subtrahend constructed!";

    BRepAlgoAPI_Cut cutter(minuend, subtrahend);
    cutter.SetRunParallel(true);
    cutter.SetFuzzyValue(1.e-5);
    cutter.SetNonDestructive(true);
    cutter.SetGlue(BOPAlgo_GlueShift);
    cutter.SetCheckInverted(true);

    cutter.Build();

    if (cutter.HasErrors())
    {
        qDebug() << "BRepAlgoAPI_Cut::HasErrors()";
        // cutter.GetReport()->Dump(std::cout);
        return LineArcGeometry::MultiShape();
    }

    if (cutter.HasWarnings())
    {
        qDebug() << "BRepAlgoAPI_Cut::HasWarnings()";
        // cutter.GetReport()->Dump(std::cout);
    }

    // clean up output
    const TopoDS_Shape &r = cutter.Shape();
    ShapeUpgrade_UnifySameDomain su(r);
    su.Build();
    const TopoDS_Shape result = su.Shape();
    // qDebug() << "...converting results to MultiShape...";
    return TopoDS_ShapeToMultiShape(result);
#endif
}

LineArcGeometry::MultiShape GeometryOperationsOCCT::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    // qDebug() << "GeometryOperationsOCCT::offset()";
    BRepOffsetAPI_MakeOffset off;
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        const LineArcGeometry::Shape &shape = *shape_it;

        // add boundary
        const LineArcGeometry::Contour fixed_boundary = (shape.boundary.orientation() == LineArcGeometry::Segment::Clockwise) ? shape.boundary.reversed() : shape.boundary;
        off.AddWire(ContourToTopoDS_Wire(fixed_boundary));

        // add holes
        for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
        {
            const LineArcGeometry::Contour fixed_hole = (hole_it->orientation() != LineArcGeometry::Segment::Clockwise) ? hole_it->reversed() : *hole_it;
            off.AddWire(ContourToTopoDS_Wire(fixed_hole));
        }
    }
    off.Init(GeomAbs_Arc);
    off.Perform(radius);

    if (!off.IsDone())
    {
        qDebug() << "ERROR: BRepOffsetAPI_MakeOffset::IsDone() is false!";
    }
    const TopoDS_Shape& r = off.Shape();
    if (r.IsNull())
    {
        qDebug() << "WARNING: TopoDS_Shape::IsNull() is true (might be okay)";
    }
    // return TopoDS_ShapeToMultiShape(r, false);

    ShapeUpgrade_UnifySameDomain su(r);
    su.Build();
    const TopoDS_Shape result = su.Shape();
    
    if (0)
    {
        qDebug() << "Exporting IGES...";
        IGESControl_Writer writer;
        writer.AddShape(result);
        std::ofstream outfile("testcases/output.igs", std::ofstream::out);
        writer.Write(outfile);
        outfile.close();
    }
    
    // qDebug() << "Converting offset results...";
    return TopoDS_ShapeToMultiShape(result, false);
}

} // namespace LineArcOffsetDemo
