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
	bool _restoreprevioussession;
	std::vector<Point> _points;
	QHull _qhull;

	//! GL Camera.
	GLCamera _camera;

	/************************************************************************/
	/*							GL resources                                */
	/************************************************************************/

	GLVertexBufferSet* _unitcubegl;		//! GL geometry: Unit cube.
	GLVertexBufferSet* _axisgl;			//! GL geometry: Axis.
	GLVertexBufferSet* _pointsgl;		//! GL geometry: Point cloud.
	GLVertexBufferSet* _epgl;			//! GL geometry: Extreme points.
	GLVertexBufferSet* _hullgl;			//! GL geometry: Convex hull.

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
	void createGLGeometry();
	void updateGLGeometry();
	void destroyGLGeometry();

	void drawUnitCube();
	void drawAxis();
	void drawPoints();
	void drawExtremePoints();
	void drawHull();
};

#endif