#include "jhull_2d.h"

#include <numeric>

void JHull2d::initialize(const std::vector<gk::Vec2>* points)
{
	clear();

	_points = points;

	// Find the lexicographical minimal point and insert this one into the convex hull
	int minidx;
	gk::Vec2 min(HUGE_VALF, HUGE_VALF);

	for (int i = 0; i < (int)_points->size(); ++i)
	{
		gk::Vec2 p = (*_points)[i];
		if (p.x < min.x || (p.x == min.x && p.y < min.y))
		{
			min = p;
			minidx = i;
		}
	}

	_hullpointsidx.push_back(minidx);
}