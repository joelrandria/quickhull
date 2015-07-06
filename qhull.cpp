#include "qhull.h"

QHull::QHull()
	: _inputpoints(nullptr)
{
}
QHull::QHull(const std::vector<gk::Point>* points)
	: _inputpoints(points)
{
	initialize();
}
QHull::QHull(QHull&& hull)
{
	*this = std::move(hull);
}

QHull& QHull::operator=(QHull&& hull)
{
	if (this != &hull)
	{
		_inputpoints = std::move(hull._inputpoints);
		_vertices = std::move(hull._vertices);
		_edges = std::move(hull._edges);
		_faces = std::move(hull._faces);
		_pendingfaces = std::move(hull._pendingfaces);
		_hull = std::move(hull._hull);

		std::copy_n(hull.epindices, 6, epindices);
		std::copy_n(hull.hullindices, 4, hullindices);

		initialize();
	}

	return *this;
}

void QHull::initialize()
{
	createVertices();
	createStartTetrahedron();
}
void QHull::createVertices()
{
	HEVertex* v;

	const int pointcount = (int)_inputpoints->size();

	_vertices.reserve(pointcount);

	for (int i = 0; i < pointcount; ++i)
	{
		v = new HEVertex();
		v->index = i;
		v->edge = nullptr;

		_vertices.push_back(std::unique_ptr<HEVertex>(v));
	}
}
void QHull::createStartTetrahedron()
{
	float d;
	float dmax;
	
	int tetraindices[4];

	// Get extreme points
	for (int i = 0; i < 6; ++i)
		epindices[i] = -1;

	for (int i = 0; i < (int)_inputpoints->size(); ++i)
	{
		const gk::Point& p = (*_inputpoints)[i];

		if (epindices[0] < 0 || p.x < (*_inputpoints)[epindices[0]].x)
			epindices[0] = i;
		if (epindices[1] < 0 || p.x > (*_inputpoints)[epindices[1]].x)
			epindices[1] = i;

		if (epindices[2] < 0 || p.y < (*_inputpoints)[epindices[2]].y)
			epindices[2] = i;
		if (epindices[3] < 0 || p.y > (*_inputpoints)[epindices[3]].y)
			epindices[3] = i;

		if (epindices[4] < 0 || p.z < (*_inputpoints)[epindices[4]].z)
			epindices[4] = i;
		if (epindices[5] < 0 || p.z > (*_inputpoints)[epindices[5]].z)
			epindices[5] = i;
	}

	// Find the most distant EP pair to build base triangle's first edge
	dmax = 0.f;

	for (int i = 0; i < 5; ++i)
	{
		for (int j = i + 1; j < 6; ++j)
		{
			gk::Vector vij((*_inputpoints)[epindices[i]], (*_inputpoints)[epindices[j]]);

			if ((d = vij.LengthSquared()) > dmax)
			{
				tetraindices[0] = epindices[i];
				tetraindices[1] = epindices[j];

				dmax = d;
			}
		}
	}

	// Find the most distant EP from the first edge's support line to complete the base triangle
	dmax = 0.f;
	Line baseline((*_inputpoints)[tetraindices[0]], (*_inputpoints)[tetraindices[1]]);

	for (int i = 0; i < 6; ++i)
	{
		if (epindices[i] == tetraindices[0] || epindices[i] == tetraindices[1])
			continue;

		if ((d = baseline.squareddistance((*_inputpoints)[epindices[i]])) > dmax)
		{
			tetraindices[2] = epindices[i];

			dmax = d;
		}
	}

	//! Find the most distant point from the base triangle within the point cloud to complete the tetrahedron
	dmax = 0.f;
	Plane baseplane((*_inputpoints)[tetraindices[0]], (*_inputpoints)[tetraindices[1]], (*_inputpoints)[tetraindices[2]]);

	for (int i = 0; i < (int)_inputpoints->size(); ++i)
	{
		if (i == tetraindices[0] || i == tetraindices[1] || i == tetraindices[2])
			continue;
		
		if ((d = baseplane.distance((*_inputpoints)[i])) > dmax)
		{
			tetraindices[3] = i;

			dmax = d;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ToDO JRA: Remove this test code

	std::copy_n(tetraindices, 4, hullindices);

	//////////////////////////////////////////////////////////////////////////

	//! ToDo JRA: Build the tetrahedron's mesh
}

void QHull::iterate()
{
}