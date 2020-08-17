#include "MainWindow.h"
#include "Actions.h"
#include "MenuBar.h"
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
#include <QTreeWidget>
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

static const int TREE_COLUMN_CHECKBOX = 0;
static const int TREE_COLUMN_NAME = 1;
static const int QTREEWIDGETITEM_DATA_ROLE_QGRAPHICSITEM_POINTER = Qt::UserRole + 1;

static QColor ColorWithAlpha(const QColor &color, int alpha) __attribute__((unused));
static QColor ColorWithAlpha(const QColor &color, int alpha)
{
    QColor result = color;
    result.setAlpha(alpha);
    return result;
}

static LineArcGeometry::MultiShape ExplodeContours(const LineArcGeometry::MultiShape &multiShape)
{
    LineArcGeometry::MultiShape result;
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        const LineArcGeometry::Shape &shape = *shape_it;
        // add boundary
        result.shapes.push_back(LineArcGeometry::Shape(shape.boundary));
        // add holes
        for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
        {
            result.shapes.push_back(LineArcGeometry::Shape(*hole_it));
        }
    }
    return result;
}

static LineArcGeometry::MultiShape OuterBoundaries(const LineArcGeometry::MultiShape &multiShape, LineArcOffsetDemo::GeometryOperations &ops)
{
    const LineArcGeometry::MultiShape exploded = ExplodeContours(multiShape);
    const LineArcGeometry::MultiShape joined = ops.join(exploded);
    return joined;
}

