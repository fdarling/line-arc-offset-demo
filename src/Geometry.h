#ifndef LINEARCOFFSETDEMO_GEOMETRY_H
#define LINEARCOFFSETDEMO_GEOMETRY_H

#include <list>

namespace LineArcGeometry {

typedef double CoordinateType;

class Point
{
public:
    Point() :
        x(0),
        y(0)
    {
    }
    Point(CoordinateType xx, CoordinateType yy) :
        x(xx),
        y(yy)
    {
    }
    bool operator==(const Point &other) const
    {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Point &other) const
    {
        return x != other.x || y != other.y;
    }
    Point & operator*=(CoordinateType scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    Point & operator/=(CoordinateType scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }
    Point & operator+=(const Point &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    Point & operator-=(const Point &other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    CoordinateType x;
    CoordinateType y;
};

inline Point operator*(const Point &point, CoordinateType scalar)
{
    return Point(point.x*scalar, point.y*scalar);
}

inline Point operator*(CoordinateType scalar, const Point &point)
{
    return operator*(point, scalar);
}

inline Point operator/(const Point &point, CoordinateType scalar)
{
    return Point(point.x/scalar, point.y/scalar);
}

inline Point operator+(const Point &p1, const Point &p2)
{
    return Point(p1.x + p2.x, p1.y + p2.y);
}

inline Point operator-(const Point &p1, const Point &p2)
{
    return Point(p1.x - p2.x, p1.y - p2.y);
}

class Line
{
public:
    Line()
    {
    }
    Line(const Point &pa, const Point &pb) :
        p1(pa),
        p2(pb)
    {
    }
    bool operator==(const Line &other) const
    {
        return p1 == other.p1 && p2 == other.p2;
    }
    bool operator!=(const Line &other) const
    {
        return p1 != other.p1 || p2 != other.p2;
    }
    Line reversed() const
    {
        return Line(p2, p1);
    }
    CoordinateType length() const;
    CoordinateType distanceTo(const Point &pt) const;
    Point p1;
    Point p2;
};

class Segment
{
public:
    enum Orientation
    {
        Clockwise,
        CounterClockwise
    };
public:
    Segment() :
        orientation(Clockwise),
        isArc(false)
    {
    }
    Segment(const Line &l) :
        line(l),
        orientation(Clockwise), // doesn't really matter
        isArc(false)
    {
    }
    Segment(const Line &l, const Point &cen, Orientation orient = Clockwise) :
        line(l),
        center(cen),
        orientation(orient),
        isArc(true)
    {
    }
    Segment reversed() const
    {
        return Segment(line.reversed(), center, (orientation == Clockwise) ? CounterClockwise : Clockwise, isArc);
    }
    CoordinateType radius() const;
    Point midPoint() const;
    Line line;
    Point center;
    Orientation orientation;
    bool isArc;
private:
    Segment(const Line &l, const Point &cen, Orientation orient, bool arc) :
        line(l),
        center(cen),
        orientation(orient),
        isArc(arc)
    {
    }
};

class Contour
{
public:
    Contour()
    {
    }
    Contour(const Segment &seg)
    {
        segments.push_back(seg); // TODO maybe use initializer list for performance?
    }
    Contour(const std::list<Segment> &segs) :
        segments(segs)
    {
    }
    Contour reversed() const
    {
        Contour result;
        for (std::list<Segment>::const_reverse_iterator it = segments.rbegin(); it != segments.rend(); ++it)
        {
            result.segments.push_back(it->reversed());
        }
        return std::move(result);
    }
    CoordinateType area() const;
    Segment::Orientation orientation() const;
    bool isValid() const;
    bool isCircle() const;
    void fixSegmentOrientations();
    void fixSegmentEndpoints();
    Contour approximatedArcs() const;
    Contour arcsRecovered() const;
    std::list<Segment> segments;
};

class Shape
{
public:
    Shape()
    {
    }
    Shape(const Contour &bound) :
        boundary(bound)
    {
    }
    Shape(const Contour &bound, const std::list<Contour> hls) :
        boundary(bound),
        holes(hls)
    {
    }
    bool isAnnulus() const;
    Shape approximatedArcs() const;
    Shape arcsRecovered() const;
    Contour boundary;
    std::list<Contour> holes;
};

class MultiShape
{
public:
    MultiShape()
    {
    }
    MultiShape(const std::list<Shape> &shps) :
        shapes(shps)
    {
    }
    MultiShape approximatedArcs() const;
    MultiShape arcsRecovered() const;
    std::list<Shape> shapes;
};

Contour ContourFromLineAndRadius(const Line &line, double radius);

Segment::Orientation LinePointOrientation(const Line &line, const Point &pt);

inline Segment::Orientation OrientationReversed(Segment::Orientation orientation)
{
    return (orientation == Segment::Clockwise) ? Segment::CounterClockwise : Segment::Clockwise;
}

} // namespace LineArcGeometry

#endif // LINEARCOFFSETDEMO_GEOMETRY_H
