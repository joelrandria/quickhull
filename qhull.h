#ifndef QHULL_H
#define QHULL_H

#include <App.h>
#include <GL/GLProgram.h>
#include <GL/GLQuery.h>
#include <Widgets/nvSDLContext.h>

using namespace gk;

class qhull : public App
{
private:

	//! Current shader used for rendering.
	gk::GLProgram* _program;
	//! Timer.
	gk::GLCounter* _timer;

	//! UI system.
	nv::SdlContext _widgets;

public:

	qhull();

	int init();
	int quit();
	int draw();
};

#endif