#ifndef GLVIEWER_H
#define GLVIEWER_H

#include "gl_camera.h"
#include "gkit_utils.h"
#include "qhull.h"

#include <App.h>
#include <Geometry.h>
#include <GL/GLProgram.h>
#include <GL/GLQuery.h>
#include <GL/GLVertexArray.h>
#include <Widgets/nvSDLContext.h>

#include <vector>

using namespace gk;

class GLViewer : public App
{
private:

	//! Current shader.
	GLProgram* _program;
	//! Timer.
	GLCounter* _timer;

	//! UI system.
	nv::SdlContext _widgets;

	//! Data.
	std::vector<Point> _points;
	QHull _qhull;

	//! Options
	bool _restoreprevioussession;

	//! Camera.
	GLCamera _camera;

	/************************************************************************/
	/*							GL resources                                */
	/************************************************************************/

	GLVertexBufferSet* _unitcubegl;			//! GL geometry: Unit cube.
	GLVertexBufferSet* _axisgl;				//! GL geometry: Axis.
	GLVertexBufferSet* _pointsgl;			//! GL geometry: Point cloud.

	GLVertexBufferSet* _hullglvertices;			//! GL geometry: Convex hull points.
	GLVertexBufferSet* _hullglfaces;			//! GL geometry: Convex hull faces.
	GLVertexBufferSet* _hullgledges;			//! GL geometry: Convex hull edges.
	GLVertexBufferSet* _hullglfacesextremes;	//! GL geometry: Convex hull faces extreme points.

public:

	GLViewer(const GLCamera& camera, int windowwidth, int windowheight, bool restoreprevioussession);

	int init();
	int quit();
	int draw();

	void processWindowResize(SDL_WindowEvent& event);
	void processMouseButtonEvent(SDL_MouseButtonEvent& event);
	void processMouseMotionEvent(SDL_MouseMotionEvent& event);
	void processKeyboardEvent(SDL_KeyboardEvent& event);

private:

	void reset();

	std::vector<Point> loadRandomPoints(int count);
	std::vector<Point> loadPoints(const std::string& filename);
	void savePoints(std::vector<Point>& points, const std::string& filename);

	void initGLGeometry();
	void createGLGeometry();
	void updateGLGeometry();
	void destroyGLGeometry();
};

#endif