static LineArcGeometry::MultiShape PeelMultiShape(const LineArcGeometry::MultiShape &multiShape, LineArcOffsetDemo::GeometryOperations &ops)
{
    const LineArcGeometry::MultiShape outerBoundaries = OuterBoundaries(multiShape, ops);
    const LineArcGeometry::MultiShape result = ops.symmetricDifference(multiShape, outerBoundaries);
    return result;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _actions(new Actions(this)), _menuBar(new MenuBar(_actions)), _tree(new QTreeWidget), _geomView(new GeometryView), _geomScene(new GeometryScene(this)), _coordinateLabel(new QLabel), _settings(nullptr)
{
    setMenuBar(_menuBar);
    statusBar()->addPermanentWidget(_coordinateLabel);
    setCentralWidget(_geomView);
    _geomView->setScene(_geomScene);
    _tree->setColumnCount(2);
    _tree->setHeaderLabels({"Geometry", "Name"});
    _tree->setContextMenuPolicy(Qt::CustomContextMenu);

    {
        QDockWidget * const dock = new QDockWidget("Geometry Tree");
        dock->setObjectName("tree_dock");
        dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
        addDockWidget(Qt::LeftDockWidgetArea, dock);
        dock->setWidget(_tree);
    }

    _settings = new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName(), this);
    _loadSettings();

    connect(_actions->fileNew, &QAction::triggered, this, &MainWindow::slot_FileNew);
    connect(_actions->fileQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(_tree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::slot_TreeContextMenuRequested);
    connect(_tree, &QTreeWidget::currentItemChanged, this, &MainWindow::slot_TreeCurrentItemChanged);
    connect(_tree, &QTreeWidget::itemChanged, this, &MainWindow::slot_TreeItemChanged);
    connect(_geomView, &GeometryView::pointHovered, this, &MainWindow::slot_CoordinateHovered);

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
        _tree->resizeColumnToContents(TREE_COLUMN_CHECKBOX);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::slot_FileNew()
{
    _tree->clear();
    _geomScene->clear();
}

static void RestoreGraphicsItemPen(QGraphicsPathItem *item)
{
    const QPen oldPen = item->data(0).value<QPen>();
    item->setPen(oldPen);
}

static void OverrideGraphicsItemPen(QGraphicsPathItem *item)
{
    const QPen oldPen = item->pen();
    item->setData(0, QVariant::fromValue(oldPen));
    QPen newPen(Qt::red, 0.0);
    item->setPen(newPen);
}

void MainWindow::slot_TreeContextMenuRequested(const QPoint &pos)
{
    // TODO deselect current item when clicking in blank area of tree widget
    Q_UNUSED(pos);

    QMenu menu;
    menu.addAction("Clear selection");
    QAction * const picked = menu.exec(_tree->mapToGlobal(pos));
    if (picked)
    {
        _tree->setCurrentItem(nullptr);
    }
}

void MainWindow::slot_TreeCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (previous)
    {
        for (QTreeWidgetItemIterator it(previous); *it && (*it == previous || (*it)->parent() != previous->parent()); ++it)
        {
            QGraphicsItem * const rawSceneItem = (*it)->data(0, QTREEWIDGETITEM_DATA_ROLE_QGRAPHICSITEM_POINTER).value<QGraphicsItem*>();
            QGraphicsPathItem * const sceneItem = dynamic_cast<QGraphicsPathItem*>(rawSceneItem);
            if (sceneItem)
            {
                RestoreGraphicsItemPen(sceneItem);
            }
        }
    }
    if (current)
    {
        for (QTreeWidgetItemIterator it(current); *it && (*it == current || (*it)->parent() != current->parent()); ++it)
        {
            QGraphicsItem * const rawSceneItem = (*it)->data(0, QTREEWIDGETITEM_DATA_ROLE_QGRAPHICSITEM_POINTER).value<QGraphicsItem*>();
            QGraphicsPathItem * const sceneItem = dynamic_cast<QGraphicsPathItem*>(rawSceneItem);
            if (sceneItem)
            {
                OverrideGraphicsItemPen(sceneItem);
            }
        }
    }
}

void MainWindow::slot_TreeItemChanged(QTreeWidgetItem *item, int column)
{
    if (column == TREE_COLUMN_CHECKBOX)
    {
        QGraphicsItem * const sceneItem = item->data(0, QTREEWIDGETITEM_DATA_ROLE_QGRAPHICSITEM_POINTER).value<QGraphicsItem*>();
        if (sceneItem)
        {
            sceneItem->setVisible(item->checkState(0) != Qt::Unchecked);
        }
    }
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

static QTreeWidgetItem * CreateContourTreeItem(const LineArcGeometry::Contour &contour, const QString &txt)
{
    QTreeWidgetItem * const contourItem = new QTreeWidgetItem;
    contourItem->setText(TREE_COLUMN_CHECKBOX, txt);
    for (std::list<LineArcGeometry::Segment>::const_iterator seg_it = contour.segments.begin(); seg_it != contour.segments.end(); ++seg_it)
    {
        const LineArcGeometry::Segment &segment = *seg_it;
        // create tree Segment row
        QTreeWidgetItem * const segmentItem = new QTreeWidgetItem;
        segmentItem->setText(TREE_COLUMN_CHECKBOX, segment.isArc ? "Arc" : "Line");
        contourItem->addChild(segmentItem);
    }
    return contourItem;
}

QTreeWidgetItem * MainWindow::_AddMultiShape(const LineArcGeometry::MultiShape &multiShape, const QString &name, const QPen &pen, const QBrush &brush)
{
    // create graphics MultiShape item
    QGraphicsItemGroup * const sceneItemGroup = new QGraphicsItemGroup;
    _geomScene->addItem(sceneItemGroup);

    // create tree MultiShape row
    QTreeWidgetItem * const multiShapeItem = new QTreeWidgetItem;
    multiShapeItem->setText(TREE_COLUMN_CHECKBOX, "MultiShape (" + QString::number(multiShape.shapes.size()) + " shapes)");
    multiShapeItem->setText(TREE_COLUMN_NAME, name);
    multiShapeItem->setData(TREE_COLUMN_CHECKBOX, QTREEWIDGETITEM_DATA_ROLE_QGRAPHICSITEM_POINTER, QVariant::fromValue(static_cast<QGraphicsItem*>(sceneItemGroup)));
    multiShapeItem->setFlags(multiShapeItem->flags() | Qt::ItemIsAutoTristate);
    {
        QTreeWidgetItem * topItem = _tree->topLevelItem(0);
        if (!topItem)
        {
            topItem = new QTreeWidgetItem;
            topItem->setText(TREE_COLUMN_CHECKBOX, "testcase");
            _tree->addTopLevelItem(topItem);
        }
        topItem->addChild(multiShapeItem);
        topItem->setExpanded(true);
    }

    // create tree/graphics Shape rows/items
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        const LineArcGeometry::Shape shape = *shape_it;
        // create grahics Shape
        QGraphicsItem * const sceneItem = _geomScene->addShape(shape, pen, brush);
        sceneItemGroup->addToGroup(sceneItem);

        // create tree Shape row
        QTreeWidgetItem * const shapeItem = new QTreeWidgetItem;
        shapeItem->setText(TREE_COLUMN_CHECKBOX, "Shape (" + QString::number(shape.holes.size()) + " holes)");
        shapeItem->setData(TREE_COLUMN_CHECKBOX, QTREEWIDGETITEM_DATA_ROLE_QGRAPHICSITEM_POINTER, QVariant::fromValue(sceneItem));
        shapeItem->setCheckState(TREE_COLUMN_CHECKBOX, Qt::Checked);
        multiShapeItem->addChild(shapeItem);

        // create tree Contour rows
        shapeItem->addChild(CreateContourTreeItem(shape.boundary, "boundary (" + QString::number(shape.boundary.segments.size()) + " segments)"));
        for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
        {
            shapeItem->addChild(CreateContourTreeItem(*hole_it, "hole (" + QString::number(hole_it->segments.size()) + " segments)"));
        }
    }

    return multiShapeItem;
}

void MainWindow::_RunTests(GeometryOperations &ops)
{
    const LineArcGeometry::MultiShape overlappingShapes = SVG_Load("testcases/traces_01.svg");
    // const LineArcGeometry::MultiShape overlappingShapes = AleksFile_Load("testcases/input_1_shape.txt");
    // const LineArcGeometry::MultiShape overlappingShapes = AleksFile_Load("testcases/input_2_shapes.txt");
    // AleksFile_Save("testcases/output.txt", overlappingShapes);
    enum TestType
    {
        TEST_NONE,
        TEST_RAW,
        TEST_IDENTITY,
        TEST_UNARY_UNION,
        TEST_UNION,
        TEST_INTERSECTION,
        TEST_DIFFERENCE,
        TEST_UNARY_XOR,
        TEST_XOR,
        TEST_OFFSET,
        TEST_ONLY_BOUNDARIES,
        TEST_ONLY_HOLES
    } testType;
    // testType = TEST_NONE;
    // testType = TEST_RAW;
    // testType = TEST_IDENTITY;
    // testType = TEST_UNARY_UNION;
    // testType = TEST_UNION;
    // testType = TEST_INTERSECTION;
    // testType = TEST_DIFFERENCE;
    // testType = TEST_UNARY_XOR;
    // testType = TEST_XOR;
    testType = TEST_OFFSET;
    // testType = TEST_ONLY_BOUNDARIES;
    // testType = TEST_ONLY_HOLES;
    if (testType == TEST_RAW)
    {
        // display the raw geometry imported from the SVG (before any operations are run on it)
        _AddMultiShape(overlappingShapes, "overlapping");
        SVG_Save("testcases/output.svg", overlappingShapes);
        return;
    }
    if (testType == TEST_IDENTITY)
    {
        // test if geometry survives converting back and forth from the engine's internal format
        const LineArcGeometry::MultiShape reconverted = ops.identity(overlappingShapes);
        _AddMultiShape(reconverted, "identity");
        SVG_Save("testcases/output.svg", reconverted);
        return;
    }
    // combine the overlapping contents of the test case file using union
    const LineArcGeometry::MultiShape joined = ops.join(overlappingShapes);
    if (testType == TEST_UNARY_UNION)
    {
        _AddMultiShape(joined, "unary union");
        SVG_Save("testcases/output.svg", joined);
    }
    else if (testType == TEST_UNION)
    {
        // further test union
        const LineArcGeometry::MultiShape addend = SVG_Load("testcases/traces_02.svg");
        // _AddMultiShape(addend);
        const LineArcGeometry::MultiShape sum = ops.join(joined, addend);
        _AddMultiShape(sum, "union");
        SVG_Save("testcases/output.svg", sum);
    }
    else if (testType == TEST_INTERSECTION)
    {
        // test intersection
        const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/traces_02.svg");
        // _AddMultiShape(cutter);
        const LineArcGeometry::MultiShape intersection = ops.intersection(joined, cutter);
        _AddMultiShape(intersection, "intersection");
        SVG_Save("testcases/output.svg", intersection);
    }
    else if (testType == TEST_DIFFERENCE)
    {
        // test difference
        // const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/thermal.svg");
        const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/traces_02.svg");
        // _AddMultiShape(cutter);
        const LineArcGeometry::MultiShape diffed = ops.difference(joined, cutter);
        _AddMultiShape(diffed, "difference");
        SVG_Save("testcases/output.svg", diffed);
    }
    else if (testType == TEST_UNARY_XOR)
    {
        // test unary xor (symmetric difference)
        _AddMultiShape(overlappingShapes, "overlapping");
        const LineArcGeometry::MultiShape xorResult = ops.symmetricDifference(overlappingShapes);
        _AddMultiShape(xorResult, "xor", QPen(Qt::blue, 0.0), ColorWithAlpha(Qt::blue, 64));
        // SVG_Save("testcases/output.svg", xorResult);
    }
    else if (testType == TEST_XOR)
    {
        // test boolean xor (symmetric difference)
        _AddMultiShape(joined, "joined");
        const LineArcGeometry::MultiShape cutter = SVG_Load("testcases/traces_02.svg");
        _AddMultiShape(cutter, "cutter");
        const LineArcGeometry::MultiShape xorResult = ops.symmetricDifference(joined, cutter);
        _AddMultiShape(xorResult, "xor", QPen(Qt::blue, 0.0), ColorWithAlpha(Qt::blue, 64));
        // SVG_Save("testcases/output.svg", xorResult);
    }
    else if (testType == TEST_OFFSET)
    {
        // show the original geometry
        _AddMultiShape(joined, "unary union");

        // const double radii[] = {0.010, 0.020, 0.030, 0.040};
        // const double radii[] = {0.010, 0.020, 0.030, 0.040, 0.050, 0.060, 0.070, 0.080, 0.090, 0.100, 0.110, 0.120, 0.130, 0.140, 0.150, 0.160, 0.170};
        // const double radii[] = {0.011};
        const double radii[] = {0.002, 0.004, 0.006, 0.008, 0.010, 0.012, 0.014, 0.016, 0.018, 0.020, 0.022, 0.024};
        // const double radii[] = {0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.010, 0.011, 0.012, 0.013, 0.014, 0.015, 0.016};
        // const double radii[] = {0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008};
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
            // const double radius = radii[i];
            const double radius = -radii[i];
            qDebug() << "Generating offset polygon" << (i+1) << "of" << NUM_RADII << "(" << radius << ")...";
            timer.start();
            const LineArcGeometry::MultiShape offset_shapes = ops.offset(joined, radius);
            qDebug() << ("...took " + QString::number(timer.nsecsElapsed()/1.0e9, 'f', 3) + " seconds");
            const QString name = QString("offset ") + (radius > 0.0 ? "+" : "") + QString::number(radius);
            const QBrush brush = Qt::NoBrush;
            // const QBrush brush = ColorWithAlpha(Qt::green, 64);
            _AddMultiShape(offset_shapes, name, QPen(Qt::green, 0.0), brush);
            SVG_Save("testcases/output.svg", offset_shapes);
            progress.setValue(i+1);
        }
        qDebug() << ("Took " + QString::number(timerTotal.nsecsElapsed()/1.0e9, 'f', 3) + " seconds to generate all the offsets.");
    }
    else if (testType == TEST_ONLY_BOUNDARIES)
    {
        _AddMultiShape(joined, "unary union");
        const LineArcGeometry::MultiShape outerBoundaries = OuterBoundaries(joined, ops);
        _AddMultiShape(outerBoundaries, "outer boundaries", QPen(Qt::blue, 0.0), ColorWithAlpha(Qt::blue, 64));
    }
    else if (testType == TEST_ONLY_HOLES)
    {
        _AddMultiShape(joined, "unary union");
        const LineArcGeometry::MultiShape peeled = PeelMultiShape(joined, ops);
        _AddMultiShape(peeled, "peeled", QPen(Qt::blue, 0.0), ColorWithAlpha(Qt::blue, 64));
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
