#ifndef QHULL3D_H
#define QHULL3D_H

#include "convex_hull_3d.h"
#include "convex_hull_2d.h"

#include <vector>
#include <stack>
#include <unordered_map>
#include <memory>

//! Quick hull algorithm implementation for 3D convex hull (O(n log(n)) average complexity).
//! http://www.cise.ufl.edu/~ungor/courses/fall06/papers/QuickHull.pdf
//! Fall back to a 2D algorithm when all the specified points are coplanar.
class QHull3d : public ConvexHull3d
{
private:

	/************************************************************************/
	/*						Half-edge data types								*/
	/************************************************************************/

	class HEVertex;
	class HEEdge;
	class HEFace;

	//! Half-edge vertex.
	class HEVertex
	{
	private:

		//! Global point set.
		const std::vector<gk::Point>* _points;

	public:

		int index;			//! Point list index

		HEEdge* edge;	//! One of the half-edges emanating from the vertex

		HEVertex(const std::vector<gk::Point>* points) : _points(points), index(-1), edge(nullptr) {}

		const gk::Point& getPoint() const { return (*_points)[index]; }

		//! Get all the faces connected to this vertex.
		std::vector<HEFace*> getConnectedFaces() const;
	};

	//! Half-edge.
	class HEEdge
	{
	public:

		int id;				//! Edge unique identifier

		HEVertex* vertex;	//! Vertex at the end of the half-edge

		HEEdge* coedge;		//! Oppositely oriented adjacent half-edge
		HEEdge* next;		//! Next half-edge around the face

		HEFace* face;		//! Face the half-edge borders

		HEEdge() : vertex(nullptr), coedge(nullptr), next(nullptr), face(nullptr) {}
	};

	//! Half-edge face.
	class HEFace
	{
	private:

		gk::Vector _n;				//! Face normal
		float _d;					//! Signed distance to the origin

		float _extremedistance;		//! Furthest vertex distance

	public:

		int id;								//! Face unique identifier

		HEEdge* edge;						//! One of the half-edges bordering the face

		int iterationid;					//! Iteration identifier

		std::vector<HEVertex*> vertices;	//! Visible vertices

		HEFace() : edge(nullptr), iterationid(-1), _d(0.f), _extremedistance(0.f) {}

		//! Get bordering vertices.
		std::vector<HEVertex*> getBorderingVertices() const;

		//! Get adjacent faces.
		std::vector<HEFace*> getAdjacentFaces() const;
		//! Get all connected faces.
		std::vector<HEFace*> getConnectedFaces() const;

		//! Reverse the face orientation.
		void reverse();

		//! Update support plane's internal data.
		void updateSupportPlane();

		//! Try to assign the specified vertex into the visible set.
		//! The assignment is performed if the vertex is visible.
		//! Returns true if the assignment was successful, false otherwise.
		bool tryAssignVertex(HEVertex* v);

		//! Get the signed orthogonal distance to the specified point, according to the normal direction.
		float distance(const gk::Point& p) const { return _n.x * p.x + _n.y * p.y + _n.z * p.z + _d; }

	private:

		//! Get all connected faces.
		void getConnectedFaces(const HEFace* face, std::unordered_map<HEFace*, HEFace*>& faceRegistry) const;
	};

	/************************************************************************/
	/*								Internals								*/
	/************************************************************************/

	//! Iteration identifier.
	int _iterationid;

	//! Input points.
	const std::vector<gk::Point>* _points;

	//! Global vertex set.
	std::vector<std::unique_ptr<HEVertex>> _vertices;
	//! Global edge set.
	std::vector<std::unique_ptr<HEEdge>> _edges;
	//! Global face set.
	std::vector<std::unique_ptr<HEFace>> _faces;

	//! Faces currently processed.
	std::stack<HEFace*> _processingfaces;

	//! Convex hull first vertex.
	HEVertex* _hull;

	//! 2D points.
	std::vector<gk::Vec2> _points2d;
	//! 2D convex hull internal algorithm.
	std::unique_ptr<ConvexHull2d> _hull2d;

public:

	QHull3d() { clear(); }
	QHull3d(QHull3d&& hull) { *this = std::move(hull); }

	QHull3d& operator=(QHull3d&& hull);

	/************************************************************************/
	/*						ConvexHull3d features							*/
	/************************************************************************/

	virtual void clear();

	virtual void initialize(const std::vector<gk::Point>* points);

	virtual int build();
	virtual bool iterate();

	std::vector<Face> hull() const;

