#ifndef LINEARCOFFSETDEMO_ALEKSFILE_H
#define LINEARCOFFSETDEMO_ALEKSFILE_H

#include "Geometry.h"

#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape AleksFile_Load(const QString &filePath);
bool AleksFile_Save(const QString &filePath, const LineArcGeometry::MultiShape &multiShape);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_ALEKSFILE_H
