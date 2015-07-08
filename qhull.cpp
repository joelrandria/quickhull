#include "qhull.h"

QHull::QHull()
	: _points(nullptr)
{
}
QHull::QHull(const std::vector<gk::Point>* points)
	: _points(points)
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
		_pendingfaces = std::move(hull._pendingfaces);
		_hull = std::move(hull._hull);

		apexidx = hull.apexidx;
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

	//! Find the most distant point from the base triangle within the point cloud to complete the initial tetrahedron
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

	//! Reverse the base triangle if not clockwise oriented according to the tetrahedron surface
	if (dmax > 0)
		tetrabase->reverse();

	//! Complete the tetrahedron's mesh
	std::vector<HEFace*> tetrafaces = extrudeOut(tetrabase, tetraidx[3]);

	//! Store current faces
	_hull.push_back(tetrabase);
	_hull.insert(_hull.end(), tetrafaces.begin(), tetrafaces.end());
	
	//////////////////////////////////////////////////////////////////////////
	// ToDo JRA: Remove this test code

	apexidx = tetraidx[3];

	checkManifold(tetrabase);
}

std::vector<QHull::HEFace*> QHull::getConnectedFaces(const HEFace* face) const
{
	std::vector<HEFace*> connectedfaces;
	std::unordered_map<HEFace*, HEFace*> faceregistry;

	getConnectedFaces(face, faceregistry);

	connectedfaces.reserve(faceregistry.size());
	for (auto it = faceregistry.begin(); it != faceregistry.end(); ++it)
		connectedfaces.push_back(it->second);

	return connectedfaces;
}
void QHull::getConnectedFaces(const HEFace* face, std::unordered_map<HEFace*, HEFace*>& faceRegistry) const
{
	if (faceRegistry.count((HEFace*)face))
		return;

	faceRegistry[(HEFace*)face] = (HEFace*)face;

	std::vector<HEFace*> adjacentfaces = face->getAdjacentFaces();
	for (int i = 0; i < (int)adjacentfaces.size(); ++i)
		getConnectedFaces(adjacentfaces[i], faceRegistry);
}

void QHull::checkManifold(const HEFace* face) const
{
	// Get all connected faces
	std::vector<QHull::HEFace*> mesh = getConnectedFaces(face);
	if (mesh.size() != 4)
		throw new std::logic_error("Invalid manifold: Bad face count");	

	for (int f = 0; f < mesh.size(); ++f)
	{
		HEFace* face = mesh[f];

		// Check edge circularity
		if (face->edge != face->edge->next->next->next)
			throw new std::logic_error("Invalid manifold: Face's edges not circular");

		// Check co-edges
		HEEdge* edge = face->edge;
		for (int e = 0; e < 3; ++e)
		{
			if (!edge->pair)
				throw new std::logic_error("Invalid manifold: Co-edge NULL");
			if (edge->pair->pair != edge)
				throw new std::logic_error("Invalid manifold: Co-edges not symmetrical");

			edge = edge->next;
		}
	}
}

void QHull::iterate()
{
}

std::vector<QHull::Face> QHull::hull() const
{
	std::vector<Face> faces;

	faces.reserve(_hull.size());

	for (int i = 0; i < (int)_hull.size(); ++i)
		faces.push_back({
		_hull[i]->edge->vertex->index,
		_hull[i]->edge->next->vertex->index,
		_hull[i]->edge->next->next->vertex->index
	});

	return faces;
}