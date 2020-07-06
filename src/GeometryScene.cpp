#include "GeometryScene.h"
#include "GeometryQt.h"
#ifdef USING_CGAL
#include "cgal/GeometryOperationsCGAL.h"
// #include "cgal/GeometryCGAL.h" // WARNING: pulling in CGAL headers will slow down compilation significantly
// #include "cgal/CGALWrapper.h" // WARNING: pulling in CGAL headers will slow down compilation significantly
#endif // USING_CGAL
#ifdef USING_OCCT
#include "occt/GeometryOperationsOCCT.h"
#include "occt/GeometryOCCT.h"
#endif // USING_OCCT
#include "Svg.h"

#include <QProgressDialog>
#include <QElapsedTimer>
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
    scene->addPath(path, QPen(color, 0.0));
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
}

void GeometryScene::runTests()
{
    const LineArcGeometry::MultiShape overlappingShapes = SVG_Load("testcases/traces_01.svg");
    enum TestType
    {
        TEST_RAW,
        TEST_IDENTITY,
        TEST_UNARY_UNION,
        TEST_UNION,
        TEST_INTERSECTION,
        TEST_DIFFERENCE,
        TEST_OFFSET
    } testType;
    // testType = TEST_RAW;
    // testType = TEST_IDENTITY;
    // testType = TEST_UNARY_UNION;
    // testType = TEST_UNION;
    // testType = TEST_INTERSECTION;
    // testType = TEST_DIFFERENCE;
    testType = TEST_OFFSET;
    if (testType == TEST_RAW)
    {
        // display the raw geometry imported from the SVG (before any operations are run on it)
        AddMultiShapeToScene(this, overlappingShapes);
        SVG_Save("testcases/output.svg", overlappingShapes);
        return;
    }
#if defined(USING_CGAL)
    GeometryOperationsCGAL ops;
#elif defined(USING_OCCT)
    GeometryOperationsOCCT ops;
#else // !defined(USING_CGAL) && !defined(USING_OCCT)
#error "Currently the program only supports one backend (CGAL or OCCT) at a time, in the future it will be more flexible"
#endif // defined(USING_CGAL)
    if (testType == TEST_IDENTITY)
    {
        // test if geometry survives converting back and forth from the engine's internal format
        const LineArcGeometry::MultiShape reconverted = ops.identity(overlappingShapes);
        AddMultiShapeToScene(this, reconverted);
        SVG_Save("testcases/output.svg", reconverted);
        return;
    }
    // combine the overlapping contents of the test case file using union
    const LineArcGeometry::MultiShape joined = ops.join(overlappingShapes);
    if (testType == TEST_UNARY_UNION)
    {
        AddMultiShapeToScene(this, joined);
        SVG_Save("testcases/output.svg", joined);
    }
    else if (testType == TEST_UNION)
    {
        // further test union
        const LineArcGeometry::MultiShape addend = SVG_Load("testcases/traces_02.svg");
        // AddMultiShapeToScene(this, addend);
        const LineArcGeometry::MultiShape sum = ops.join(joined, addend);
        AddMultiShapeToScene(this, sum);
        SVG_Save("testcases/output.svg", sum);
    }
    else if (testType == TEST_INTERSECTION)
    {
        // test intersection
        const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/traces_02.svg");
        // AddMultiShapeToScene(this, cutter);
        const LineArcGeometry::MultiShape intersection = ops.intersection(joined, cutter);
        AddMultiShapeToScene(this, intersection);
        SVG_Save("testcases/output.svg", intersection);
    }
    else if (testType == TEST_DIFFERENCE)
    {
        // test difference
        // const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/thermal.svg");
        const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/traces_02.svg");
        // AddMultiShapeToScene(this, cutter);
        const LineArcGeometry::MultiShape diffed = ops.difference(joined, cutter);
        AddMultiShapeToScene(this, diffed);
        SVG_Save("testcases/output.svg", diffed);
    }
    else if (testType == TEST_OFFSET)
    {
        // show the original geometry
        AddMultiShapeToScene(this, joined);

        // const double radii[] = {0.010, 0.020, 0.030, 0.040};
        const double radii[] = {0.010, 0.020, 0.030, 0.040, 0.050, 0.060, 0.070, 0.080, 0.090, 0.100, 0.110, 0.120, 0.130, 0.140, 0.150, 0.160, 0.170};
        // const double radii[] = {0.011};
        // const double radii[] = {0.005};
        // const double radii[] = {0.020};
        const size_t NUM_RADII = sizeof(radii)/sizeof(radii[0]);
        QProgressDialog progress("Generating offsets...", "Stop", 0, NUM_RADII/*, this*/);
        progress.setWindowModality(Qt::WindowModal);
        QElapsedTimer timer;
        progress.setValue(0);
        for (size_t i = 0; i < NUM_RADII; i++)
        {
            if (progress.wasCanceled())
                break;
            const double radius = radii[i];
            qDebug() << "Generating offset polygon" << (i+1) << "of" << NUM_RADII << "(" << radius << ")...";
            timer.start();
            const LineArcGeometry::MultiShape offset_shapes = ops.offset(joined, radius);
            qDebug() << "...took" << (timer.nsecsElapsed()/1.0e9) << "seconds";
            AddMultiShapeToScene(this, offset_shapes, Qt::green);
            // SVG_Save("testcases/output.svg", offset_shapes);
            progress.setValue(i+1);
        }
    }
}

} // namespace LineArcOffsetDemo
