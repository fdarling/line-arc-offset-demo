#include "MainWindow.h"
#include "GeometryView.h"
#include "GeometryScene.h"
#include "GeometryOperations.h"
#include "AleksFile.h"
#include "Svg.h"

#ifdef USING_CGAL
#include "cgal/GeometryOperationsCGAL.h"
// #include "cgal/GeometryCGAL.h" // WARNING: pulling in CGAL headers will slow down compilation significantly
// #include "cgal/CGALWrapper.h" // WARNING: pulling in CGAL headers will slow down compilation significantly
#endif // USING_CGAL
#ifdef USING_OCCT
#include "occt/GeometryOperationsOCCT.h"
#include "occt/GeometryOCCT.h"
#endif // USING_OCCT
#ifdef USING_CLIPPER
#include "clipper/GeometryOperationsClipper.h"
#endif // USING_CLIPPER
#ifdef USING_BOOST
#include "boost/GeometryOperationsBoost.h"
#endif // USING_BOOST
#ifdef USING_GEOS
#include "geos/GeometryOperationsGEOS.h"
#endif // USING_GEOS

#include <QtWidgets>
#include <QDockWidget>
#include <QListWidget>
#include <QLabel>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QDomDocument>
#include <QMessageBox>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QProgressDialog>
#include <QElapsedTimer>
#include <QDebug>

