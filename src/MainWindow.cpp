#include "MainWindow.h"
#include "GeometryView.h"
#include "GeometryScene.h"

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

namespace LineArcOffsetDemo {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _geomView(new GeometryView), _geomScene(new GeometryScene), _coordinateLabel(new QLabel), _settings(nullptr)
{
    _coordinateLabel = new QLabel;
    statusBar()->addPermanentWidget(_coordinateLabel);
    setCentralWidget(_geomView);
    _geomView->setScene(_geomScene);

    _settings = new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
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
    if (ops)
        _geomScene->runTests(*ops);
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
