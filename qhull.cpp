#include "qhull.h"

#include <ProgramManager.h>
#include <GL/GLTexture.h>
#include <GL/GLBuffer.h>

#include <numeric>

qhull::qhull(const std::vector<Point>& points, const GLCamera& camera, int windowwidth, int windowheight)
	:_points(std::move(points)),
	_camera(camera)
{
	AppSettings settings;
	settings.setGLVersion(3, 3);
	settings.setGLCoreProfile();
	settings.setGLDebugContext();

	if (createWindow(windowwidth, windowheight, settings) < 0)
		closeWindow();

	_widgets.init();
	_widgets.reshape(windowWidth(), windowHeight());
}

int qhull::init()
{
	_program = gk::createProgram("basic.glsl");
	if (_program == gk::GLProgram::null())
		return -1;

	initGLGeometry();
	updateGLGeometry();

	_timer = gk::createTimer();

	return 0;
}
void qhull::initGLGeometry()
{
	_pointsvao = 0;
}
void qhull::updateGLGeometry()
{
	destroyGLGeometry();
	createGLGeometry();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void qhull::createGLGeometry()
{
	std::vector<gk::Vec4> colors;
	std::vector<unsigned int> indices;

	// Point cloud
	_pointsvao = gk::createVertexArray();

	gk::createBuffer(GL_ARRAY_BUFFER, _points);
	glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_position"));

	colors.resize(_points.size(), gk::Vec4(1, 1, 1, 1));
	gk::createBuffer(GL_ARRAY_BUFFER, colors);
	glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_color"));

	indices.resize(_points.size());
	std::iota(indices.begin(), indices.end(), 0);
	_pointsindexcount = _points.size();
	gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
}
void qhull::destroyGLGeometry()
{
	if (_pointsvao)
		_pointsvao->release();

	initGLGeometry();
}

int qhull::quit()
{
	return 0;
}
int qhull::draw()
{
	// Keyboard
	if (key(SDLK_ESCAPE))
		closeWindow();
	if (key('r'))
	{
		key('r') = 0;
		reloadPrograms();
	}
	if (key('c'))
	{
		key('c') = 0;
		gk::writeFramebuffer("screenshot.png");
	}

	if (key(SDLK_z))
	{
		key(SDLK_z) = 0;
		_camera.localTranslate(gk::Vector(0, 0, -_camera.moveUnit));
	}
	if (key(SDLK_s))
	{
		key(SDLK_s) = 0;
		_camera.localTranslate(gk::Vector(0, 0, _camera.moveUnit));
	}
	if (key(SDLK_q))
	{
		key(SDLK_q) = 0;
		_camera.localTranslate(gk::Vector(-_camera.moveUnit, 0, 0));
	}
	if (key(SDLK_d))
	{
		key(SDLK_d) = 0;
		_camera.localTranslate(gk::Vector(_camera.moveUnit, 0, 0));
	}
	if (key(SDLK_PAGEUP))
	{
		key(SDLK_PAGEUP) = 0;
		_camera.localTranslate(gk::Vector(0, _camera.moveUnit, 0));
	}
	if (key(SDLK_PAGEDOWN))
	{
		key(SDLK_PAGEDOWN) = 0;
		_camera.localTranslate(gk::Vector(0, -_camera.moveUnit, 0));
	}

	// 3D geometry
	glViewport(0, 0, windowWidth(), windowHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_timer->start();

	glUseProgram(_program->name);

	// Draw pass #0: Point cloud
	glPointSize(2);

	_program->uniform("mvpMatrix") = (_camera.projectionTransform() * _camera.viewTransform()).matrix();

	glBindVertexArray(_pointsvao->name);
	glDrawElements(GL_POINTS, _pointsindexcount, GL_UNSIGNED_INT, 0);

	//	GL cleaning
	glUseProgram(0);
	glBindVertexArray(0);

	_timer->stop();

	// UI
	{
		_widgets.begin();
		_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);

		_widgets.doLabel(nv::Rect(), _timer->summary("draw").c_str());

		_widgets.endGroup();
		_widgets.end();
	}

	present();

	return 1;
}

void qhull::processWindowResize(SDL_WindowEvent& event)
{
	_widgets.reshape(event.data1, event.data2);
}
void qhull::processMouseButtonEvent(SDL_MouseButtonEvent& event)
{
	_widgets.processMouseButtonEvent(event);

	if (event.state == SDL_RELEASED && event.button == SDL_BUTTON_RIGHT)
	{
		// 		_patch.debug = true;
		// 		raycast(event.x, windowHeight() - event.y);		
		// 		_patch.debug = false;
	}
}
void qhull::processMouseMotionEvent(SDL_MouseMotionEvent& event)
{
	_widgets.processMouseMotionEvent(event);

	if (event.state & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		_camera.yaw(-event.xrel);
		_camera.pitch(-event.yrel);
	}
}
void qhull::processKeyboardEvent(SDL_KeyboardEvent& event)
{
	_widgets.processKeyboardEvent(event);
}