	//! Get current hull faces' extreme vertex indices.
	std::vector<int> getFacesExtremesIndices() const
	{
		std::vector<int> extremeindices;
		if (_hull)
		{
			std::vector<HEFace*> hullfaces = _hull->getConnectedFaces();

			for (int i = 0; i < (int)hullfaces.size(); ++i)
				if (!hullfaces[i]->vertices.empty())
					extremeindices.push_back(hullfaces[i]->vertices[0]->index);
		}

		return extremeindices;
	}

private:

	//! Build internal vertices from input points.
	void createVertices();
	//! Build initial tetrahedron.
	void createInitialTetrahedron();

	//! Initialize the internal 2D convex hull computing (coplanarity case).
	void initialize2d();

	//! Create a new managed edge.
	HEEdge* createEdge();
	//! Create a new managed face.
	HEFace* createFace();
	//! Create a new managed face bordered by the specified vertices.
	//! Vertices are assumed to be specified in counter clockwise order according to the underlying surface.
	HEFace* createFace(int v1idx, int v2idx, int v3idx);

	//! Create new faces (fan configuration) by extruding the specified edge loop toward the specified vertex.
	//! The target vertex is assumed to be in the specified edge loop's positive half-space.
	//! The specified edge loop is assumed to be valid and counter clockwise oriented.
	std::vector<HEFace*> extrudeIn(const std::vector<HEEdge*>& loop, int vidx);
	//! Create new faces (fan configuration) by extruding the specified edge loop toward the specified vertex.
	//! The target vertex is assumed to be in the specified face's negative half-space.
	std::vector<HEFace*> extrudeOut(HEFace* face, int vidx);
	//! Create new faces (fan configuration) by extruding the specified edge loop toward the specified vertex.
	//! The target vertex is assumed to be in the specified edge loop's negative half-space.
	//! The specified edge loop is assumed to be valid and counter clockwise oriented.
	std::vector<HEFace*> extrudeOut(const std::vector<HEEdge*>& loop, int vidx);

	//! Get all unvisited faces connected to the specified faces, visible by the specified vertex.
	//! A face is considered unvisited if its iteration identifier does not match the specified one.
	void getVisibleUnvisitedConnectedFaces(const int iterationId, const HEFace* face, const gk::Point& p, std::vector<HEFace*>& visiblefaces);
	//! Get the horizon edge loop within the specified visible face set.
	//! The obtained horizon edge loop is counter clockwise oriented.
	std::vector<HEEdge*> getHorizonEdgeLoop(const int iterationId, const std::vector<HEFace*>& visiblefaces) const;
	//! Get the horizon edge directly following the specified one in the counter clockwise direction.
	HEEdge* getNextHorizonEdge(const int iterationId, HEEdge* horizonedge) const;

	//! Check the specified manifold validity (debug purpose only).
	void assertManifoldValidity(const HEVertex* vertex) const
	{
		// Get all connected faces
		std::vector<QHull3d::HEFace*> mesh = vertex->getConnectedFaces();

		for (int f = 0; f < (int)mesh.size(); ++f)
		{
			HEFace* face = mesh[f];

			// Check edge circularity
			if (face->edge != face->edge->next->next->next)
				throw new std::logic_error("Invalid manifold: Face's edges not circular");

			// Check co-edges
			HEEdge* edge = face->edge;
			for (int e = 0; e < 3; ++e)
			{
				if (!edge->coedge)
					throw new std::logic_error("Invalid manifold: Co-edge NULL");
				if (edge->coedge->coedge != edge)
					throw new std::logic_error("Invalid manifold: Co-edges not symmetrical");

				edge = edge->next;
			}
		}
	}
};

inline std::vector<QHull3d::HEFace*> QHull3d::HEVertex::getConnectedFaces() const
{
	return edge->face->getConnectedFaces();
}

inline void QHull3d::HEFace::reverse()
{
	HEEdge* edge3 = edge;
	HEEdge* edge1 = edge3->next;
	HEEdge* edge2 = edge1->next;

	HEVertex* v1 = edge2->vertex;
	HEVertex* v2 = edge3->vertex;
	HEVertex* v3 = edge1->vertex;

	edge3->vertex = v1;
	edge3->next = edge2;

	edge2->vertex = v3;
	edge2->next = edge1;

	edge1->vertex = v2;
	edge1->next = edge3;

	v1->edge = edge2;
	v2->edge = edge3;
	v3->edge = edge1;

	_n *= -1;
	_d *= -1;
}

