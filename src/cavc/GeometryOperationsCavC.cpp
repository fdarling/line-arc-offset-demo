#include "GeometryOperationsCavC.h"
#include "GeometryCavC.h"
#include "CavCQt.h"
#include "../GeometryQt.h"

#include <cavc/polylinecombine.hpp>

#include <QDebug>

#include <cassert>

namespace LineArcOffsetDemo {

LineArcGeometry::MultiShape GeometryOperationsCavC::identity(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsCavC::identity()";
    const CavC_MultiShape cavcMultiShape(MultiShapeToCavC_MultiShape(multiShape));
    const LineArcGeometry::MultiShape reconverted(CavC_MultiShapeToMultiShape(cavcMultiShape));
    return reconverted;
}

static void appendPolylines(Polylines &dest, Polylines &&src)
{
    dest.insert(dest.end(), std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()));
}

static Polylines PolylinesMinusPolyline(const Polylines &minuend, const Polyline &subtrahend, bool *nested)
{
    Polylines difference;
    for (Polylines::const_iterator it = minuend.begin(); it != minuend.end(); ++it)
    {
        // subtracting the polyline
        cavc::CombineResult<CavC_Real> differenceResult = cavc::combinePolylines(*it, subtrahend, cavc::PlineCombineMode::Exclude);
        // make sure the subtrahend wasn't fully contained inside a hole
        if (differenceResult.subtracted.empty())
        {
            // move the produced Polylines to the final result (there may be none)
            appendPolylines(difference, std::move(differenceResult.remaining));
        }
        else
        {
            // the subtrathend was completely inside of a hole,
            // and a hole within a hole is an island. This means
            // the shapes were actually mutually exclusive
            *nested = true;
        }
    }
    return difference;
}

static Polylines PolylinesIntersectPolylines(const Polylines &a, const Polylines &b)
{
    Polylines intersection;
    for (Polylines::const_iterator itA = a.begin(); itA != a.end(); ++itA)
    {
        for (Polylines::const_iterator itB = b.begin(); itB != b.end(); ++itB)
        {
            // intersect the polyline
            cavc::CombineResult<CavC_Real> intersectionResult = cavc::combinePolylines(*itA, *itB, cavc::PlineCombineMode::Intersect);
            /*// TODO is this something we should check for?
            if (!intersectionResult.subtracted.empty())
            {
                qDebug() << "WARNING: PolylinesIntersectPolylines() has Polylines in 'subtracted' portion!";
            }*/
            // move the produced Polylines to the final result (there may be none)
            appendPolylines(intersection, std::move(intersectionResult.remaining));
        }
    }
    return intersection;
}

static CavC_Shape TryUnionCavC_Shapes(const CavC_Shape &a, const CavC_Shape &b)
{
    CavC_Shape result;
    cavc::CombineResult<CavC_Real> unionResult = cavc::combinePolylines(a.boundary, b.boundary, cavc::PlineCombineMode::Union);
    if (unionResult.remaining.size() == 1)
    {
        result.boundary = unionResult.remaining.at(0);
        appendPolylines(result.holes, std::move(unionResult.subtracted));
        bool nested = false;
        appendPolylines(result.holes, PolylinesMinusPolyline(a.holes, b.boundary, &nested));
        appendPolylines(result.holes, PolylinesMinusPolyline(b.holes, a.boundary, &nested));
        if (nested)
        {
            // signal to the calling function that
            // the shapes didn't actually merge
            return CavC_Shape();
        }
        appendPolylines(result.holes, PolylinesIntersectPolylines(a.holes, b.holes));
    }
    return result;
}

LineArcGeometry::MultiShape GeometryOperationsCavC::join(const LineArcGeometry::MultiShape &multiShape)
{
    // qDebug() << "GeometryOperationsCavC::join()";
    if (multiShape.shapes.size() < 2)
    {
        return multiShape;
    }
    CavC_MultiShape result = MultiShapeToCavC_MultiShape(multiShape);
    for (CavC_MultiShape::iterator itA = result.begin(); itA != result.end(); ++itA)
    {
        for (CavC_MultiShape::iterator itB = std::next(itA); itB != result.end(); ++itB)
        {
            const CavC_Shape combined = TryUnionCavC_Shapes(*itA, *itB);
            if (combined.boundary.vertexes().empty()) // TODO request .empty() be added to Polyline class
                continue;
            *itA = combined;
            result.erase(itB);
            itB = itA;
        }
    }
    return CavC_MultiShapeToMultiShape(result);
}

LineArcGeometry::MultiShape GeometryOperationsCavC::join(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsCavC::join()";
    LineArcGeometry::MultiShape both;
    // TODO make this more efficient!
    both.shapes.insert(both.shapes.end(), a.shapes.begin(), a.shapes.end());
    both.shapes.insert(both.shapes.end(), b.shapes.begin(), b.shapes.end());
    return join(both);
}

