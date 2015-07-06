#ifndef GLVIEWER_H
#define GLVIEWER_H

#include "gl_camera.h"
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
	bool _restoreprevioussession;
	std::vector<Point> _points;
	QHull _qhull;

	//! GL Camera.
	GLCamera _camera;

	/************************************************************************/
	/*							GL resources                                */
	/************************************************************************/

	//! GL geometry: Unit cube.
	gk::GLVertexArray* _unitcubevao;
	unsigned int _unitcubeindexcount;

	//! GL geometry: Axis.
	gk::GLVertexArray* _axisvao;
	unsigned int _axisindexcount;

	//! GL geometry: Point cloud.
	gk::GLVertexArray* _pointsvao;
	unsigned int _pointsindexcount;

	//! GL geometry: Extreme points.
	gk::GLVertexArray* _epvao;
	unsigned int _epindexcount;

	//! GL geometry: Convex hull.
	gk::GLVertexArray* _hullvao;
	unsigned int _hullindexcount;

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

	void loadRandomPoints(std::vector<Point>& points, int count);
	void loadPoints(std::vector<Point>& points, const std::string& filename);
	void savePoints(std::vector<Point>& points, const std::string& filename);

	void initGLGeometry();
	void updateGLGeometry();
	void createGLGeometry();
	void destroyGLGeometry();

	void drawUnitCube();
	void drawAxis();
	void drawPoints();
	void drawExtremePoints();
	void drawHull();
};

#endif