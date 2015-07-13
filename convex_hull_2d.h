#ifndef CONVEXHULL2D_H
#define CONVEXHULL2D_H

#include <Geometry.h>

#include <vector>

//! 2D convex hull computing base class.
class ConvexHull2d
{
public:

	virtual ~ConvexHull2d() {}

	//! Clear internal data.
	virtual void clear() = 0;

	//! Initialize the hull computing for the specified point set.
	virtual void initialize(const std::vector<gk::Vec2>* points) = 0;

	//! Build the point set's convex hull.
	//! Return the number of performed iteration to build the hull.
	virtual int build() = 0;
	//! Performs an iteration of the algorithm.
	//! Returns false if the convex hull is complete, true otherwise.
	virtual bool iterate() = 0;

	//! Get the ordered index list of vertex making up the convex hull.
	virtual std::vector<int> hull() const = 0;
};

#endif