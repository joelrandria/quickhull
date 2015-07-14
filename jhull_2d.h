#ifndef JHULL2D_H
#define JHULL2D_H

#include "convex_hull_2d.h"

#include <algorithm>

//! Jarvis march (gift wrapping) 2D algorithm implementation for convex hull (O(nh) average complexity with h = hull face count).
class JHull2d : public ConvexHull2d
{
private:

	//! End flag.
	bool _done;
	//! Iteration identifier.
	int _iterationid;

	//! Input points.
	const std::vector<gk::Vec2>* _points;

	//! Convex hull's point indices.
	std::vector<int> _hullpointsidx;

public:

	JHull2d() { clear(); }
	JHull2d(JHull2d&& hull) { *this = std::move(hull); }

	JHull2d& operator=(JHull2d&& hull);

	virtual void clear();

	virtual void initialize(const std::vector<gk::Vec2>* points);

	virtual int build();
	virtual bool iterate();

	virtual std::vector<int> hull() const { return _hullpointsidx; }
};

inline JHull2d& JHull2d::operator=(JHull2d&& hull)
{
	if (this != &hull)
	{
		_done = hull._done;
		_iterationid = hull._iterationid;

		_points = std::move(hull._points);

		_hullpointsidx = std::move(hull._hullpointsidx);
	}

	return *this;
}

inline void JHull2d::clear()
{
	_hullpointsidx.clear();

	_points = nullptr;
	_iterationid = -1;
	_done = false;
}

inline int JHull2d::build()
{
	while (iterate());

	return _iterationid + 1;
}
inline bool JHull2d::iterate()
{
	int p0idx;
	gk::Vec2 p0;

	int p1idx;
	gk::Vec2 p1;
	gk::Vec2 n01;

	gk::Vec2 p2;

	float d;

	if (_done)
		return false;

	++_iterationid;

	p0idx = _hullpointsidx.back();
	p0 = (*_points)[p0idx];

	for (p1idx = 0; p1idx < (int)_points->size(); ++p1idx)
		if (p1idx != p0idx)
			break;
	p1 = (*_points)[p1idx];

	n01 = gk::Vec2(p0.y - p1.y, p1.x - p0.x);

	for (int i = 0; i < (int)_points->size(); ++i)
	{
		if (i == p0idx || i == p1idx)
			continue;

		p2 = (*_points)[i];

		d = n01.x * (p2.x - p0.x) + n01.y * (p2.y - p0.y);

		if (d == 0)
		{
			gk::Vec2 v01(p1.x - p0.x, p1.y - p0.y);
			gk::Vec2 v02(p2.x - p0.x, p2.y - p0.y);

			if ((v02.x * v02.x + v02.y * v02.y) > (v01.x * v01.x + v01.y * v01.y))
			{
				p1idx = i;
				p1 = p2;
			}
		}
		else if (d < 0)
		{
			p1idx = i;
			p1 = p2;
			n01 = gk::Vec2(p0.y - p1.y, p1.x - p0.x);
		}
	}

	if (p1idx == _hullpointsidx.front())
		_done = true;
	else
		_hullpointsidx.push_back(p1idx);

	return true;
}

#endif