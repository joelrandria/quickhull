#include "qhull.h"

#include <Geometry.h>
#include <Transform.h>

#include <vector>
#include <cstdlib>
#include <ctime>

#define WINDOW_WIDTH	512
#define WINDOW_HEIGHT	512

void randomize(std::vector<Point>& points, int count)
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

int main(int argc, char** argv)
{
	std::vector<Point> points;
	srand(time(0));
	randomize(points, 16);

	qhull app(points,
		GLCamera(
		Point(0.5f, 0.5f, 1.5f),
		Vector(0, 1, 0),
		Vector(0, 0, -1),
		Vector(0, 1, 0),
		0.05f,
		gk::Perspective(60, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 10)
		),
		WINDOW_WIDTH,
		WINDOW_HEIGHT);
	app.run();

	return 0;
}