inline void QHull3d::HEFace::updateSupportPlane()
{
	const gk::Point& v1 = edge->vertex->getPoint();
	const gk::Point& v2 = edge->next->vertex->getPoint();
	const gk::Point& v3 = edge->next->next->vertex->getPoint();

	_n = gk::Normalize(gk::Cross(gk::Vector(v1, v2), gk::Vector(v1, v3)));
	_d = -(v1.x * _n.x + v1.y * _n.y + v1.z * _n.z);
}

inline bool QHull3d::HEFace::tryAssignVertex(QHull3d::HEVertex* v)
{
	float d;

	if ((d = distance(v->getPoint())) < 0)
		return false;

	//if (d > _extremedistance)
	if (d >= _extremedistance)
	{
		vertices.insert(vertices.begin(), v);

		_extremedistance = d;
	}
	else
	{
		vertices.push_back(v);
	}

	return true;
}

inline std::vector<QHull3d::HEVertex*> QHull3d::HEFace::getBorderingVertices() const
{
	std::vector<QHull3d::HEVertex*> vertices;

	vertices.push_back(edge->vertex);
	vertices.push_back(edge->next->vertex);
	vertices.push_back(edge->next->next->vertex);

	return vertices;
}

inline std::vector<QHull3d::HEFace*> QHull3d::HEFace::getAdjacentFaces() const
{
	std::vector<HEFace*> adjacentfaces;

	if (edge->coedge)
		adjacentfaces.push_back(edge->coedge->face);
	if (edge->next->coedge)
		adjacentfaces.push_back(edge->next->coedge->face);
	if (edge->next->next->coedge)
		adjacentfaces.push_back(edge->next->next->coedge->face);

	return adjacentfaces;
}
inline std::vector<QHull3d::HEFace*> QHull3d::HEFace::getConnectedFaces() const
{
	std::vector<HEFace*> connectedfaces;
	std::unordered_map<HEFace*, HEFace*> faceregistry;

	getConnectedFaces(this, faceregistry);

	connectedfaces.reserve(faceregistry.size());
	for (auto it = faceregistry.begin(); it != faceregistry.end(); ++it)
		connectedfaces.push_back(it->second);

	return connectedfaces;
}
inline void QHull3d::HEFace::getConnectedFaces(const QHull3d::HEFace* face, std::unordered_map<QHull3d::HEFace*, QHull3d::HEFace*>& faceRegistry) const
{
	if (faceRegistry.count((HEFace*)face))
		return;

	faceRegistry[(HEFace*)face] = (HEFace*)face;

	std::vector<HEFace*> adjacentfaces = face->getAdjacentFaces();
	for (int i = 0; i < (int)adjacentfaces.size(); ++i)
		getConnectedFaces(adjacentfaces[i], faceRegistry);
}

inline QHull3d& QHull3d::operator=(QHull3d&& hull)
{
	if (this != &hull)
	{
		_iterationid = hull._iterationid;

		_points = hull._points;

		_vertices = std::move(hull._vertices);
		_edges = std::move(hull._edges);
		_faces = std::move(hull._faces);
		_processingfaces = std::move(hull._processingfaces);
		_hull = std::move(hull._hull);

		_points2d = std::move(hull._points2d);
		_hull2d = std::move(hull._hull2d);
	}

	return *this;
}

inline void QHull3d::clear()
{
	_hull2d.reset();
	_points2d.clear();

	_hull = nullptr;
	while (!_processingfaces.empty())
		_processingfaces.pop();

	_faces.clear();
	_edges.clear();
	_vertices.clear();

	_iterationid = -1;

	_points = nullptr;
}

inline QHull3d::HEEdge* QHull3d::createEdge()
{
	HEEdge* edge = new HEEdge();

	static int edgeid = 1;
	edge->id = edgeid++;

	_edges.push_back(std::unique_ptr<HEEdge>(edge));

	return edge;
}
inline QHull3d::HEFace* QHull3d::createFace()
{
	HEFace* face = new HEFace();

	static int faceid = 1;
	face->id = faceid++;

	_faces.push_back(std::unique_ptr<HEFace>(face));

	return face;
}
inline QHull3d::HEFace* QHull3d::createFace(int v1idx, int v2idx, int v3idx)
{
	// Build the mesh
	HEFace* face = createFace();

	HEEdge* edge1 = createEdge();
	HEEdge* edge2 = createEdge();
	HEEdge* edge3 = createEdge();

	HEVertex* v1 = _vertices[v1idx].get();
	HEVertex* v2 = _vertices[v2idx].get();
	HEVertex* v3 = _vertices[v3idx].get();

	edge1->vertex = v3;
	edge1->next = edge2;
	edge1->face = face;

	edge2->vertex = v1;
	edge2->next = edge3;
	edge2->face = face;

	edge3->vertex = v2;
	edge3->next = edge1;
	edge3->face = face;

	v1->edge = edge3;
	v2->edge = edge1;
	v3->edge = edge2;

	face->edge = edge3;

	// Compute the support <N,D> plane
	face->updateSupportPlane();

	return face;
}

