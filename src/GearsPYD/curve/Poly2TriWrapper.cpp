#include "Poly2TriWrapper.h"
#include "stdafx.h"

#include <stdexcept>

namespace p2t {

Poly2TriWrapper::Poly2TriWrapper (const pybind11::list& pts)
{
    auto size  = len (pts);
    bool error = false;
    if (size % 2 == 0 && size >= 6) {
        points_store.reserve (size / 2);
        for (int i = 0; i < len (pts) - 1; i += 2) {
            double x (pts[i].cast<double> ());
            double y (pts[i + 1].cast<double> ());

            points_store.emplace_back (x, y);
            points.push_back (&(points_store.back ()));
        }
        if (error) {
            points.clear ();
            points_store.clear ();
        } else {
            instance = std::make_unique<CDT> (points);
        }
    } else {
        throw std::runtime_error (Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ());
    }
}

pybind11::list Poly2TriWrapper::GetTriangles ()
{
    if (instance) {
        std::vector<Triangle*> triangles = instance->GetTriangles ();
        pybind11::list         result;
        for (auto t : triangles) {
            Point* p0 = t->GetPoint (0);
            Point* p1 = t->GetPoint (1);
            Point* p2 = t->GetPoint (2);

            // Add in CCW
            result.append ((float)(p0->x));
            result.append ((float)(p0->y));
            result.append ((float)(p2->x));
            result.append ((float)(p2->y));
            result.append ((float)(p1->x));
            result.append ((float)(p1->y));
        }
        return result;
    }
    return pybind11::list ();
}

void Poly2TriWrapper::Triangulate ()
{
    if (instance)
        instance->Triangulate ();
}
} // namespace p2t