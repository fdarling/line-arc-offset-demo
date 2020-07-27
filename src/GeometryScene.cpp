#include "GeometryScene.h"
#include "GeometryQt.h"
#include "GeometryOperations.h"
#include "AleksFile.h"
#include "Svg.h"

#include <QProgressDialog>
#include <QElapsedTimer>
#include <QDebug>

namespace LineArcOffsetDemo {

static void AddContourToPath(QPainterPath &path, const LineArcGeometry::Contour &contour)
{
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        // const bool doMoveTo = true;
        const bool doMoveTo = (it == contour.segments.begin());
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
                if (doMoveTo) path.moveTo(startPoint);
                path.lineTo(endPoint);
            }
            else
            {
                if (doMoveTo) path.arcMoveTo(circleRect, startAngle);
                path.arcTo(circleRect, startAngle, isCCW ? (sweepAngle - 360.0) : sweepAngle);
            }
        }
        else
        {
            if (doMoveTo) path.moveTo(pt1);
            path.lineTo(pt2);
        }
    }
}

static void AddShapeToScene(QGraphicsScene *scene, const LineArcGeometry::Shape &shape, const QColor &color, const QBrush &brush)
{
    QPainterPath path;
    AddContourToPath(path, shape.boundary);
    for (std::list<LineArcGeometry::Contour>::const_iterator it = shape.holes.begin(); it != shape.holes.end(); ++it)
    {
        AddContourToPath(path, *it);
    }
    scene->addPath(path, QPen(color, 0.0), brush);
}

static QColor ColorWithAlpha(const QColor &color, int alpha) __attribute__((unused));
static QColor ColorWithAlpha(const QColor &color, int alpha)
{
    QColor result = color;
    result.setAlpha(alpha);
    return result;
}

static void AddMultiShapeToScene(QGraphicsScene *scene, const LineArcGeometry::MultiShape &multiShape, const QColor &color = Qt::black, const QBrush &brush = Qt::NoBrush)
// static void AddMultiShapeToScene(QGraphicsScene *scene, const LineArcGeometry::MultiShape &multiShape, const QColor &color = Qt::black, const QBrush &brush = ColorWithAlpha(Qt::gray, 64))
{
    for (std::list<LineArcGeometry::Shape>::const_iterator it = multiShape.shapes.begin(); it != multiShape.shapes.end(); ++it)
    {
        AddShapeToScene(scene, *it, color, brush);
    }
}

GeometryScene::GeometryScene(QObject *parent) : QGraphicsScene(parent)
{
}

void GeometryScene::runTests(GeometryOperations &ops)
{
    const LineArcGeometry::MultiShape overlappingShapes = SVG_Load("testcases/traces_01.svg");
    // const LineArcGeometry::MultiShape overlappingShapes = AleksFile_Load("testcases/input_1_shape.txt");
    // const LineArcGeometry::MultiShape overlappingShapes = AleksFile_Load("testcases/input_2_shapes.txt");
    // AleksFile_Save("testcases/output.txt", overlappingShapes);
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
        // const double radii[] = {0.002, 0.004, 0.006, 0.008, 0.010, 0.012, 0.014, 0.016, 0.018, 0.020, 0.022, 0.024};
        // const double radii[] = {0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.010, 0.011, 0.012, 0.013, 0.014, 0.015, 0.016};
        // const double radii[] = {0.005};
        // const double radii[] = {0.0049};
        // const double radii[] = {0.004};
        // const double radii[] = {0.45};
        // const double radii[] = {0.020};
        const size_t NUM_RADII = sizeof(radii)/sizeof(radii[0]);
        QProgressDialog progress("Generating offsets...", "Stop", 0, NUM_RADII/*, this*/);
        progress.setMinimumDuration(100);
        progress.setWindowModality(Qt::WindowModal);
        QElapsedTimer timerTotal;
        QElapsedTimer timer;
        progress.setValue(0);
        timerTotal.start();
        for (size_t i = 0; i < NUM_RADII; i++)
        {
            if (progress.wasCanceled())
                break;
            const double radius = radii[i];
            // const double radius = -radii[i];
            qDebug() << "Generating offset polygon" << (i+1) << "of" << NUM_RADII << "(" << radius << ")...";
            timer.start();
            const LineArcGeometry::MultiShape offset_shapes = ops.offset(joined, radius);
            qDebug() << ("...took " + QString::number(timer.nsecsElapsed()/1.0e9, 'f', 3) + " seconds");
            // AddMultiShapeToScene(this, offset_shapes, QColor(0, 80+5*i, 40));
            AddMultiShapeToScene(this, offset_shapes, Qt::green);
            // AddMultiShapeToScene(this, offset_shapes, Qt::green, ColorWithAlpha(Qt::green, 64));
            SVG_Save("testcases/output.svg", offset_shapes);
            progress.setValue(i+1);
        }
        qDebug() << ("Took " + QString::number(timerTotal.nsecsElapsed()/1.0e9, 'f', 3) + " seconds to generate all the offsets.");
    }
}

} // namespace LineArcOffsetDemo
