#ifndef QHULL_H
#define QHULL_H

#include <Geometry.h>

#include <stack>
#include <vector>
#include <memory>
#include <stdexcept>

//! Quick hull algorithm implementation for convex hull.
//! http://www.cise.ufl.edu/~ungor/courses/fall06/papers/QuickHull.pdf
class QHull
{
public:

	//! Triangle face minimal representation.
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

		gk::Vector _n;		//! Face's normal
		float _d;			//! Signed distance to the origin

	public:

		HEEdge* edge;	//! One of the half-edges bordering the face

		HEFace() : edge(nullptr), _d(0.f) {}

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

	//! Get all the faces making up the geometry hull.
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

	//! Create a new managed face bordered by extruding the specified edge toward the specified vertex.
	//QHullFace* extrude(QHullEdge* adjacentedge, int vidx);
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

// inline QHull::QHullFace* QHull::extrude(QHullEdge* adjacentedge, int vidx)
// {
// 	if (adjacentedge->pair)
// 		throw std::logic_error("QHull::extrude(): A face already exists at the specified location");
// 
// 	// Build the mesh
// 	QHullFace* face = createFace();
// 
// 	QHullEdge* edge1 = createEdge();
// 	QHullEdge* edge2 = createEdge();
// 	QHullEdge* edge3 = createEdge();
// 
// 	QHullVertex* v1 = adjacentedge->vertex;
// 	QHullVertex* v2 = adjacentedge->next->next->vertex;
// 	QHullVertex* v3 = _vertices[vidx].get();
// 
// 	edge1->vertex = v3;
// 	edge1->next = edge2;
// 	edge1->face = face;
// 
// 	edge2->vertex = v1;
// 	edge2->next = edge3;
// 	edge2->face = face;
// 
// 	edge3->vertex = v2;
// 	edge3->next = edge1;
// 	edge3->face = face;
// 
// 	v3->edge = edge2;
// 
// 	adjacentedge->pair = edge3;
// 	edge3->pair = adjacentedge;
// 
// 	face->edge = edge3;
// 
// 	// Compute the support <N,D> plane
// 	face->updateSupportPlane();
// 
// 	return face;
// }

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