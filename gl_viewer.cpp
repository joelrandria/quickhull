#include "gl_viewer.h"

#include <ProgramManager.h>
#include <GL/GLTexture.h>
#include <GL/GLBuffer.h>

#include <numeric>
#include <fstream>

#define DEFAULT_POINT_COUNT		16
#define DEFAULT_POINT_FILENAME	"points.txt"

GLViewer::GLViewer(const GLCamera& camera, int windowwidth, int windowheight, bool restoreprevioussession)
	: _camera(camera),
	_restoreprevioussession(restoreprevioussession)
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

int GLViewer::init()
{
	_program = gk::createProgram("basic.glsl");
	if (_program == gk::GLProgram::null())
		return -1;

	initGLGeometry();

	reset();

	_timer = gk::createTimer();

	return 0;
}
void GLViewer::reset()
{
	// Load points
	_points.clear();
	if (_restoreprevioussession)
	{
		loadPoints(_points, DEFAULT_POINT_FILENAME);
	}
	else
	{
		loadRandomPoints(_points, DEFAULT_POINT_COUNT);
		savePoints(_points, DEFAULT_POINT_FILENAME);
	}

	// Build convex hull
	_qhull = std::move(QHull(&_points));

	// Create GL geometry
	updateGLGeometry();
}
int GLViewer::quit()
{
	return 0;
}

void GLViewer::loadRandomPoints(std::vector<Point>& points, int count)
{
	Point p;

	points.reserve(count);

	while (count-- > 0)
	{
		p.x = (float)rand() / RAND_MAX;
		p.y = (float)rand() / RAND_MAX;
		p.z = (float)rand() / RAND_MAX;

		points.push_back(p);
	}
}
void GLViewer::loadPoints(std::vector<Point>& points, const std::string& filename)
{
	std::ifstream file(filename);

	int i = 0;

	while (!file.eof())
	{
		Point p;

		file >> p.x;
		file >> p.y;
		file >> p.z;

		points.push_back(p);

		i++;
	}

	file.close();
}
void GLViewer::savePoints(std::vector<Point>& points, const std::string& filename)
{
	std::ofstream file(filename);

	for (int i = 0; i < (int)points.size(); ++i)
	{
		Point p = points[i];

		file << p.x << " ";
		file << p.y << " ";
		file << p.z;

		if (i < ((int)points.size() - 1))
			file << std::endl;
	}

	file.close();
}

