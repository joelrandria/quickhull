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
	/*							Geometry utilities							*/
	/************************************************************************/

	//! Defines a line via 2 distinct points.
	class Line
	{
	private:

		const gk::Point _a;
		const gk::Point _b;

		gk::Vector _dir;

	public:

		Line(const gk::Point& a = gk::Point(), const gk::Point& b = gk::Point());

		float squareddistance(const gk::Point& p) const;
	};

	//! Define an <N,D> plane via 3 distinct points.
	class Plane
	{
	private:

		const gk::Point _a;
		const gk::Point _b;
		const gk::Point _c;

		gk::Vector _n;
		float _d;

	public:

		Plane(const gk::Point& a = gk::Point(), const gk::Point& b = gk::Point(), const gk::Point& c = gk::Point());

		float distance(const gk::Point& p) const;
	};

	/************************************************************************/
	/*                      Half-edge data structures                       */
	/************************************************************************/

	//! Half-edge vertex.
	typedef struct
	{
		//! Input point list index.
		int index;

		struct HEEdge* edge;
	} HEVertex;

	//! Half-edge.
	typedef struct
	{
		struct HEVertex* vertex;
		struct HEEdge* pair;
		struct HEFace* face;
		struct HEEdge* next;
	} HEEdge;

	//! Half-edge face.
	typedef struct
	{
		struct HEEdge* edge;
	} HEFace;

	/************************************************************************/
	/*								Internals								*/
	/************************************************************************/

	//! Input points.
	const std::vector<gk::Point>* _inputpoints;

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

	int epindices[6];
	int hullindices[4];

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
	//! Build start tetrahedron.
	void createStartTetrahedron();
};

inline QHull::Line::Line(const gk::Point& a, const gk::Point& b)
	: _a(a), _b(b), _dir(gk::Normalize(gk::Vector(_a, _b)))
{
}
inline float QHull::Line::squareddistance(const gk::Point& p) const
{
	gk::Vector ap(_a, p);
	float pprojlength = gk::Dot(ap, _dir);

	return ap.LengthSquared() - (pprojlength * pprojlength);
}

inline QHull::Plane::Plane(const gk::Point& a, const gk::Point& b, const gk::Point& c)
	: _a(a), _b(b), _c(c)
{
	_n = gk::Normalize(gk::Cross(gk::Vector(_a, _b), gk::Vector(_a, _c)));
	_d = -(_a.x * _n.x + _a.y * _n.y + _a.z * _n.z);
}
inline float QHull::Plane::distance(const gk::Point& p) const
{
	return fabs(_n.x * p.x + _n.y * p.y + _n.z * p.z + _d);
}

#endif