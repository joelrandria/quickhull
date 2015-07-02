#include "qhull.h"

#include <ProgramManager.h>
#include <GL/GLTexture.h>

#define WINDOW_WIDTH	512
#define WINDOW_HEIGHT	512

qhull::qhull()
{
	AppSettings settings;
	settings.setGLVersion(3, 3);
	settings.setGLCoreProfile();
	settings.setGLDebugContext();

	if (createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, settings) < 0)
		closeWindow();

	_widgets.init();
	_widgets.reshape(windowWidth(), windowHeight());
}

int qhull::init()
{
	_program = gk::createProgram("basic.glsl");
	if (_program == gk::GLProgram::null())
		return -1;

// 	initializeGLGeometry();
// 	updateGLGeometry();

	_timer = gk::createTimer();

	return 0;
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

	// 3D geometry
	glViewport(0, 0, windowWidth(), windowHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_timer->start();

	glUseProgram(_program->name);

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