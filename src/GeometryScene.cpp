#include "GeometryScene.h"
#include "GeometryQt.h"
#ifdef USING_CGAL
#include "cgal/GeometryOperationsCGAL.h"
// #include "cgal/GeometryCGAL.h" // WARNING: pulling in CGAL headers will slow down compilation significantly
#endif // USING_CGAL
#ifdef USING_OCCT
#include "occt/GeometryOperationsOCCT.h"
#include "occt/GeometryOCCT.h"
#endif // USING_OCCT
#include "Svg.h"

#include <QDebug>

namespace LineArcOffsetDemo {

static void AddContourToPath(QPainterPath &path, const LineArcGeometry::Contour &contour)
{
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        QPointF pt1(PointToQPointF(it->line.p1));
        QPointF pt2(PointToQPointF(it->line.p2));
        if (it->isArc)
        {
            const QPointF circleCenter(PointToQPointF(it->center));
            const double radius = QLineF(circleCenter, pt2).length();
            const QRectF  circleRect(it->center.x - radius, it->center.y - radius, radius*2.0, radius*2.0);
            QPointF  startPoint = pt1;
            QPointF    endPoint = pt2;
            const bool isCCW = (it->orientation != LineArcGeometry::Segment::Clockwise);
            const double startAngle = QLineF(circleCenter, startPoint).angle();
            // const double   endAngle = QLineF(circleCenter,   endPoint).angle();
            const double sweepAngle = QLineF(circleCenter, startPoint).angleTo(QLineF(circleCenter, endPoint));
            const double lineLength = QLineF(startPoint, endPoint).length();
            if (lineLength < 0.0001) // HACK, this is to fix some sort of rounding errors with .angle()...
            {
                path.moveTo(startPoint);
                path.lineTo(endPoint);
            }
            else
            {
                path.arcMoveTo(circleRect, startAngle);
                path.arcTo(circleRect, startAngle, isCCW ? (sweepAngle - 360.0) : sweepAngle);
            }
        }
        else
        {
            path.moveTo(pt1);
            path.lineTo(pt2);
        }
    }
}

static void AddShapeToScene(QGraphicsScene *scene, const LineArcGeometry::Shape &shape, const QColor &color)
{
    QPainterPath path;
    AddContourToPath(path, shape.boundary);
    for (std::list<LineArcGeometry::Contour>::const_iterator it = shape.holes.begin(); it != shape.holes.end(); ++it)
    {
        AddContourToPath(path, *it);
    }
    scene->addPath(path, QPen(color, 0.0)); // TODO allow color to be specified
}

static void AddMultiShapeToScene(QGraphicsScene *scene, const LineArcGeometry::MultiShape &multiShape, const QColor &color = Qt::black)
{
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        AddShapeToScene(scene, *it, color);
    }
}

GeometryScene::GeometryScene(QObject *parent) : QGraphicsScene(parent)
{
    LineArcGeometry::MultiShape multiShape = SVG_Load("testcases/traces_01.svg");
    // display the raw geometry imported from the SVG (before any operations are run on it)
    // AddMultiShapeToScene(this, multiShape);
    // SVG_Save("testcases/output.svg", multiShape);
#ifdef USING_CGAL
    {
        // test if geometry survives converting back and forth from CGAL data structures
        // const LineArcGeometry::MultiShape reconverted = PolygonWithHolesListToMultiShape(MultiShapeToPolygonWithHolesList(multiShape));
        // AddMultiShapeToScene(this, reconverted);
        // SVG_Save("testcases/output.svg", reconverted);
    }
    {
        // test CGAL implemented union boolean operation
        LineArcGeometry::MultiShape joined = GeometryOperationsCGAL().join(multiShape);
        AddMultiShapeToScene(this, joined);
        SVG_Save("testcases/output.svg", joined);
    }
#endif // USING_CGAL
#ifdef USING_OCCT
    {
        // test if geometry survives converting back and forth from OCCT data structures
        // const LineArcGeometry::MultiShape reconverted = TopoDS_ShapeToMultiShape(MultiShapeToTopoDS_Face(multiShape));
        // AddMultiShapeToScene(this, reconverted);
        // SVG_Save("testcases/output.svg", reconverted);
    }
    {
        // test OCCT implemented union boolean operation
        LineArcGeometry::MultiShape joined = GeometryOperationsOCCT().join(multiShape);
        AddMultiShapeToScene(this, joined);
        SVG_Save("testcases/output.svg", joined);
    }
#endif // USING_OCCT
}

} // namespace LineArcOffsetDemo
