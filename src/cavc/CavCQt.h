#ifndef LINEARCOFFSETDEMO_CAVCQT_H
#define LINEARCOFFSETDEMO_CAVCQT_H

#include "GeometryCavC.h"

#include <QDebug>

QT_BEGIN_NAMESPACE

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::PVertex &v);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Polyline &polyline);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Polylines &polylines);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::CavC_Shape &shape);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::CavC_MultiShape &multiShape);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::OffsetLoop &loop);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::OffsetLoops &loops);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::OffsetLoopSet &loopSet);

QT_END_NAMESPACE

#endif // LINEARCOFFSETDEMO_CAVCQT_H
