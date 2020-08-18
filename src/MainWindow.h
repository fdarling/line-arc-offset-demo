#ifndef LINEARCOFFSETDEMO_MAINWINDOW_H
#define LINEARCOFFSETDEMO_MAINWINDOW_H

#include <QMainWindow>
#include <QPen>

QT_BEGIN_NAMESPACE
class QTreeWidget;
class QTreeWidgetItem;
class QGraphicsItem;
class QSettings;
class QLabel;
QT_END_NAMESPACE

namespace LineArcGeometry {

class MultiShape;
class Shape;

} // namespace LineArcGeometry

namespace LineArcOffsetDemo {

class Actions;
class MenuBar;
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
    void slot_FileNew();
    void slot_TreeContextMenuRequested(const QPoint &pos);
    void slot_TreeCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void slot_TreeItemChanged(QTreeWidgetItem *item, int column);
    void slot_CoordinateHovered(const QPointF &pt);
protected:
    void closeEvent(QCloseEvent *event);
    QTreeWidgetItem * _AddMultiShape(const LineArcGeometry::MultiShape &multiShape, const QString &name = QString(), const QPen &pen = QPen(Qt::black, 0.0), const QBrush &brush = Qt::NoBrush);
    void _RunTests(GeometryOperations &ops);
    void _saveSettings();
    void _loadSettings();

    Actions *_actions;
    MenuBar *_menuBar;
    QTreeWidget *_tree;
    GeometryView *_geomView;
    GeometryScene *_geomScene;
    QLabel *_coordinateLabel;
    QSettings *_settings;
    QGraphicsItem *_highlightedContourItem;
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_MAINWINDOW_H
