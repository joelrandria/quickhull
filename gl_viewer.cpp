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
	if (_restoreprevioussession)
	{
		_points = loadPoints(DEFAULT_POINT_FILENAME);
	}
	else
	{
		_points = loadRandomPoints(DEFAULT_POINT_COUNT);
		savePoints(_points, DEFAULT_POINT_FILENAME);
	}

	// Build convex hull
	_qhull.initialize(&_points);
	
	std::cout << "Convex hull built in " << _qhull.build() << " iterations" << std::endl;

	// Create GL geometry
	updateGLGeometry();

	_restoreprevioussession = false;
}
int GLViewer::quit()
{
	return 0;
}

std::vector<Point> GLViewer::loadRandomPoints(int count)
{
	std::vector<Point> points;

	points.reserve(count);

	// Generate random 3D points within unit cube
	while (count-- > 0)
		points.push_back(Point(
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX));

	if (rand() % 3 == 0)
	{
		int nulldim = rand() % 3;

		// Points coplanarization
		for (int i = 0; i < (int)points.size(); ++i)
			points[i][nulldim] = 0;
	}

	return points;
}
std::vector<Point> GLViewer::loadPoints(const std::string& filename)
{
	std::vector<Point> points;

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

	return points;
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
	_unitcubegl = nullptr;
	_axisgl = nullptr;
	_pointsgl = nullptr;

	_hullglvertices = nullptr;
	_hullglfaces = nullptr;
	_hullgledges = nullptr;
	_hullglfacesextremes = nullptr;
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

	_unitcubegl = new GLVertexBufferSet();
	_unitcubegl->vao = createGLUnmanagedVertexArray();
	_unitcubegl->indexcount = (unsigned int)indices.size();

	_unitcubegl->positions = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, positions);
	glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_position"));

	_unitcubegl->colors = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, colors);
	glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_color"));

	_unitcubegl->indices = createGLUnmanagedBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

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

	_axisgl = new GLVertexBufferSet();
	_axisgl->vao = createGLUnmanagedVertexArray();
	_axisgl->indexcount = (unsigned int)indices.size();

	_axisgl->positions = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, positions);
	glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_position"));

	_axisgl->colors = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, colors);
	glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_color"));

	_axisgl->indices = createGLUnmanagedBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

	// Points
	colors = std::vector<gk::Vec4>(_points.size(), gk::Vec4(1, 1, 1, 1));

	indices.resize(_points.size());
	std::iota(indices.begin(), indices.end(), 0);

	_pointsgl = new GLVertexBufferSet();
	_pointsgl->vao = createGLUnmanagedVertexArray();
	_pointsgl->indexcount = (unsigned int)_points.size();

	_pointsgl->positions = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, _points);
	glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_position"));

	_pointsgl->colors = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, colors);
	glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(_program->attribute("vertex_color"));

	_pointsgl->indices = createGLUnmanagedBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

	// Hull vertices
	std::vector<QHull3d::Face> hull = _qhull.hull();
	if (!hull.empty())
	{
		colors = std::vector<gk::Vec4>(_points.size(), gk::Vec4(1, 1, 0, 1));

		indices.resize(hull.size() * 3);
		for (int i = 0; i < (int)hull.size(); ++i)
		{
			indices[i * 3] = (unsigned int)(hull[i].idx[0]);
			indices[(i * 3) + 1] = (unsigned int)(hull[i].idx[1]);
			indices[(i * 3) + 2] = (unsigned int)(hull[i].idx[2]);
		}

		_hullglvertices = new GLVertexBufferSet();
		_hullglvertices->vao = createGLUnmanagedVertexArray();
		_hullglvertices->indexcount = (unsigned int)indices.size();

		_hullglvertices->positions = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, _points);
		glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(_program->attribute("vertex_position"));

		_hullglvertices->colors = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, colors);
		glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(_program->attribute("vertex_color"));

		_hullglvertices->indices = createGLUnmanagedBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

		// Hull faces
		std::vector<gk::Point> hullvertices;
		hullvertices.reserve(hull.size() * 3);
		for (int i = 0; i < (int)hull.size(); ++i)
		{
			hullvertices.push_back(_points[hull[i].idx[0]]);
			hullvertices.push_back(_points[hull[i].idx[1]]);
			hullvertices.push_back(_points[hull[i].idx[2]]);
		}

		colors = std::vector<gk::Vec4>(hullvertices.size(), gk::Vec4(1, 0, 0, 1));

		indices.resize(hull.size() * 3);
		std::iota(indices.begin(), indices.end(), 0);

		_hullglfaces = new GLVertexBufferSet();
		_hullglfaces->vao = createGLUnmanagedVertexArray();
		_hullglfaces->indexcount = (unsigned int)indices.size();

		_hullglfaces->positions = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, hullvertices);
		glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(_program->attribute("vertex_position"));

		_hullglfaces->colors = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, colors);
		glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(_program->attribute("vertex_color"));

		_hullglfaces->indices = createGLUnmanagedBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

		// Hull edges
		colors = std::vector<gk::Vec4>(_points.size(), gk::Vec4(1, 1, 1, 1));

		indices.resize(hull.size() * 6);
		for (int i = 0; i < (int)hull.size(); ++i)
		{
			indices[i * 6] = (unsigned int)(hull[i].idx[0]);
			indices[(i * 6) + 1] = (unsigned int)(hull[i].idx[1]);

			indices[(i * 6) + 2] = (unsigned int)(hull[i].idx[1]);
			indices[(i * 6) + 3] = (unsigned int)(hull[i].idx[2]);

			indices[(i * 6) + 4] = (unsigned int)(hull[i].idx[2]);
			indices[(i * 6) + 5] = (unsigned int)(hull[i].idx[0]);
		}

		_hullgledges = new GLVertexBufferSet();
		_hullgledges->vao = createGLUnmanagedVertexArray();
		_hullgledges->indexcount = (unsigned int)indices.size();

		_hullgledges->positions = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, _points);
		glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(_program->attribute("vertex_position"));

		_hullgledges->colors = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, colors);
		glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(_program->attribute("vertex_color"));

		_hullgledges->indices = createGLUnmanagedBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

		// Hull faces extremes
		std::vector<int> facesextremes = _qhull.getFacesExtremesIndices();
		if (!facesextremes.empty())
		{
			colors = std::vector<gk::Vec4>(_points.size(), gk::Vec4(1, 1, 0, 1));

			indices.resize(facesextremes.size());
			for (int i = 0; i < (int)facesextremes.size(); ++i)
				indices[i] = (unsigned int)facesextremes[i];

			_hullglfacesextremes = new GLVertexBufferSet();
			_hullglfacesextremes->vao = createGLUnmanagedVertexArray();
			_hullglfacesextremes->indexcount = (unsigned int)indices.size();

			_hullglfacesextremes->positions = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, _points);
			glVertexAttribPointer(_program->attribute("vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(_program->attribute("vertex_position"));

			_hullglfacesextremes->colors = createGLUnmanagedBuffer(GL_ARRAY_BUFFER, colors);
			glVertexAttribPointer(_program->attribute("vertex_color"), 4, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(_program->attribute("vertex_color"));

			_hullglfacesextremes->indices = createGLUnmanagedBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
		}
	}
}
void GLViewer::updateGLGeometry()
{
	destroyGLGeometry();
	createGLGeometry();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void GLViewer::destroyGLGeometry()
{
	if (_unitcubegl)
	{
		delete _unitcubegl;
		_unitcubegl = nullptr;
	}
	if (_axisgl)
	{
		delete _axisgl;
		_axisgl = nullptr;
	}
	if (_pointsgl)
	{
		delete _pointsgl;
		_pointsgl = nullptr;
	}

	if (_hullglvertices)
	{
		delete _hullglvertices;
		_hullglvertices = nullptr;
	}
	if (_hullglfaces)
	{
		delete _hullglfaces;
		_hullglfaces = nullptr;
	}
	if (_hullgledges)
	{
		delete _hullgledges;
		_hullgledges = nullptr;
	}
	if (_hullglfacesextremes)
	{
		delete _hullglfacesextremes;
		_hullglfacesextremes = nullptr;
	}

	initGLGeometry();
}

int GLViewer::draw()
{
	// Keyboard: General
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

	// Keyboard: Camera navigation
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

	// Keyboard: Algorithm
	if (key(SDLK_KP_PLUS))
	{
		key(SDLK_KP_PLUS) = 0;

		_qhull.iterate();
		updateGLGeometry();
	}

	// Keyboard: Misc
	if (key('i'))
	{
		key('i') = 0;

		reset();
	}
	if (key('v'))
	{
		key('v') = 0;

		_camera.print();
	}

	// 3D geometry rendering
	glViewport(0, 0, windowWidth(), windowHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_timer->start();

	glUseProgram(_program->name);

	_program->uniform("mvpMatrix") = (_camera.projectionTransform() * _camera.viewTransform()).matrix();

	// Unit cube
	glBindVertexArray(_unitcubegl->vao->name);
	glDrawElements(GL_LINES, _unitcubegl->indexcount, GL_UNSIGNED_INT, 0);

	// Axis
	glBindVertexArray(_axisgl->vao->name);
	glDrawElements(GL_LINES, _axisgl->indexcount, GL_UNSIGNED_INT, 0);

	// Points
	glPointSize(2);

	glBindVertexArray(_pointsgl->vao->name);
	glDrawElements(GL_POINTS, _pointsgl->indexcount, GL_UNSIGNED_INT, 0);

	// Hull vertices
	if (_hullglvertices)
	{
		glPointSize(6);

		glBindVertexArray(_hullglvertices->vao->name);
		glDrawElements(GL_POINTS, _hullglvertices->indexcount, GL_UNSIGNED_INT, 0);
	}

	// Hull faces
	if (_hullglfaces)
	{
		glBindVertexArray(_hullglfaces->vao->name);
		glDrawElements(GL_TRIANGLES, _hullglfaces->indexcount, GL_UNSIGNED_INT, 0);
	}

	// Hull edges
	if (_hullgledges)
	{
		glLineWidth(2);

		glBindVertexArray(_hullgledges->vao->name);
		glDrawElements(GL_LINES, _hullgledges->indexcount, GL_UNSIGNED_INT, 0);
	}

	// Hull faces extreme points
	if (_hullglfacesextremes)
	{
		glPointSize(6);

		glBindVertexArray(_hullglfacesextremes->vao->name);
		glDrawElements(GL_POINTS, _hullglfacesextremes->indexcount, GL_UNSIGNED_INT, 0);
	}

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