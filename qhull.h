#ifndef QHULL_H
#define QHULL_H

#include <Geometry.h>

#include <stack>
#include <vector>
#include <memory>

//////////////////////////////////////////////////////////////////////////
// ToDo JRA: Remove this test code

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////

//! Quick hull algorithm implementation for convex hull.
//! http://www.cise.ufl.edu/~ungor/courses/fall06/papers/QuickHull.pdf
class QHull
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

		const gk::Point& point() const { return (*_points)[index]; }
	};

	//! Half-edge.
	class HEEdge
	{
	public:

		HEVertex* vertex;	//! Vertex at the end of the half-edge

		HEEdge* pair;		//! Oppositely oriented adjacent half-edge
		HEEdge* next;		//! Next half-edge around the face

		HEFace* face;		//! Face the half-edge borders

		HEEdge() : vertex(nullptr), pair(nullptr), next(nullptr), face(nullptr) {}
	};

	//! Half-edge face.
	class HEFace
	{
	private:

		gk::Vector _n;		//! Face normal
		float _d;			//! Signed distance to the origin

	public:

		HEEdge* edge;	//! One of the half-edges bordering the face

		HEFace() : edge(nullptr), _d(0.f) {}

		//! Get adjacent faces.
		std::vector<HEFace*> getAdjacentFaces() const;

		//! Reverse the face orientation.
		void reverse();

		//! Update support plane's internal data.
		void updateSupportPlane();

		//! Get the signed orthogonal distance to the specified point, according to the normal direction.
		float distance(const gk::Point& p) const { return _n.x * p.x + _n.y * p.y + _n.z * p.z + _d; }
	};

	/************************************************************************/
	/*								Internals								*/
	/************************************************************************/

	//! Input points.
	const std::vector<gk::Point>* _points;

	//! Global vertex set.
	std::vector<std::unique_ptr<HEVertex>> _vertices;
	//! Global edge set.
	std::vector<std::unique_ptr<HEEdge>> _edges;
	//! Global face set.
	std::vector<std::unique_ptr<HEFace>> _faces;

	//! Faces currently processed.
	std::stack<HEFace*> _pendingfaces;

	//! Computed hull.
	std::vector<HEFace*> _hull;

	//////////////////////////////////////////////////////////////////////////
	// ToDo JRA: Remove this test code

public:

	int apexidx;

	//////////////////////////////////////////////////////////////////////////

public:

	QHull();
	QHull(const std::vector<gk::Point>* points);
	QHull(QHull&& hull);

	QHull& operator=(QHull&& hull);

	//! Compute one iteration.
	void iterate();

	//! Get all the faces making up the convex hull.
	std::vector<Face> hull() const;

private:

	//! Initialize internal data and initial convex hull.
	void initialize();
	//! Build internal vertices from input points.
	void createVertices();
	//! Build initial tetrahedron.
	void createInitialTetrahedron();

	//! Create a new managed edge.
	HEEdge* createEdge();
	//! Create a new managed face.
	HEFace* createFace();
	//! Create a new managed face bordered by the specified vertices.
	//! Vertices are assumed to be specified in clockwise order according to the underlying surface.
	HEFace* createFace(int v1idx, int v2idx, int v3idx);

	//! Create new faces (fan configuration) by extruding the specified edge loop toward the specified vertex.
	//! The target vertex is assumed to be in the specified face's negative half-space.
	std::vector<HEFace*> extrudeOut(HEFace* face, int vidx);
	//! Create new faces (fan configuration) by extruding the specified edge loop toward the specified vertex.
	//! The target vertex is assumed to be in the specified edge loop's negative half-space.
	//! The specified edge loop is assumed to be valid and clockwise oriented.
	std::vector<HEFace*> extrudeOut(const std::vector<HEEdge*>& loop, int vidx);

	//////////////////////////////////////////////////////////////////////////
	// ToDo JRA: Remove this test code

	std::vector<HEFace*> getConnectedFaces(const HEFace* face) const;
	void getConnectedFaces(const HEFace* face, std::unordered_map<HEFace*, HEFace*>& faceRegistry) const;

	void checkManifold(const HEFace* face) const;
};

inline QHull::HEEdge* QHull::createEdge()
{
	HEEdge* edge = new HEEdge();

	_edges.push_back(std::unique_ptr<HEEdge>(edge));

	return edge;
}
inline QHull::HEFace* QHull::createFace()
{
	HEFace* face = new HEFace();

	_faces.push_back(std::unique_ptr<HEFace>(face));

	return face;
}
inline QHull::HEFace* QHull::createFace(int v1idx, int v2idx, int v3idx)
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

inline std::vector<QHull::HEFace*> QHull::extrudeOut(QHull::HEFace* face, int vidx)
{
	return extrudeOut({ face->edge, face->edge->next, face->edge->next->next }, vidx);
}
inline std::vector<QHull::HEFace*> QHull::extrudeOut(const std::vector<QHull::HEEdge*>& loop, int vidx)
{
	std::vector<HEFace*> faces;

	HEEdge* lastedge2 = nullptr;
	HEEdge* firstedge1 = nullptr;

	HEVertex* v3 = _vertices[vidx].get();

	faces.reserve(loop.size());

	for (int i = 0; i < (int)loop.size(); ++i)
	{
		HEEdge* edge = loop[i];

		assert(!edge->pair && "Specified edge already has a conjugate face");

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
		edge->pair = edge3;
		edge3->pair = edge;

		if (lastedge2)
		{
			lastedge2->pair = edge1;
			edge1->pair = lastedge2;
		}
		lastedge2 = edge2;

		if (!firstedge1)
			firstedge1 = edge1;

		faces.push_back(face);
	}

	// Sew first and last faces
	lastedge2->pair = firstedge1;
	firstedge1->pair = lastedge2;

	return faces;
}

inline std::vector<QHull::HEFace*> QHull::HEFace::getAdjacentFaces() const
{
	std::vector<HEFace*> adjacentfaces;

	if (edge->pair)
		adjacentfaces.push_back(edge->pair->face);
	if (edge->next->pair)
		adjacentfaces.push_back(edge->next->pair->face);
	if (edge->next->next->pair)
		adjacentfaces.push_back(edge->next->next->pair->face);

	return adjacentfaces;
}

inline void QHull::HEFace::reverse()
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

inline void QHull::HEFace::updateSupportPlane()
{
	const gk::Point& v1 = edge->vertex->point();
	const gk::Point& v2 = edge->next->vertex->point();
	const gk::Point& v3 = edge->next->next->vertex->point();

	_n = gk::Normalize(gk::Cross(gk::Vector(v1, v2), gk::Vector(v1, v3)));
	_d = -(v1.x * _n.x + v1.y * _n.y + v1.z * _n.z);
}

#endif