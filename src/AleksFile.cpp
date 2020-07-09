#include "AleksFile.h"
#include "Geometry.h"
#include "GeometryQt.h"

#include <cmath>
#include <algorithm>

#include <QFile>
#include <QString>
#include <QTextStream>

using namespace LineArcGeometry;

namespace LineArcOffsetDemo {

static void ReadArc(Contour &contour, const Point &center, double radius, double angle1, double angle2, bool usingDegrees)
{
    bool full_circle = false;
    if (usingDegrees)
    {
        if (qFuzzyCompare(std::abs(angle2 - angle1), 360))
        {
            full_circle = true;
        }
        else
        {
            angle1 *= (M_PI / 180.0);
            angle2 *= (M_PI / 180.0);
        }
    }
    else
    {
        if (qFuzzyCompare(std::abs(angle2 - angle1), 2 * M_PI))
        {
            full_circle = true;
        }
    }

    if (qFuzzyCompare(angle1, angle2))
    {
        full_circle = true;
    }

    Point p0(center), p1(center), pm(center);

    if (!full_circle)
    {
        double mid_angle = 0.5 * (angle1 + angle2);
        if (angle1 > angle2)
        {
            mid_angle += M_PI;
        }

        p0.x += radius * cos(angle1);
        p0.y += radius * sin(angle1);

        p1.x += radius * cos(angle2);
        p1.y += radius * sin(angle2);

        pm.x += radius * cos(mid_angle);
        pm.y += radius * sin(mid_angle);

        const Line line(p0, p1);
        contour.segments.push_back(Segment(line, center, OrientationReversed(LinePointOrientation(line, pm))));
    }
    else
    {
        p0.x -= radius;
        p1.x += radius;
        pm.y += radius;

        contour.segments.push_back(Segment(Line(p0, p1), center, Segment::CounterClockwise));
        contour.segments.push_back(Segment(Line(p1, p0), center, Segment::CounterClockwise));
    }
}

static void FixArcs(Contour &contour)
{
    if (contour.segments.size() < 2)
        return;
    Segment *prev = &contour.segments.back();
    for (std::list<Segment>::iterator it = contour.segments.begin(); it != contour.segments.end(); prev = &*it, ++it)
    {
        const LineArcGeometry::CoordinateType len1 = Line(prev->line.p2, it->line.p1).length();
        const LineArcGeometry::CoordinateType len2 = Line(prev->line.p2, it->line.p2).length();
        if (len2 < len1 && len2 < 0.0001)
        {
            *it = it->reversed();
        }

        if (Line(prev->line.p2, it->line.p1).length() < 0.0001)
        {
            if (it->isArc)
            {
                it->line.p1 = prev->line.p2;
            }
            else
            {
                prev->line.p2 = it->line.p1;
            }
        }
    }
}

LineArcGeometry::MultiShape AleksFile_Load(const QString &filePath)
{
    LineArcGeometry::MultiShape multiShape;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return multiShape;

    while (!file.atEnd())
    {
        const QByteArray line = file.readLine().trimmed();
        const QStringList words = QString(line).split(' ', QString::SkipEmptyParts); // Qt::SkipEmptyParts in later versions of Qt?
        if (words.isEmpty())
            continue;
        // TODO validity checking
        if (words[0] == "border")
        {
            multiShape.shapes.push_back(Shape());
        }
        else if (words[0] == "hole")
        {
            if (multiShape.shapes.empty())
                continue;
            multiShape.shapes.back().holes.push_back(Contour());
        }
        else if (words[0] == "segment")
        {
            const double x1 = words[1].toDouble();
            const double y1 = words[2].toDouble();
            const double x2 = words[3].toDouble();
            const double y2 = words[4].toDouble();

            if (multiShape.shapes.empty())
                continue;
            Shape &shape = multiShape.shapes.back();
            Contour &contour = shape.holes.empty() ? shape.boundary : shape.holes.front();
            contour.segments.push_back(Segment(Line(Point(x1, y1), Point(x2, y2))));
        }
        else if (words[0] == "arc" || words[0] == "arc_degrees")
        {
            const double cx = words[1].toDouble();
            const double cy = words[2].toDouble();
            const double r  = words[3].toDouble();
            const double a1 = words[4].toDouble();
            const double a2 = words[5].toDouble();

            if (multiShape.shapes.empty())
                continue;
            Shape &shape = multiShape.shapes.back();
            Contour &contour = shape.holes.empty() ? shape.boundary : shape.holes.front();
            ReadArc(contour, Point(cx, cy), r, a1, a2, words[0] == "arc_degrees");
        }
    }

    // make sure the contour is continuous
    for (std::list<Shape>::iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        Shape &shape = *shape_it;
        FixArcs(shape_it->boundary);
        for (std::list<Contour>::iterator hole_it = shape.holes.begin(); hole_it != shape.holes.end(); ++hole_it)
        {
            FixArcs(*hole_it);
        }
    }

    return multiShape;
}

static void calculate_arc_angles(const Point &center, const Point &f, const Point &m, const Point &l, double &angle_start, double &angle_end)
{
    angle_start = atan2(f.y - center.y, f.x - center.x);
    double angle_mid = atan2(m.y - center.y, m.x - center.x);
    angle_end   = atan2(l.y - center.y, l.x - center.x);
    double v = std::abs(angle_start - angle_end);
    static const double t = 0.00001;
    if ((v > t) && (v < 2 * M_PI - t))
    {
        if (angle_start < 0)
        {
            angle_start += 2 * M_PI;
        }
        if (angle_end < 0)
        {
            angle_end += 2 * M_PI;
        }
        if (angle_start > angle_end)
        {
            std::swap(angle_start, angle_end);
        }
        if (angle_mid < 0)
        {
            angle_mid += 2 * M_PI;
        }

        if (angle_mid < angle_start || angle_mid > angle_end)
        {
            std::swap(angle_start, angle_end);
        }
    }
    else
    {
        angle_start = 0;
        angle_end = 2 * M_PI;
    }
}

static void Write_Contour(QTextStream &stream, const Contour &contour)
{
    for (std::list<Segment>::const_iterator segment_it = contour.segments.begin(); segment_it != contour.segments.end(); ++segment_it)
    {
        const Segment &segment = *segment_it;
        if(segment.isArc)
        {
            double angle1 = 0.0, angle2 = 0.0;
            calculate_arc_angles(segment.center, segment.line.p1, segment.midPoint(), segment.line.p2, angle1, angle2);
            stream << "arc " << segment.center.x << " " << segment.center.y << " " << segment.radius() << " " << angle1 << " " << angle2 << "\n";
        }
        else
        {
            stream << "segment " << segment.line.p1.x << " " << segment.line.p1.y << " " << segment.line.p2.x << " " << segment.line.p2.y << "\n";
        }
    }
}

bool AleksFile_Save(const QString &filePath, const LineArcGeometry::MultiShape &multiShape)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QTextStream stream(&file);

    for (std::list<Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        const Shape &shape = *shape_it;

        // export boundary
        stream << "border\n";
        Write_Contour(stream, shape.boundary);

        // export holes
        for (std::list<Contour>::const_iterator contour_it = shape.holes.begin(); contour_it != shape.holes.end(); ++contour_it)
        {
            stream << "hole\n";
            Write_Contour(stream, *contour_it);
        }
    }

    return true;
}

} // namespace LineArcOffsetDemo