inline std::vector<QHull3d::HEFace*> QHull3d::extrudeIn(const std::vector<QHull3d::HEEdge*>& loop, int vidx)
{
	std::vector<HEFace*> faces;

	HEEdge* lastedge1 = nullptr;
	HEEdge* firstedge2 = nullptr;

	HEVertex* v3 = _vertices[vidx].get();

	faces.reserve(loop.size());

	for (int i = 0; i < (int)loop.size(); ++i)
	{
		HEEdge* edge = loop[i];

		// Build the mesh
		HEFace* face = createFace();

		HEEdge* edge1 = createEdge();
		HEEdge* edge2 = createEdge();
		HEEdge* edge3 = createEdge();

		HEVertex* v1 = edge->next->next->vertex;
		HEVertex* v2 = edge->vertex;

		edge1->vertex = v3;
		edge1->next = edge2;
		edge1->face = face;

		edge2->vertex = v1;
		edge2->next = edge3;
		edge2->face = face;

		edge3->vertex = v2;
		edge3->next = edge1;
		edge3->face = face;

		if (!v3->edge)
			v3->edge = edge2;

		face->edge = edge3;

		// Compute the support <N,D> plane
		face->updateSupportPlane();

		// Sew adjacent faces
		edge3->coedge = edge->coedge;
		edge->coedge->coedge = edge3;

		if (lastedge1)
		{
			lastedge1->coedge = edge2;
			edge2->coedge = lastedge1;
		}
		lastedge1 = edge1;

		if (!firstedge2)
			firstedge2 = edge2;

		faces.push_back(face);
	}

	// Sew first and last faces
	lastedge1->coedge = firstedge2;
	firstedge2->coedge = lastedge1;

	return faces;
}
inline std::vector<QHull3d::HEFace*> QHull3d::extrudeOut(QHull3d::HEFace* face, int vidx)
{
	return extrudeOut({ face->edge, face->edge->next, face->edge->next->next }, vidx);
}
inline std::vector<QHull3d::HEFace*> QHull3d::extrudeOut(const std::vector<QHull3d::HEEdge*>& loop, int vidx)
{
	std::vector<HEFace*> faces;

	HEEdge* lastedge2 = nullptr;
	HEEdge* firstedge1 = nullptr;

	HEVertex* v3 = _vertices[vidx].get();

	faces.reserve(loop.size());

	for (int i = 0; i < (int)loop.size(); ++i)
	{
		HEEdge* edge = loop[i];

		assert(!edge->coedge && "Specified edge already has a co-face");

		// Build the mesh
		HEFace* face = createFace();

		HEEdge* edge1 = createEdge();
		HEEdge* edge2 = createEdge();
		HEEdge* edge3 = createEdge();

		HEVertex* v1 = edge->vertex;
		HEVertex* v2 = edge->next->next->vertex;

		edge1->vertex = v3;
		edge1->next = edge2;
		edge1->face = face;

		edge2->vertex = v1;
		edge2->next = edge3;
		edge2->face = face;

		edge3->vertex = v2;
		edge3->next = edge1;
		edge3->face = face;

		if (!v3->edge)
			v3->edge = edge2;

		face->edge = edge3;

		// Compute the support <N,D> plane
		face->updateSupportPlane();

		// Sew adjacent faces
		edge->coedge = edge3;
		edge3->coedge = edge;

		if (lastedge2)
		{
			lastedge2->coedge = edge1;
			edge1->coedge = lastedge2;
		}
		lastedge2 = edge2;

		if (!firstedge1)
			firstedge1 = edge1;

		faces.push_back(face);
	}

	// Sew first and last faces
	lastedge2->coedge = firstedge1;
	firstedge1->coedge = lastedge2;

	return faces;
}

