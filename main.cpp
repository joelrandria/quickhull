#include "gl_viewer.h"

#include <cstdlib>
#include <cstring>
#include <ctime>

#define WINDOW_WIDTH	768
#define WINDOW_HEIGHT	768

int main(int argc, char** argv)
{
	srand(time(0));

	GLViewer app(
		GLCamera(
		Point(0.5f, 0.5f, 2),
		Vector(0, 1, 0),
		Vector(0, 0, -1),
		Vector(0, 1, 0),
		0.05f,
		gk::Perspective(60, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 10)
		),
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		argc > 1 && !strcmp(argv[1], "-r"));
	app.run();

	return 0;
}