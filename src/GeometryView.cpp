#include "GeometryView.h"

#include <QWheelEvent>
#include <QtMath>

namespace LineArcOffsetDemo {

GeometryView::GeometryView(QWidget *parent) : QGraphicsView(parent)
{
    // const float factor = 1500.0*qPow(1.1, -19);
    const float factor = 1500.0*qPow(1.1, -5);
    // const float factor = 1500.0;
    scale(factor, -factor);

    setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
  //setBackgroundBrush(QBrush(Qt::black));
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void GeometryView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    emit pointHovered(mapToScene(event->pos()));
}

void GeometryView::wheelEvent(QWheelEvent *event)
{
    // TODO this needs to be tested with touchpads and high resolution mousewheels (detent-less)
    // QGraphicsView::wheelEvent(event);
    const QPoint numPixels  = event->pixelDelta();
    const QPoint numDegrees = event->angleDelta() / 8;

    if (!numPixels.isNull())
    {
        // scrollWithPixels(numPixels);
    } else if (!numDegrees.isNull())
    {
        const QPoint numSteps = numDegrees / 15;
        // scrollWithDegrees(numSteps);
        const float factor = qPow(1.1, numSteps.y());
        scale(factor, factor);
    }
}

} // namespace LineArcOffsetDemo
