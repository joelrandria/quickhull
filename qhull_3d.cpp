#include "qhull_3d.h"
#include "jhull_2d.h"

#include <Transform.h>

void QHull3d::initialize(const std::vector<gk::Point>* points)
{
	clear();

	_points = points;

	createVertices();
	createInitialTetrahedron();
}
void QHull3d::createVertices()
{
	HEVertex* v;

	const int pointcount = (int)_points->size();

	_vertices.reserve(pointcount);

	for (int i = 0; i < pointcount; ++i)
	{
		v = new HEVertex(_points);
		v->index = i;
		v->edge = nullptr;

		_vertices.push_back(std::unique_ptr<HEVertex>(v));
	}
}
void QHull3d::createInitialTetrahedron()
{
	float d;
	float dmax;

	int epidx[6];
	int tetraidx[4];

	// Get extreme points (EP)
	for (int i = 0; i < 6; ++i)
		epidx[i] = -1;

	for (int i = 0; i < (int)_points->size(); ++i)
	{
		const gk::Point& p = (*_points)[i];

		if (epidx[0] < 0 || p.x < (*_points)[epidx[0]].x)
			epidx[0] = i;
		if (epidx[1] < 0 || p.x > (*_points)[epidx[1]].x)
			epidx[1] = i;

		if (epidx[2] < 0 || p.y < (*_points)[epidx[2]].y)
			epidx[2] = i;
		if (epidx[3] < 0 || p.y > (*_points)[epidx[3]].y)
			epidx[3] = i;

		if (epidx[4] < 0 || p.z < (*_points)[epidx[4]].z)
			epidx[4] = i;
		if (epidx[5] < 0 || p.z > (*_points)[epidx[5]].z)
			epidx[5] = i;
	}

	// Find the most distant EP pair to build base triangle's first edge
	dmax = 0.f;

	for (int i = 0; i < 5; ++i)
	{
		for (int j = i + 1; j < 6; ++j)
		{
			gk::Vector vij((*_points)[epidx[i]], (*_points)[epidx[j]]);

			if ((d = vij.LengthSquared()) > dmax)
			{
				tetraidx[0] = epidx[i];
				tetraidx[1] = epidx[j];

				dmax = d;
			}
		}
	}

	// Find the most distant EP from the first edge's support line to complete the base triangle
	dmax = 0.f;
	tetraidx[2] = -1;

	const gk::Point& t0 = (*_points)[tetraidx[0]];
	gk::Vector t01 = gk::Normalize(gk::Vector(t0, (*_points)[tetraidx[1]]));

	for (int i = 0; i < 6; ++i)
	{
		if (epidx[i] == tetraidx[0] || epidx[i] == tetraidx[1])
			continue;

		gk::Vector t0i(t0, (*_points)[epidx[i]]);
		float pprojlength = gk::Dot(t0i, t01);
		d = t0i.LengthSquared() - (pprojlength * pprojlength);

		if (d > dmax)
		{
			tetraidx[2] = epidx[i];

			dmax = d;
		}
	}

	// Special case where there are only 2 extreme points => Pick any remaining point
	if (tetraidx[2] < 0)
	{
		for (int i = 0; i < (int)_points->size(); ++i)
		{
			if (i != tetraidx[0] && i != tetraidx[1])
			{
				tetraidx[2] = i;
				break;
			}
		}
	}

	// Find the most distant point from the base triangle within the point cloud to complete the initial tetrahedron
	dmax = 0.f;
	HEFace* tetrabase = createFace(tetraidx[0], tetraidx[1], tetraidx[2]);

	for (int i = 0; i < (int)_points->size(); ++i)
	{
		if (i == tetraidx[0] || i == tetraidx[1] || i == tetraidx[2])
			continue;

		//if (fabs(d = tetrabase->distance((*_points)[i])) > fabs(dmax))
		if (fabs(d = tetrabase->distance((*_points)[i])) >= fabs(dmax))
		{
			tetraidx[3] = i;

			dmax = d;
		}
	}

	// Coplanarity detection
	if (dmax == 0)
	{
		initialize2d();

		return;
	}

	// Reverse the base triangle if not counter clockwise oriented according to the tetrahedron outer surface
	if (dmax > 0)
		tetrabase->reverse();

	// Complete the tetrahedron's mesh
	std::vector<HEFace*> tetrafaces = extrudeOut(tetrabase, tetraidx[3]);
	tetrafaces.insert(tetrafaces.begin(), tetrabase);

	// Assign remaining points to their corresponding face
	for (int i = 0; i < (int)_points->size(); ++i)
	{
		if (i == tetraidx[0] || i == tetraidx[1] || i == tetraidx[2] || i == tetraidx[3])
			continue;

		for (int f = 0; f < (int)tetrafaces.size(); ++f)
		{
			if (tetrafaces[f]->tryAssignVertex(_vertices[i].get()))
				break;
		}
	}

	//! Add the tetrahedron's not empty faces to the processing stack
	for (int i = 0; i < (int)tetrafaces.size(); ++i)
		if (!tetrafaces[i]->vertices.empty())
			_processingfaces.push(tetrafaces[i]);

	// Store hull first vertex
	_hull = _vertices[tetraidx[0]].get();

	//////////////////////////////////////////////////////////////////////////
	// ToDo JRA: Remove this test code

	assertManifoldValidity(_hull);
}