static void SubtractPolylinesFromShapes(CavC_MultiShape &multiShape, const Polylines &polylines)
{
    for (CavC_MultiShape::iterator itA = multiShape.begin(); itA != multiShape.end();)
    {
        for (Polylines::const_iterator itB = polylines.begin(); itB != polylines.end(); ++itB)
        {
            assert(!itA->boundary.vertexes().empty());
            assert(!itB->vertexes().empty());
            cavc::CombineResult<CavC_Real> subtractionResult = cavc::combinePolylines(itA->boundary, *itB, cavc::PlineCombineMode::Exclude);
            if (!subtractionResult.subtracted.empty())
            {
                // add hole
                assert(subtractionResult.subtracted.size() == 1);
                assert(subtractionResult.remaining.size() == 1);
                itA->holes.insert(itA->holes.end(), std::make_move_iterator(subtractionResult.subtracted.begin()), std::make_move_iterator(subtractionResult.subtracted.end()));
            }
            else if (subtractionResult.remaining.empty())
            {
                // make sure this element in A gets removed
                multiShape.erase(itA++);
                goto skip_increment;
            }
            else // if (!subtractionResult.remaining.empty())
            {
                // move the formed islands into A, we'll keep processing with the reduced element in A
                // overwrite the current element with the first of the broken up pieces
                itA->boundary = std::move(subtractionResult.remaining.front());
                // insert the rest of the broken up pieces after the current element (or at the and of the list, makes no difference!);
                for (Polylines::iterator it = std::next(subtractionResult.remaining.begin()); it != subtractionResult.remaining.end(); ++it)
                {
                    CavC_Shape newShape;
                    newShape.boundary = std::move(*it);
                    multiShape.insert(std::next(itA), std::move(newShape));
                    // multiShape.push_back(std::move(newShape));
                }
            }
        }
        ++itA;
        skip_increment: ; // must have a statement after a label
    }
}

static CavC_MultiShape IntersectCavC_Shapes(const CavC_Shape &a, const CavC_Shape &b)
{
    // (A.boundary & B.boundary) - A.holes - B.holes
    CavC_MultiShape result;
    // generate (A.boundary & B.boundary)
    {
        assert(!a.boundary.vertexes().empty());
        assert(!b.boundary.vertexes().empty());
        cavc::CombineResult<CavC_Real> intersectionResult = cavc::combinePolylines(a.boundary, b.boundary, cavc::PlineCombineMode::Intersect);
        for (Polylines::iterator it = intersectionResult.remaining.begin(); it != intersectionResult.remaining.end(); ++it)
        {
            CavC_Shape shape;
            shape.boundary = std::move(*it);
            result.push_back(std::move(shape));
        }
        assert(intersectionResult.subtracted.empty());
    }
    // generate (A.boundary & B.boundary) - A.holes
    SubtractPolylinesFromShapes(result, a.holes);
    // generate ((A.boundary & B.boundary) - A.holes) - B.holes
    SubtractPolylinesFromShapes(result, b.holes);
    return result;
}

LineArcGeometry::MultiShape GeometryOperationsCavC::intersection(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsCavC::intersection()";
    const CavC_MultiShape aa = MultiShapeToCavC_MultiShape(a);
    const CavC_MultiShape bb = MultiShapeToCavC_MultiShape(b);
    CavC_MultiShape result;
    for (CavC_MultiShape::const_iterator itA = aa.begin(); itA != aa.end(); ++itA)
    {
        for (CavC_MultiShape::const_iterator itB = bb.begin(); itB != bb.end(); ++itB)
        {
            // perform individial intersection, since both shapes are planar
            // this the resulting intersection is guaranteed to not touch
            // anything (because it wasn't before, and it's only smaller now)
            CavC_MultiShape intersected = IntersectCavC_Shapes(*itA, *itB);
            // save the results into the final product
            result.insert(result.end(), std::make_move_iterator(intersected.begin()), std::make_move_iterator(intersected.end()));
        }
    }
    return CavC_MultiShapeToMultiShape(result);
}

