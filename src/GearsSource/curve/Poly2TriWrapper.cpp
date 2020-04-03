#include "stdafx.h"
#include "Poly2TriWrapper.h"

namespace p2t
{
	Poly2TriWrapper::Poly2TriWrapper(const boost::python::list& pts)
	{
		auto size = len(pts);
		bool error = false;
		if(size % 2 == 0 && size >= 6)
		{
			points_store.reserve(size / 2);
			for(int i = 0; i < len(pts) - 1; i += 2)
			{
				boost::python::extract<double> x(pts[i]);
				boost::python::extract<double> y(pts[i + 1]);
				if(x.check() && y.check())
				{
					points_store.emplace_back(x, y);
					points.push_back(&(points_store.back()));
				}
				else
				{
					PyErr_SetString(PyExc_RuntimeError, "One of the element is not float or int.");
					boost::python::throw_error_already_set();
					error = true;
				}
			}
			if(error)
			{
				points.clear();
				points_store.clear();
			}
			else
			{
				instance = std::make_unique<CDT>(points);
			}
		}
		else
		{
			PyErr_SetString(PyExc_RuntimeError, "The size of the given list is not even or hass less than 6 elements");
			boost::python::throw_error_already_set();
		}
	}

	boost::python::list Poly2TriWrapper::GetTriangles()
	{
		if(instance)
		{
			std::vector<Triangle*> triangles = instance->GetTriangles();
			boost::python::list result;
			for(auto t : triangles)
			{
				Point* p0 = t->GetPoint(0);
				Point* p1 = t->GetPoint(1);
				Point* p2 = t->GetPoint(2);
				
				// Add in CCW
				result.append((float)(p0->x));
				result.append((float)(p0->y));
				result.append((float)(p2->x));
				result.append((float)(p2->y));
				result.append((float)(p1->x));
				result.append((float)(p1->y));
			}
			return result;
		}
		return boost::python::list();
	}

	void Poly2TriWrapper::Triangulate()
	{
		if(instance)
			instance->Triangulate();
	}
}