inline int QHull3d::build()
{
	// Coplanarity case
	if (_hull2d)
		return _hull2d->build();

	while (iterate());

	return _iterationid + 1;
}
inline bool QHull3d::iterate()
{
	HEFace* face;

	// Coplanarity case
	if (_hull2d)
		return _hull2d->iterate();

	face = nullptr;

	// Get the next non-empty face to process
	while (face == nullptr || face->vertices.empty())
	{
		if (_processingfaces.empty())
			return false;

		face = _processingfaces.top();
		_processingfaces.pop();
	}

	// Tag the face as visible with the current iteration identifier and store it into the visible set
	std::vector<HEFace*> visiblefaces;

	face->iterationid = ++_iterationid;
	visiblefaces.push_back(face);

	// Pop extreme vertex
	_hull = face->vertices[0];
	face->vertices.erase(face->vertices.begin());

	// Get all unvisited connected faces visible from the current face's extreme point
	getVisibleUnvisitedConnectedFaces(_iterationid, visiblefaces[0], _hull->getPoint(), visiblefaces);

	// Get horizon edges within the visible face set
	std::vector<QHull3d::HEEdge*> horizoneedgeloop = getHorizonEdgeLoop(_iterationid, visiblefaces);

	// Extrude the horizon to the extreme point
	std::vector<HEFace*> newfaces = extrudeIn(horizoneedgeloop, _hull->index);

	// Assign the old visible faces remaining points to the new faces
	for (int of = 0; of < (int)visiblefaces.size(); ++of)
	{
		HEFace* oldface = visiblefaces[of];

		for (int v = 0; v < (int)oldface->vertices.size(); ++v)
		{
			HEVertex* vertex = oldface->vertices[v];
			for (int nf = 0; nf < (int)newfaces.size(); ++nf)
				if (newfaces[nf]->tryAssignVertex(vertex))
					break;
		}

		oldface->vertices.clear();
	}

	// Push the new created faces on the processing stack
	for (int i = 0; i < (int)newfaces.size(); ++i)
		_processingfaces.push(newfaces[i]);

	//////////////////////////////////////////////////////////////////////////
	// ToDo JRA: Remove this test code

	assertManifoldValidity(_hull);

	return true;
}

inline void QHull3d::getVisibleUnvisitedConnectedFaces(const int iterationId, const HEFace* face, const gk::Point& p, std::vector<HEFace*>& visiblefaces)
{
	std::vector<HEFace*> adjacentfaces = face->getAdjacentFaces();

	for (int i = 0; i < (int)adjacentfaces.size(); ++i)
	{
		HEFace* adjacentface = adjacentfaces[i];

		//if (adjacentface->iterationid != iterationId && adjacentface->distance(p) > 0)
		if (adjacentface->iterationid != iterationId && adjacentface->distance(p) >= 0)
		{
			adjacentface->iterationid = iterationId;
			visiblefaces.push_back(adjacentface);

			getVisibleUnvisitedConnectedFaces(iterationId, adjacentface, p, visiblefaces);
		}
	}
}
inline std::vector<QHull3d::HEEdge*> QHull3d::getHorizonEdgeLoop( const int iterationId, const std::vector<HEFace*>& visiblefaces) const
{
	std::vector<QHull3d::HEEdge*> horizonedgeloop;

	// Get a first horizon edge
	HEEdge* starthorizonedge = nullptr;

	for (int f = 0; f < (int)visiblefaces.size(); ++f)
	{
		HEEdge* edge = visiblefaces[f]->edge;
		for (int i = 0; i < 3; ++i)
		{
			if (edge->coedge->face->iterationid != iterationId)
			{
				starthorizonedge = edge;

				break;
			}

			edge = edge->next;
		}

		if (starthorizonedge)
			break;
	}

	// Complete the loop
	HEEdge* nexthorizonedge;
	HEEdge* currenthorizonedge = starthorizonedge;

	horizonedgeloop.push_back(starthorizonedge);

	while ((nexthorizonedge = getNextHorizonEdge(iterationId, currenthorizonedge)) != starthorizonedge)
	{
		horizonedgeloop.push_back(nexthorizonedge);

		currenthorizonedge = nexthorizonedge;
	}

	return horizonedgeloop;
}
inline QHull3d::HEEdge* QHull3d::getNextHorizonEdge(const int iterationId, HEEdge* horizonedge) const
{
	// Turn around the specified edge's target vertex until the bordered face is visible
	horizonedge = horizonedge->coedge;
	while (horizonedge->face->iterationid != iterationId)
		horizonedge = horizonedge->next->next->coedge;

	return horizonedge;
}

#endif