void QHull3d::initialize2d()
{
	// Get plane's normal
	gk::Point p0;

	gk::Vector n;
	gk::Vector v0;
	gk::Vector v1;

	p0 = (*_points)[0];
	v0 = gk::Vector(p0, (*_points)[1]);

	for (int i = 2; i < (int)_points->size(); ++i)
	{
		n = gk::Cross(v0, gk::Vector(p0, (*_points)[i]));
		if (n.x != 0 || n.y != 0 || n.z != 0)
			break;
	}

	// Compute the plane's coordinate system
	n = gk::Normalize(n);
	v0 = gk::Normalize(v0);
	v1 = gk::Cross(v0, n);

// 	gk::Transform worldToPlane = gk::Transform({
// 		v0.x, n.x, v1.x, p0.x,
// 		v0.y, n.y, v1.y, p0.y,
// 		v0.z, n.z, v1.z, p0.z,
// 		0, 0, 0, 1
// 	}).inverse();

	gk::Transform worldToPlane = gk::Transform({
		v0.x,	v0.y,	v0.z,	-(v0.x * p0.x + v0.y * p0.y + v0.z * p0.z),
		n.x,	n.y,	n.z,	-(n.x * p0.x + n.y * p0.y + n.z * p0.z),
		v1.x,	v1.y,	v1.z,	-(v1.x * p0.x + v1.y * p0.y + v1.z * p0.z),
		0,		0,		0,		1
	});

	// Move all point to the planar coordinate system
	for (int i = 0; i < (int)_points->size(); ++i)
	{
		gk::Point p = worldToPlane((*_points)[i]);

		_points2d.push_back(gk::Vec2(p.x, p.z));
	}

	// Initialize the computation of the 2D convex hull
	_hull2d = std::make_unique<JHull2d>();
	_hull2d->initialize(&_points2d);
}

std::vector<QHull3d::Face> QHull3d::hull() const
{
	std::vector<Face> faces;

	if (_hull2d)
	{
		std::vector<int> hullidx = _hull2d->hull();
		
		if (hullidx.size() > 2)
		{
			faces.reserve(hullidx.size() - 2);

			for (int i = 1; i < (int)hullidx.size(); ++i)
				faces.push_back({
				hullidx[0],
				hullidx[i - 1],
				hullidx[i]
			});
		}
	}

	if (_hull)
	{
		std::vector<HEFace*> hullfaces = _hull->getConnectedFaces();

		faces.reserve(hullfaces.size());

		for (int i = 0; i < (int)hullfaces.size(); ++i)
			faces.push_back({
			hullfaces[i]->edge->vertex->index,
			hullfaces[i]->edge->next->vertex->index,
			hullfaces[i]->edge->next->next->vertex->index
		});
	}

	return faces;
}