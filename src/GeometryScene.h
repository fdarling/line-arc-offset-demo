#ifndef LINEARCOFFSETDEMO_GEOMETRYSCENE_H
#define LINEARCOFFSETDEMO_GEOMETRYSCENE_H

#include <QGraphicsScene>

namespace LineArcGeometry {

class MultiShape;
class Shape;
class Contour;
class Segment;

} // namespace LineArcGeometry

namespace LineArcOffsetDemo {

class GeometryScene : public QGraphicsScene
{
    Q_OBJECT
public:
    GeometryScene(QObject *parent = nullptr);
    QGraphicsItem * addSegment(const LineArcGeometry::Segment &segment, const QPen &pen = QPen(Qt::black, 0.0));
    QGraphicsItem * addContour(const LineArcGeometry::Contour &contour, const QPen &pen = QPen(Qt::black, 0.0));
    QGraphicsItem * addShape(const LineArcGeometry::Shape &shape, const QPen &pen = QPen(Qt::black, 0.0), const QBrush &brush = Qt::NoBrush);
    QGraphicsItem * addMultiShape(const LineArcGeometry::MultiShape &multiShape, const QPen &pen = QPen(Qt::black, 0.0), const QBrush &brush = Qt::NoBrush);
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYSCENE_H
