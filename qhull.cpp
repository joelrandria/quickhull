#include "qhull.h"

QHull::QHull(const std::vector<gk::Point>* points)
	: _points(points),
	_iterationid(-1)
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
		_points = hull._points;

		_vertices = std::move(hull._vertices);
		_edges = std::move(hull._edges);
		_faces = std::move(hull._faces);
		_processingfaces = std::move(hull._processingfaces);
		_hull = std::move(hull._hull);

		_iterationid = hull._iterationid;
	}

	return *this;
}

void QHull::initialize()
{
	createVertices();
	createInitialTetrahedron();
}
void QHull::createVertices()
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
void QHull::createInitialTetrahedron()
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

	// Find the most distant point from the base triangle within the point cloud to complete the initial tetrahedron
	dmax = 0.f;
	HEFace* tetrabase = createFace(tetraidx[0], tetraidx[1], tetraidx[2]);

	for (int i = 0; i < (int)_points->size(); ++i)
	{
		if (i == tetraidx[0] || i == tetraidx[1] || i == tetraidx[2])
			continue;
		
		if (fabs(d = tetrabase->distance((*_points)[i])) > fabs(dmax))
		{
			tetraidx[3] = i;

			dmax = d;
		}
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

std::vector<QHull::Face> QHull::hull() const
{
	std::vector<Face> faces;
	std::vector<HEFace*> hullfaces = _hull->getConnectedFaces();

	faces.reserve(hullfaces.size());

	for (int i = 0; i < (int)hullfaces.size(); ++i)
		faces.push_back({
		hullfaces[i]->edge->vertex->index,
		hullfaces[i]->edge->next->vertex->index,
		hullfaces[i]->edge->next->next->vertex->index
	});

	return faces;
}