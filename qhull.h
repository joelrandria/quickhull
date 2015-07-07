#ifndef QHULL_H
#define QHULL_H

#include <Geometry.h>

#include <stack>
#include <vector>
#include <memory>

//! Quick hull algorithm implementation for convex hull.
//! http://www.cise.ufl.edu/~ungor/courses/fall06/papers/QuickHull.pdf
class QHull
{
private:

	/************************************************************************/
	/*						Half-edge data types								*/
	/************************************************************************/

	class QHullVertex;
	class QHullEdge;
	class QHullFace;

	//! Half-edge vertex.
	class QHullVertex
	{
	private:

		//! Global point set.
		const std::vector<gk::Point>* _points;

	public:

		int index;			//! Point list index

		QHullEdge* edge;	//! One of the half-edges emanating from the vertex

		QHullVertex(const std::vector<gk::Point>* points) : _points(points), index(-1), edge(nullptr) {}

		const gk::Point& point() const { return (*_points)[index]; }
	};

	//! Half-edge.
	class QHullEdge
	{
	public:

		QHullVertex* vertex;	//! Vertex at the end of the half-edge

		QHullEdge* pair;		//! Oppositely oriented adjacent half-edge
		QHullEdge* next;		//! Next half-edge around the face

		QHullFace* face;		//! Face the half-edge borders

		QHullEdge() : vertex(nullptr), pair(nullptr), next(nullptr), face(nullptr) {}
	};

	//! Half-edge face.
	class QHullFace
	{
	private:

		gk::Vector _n;		//! Face's normal
		float _d;			//! Signed distance to the origin

	public:

		QHullEdge* edge;	//! One of the half-edges bordering the face

		QHullFace() : edge(nullptr), _d(0.f) {}

		//! Reverse the face direction.
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
	std::vector<std::unique_ptr<QHullVertex>> _vertices;
	//! Global edge set.
	std::vector<std::unique_ptr<QHullEdge>> _edges;
	//! Global face set.
	std::vector<std::unique_ptr<QHullFace>> _faces;

	//! Faces currently processed.
	std::stack<QHullFace*> _pendingfaces;

	//! Computed hull.
	std::vector<QHullFace*> _hull;

	//////////////////////////////////////////////////////////////////////////
	// ToDo JRA: Remove this test code

public:
	
	int epidx[6];
	int hullidx[4];

	//////////////////////////////////////////////////////////////////////////

public:

	QHull();
	QHull(const std::vector<gk::Point>* points);
	QHull(QHull&& hull);

	QHull& operator=(QHull&& hull);

	//! Compute one iteration.
	void iterate();

private:

	//! Initialize internal data and initial convex hull.
	void initialize();
	//! Build internal vertices from input points.
	void createVertices();
	//! Build initial tetrahedron.
	void createStartTetrahedron();

	//! Create a new managed edge.
	QHullEdge* createEdge();
	//! Create a new managed face.
	QHullFace* createFace();
	//! Create a new managed face bordered by the specified vertices.
	//! Vertices are assumed to be specified in clockwise order according to the underlying surface.
	QHullFace* createFace(int v1idx, int v2idx, int v3idx);
	//! Create a new managed face bordered by the specified vertex and adjacent edge.
	QHullFace* createFace(QHullEdge* adjacentedge, int vidx);
};

inline QHull::QHullEdge* QHull::createEdge()
{
	QHullEdge* edge = new QHullEdge();

	_edges.push_back(std::unique_ptr<QHullEdge>(edge));

	return edge;
}
inline QHull::QHullFace* QHull::createFace()
{
	QHullFace* face = new QHullFace();

	_faces.push_back(std::unique_ptr<QHullFace>(face));

	return face;
}
inline QHull::QHullFace* QHull::createFace(int v1idx, int v2idx, int v3idx)
{
	// Build the mesh
	QHullFace* face = createFace();

	QHullEdge* edge1 = createEdge();
	QHullEdge* edge2 = createEdge();
	QHullEdge* edge3 = createEdge();

	QHullVertex* v1 = _vertices[v1idx].get();
	QHullVertex* v2 = _vertices[v2idx].get();
	QHullVertex* v3 = _vertices[v3idx].get();

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
inline QHull::QHullFace* QHull::createFace(QHullEdge* adjacentedge, int vidx)
{
	// Build the mesh
	QHullFace* face = createFace();

	QHullEdge* edge1 = createEdge();
	QHullEdge* edge2 = createEdge();
	QHullEdge* edge3 = createEdge();

	QHullVertex* v1 = adjacentedge->vertex;
	QHullVertex* v2 = adjacentedge->next->next->vertex;
	QHullVertex* v3 = _vertices[vidx].get();

	edge1->vertex = v3;
	edge1->next = edge2;
	edge1->face = face;

	edge2->vertex = v1;
	edge2->next = edge3;
	edge2->face = face;

	edge3->vertex = v2;
	edge3->next = edge1;
	edge3->face = face;

	v3->edge = edge2;

	adjacentedge->pair = edge3;

	face->edge = edge3;

	// Compute the support <N,D> plane
	face->updateSupportPlane();

	return face;
}

inline void QHull::QHullFace::reverse()
{
	QHullEdge* edge3 = edge;
	QHullEdge* edge1 = edge3->next;
	QHullEdge* edge2 = edge1->next;

	QHullVertex* v1 = edge2->vertex;
	QHullVertex* v2 = edge3->vertex;
	QHullVertex* v3 = edge1->vertex;

	edge3->vertex = v1;
	edge3->next = edge2;

	edge2->vertex = v3;
	edge2->next = edge1;

	edge1->vertex = v2;
	edge1->next = edge3;
	
	v1->edge = edge2;
	v2->edge = edge3;
	v3->edge = edge1;
}

inline void QHull::QHullFace::updateSupportPlane()
{
	const gk::Point& v1 = edge->vertex->point();
	const gk::Point& v2 = edge->next->vertex->point();
	const gk::Point& v3 = edge->next->next->vertex->point();

	_n = gk::Normalize(gk::Cross(gk::Vector(v1, v2), gk::Vector(v1, v3)));
	_d = -(v1.x * _n.x + v1.y * _n.y + v1.z * _n.z);
}

#endif