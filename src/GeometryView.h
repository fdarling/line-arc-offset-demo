#ifndef LINEARCOFFSETDEMO_GEOMETRYVIEW_H
#define LINEARCOFFSETDEMO_GEOMETRYVIEW_H

#include <QGraphicsView>

namespace LineArcOffsetDemo {

class GeometryView : public QGraphicsView
{
    Q_OBJECT
public:
    GeometryView(QWidget *parent = nullptr);
signals:
    void pointHovered(const QPointF &);
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    // bool eventFilter(QObject *object, QEvent *event);
    QPointF _lastPos;
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYVIEW_H
