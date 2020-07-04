#include <QtWidgets>
#include <QDockWidget>
#include <QListWidget>
#include <QLabel>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QDomDocument>
#include <QMessageBox>
#include <QDebug>

#include "MainWindow.h"
#include "GeometryView.h"
#include "GeometryScene.h"

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