namespace LineArcOffsetDemo {

static QColor ColorWithAlpha(const QColor &color, int alpha) __attribute__((unused));
static QColor ColorWithAlpha(const QColor &color, int alpha)
{
    QColor result = color;
    result.setAlpha(alpha);
    return result;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _geomView(new GeometryView), _geomScene(new GeometryScene(this)), _coordinateLabel(new QLabel), _settings(nullptr)
{
    statusBar()->addPermanentWidget(_coordinateLabel);
    setCentralWidget(_geomView);
    _geomView->setScene(_geomScene);

    _settings = new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName(), this);
    _loadSettings();

    connect(_geomView, SIGNAL(pointHovered(const QPointF &)), this, SLOT(slot_CoordinateHovered(const QPointF &)));

    typedef QPair<QString, std::function<GeometryOperations*()> > EngineNameConstructorPair;
    QList<EngineNameConstructorPair> engineConstructors;
#ifdef USING_CGAL
    engineConstructors.append(EngineNameConstructorPair("cgal", []() {return new GeometryOperationsCGAL();}));
#endif // USING_CGAL
#ifdef USING_OCCT
    engineConstructors.append(EngineNameConstructorPair("occt", []() {return new GeometryOperationsOCCT();}));
#endif // USING_OCCT
#ifdef USING_CLIPPER
    engineConstructors.append(EngineNameConstructorPair("clipper", []() {return new GeometryOperationsClipper();}));
#endif // USING_CLIPPER
#ifdef USING_BOOST
    engineConstructors.append(EngineNameConstructorPair("boost", []() {return new GeometryOperationsBoost();}));
#endif // USING_BOOST
#ifdef USING_GEOS
    engineConstructors.append(EngineNameConstructorPair("geos", []() {return new GeometryOperationsGEOS();}));
#endif // USING_GEOS

    QCommandLineParser parser;
    parser.setApplicationDescription("demo program for offsetting shapes and applying boolean operations");
    parser.addHelpOption();

    // An option with a value
    const QString defaultEngineString = engineConstructors.empty() ? QString() : engineConstructors.first().first;
    QCommandLineOption engineOption(QStringList() << "e" << "engine", "use geometry engine <engine>, default is " + defaultEngineString, "engine", defaultEngineString);
    parser.addOption(engineOption);

    // Process the actual command line arguments given by the user
    parser.process(*qApp);
    const QString engine = parser.value(engineOption);
    qDebug() << "using the" << engine << "engine";

    QScopedPointer<GeometryOperations> ops;
    for (int i = 0; i < engineConstructors.size(); i++)
    {
        if (engineConstructors.at(i).first == engine)
        {
            ops.reset(engineConstructors.at(i).second());
            break;
        }
    }
    if (!ops)
    {
        qDebug() << "WARNING: engine" << engine << "unsupported!";
    }
    else
    {
        _RunTests(*ops);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::slot_CoordinateHovered(const QPointF &pt)
{
    _coordinateLabel->setText(QString("X: ") + QString::number(pt.x(), 'f', 3) + ", Y: "+ QString::number(pt.y(), 'f', 3));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    _saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::_RunTests(GeometryOperations &ops)
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
        _geomScene->addMultiShape(overlappingShapes);
        SVG_Save("testcases/output.svg", overlappingShapes);
        return;
    }
    if (testType == TEST_IDENTITY)
    {
        // test if geometry survives converting back and forth from the engine's internal format
        const LineArcGeometry::MultiShape reconverted = ops.identity(overlappingShapes);
        _geomScene->addMultiShape(reconverted);
        SVG_Save("testcases/output.svg", reconverted);
        return;
    }
    // combine the overlapping contents of the test case file using union
    const LineArcGeometry::MultiShape joined = ops.join(overlappingShapes);
    if (testType == TEST_UNARY_UNION)
    {
        _geomScene->addMultiShape(joined);
        SVG_Save("testcases/output.svg", joined);
    }
    else if (testType == TEST_UNION)
    {
        // further test union
        const LineArcGeometry::MultiShape addend = SVG_Load("testcases/traces_02.svg");
        // _geomScene->addMultiShape(addend);
        const LineArcGeometry::MultiShape sum = ops.join(joined, addend);
        _geomScene->addMultiShape(sum);
        SVG_Save("testcases/output.svg", sum);
    }
    else if (testType == TEST_INTERSECTION)
    {
        // test intersection
        const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/traces_02.svg");
        // _geomScene->addMultiShape(cutter);
        const LineArcGeometry::MultiShape intersection = ops.intersection(joined, cutter);
        _geomScene->addMultiShape(intersection);
        SVG_Save("testcases/output.svg", intersection);
    }
    else if (testType == TEST_DIFFERENCE)
    {
        // test difference
        // const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/thermal.svg");
        const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/traces_02.svg");
        // _geomScene->addMultiShape(cutter);
        const LineArcGeometry::MultiShape diffed = ops.difference(joined, cutter);
        _geomScene->addMultiShape(diffed);
        SVG_Save("testcases/output.svg", diffed);
    }
    else if (testType == TEST_OFFSET)
    {
        // show the original geometry
        _geomScene->addMultiShape(joined);

        // const double radii[] = {0.010, 0.020, 0.030, 0.040};
        // const double radii[] = {0.010, 0.020, 0.030, 0.040, 0.050, 0.060, 0.070, 0.080, 0.090, 0.100, 0.110, 0.120, 0.130, 0.140, 0.150, 0.160, 0.170};
        // const double radii[] = {0.011};
        // const double radii[] = {0.002, 0.004, 0.006, 0.008, 0.010, 0.012, 0.014, 0.016, 0.018, 0.020, 0.022, 0.024};
        // const double radii[] = {0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.010, 0.011, 0.012, 0.013, 0.014, 0.015, 0.016};
        // const double radii[] = {0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008};
        // const double radii[] = {0.005};
        // const double radii[] = {0.0049};
        const double radii[] = {0.004};
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
            // const double radius = radii[i];
            const double radius = -radii[i];
            qDebug() << "Generating offset polygon" << (i+1) << "of" << NUM_RADII << "(" << radius << ")...";
            timer.start();
            const LineArcGeometry::MultiShape offset_shapes = ops.offset(joined, radius);
            qDebug() << ("...took " + QString::number(timer.nsecsElapsed()/1.0e9, 'f', 3) + " seconds");
            // _geomScene->addMultiShape(offset_shapes, QPen(QColor(0, 80+5*i, 40), 0.0));
            // _geomScene->addMultiShape(offset_shapes, QPen(Qt::green, 0.0));
            _geomScene->addMultiShape(offset_shapes, QPen(Qt::green, 0.0), ColorWithAlpha(Qt::green, 64));
            SVG_Save("testcases/output.svg", offset_shapes);
            progress.setValue(i+1);
        }
        qDebug() << ("Took " + QString::number(timerTotal.nsecsElapsed()/1.0e9, 'f', 3) + " seconds to generate all the offsets.");
    }
}

void MainWindow::_saveSettings()
{
    _settings->beginGroup("MainWindow");
    _settings->setValue("geometry", saveGeometry());
    _settings->setValue("windowState", saveState());
    _settings->endGroup();
}

void MainWindow::_loadSettings()
{
    _settings->beginGroup("MainWindow");
    restoreGeometry(_settings->value("geometry").toByteArray());
    restoreState(_settings->value("windowState").toByteArray());
    _settings->endGroup();
}

} // namespace LineArcOffsetDemo
