#include "gl_camera.h"

#include <cstdio>

GLCamera::GLCamera(const gk::Point& position,
	const gk::Vector& up,
	const gk::Vector& front,
	const gk::Vector& worlUp,
	const float _moveUnit,
	const gk::Transform& projectionTransform)
	:_position(position),
	_up(gk::Normalize(up)),
	_front(gk::Normalize(front)),
	_worldUp(gk::Normalize(worlUp)),
	moveUnit(_moveUnit),
	_projectionTransform(projectionTransform)
{
	updateRight();
	updateTransforms();
}

void GLCamera::print() const
{
	printf("\r\n");

	printf("Position: "); _position.print();

	printf("Up (length = %f): ", _up.Length());
	_up.print();
	printf("Front: (length = %f): ", _front.Length());
	_front.print();
	printf("Right: (length = %f): ", _right.Length());
	_right.print();

	printf("\r\n");

	printf("----- ViewToWorld matrix -----\r\n\r\n");
	_viewToWorldTransform.print();
	printf("------------------------------\r\n");

	printf("----- WorldToView matrix -----\r\n\r\n");
	_worldToViewTransform.print();
	printf("------------------------------\r\n");
}

void GLCamera::pitch(float localDegreeAngle)
{
	gk::Transform r = gk::Rotate(localDegreeAngle, _right);

	_up = r(_up);
	_front = r(_front);

	updateTransforms();
}
void GLCamera::yaw(float globalDegreeAngle)
{
	gk::Transform r = gk::Rotate(globalDegreeAngle, _worldUp);

	_up = r(_up);
	_right = r(_right);
	_front = r(_front);

	updateTransforms();
}

void GLCamera::translate(const gk::Vector& translation)
{
	_position += translation;

	updateTransforms();
}
void GLCamera::localTranslate(const gk::Vector& translation)
{
	_position += _viewToWorldTransform(translation);

	updateTransforms();
}

void GLCamera::updateRight()
{
	_right = gk::Cross(_front, _up);
}
void GLCamera::updateTransforms()
{
	_viewToWorldTransform = gk::Transform(gk::Matrix4x4(_right.x, _up.x, -_front.x, _position.x,
		_right.y, _up.y, -_front.y, _position.y,
		_right.z, _up.z, -_front.z, _position.z,
		0, 0, 0, 1));
	_worldToViewTransform = _viewToWorldTransform.inverse();
}