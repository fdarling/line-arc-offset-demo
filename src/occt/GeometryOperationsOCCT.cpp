#include "GeometryOperationsOCCT.h"
#include "GeometryOCCT.h"

#include <TopoDS_Face.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>

#include <QDebug>

// https://www.opencascade.com/content/remove-seams-after-fuse
// https://tracker.dev.opencascade.org/view.php?id=25462

namespace LineArcOffsetDemo {

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
            // fuser.GetReport()->Dump(std::cout);
            return LineArcGeometry::MultiShape();
        }

        if (fuser.HasWarnings())
        {
            qDebug() << "BRepAlgoAPI_Fuse::HasWarnings()";
            // fuser.GetReport()->Dump(std::cout);
        }
        // result = fuser.Shape();

        // clean up output
        // TODO is ShapeUpgrade_UnifySameDomain really necessary?
        const TopoDS_Shape &r = fuser.Shape();
        ShapeUpgrade_UnifySameDomain su(r);
        su.Build();
        result = su.Shape();
    }
    
    return TopoDS_ShapeToMultiShape(result);
}

} // namespace LineArcOffsetDemo
