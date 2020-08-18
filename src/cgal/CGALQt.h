#ifndef LINEARCOFFSETDEMO_CGALQT_H
#define LINEARCOFFSETDEMO_CGALQT_H

#include "CGALWrapper.h"
#include "GeometryCGAL.h"
#include "../GeometryQt.h"

#include <QPointF>
#include <QDebug>

QT_BEGIN_NAMESPACE

QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Point_2 &pt);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::X_monotone_curve_2 &curve);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Curve_2 &curve);
QDebug operator<<(QDebug debug, const LineArcOffsetDemo::Polygon_2 &polygon);

QT_END_NAMESPACE

namespace LineArcOffsetDemo {

Point_2 QPointF_To_Point_2(const QPointF &pt);
QPointF Point_2_To_QPointF(const Point_2 &pt);
QPointF Point_2_To_QPointF(const Traits_2::Point_2 &pt);

} // namespace LineArcOffsetDemo

#endif // LINEARCOFFSETDEMO_CGALQT_H
