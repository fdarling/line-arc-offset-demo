#ifndef LINEARCOFFSETDEMO_GEOMETRYSCENE_H
#define LINEARCOFFSETDEMO_GEOMETRYSCENE_H

#include <QGraphicsScene>

namespace LineArcOffsetDemo {

class GeometryScene : public QGraphicsScene
{
    Q_OBJECT
public:
    GeometryScene(QObject *parent = nullptr);
};

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_GEOMETRYSCENE_H