static CavC_MultiShape SubtractCavC_Shapes(const CavC_Shape &a, const CavC_Shape &b)
{
    // (A.boundary - B.boundary) + A.boundary & (B.holes - A.holes)
    // (A.boundary - B.boundary) + A.boundary & B.holes - A.holes
    // (A.boundary - B.boundary) + (A.boundary & B.holes) - A.holes
    CavC_MultiShape result;
    // generate (A.boundary - B.boundary)
    {
        assert(!a.boundary.vertexes().empty());
        assert(!b.boundary.vertexes().empty());
        cavc::CombineResult<CavC_Real> subtractionResult = cavc::combinePolylines(a.boundary, b.boundary, cavc::PlineCombineMode::Exclude);
        // qDebug() << "OMG" << subtractionResult.remaining.size() << subtractionResult.subtracted.size();
        for (Polylines::iterator it = subtractionResult.remaining.begin(); it != subtractionResult.remaining.end(); ++it)
        {
            CavC_Shape shape;
            shape.boundary = std::move(*it);
            result.push_back(std::move(shape));
        }
        if (!subtractionResult.subtracted.empty())
        {
            assert(subtractionResult.remaining.size() == 1);
            assert(subtractionResult.subtracted.size() == 1);
            // result.back().holes.push_back(std::move(subtractionResult.subtracted.back()));
            result.back().holes.insert(result.back().holes.end(), std::make_move_iterator(subtractionResult.subtracted.begin()), std::make_move_iterator(subtractionResult.subtracted.end()));
        }
    }
    // generate (A.boundary & B.holes) and combine with above results
    for (Polylines::const_iterator hole_it = b.holes.begin(); hole_it != b.holes.end(); ++hole_it)
    {
        cavc::CombineResult<CavC_Real> intersectionResult = cavc::combinePolylines(a.boundary, *hole_it, cavc::PlineCombineMode::Intersect);
        assert(intersectionResult.subtracted.empty()); // TODO unsure if this makes sense
        for (Polylines::iterator it = intersectionResult.remaining.begin(); it != intersectionResult.remaining.end(); ++it)
        {
            CavC_Shape shape;
            shape.boundary = std::move(*it);
            result.push_back(std::move(shape));
        }
    }
    // generate ((A.boundary - B.boundary) + (A.boundary & B.holes)) - A.holes
    SubtractPolylinesFromShapes(result, a.holes);
    return result;
}

LineArcGeometry::MultiShape GeometryOperationsCavC::difference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    // qDebug() << "GeometryOperationsCavC::difference()";
    CavC_MultiShape aa = MultiShapeToCavC_MultiShape(a);
    CavC_MultiShape bb = MultiShapeToCavC_MultiShape(b);
    for (CavC_MultiShape::iterator itA = aa.begin(); itA != aa.end();)
    {
        for (CavC_MultiShape::const_iterator itB = bb.begin(); itB != bb.end(); ++itB)
        {
            CavC_MultiShape subtracted = SubtractCavC_Shapes(*itA, *itB);
            if (subtracted.empty())
            {
                // the shape from B annihilated A, so remove it
                // and stop trying to subtract from it anymore
                aa.erase(itA++);
                goto skip_increment;
            }
            else
            {
                // the subtraction resulted in one or more shapes,
                // so effectively delete the original A shape and
                // replace it with the smaller subtracted shapes
                *itA = std::move(subtracted.front());
                aa.insert(std::next(itA), std::make_move_iterator(std::next(subtracted.begin())), std::make_move_iterator(subtracted.end()));
            }
        }
        ++itA;
        skip_increment: ; // must have a statement after a label
    }
    return CavC_MultiShapeToMultiShape(aa);
}

// TODO
LineArcGeometry::MultiShape GeometryOperationsCavC::symmetricDifference(const LineArcGeometry::MultiShape &multiShape)
{
    (void)multiShape;
    // qDebug() << "GeometryOperationsCavC::symmetricDifference()";
    return LineArcGeometry::MultiShape();
}

// TODO
LineArcGeometry::MultiShape GeometryOperationsCavC::symmetricDifference(const LineArcGeometry::MultiShape &a, const LineArcGeometry::MultiShape &b)
{
    (void)a; (void)b;
    // qDebug() << "GeometryOperationsCavC::symmetricDifference()";
    return LineArcGeometry::MultiShape();
}

// TODO fix resulting geometry to respect holes
LineArcGeometry::MultiShape GeometryOperationsCavC::offset(const LineArcGeometry::MultiShape &multiShape, double radius)
{
    // qDebug() << "GeometryOperationsCavC::offset()";
    const CavC_Real cavcRadius = ToCavC(radius);
    const bool reversed = cavcRadius > 0.0;
    const OffsetLoopSet loopSet = MultiShapeToOffsetLoopSet(multiShape, reversed);
    cavc::ParallelOffsetIslands<CavC_Real> alg;
    cavc::OffsetLoopSet<double> offsetResult = alg.compute(loopSet, std::abs(cavcRadius)); // abs() is redundant, but we're doing it here to be explicit
    return OffsetLoopSetToMultiShape(offsetResult, reversed); // NOTE: currently gives overlapping shapes, doesn't respect holes!
}

} // namespace LineArcOffsetDemo
