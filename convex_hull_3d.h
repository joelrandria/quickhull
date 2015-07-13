#ifndef CONVEXHULL3D_H
#define CONVEXHULL3D_H

#include <Geometry.h>

#include <vector>

//! 3D convex hull computing base class.
class ConvexHull3d
{
public:

	//! Indexed triangle face.
	struct Face
	{
		int idx[3];

		Face(int idx1, int idx2, int idx3)
		{
			idx[0] = idx1;
			idx[1] = idx2;
			idx[2] = idx3;
		}
	};

	virtual ~ConvexHull3d() {}

	//! Clear internal data.
	virtual void clear() = 0;

	//! Initialize the hull computing for the specified point set.
	virtual void initialize(const std::vector<gk::Point>* points) = 0;

	//! Build the point set's convex hull.
	//! Return the number of performed iteration to build the hull.
	virtual int build() = 0;
	//! Performs an iteration of the algorithm.
	//! Returns false if the convex hull is complete, true otherwise.
	virtual bool iterate() = 0;

	//! Get all the faces making up the convex hull.
	virtual std::vector<Face> hull() const = 0;
};

#endif