#ifndef QHULL_H
#define QHULL_H

#include "gl_camera.h"

#include <App.h>
#include <Geometry.h>
#include <GL/GLProgram.h>
#include <GL/GLQuery.h>
#include <GL/GLVertexArray.h>
#include <Widgets/nvSDLContext.h>

#include <vector>

using namespace gk;

class qhull : public App
{
private:

	//! Current shader.
	GLProgram* _program;
	//! Timer.
	GLCounter* _timer;

	//! UI system.
	nv::SdlContext _widgets;

	//! Point cloud.
	std::vector<Point> _points;
	gk::GLVertexArray* _pointsvao;
	unsigned int _pointsindexcount;

	//! Camera.
	GLCamera _camera;

public:

	qhull(const std::vector<Point>& points, const GLCamera& camera, int windowwidth, int windowheight);

	std::vector<Point>& points();

	int init();
	int quit();
	int draw();

	void processWindowResize(SDL_WindowEvent& event);
	void processMouseButtonEvent(SDL_MouseButtonEvent& event);
	void processMouseMotionEvent(SDL_MouseMotionEvent& event);
	void processKeyboardEvent(SDL_KeyboardEvent& event);

private:

	void initGLGeometry();
	void updateGLGeometry();
	void createGLGeometry();
	void destroyGLGeometry();
};

#endif