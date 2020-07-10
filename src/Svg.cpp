#include "Svg.h"
#include "GeometryQt.h"

#include <QtMath>
#include <QFile>
#include <QDomElement>
#include <QXmlStreamWriter>
#include <QLineF>
#include <QVector2D>
#include <QTextStream>
#include <QDebug>

using namespace LineArcGeometry;

namespace LineArcOffsetDemo {

static QString ContourToPathString(const LineArcGeometry::Contour &contour);
static LineArcGeometry::Shape LineElementToShape(QDomElement &elt);
static LineArcGeometry::Shape CircleElementToShape(QDomElement &elt);
static LineArcGeometry::Shape RectElementToShape(QDomElement &elt);
static LineArcGeometry::Shape PathElementToShape(QDomElement &elt);

// NOTE: this function is simplistic, it doesn't handle transforms, nor any sort of heirarchy!
LineArcGeometry::MultiShape SVG_Load(const QString &filePath)
{
    LineArcGeometry::MultiShape multiShape;

    QDomDocument doc;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return multiShape;
    if (!doc.setContent(&file)) {
        file.close();
        return multiShape;
    }
    file.close();

    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();

    for (QDomNode n = docElem.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        // make sure the node is an element
        QDomElement elt = n.toElement();
        if (elt.isNull())
            continue;

        const QString tag = elt.tagName().toLower();
        if (tag == "line")
        {
            multiShape.shapes.push_back(LineElementToShape(elt));
        }
        else if (tag == "circle")
        {
            multiShape.shapes.push_back(CircleElementToShape(elt));
        }
        else if (tag == "rect")
        {
            multiShape.shapes.push_back(RectElementToShape(elt));
        }
        else if (tag == "path")
        {
            // TODO perhaps have PathElementToShape return a MultiShape
            // for the general case and splice() the results. The
            // problem is differentiating outer and inner contours
            multiShape.shapes.push_back(PathElementToShape(elt));
        }
        else if (tag == "style")
        {
        }
        else
        {
            qDebug() << ("WARNING: importing an SVG <" + tag + "> is unimplemented!");
        }
    }

    return multiShape;
}

bool SVG_Save(const QString &filePath, const LineArcGeometry::MultiShape &multiShape)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeDTD("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n  \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">");
    stream.writeStartElement("svg");
    stream.writeAttribute("xmlns", "http://www.w3.org/2000/svg");
    stream.writeAttribute("width", "2in");
    stream.writeAttribute("height", "2in");
    stream.writeAttribute("viewBox", "-1 -1 2 2");
    stream.writeStartElement("style");
    stream.writeCharacters("path { fill-rule: evenodd; }");
    stream.writeEndElement();
    for (std::list<LineArcGeometry::Shape>::const_iterator shape_it = multiShape.shapes.begin(); shape_it != multiShape.shapes.end(); ++shape_it)
    {
        QString pathString = ContourToPathString(shape_it->boundary);
        for (std::list<LineArcGeometry::Contour>::const_iterator hole_it = shape_it->holes.begin(); hole_it != shape_it->holes.end(); ++hole_it)
        {
            pathString += " ";
            pathString += ContourToPathString(*hole_it);
        }
        stream.writeStartElement("path");
        stream.writeAttribute("d", pathString);
        stream.writeAttribute("fill", "black");
        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    return true;
}

static QString ContourToPathString(const LineArcGeometry::Contour &contour)
{
    QString result;
    QTextStream stream(&result);
    for (std::list<LineArcGeometry::Segment>::const_iterator it = contour.segments.begin(); it != contour.segments.end(); ++it)
    {
        // initial moveTo
        if (result.isEmpty())
        {
            result += "M" + QString::number(it->line.p1.x) + "," + QString::number(it->line.p1.y);
        }

        // lineTo/arcTo
        if (it->isArc)
        {
            const QPointF qpt1(PointToQPointF(it->line.p1));
            const QPointF qpt2(PointToQPointF(it->line.p2));
            const QPointF qptc(PointToQPointF(it->center));
            const double radius = QLineF(qptc, qpt2).length();
            // TODO test this fullly, the large-arc and sweep-flag parameters are tricky to get right!
            const bool sweepFlag = (it->orientation != LineArcGeometry::Segment::Clockwise);
            const QVector2D normal = QVector2D(QLineF(QPointF(), qpt2 - qpt1).normalVector().p2());
            const QVector2D centerVec = QVector2D(QLineF(QPointF(), qptc - qpt1).p2());
            const bool largeArc = (QVector2D::dotProduct(normal, centerVec) < 0.0) != sweepFlag;
            stream << " A" << radius << "," << radius << ",0," << largeArc << "," << sweepFlag << "," << it->line.p2.x << "," << it->line.p2.y;
        }
        else // it's a line
        {
            stream << " L" << it->line.p2.x << "," << it->line.p2.y;
        }
    }
    if (!result.isEmpty())
    {
        stream << " Z";
    }
    return result;
}

static Shape LineElementToShape(QDomElement &elt)
{
    const double x1 = elt.attribute("x1", "0").toDouble();
    const double y1 = elt.attribute("y1", "0").toDouble();
    const double x2 = elt.attribute("x2", "0").toDouble();
    const double y2 = elt.attribute("y2", "0").toDouble();
    const double thickness = elt.attribute("stroke-width", "0").toDouble();

    const Line line(Point(x1, y1), Point(x2, y2));

    const bool hasStroke = elt.hasAttribute("stroke-width") && thickness;
    if (hasStroke)
    {
        return ContourFromLineAndRadius(line, thickness/2.0);
    }
    return Contour(line);
}

static Contour ContourFromCircle(const Point &center, double radius)
{
    const Point left(center.x - radius, center.y);
    const Point right(center.x + radius, center.y);

    Contour contour;
    contour.segments.push_back(Segment(Line(left, right), center, Segment::CounterClockwise));
    contour.segments.push_back(Segment(Line(right, left), center, Segment::CounterClockwise));
    return contour;
}

static Shape CircleElementToShape(QDomElement &elt)
{
    const double x = elt.attribute("cx", "0").toDouble();
    const double y = elt.attribute("cy", "0").toDouble();
    const double r = elt.attribute("r", "0").toDouble();
    const double thickness = elt.attribute("stroke-width", "0").toDouble();

    const Point center(x, y);
    const Point left(x - r, y);
    const Point right(x + r, y);

    Shape shape;
    const bool hasStroke = elt.hasAttribute("stroke-width") && thickness;
    if (hasStroke)
    {
        shape.boundary = ContourFromCircle(center, r + thickness/2.0);
        if (r >= thickness/2.0)
        {
            shape.holes.push_back(ContourFromCircle(center, r - thickness/2.0));
        }
    }
    else
    {
        shape.boundary = ContourFromCircle(center, r);
    }
    return shape;
}

static Shape RectElementToShape(QDomElement &elt)
{
    const double x = elt.attribute("x", "0").toDouble();
    const double y = elt.attribute("y", "0").toDouble();
    const double w = elt.attribute("width", "0").toDouble();
    const double h = elt.attribute("height", "0").toDouble();
    // TODO support corner radius/radii
    const double thickness = elt.attribute("stroke-width", "0").toDouble();

    Shape shape;
    const bool hasStroke = elt.hasAttribute("stroke-width") && thickness;
    if (hasStroke)
    {
        const double r = thickness/2.0;
        // create boundary
        {
            const Point p1(x        , y     - r);
            const Point p2(x + w    , y     - r);
            const Point p3(x + w + r, y);
            const Point p4(x + w + r, y + h);
            const Point p5(x + w    , y + h + r);
            const Point p6(x        , y + h + r);
            const Point p7(x     - r, y + h);
            const Point p8(x     - r, y);
            const Point c1(x + w    , y);
            const Point c2(x + w    , y + h);
            const Point c3(x        , y + h);
            const Point c4(x        , y);
            // counter-clockwise
            shape.boundary.segments.push_back(Line(p1, p2));
            shape.boundary.segments.push_back(Segment(Line(p2, p3), c1, Segment::CounterClockwise));
            shape.boundary.segments.push_back(Line(p3, p4));
            shape.boundary.segments.push_back(Segment(Line(p4, p5), c2, Segment::CounterClockwise));
            shape.boundary.segments.push_back(Line(p5, p6));
            shape.boundary.segments.push_back(Segment(Line(p6, p7), c3, Segment::CounterClockwise));
            shape.boundary.segments.push_back(Line(p7, p8));
            shape.boundary.segments.push_back(Segment(Line(p8, p1), c4, Segment::CounterClockwise));
        }

        // create hole if there is room
        if (w >= thickness) // TODO is equal okay?
        {
            const Point p1(x     + r, y      + r);
            const Point p2(x + w - r, y      + r);
            const Point p3(x + w - r, y + h  - r);
            const Point p4(x     + r, y + h  - r);
            Contour hole;
            // counter-clockwise
            hole.segments.push_back(Line(p1, p2));
            hole.segments.push_back(Line(p2, p3));
            hole.segments.push_back(Line(p3, p4));
            hole.segments.push_back(Line(p4, p1));
            shape.holes.push_back(hole);
        }
    }
    else
    {
        // counter-clockwise
        shape.boundary.segments.push_back(Line(Point(x, y), Point(x + w, y)));
        shape.boundary.segments.push_back(Line(Point(x + w, y), Point(x + w, y + h)));
        shape.boundary.segments.push_back(Line(Point(x + w, y + h), Point(x, y + h)));
        shape.boundary.segments.push_back(Line(Point(x, y + h), Point(x, y)));
    }
    return shape;
}

// https://stackoverflow.com/questions/36211171/finding-center-of-a-circle-given-two-points-and-radius
static Point CircleCenterFromChordAndRadius(const Point &p1, const Point &p2, const double radius, bool side)
{
    const double radsq = radius * radius;
    const double q = qSqrt(((p2.x - p1.x) * (p2.x - p1.x)) + ((p2.y - p1.y) * (p2.y - p1.y)));
    const Point midP = (p1 + p2)/2.0;
    const double scalar = side ? 1.0 : -1.0;
    return Point(midP.x + scalar*qSqrt(radsq - ((q / 2) * (q / 2))) * ((p1.y - p2.y) / q),
                 midP.y + scalar*qSqrt(radsq - ((q / 2) * (q / 2))) * ((p2.x - p1.x) / q));
}

static Shape PathElementToShape(QDomElement &elt)
{
    Shape shape;
    QString pathData = elt.attribute("d").replace(',', ' ').simplified();
    QTextStream stream(&pathData);

    QPointF firstPos;
    QPointF oldPos;
    while (!stream.atEnd())
    {
        QChar command;
        stream >> command;
        const bool is_relative = command.isLower();
        if (command == 'M')
        {
            qreal x, y;
            stream >> x >> y;
            const QPointF newPos(x, y);
            if (is_relative)
                oldPos += newPos;
            else
                oldPos = newPos;
            oldPos = newPos;
            firstPos = newPos;
        }
        else if (command == 'A')
        {
            qreal crx = 0.0, cry = 0.0, rot = 0.0, x = 0.0, y = 0.0;
            int largeArc = 0, sweepFlag = 0;
            stream >> crx >> cry >> rot >> largeArc >> sweepFlag >> x >> y;
            // NOTE: we assume the radii for x/y are identical!
            if (crx != cry)
            {
                qDebug() << "WARNING: SVG path arcs being elliptical is not supported!";
            }
            QPointF newPos(x, y);
            if (is_relative)
                newPos += oldPos;
            if (newPos != oldPos)
            {
                const Point p1(oldPos.x(), oldPos.y());
                const Point p2(x, y);
                const Line line(p1, p2);
                const Point center(CircleCenterFromChordAndRadius(p1, p2, crx, sweepFlag != largeArc));
                const Segment::Orientation orientation = !sweepFlag ? Segment::Clockwise : Segment::CounterClockwise;
                shape.boundary.segments.push_back(Segment(line, center, orientation));
                oldPos = newPos;
            }
        }
        else if (command == 'L')
        {
            qreal x, y;
            stream >> x >> y;
            QPointF newPos(x, y);
            if (is_relative)
                newPos += oldPos;
            if (newPos != oldPos)
            {
                const Line line(Point(oldPos.x(), oldPos.y()), Point(x, y));
                shape.boundary.segments.push_back(Segment(line));
                oldPos = newPos;
            }
        }
        else if (command == 'Z' || command == 'z')
        {
            if (oldPos != firstPos)
            {
                const Line line(Point(oldPos.x(), oldPos.y()), Point(firstPos.x(), firstPos.y()));
                shape.boundary.segments.push_back(Segment(line));
                oldPos = firstPos; // TODO is this what the spec says?
            }
        }
    }
    return shape;
}

} // namespace LineArcOffsetDemo
