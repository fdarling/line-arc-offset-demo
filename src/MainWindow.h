#ifndef LINEARCOFFSETDEMO_MAINWINDOW_H
#define LINEARCOFFSETDEMO_MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QSettings;
class QLabel;
QT_END_NAMESPACE

namespace LineArcOffsetDemo {

class GeometryView;
class GeometryScene;
class GeometryOperations;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected slots:
    void slot_CoordinateHovered(const QPointF &pt);
protected:
    void closeEvent(QCloseEvent *event);
    void _RunTests(GeometryOperations &ops);
    void _saveSettings();
    void _loadSettings();

    GeometryView *_geomView;
    GeometryScene *_geomScene;
    QLabel *_coordinateLabel;
    QSettings *_settings;
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_MAINWINDOW_H