void GLViewer::initGLGeometry()
{
	_axisvao = 0;
	_axisindexcount = 0;

	_hullvao = 0;
	_hullindexcount = 0;

	_pointsvao = 0;
	_pointsindexcount = 0;

	_unitcubevao = 0;
	_unitcubeindexcount = 0;

	_epvao = 0;
	_epindexcount = 0;

}
void GLViewer::updateGLGeometry()
{
	destroyGLGeometry();
	createGLGeometry();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void GLViewer::createGLGeometry()
{
	std::vector<gk::Point> positions;
	std::vector<gk::Vec4> colors;
	std::vector<unsigned int> indices;

	// Unit cube
	positions = {
		{ 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 },
		{ 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 }, { 0, 0, 0 }
	};

	colors = std::vector<gk::Vec4>(8, gk::Vec4(1, 1, 1, 1));

	indices = {
		0, 1, 1, 2, 2, 3, 3, 0,
		0, 7, 3, 4, 2, 5, 1, 6,
		4, 5, 5, 6, 6, 7, 7, 4
	};
	
	_unitcubevao = gk::createVertexArray();
	_unitcubeindexcount = (unsigned int)indices.size();

	gk::createBuffer(GL_ARRAY_BUFFER, positions);
	glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_position"));

	gk::createBuffer(GL_ARRAY_BUFFER, colors);
	glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_color"));

	gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

	// Axis
	positions = {
		{ 0, 0, 0 }, { 1, 0, 0 },
		{ 0, 0, 0 }, { 0, 1, 0 },
		{ 0, 0, 0 }, { 0, 0, 1 }
	};

	colors = {
		gk::Vec4(1, 0, 0, 1), gk::Vec4(1, 0, 0, 1),
		gk::Vec4(0, 1, 0, 1), gk::Vec4(0, 1, 0, 1),
		gk::Vec4(0, 0, 1, 1), gk::Vec4(0, 0, 1, 1)
	};

	indices = { 0, 1, 2, 3, 4, 5 };

	_axisvao = gk::createVertexArray();
	_axisindexcount = (unsigned int)indices.size();

	gk::createBuffer(GL_ARRAY_BUFFER, positions);
	glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_position"));

	gk::createBuffer(GL_ARRAY_BUFFER, colors);
	glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_color"));

	gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

	// Point cloud
	colors = std::vector<gk::Vec4>(_points.size(), gk::Vec4(1, 1, 1, 1));

	indices.resize(_points.size());
	std::iota(indices.begin(), indices.end(), 0);

	_pointsvao = gk::createVertexArray();
	_pointsindexcount = (unsigned int)_points.size();

	gk::createBuffer(GL_ARRAY_BUFFER, _points);
	glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_position"));

	gk::createBuffer(GL_ARRAY_BUFFER, colors);
	glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_color"));

	gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

	// Extreme points
	positions = {
		_points[_qhull.epindices[0]], _points[_qhull.epindices[1]],
		_points[_qhull.epindices[2]], _points[_qhull.epindices[3]],
		_points[_qhull.epindices[4]], _points[_qhull.epindices[5]]
	};

	colors = {
		gk::Vec4(1, 0, 0, 1), gk::Vec4(1, 0, 0, 1),
		gk::Vec4(0, 1, 0, 1), gk::Vec4(0, 1, 0, 1),
		gk::Vec4(0, 0, 1, 1), gk::Vec4(0, 0, 1, 1)
	};

	indices.resize(positions.size());
	std::iota(indices.begin(), indices.end(), 0);

	_epvao = gk::createVertexArray();
	_epindexcount = (unsigned int)positions.size();

	gk::createBuffer(GL_ARRAY_BUFFER, positions);
	glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_position"));

	gk::createBuffer(GL_ARRAY_BUFFER, colors);
	glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_color"));

	gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

	// Convex hull
	positions = { _points[_qhull.hullindices[0]], _points[_qhull.hullindices[1]], _points[_qhull.hullindices[2]], _points[_qhull.hullindices[3]] };
	colors = { gk::Vec4(1, 0, 0, 1), gk::Vec4(1, 0, 0, 1), gk::Vec4(1, 0, 0, 1), gk::Vec4(1, 1, 1, 1) };
	indices = { 0, 1, 1, 2, 2, 0, 0, 3, 3, 2, 2, 0, 1, 2, 2, 3, 3, 1 };
	
	_hullvao = gk::createVertexArray();

	gk::createBuffer(GL_ARRAY_BUFFER, positions);
	glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_position"));

	gk::createBuffer(GL_ARRAY_BUFFER, colors);
	glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_color"));

	gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
}
void GLViewer::destroyGLGeometry()
{
	if (_axisvao)
		_axisvao->release();
	if (_hullvao)
		_hullvao->release();
	if (_pointsvao)
		_pointsvao->release();
	if (_unitcubevao)
		_unitcubevao->release();
	if (_epvao)
		_epvao->release();

	initGLGeometry();
}

int GLViewer::draw()
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

	if (key('i'))
	{
		key('i') = 0;

		_restoreprevioussession = false;
		reset();
	}
	if (key('v'))
	{
		key('v') = 0;

		_camera.print();
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

	drawUnitCube();
	drawAxis();
	drawPoints();
	drawExtremePoints();
	drawHull();

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
void GLViewer::drawAxis()
{
	_program->uniform("mvpMatrix") = (_camera.projectionTransform() * _camera.viewTransform()).matrix();

	glBindVertexArray(_axisvao->name);
	glDrawElements(GL_LINES, _axisindexcount, GL_UNSIGNED_INT, 0);
}
void GLViewer::drawUnitCube()
{
	_program->uniform("mvpMatrix") = (_camera.projectionTransform() * _camera.viewTransform()).matrix();

	glBindVertexArray(_unitcubevao->name);
	glDrawElements(GL_LINES, _unitcubeindexcount, GL_UNSIGNED_INT, 0);
}
void GLViewer::drawPoints()
{
	glPointSize(2);

	_program->uniform("mvpMatrix") = (_camera.projectionTransform() * _camera.viewTransform()).matrix();

	glBindVertexArray(_pointsvao->name);
	glDrawElements(GL_POINTS, _pointsindexcount, GL_UNSIGNED_INT, 0);
}
void GLViewer::drawExtremePoints()
{
	glPointSize(5);

	_program->uniform("mvpMatrix") = (_camera.projectionTransform() * _camera.viewTransform()).matrix();

	glBindVertexArray(_epvao->name);
	glDrawElements(GL_POINTS, _epindexcount, GL_UNSIGNED_INT, 0);
}
void GLViewer::drawHull()
{
	_program->uniform("mvpMatrix") = (_camera.projectionTransform() * _camera.viewTransform()).matrix();

	glBindVertexArray(_hullvao->name);
	glDrawElements(GL_LINES, _unitcubeindexcount, GL_UNSIGNED_INT, 0);
}

void GLViewer::processWindowResize(SDL_WindowEvent& event)
{
	_widgets.reshape(event.data1, event.data2);
}
void GLViewer::processMouseButtonEvent(SDL_MouseButtonEvent& event)
{
	_widgets.processMouseButtonEvent(event);
}
void GLViewer::processMouseMotionEvent(SDL_MouseMotionEvent& event)
{
	_widgets.processMouseMotionEvent(event);

	if (event.state & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		_camera.yaw(-event.xrel);
		_camera.pitch(-event.yrel);
	}
}
void GLViewer::processKeyboardEvent(SDL_KeyboardEvent& event)
{
	_widgets.processKeyboardEvent(event);
}