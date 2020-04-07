#pragma once

// dependencies
#include "memory"
#include <vector>
//#include <boost/python.hpp>
#include <pybind11/pybind11.h>

// poly2tri headers
#include "poly2tri.h"

namespace p2t {
class Poly2TriWrapper {
    std::unique_ptr<CDT> instance;
    std::vector<Point*>  points;
    std::vector<Point>   points_store;

public:
    Poly2TriWrapper (const pybind11::list& pts);
    pybind11::list GetTriangles ();
    void           Triangulate ();
};
} // namespace p2t