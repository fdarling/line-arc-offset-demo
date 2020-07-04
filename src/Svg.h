#ifndef LINEARCOFFSETDEMO_SVG_H
#define LINEARCOFFSETDEMO_SVG_H

#include "Geometry.h"

#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape SVG_Load(const QString &filePath);
bool SVG_Save(const QString &filePath, const LineArcGeometry::MultiShape &multiShape);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_SVG_H
