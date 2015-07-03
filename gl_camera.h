#ifndef __GLCAMERA_H__
#define __GLCAMERA_H__

#include <Transform.h>

class GLCamera
{
private:

	gk::Point _position;

	gk::Vector _up;
	gk::Vector _front;
	gk::Vector _right;

	gk::Vector _worldUp;

	gk::Transform _worldToViewTransform;
	gk::Transform _viewToWorldTransform;
	gk::Transform _projectionTransform;

public:

	float moveUnit;

public:

	GLCamera(const gk::Point& position = gk::Point(),
		const gk::Vector& up = gk::Vector(0, 1, 0),
		const gk::Vector& front = gk::Vector(0, 0, -1),
		const gk::Vector& worlUp = gk::Vector(0, 1, 0),
		const float _moveUnit = 1.f,
		const gk::Transform& projectionTransform = gk::Transform());

	gk::Transform& viewTransform() { return _worldToViewTransform; }
	const gk::Transform& viewTransform() const { return _worldToViewTransform; }

	gk::Transform& projectionTransform() { return _projectionTransform; }
	const gk::Transform& projectionTransform() const { return _projectionTransform; }

	void print() const;

	void pitch(float localDegreeAngle);
	void yaw(float globalDegreeAngle);

	void translate(const gk::Vector& translation);
	void localTranslate(const gk::Vector& translation);

private:

	void updateRight();
	void updateTransforms();
};

#endif
