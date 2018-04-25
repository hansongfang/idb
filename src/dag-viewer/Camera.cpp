#include "Camera.h"
#include <glm\gtx\transform.hpp>
#include <iostream>

static const float MaxVerticalAngle = 85.0f;
Camera::Camera() :
	_position(0.0f, 0.0f, 1.0f),
	_horizontalAngle(0.0f),
	_verticalAngle(0.0f),
	_fieldOfView(50.0f),
	_nearPlane(0.01f),
	_farPlane(100.0f),
	_viewportAspectRatio(4.0f / 3.0f),
	_viewDirection(0.0f, 0.0f, -1.0f),
	UP(0.0f, 1.0f, 0.0f)
{
}

const glm::vec3& Camera::position() const {
	return _position;
}

float Camera::setPosition(const glm::vec3& position) {
	_position = position;
	//std::cout << glm::to_string(_position) << std::endl;
	return 1.0f;
}

void Camera::offsetPosition(const glm::vec3& offset) {
	_position += offset;
}

float Camera::fieldOfView() const {
	return _fieldOfView;
}

void Camera::setFieldOfView(float fieldOfView) {
	assert(fieldOfView > 0.0f && fieldOfView < 180.0f);
	_fieldOfView = fieldOfView;
}

float Camera::nearPlane() const {
	return _nearPlane;
}

float Camera::farPlane() const {
	return _farPlane;
}

void Camera::setNearAndFarPlanes(float nearPlane, float farPlane) {
	assert(nearPlane > 0.0f);
	assert(farPlane > nearPlane);
	_nearPlane = nearPlane;
	_farPlane = farPlane;
}

glm::mat4 Camera::orientation() const {
	glm::mat4 orientation;
	orientation = glm::rotate(orientation, _verticalAngle, glm::vec3(1, 0, 0));
	orientation = glm::rotate(orientation, _horizontalAngle, glm::vec3(0, 1, 0));
	return orientation;
}
void Camera::offsetOrientation(float upAngle, float rightAngle) {
	_horizontalAngle += rightAngle;
	_verticalAngle += upAngle;
	normalizeAngles();
}

void Camera::lookAt(glm::vec3 position) {
	assert(position != _position);
	glm::vec3 direction = glm::normalize(position - _position);
	_verticalAngle = glm::degrees(asinf(-direction.y));
	
	_horizontalAngle = -glm::degrees(atan2f(-direction.x, -direction.z));
	
	normalizeAngles();
}

float Camera::viewportAspectRatio() const {
	return _viewportAspectRatio;
}

void Camera::setViewportAspectRatio(float viewportAspectRatio) {
	assert(viewportAspectRatio > 0.0);
	_viewportAspectRatio = viewportAspectRatio;
}
glm::vec3 Camera::forward() const {
	glm::vec4 forward = glm::inverse(orientation()) * glm::vec4(0, 0, -1, 1);
	return glm::vec3(forward);
}

glm::vec3 Camera::right() const {
	glm::vec4 right = glm::inverse(orientation()) * glm::vec4(1, 0, 0, 1);
	return glm::vec3(right);
}

glm::vec3 Camera::up() const {
	glm::vec4 up = glm::inverse(orientation()) * glm::vec4(0, 1, 0, 1);
	return glm::vec3(up);
}

glm::mat4 Camera::matrix() const {
	return projection() * view();
}

glm::mat4 Camera::projection() const {
	return glm::perspective(_fieldOfView, _viewportAspectRatio, _nearPlane, _farPlane);
}

glm::mat4 Camera::view() const {
	return orientation() * glm::translate(glm::mat4(), -_position);
}

void Camera::normalizeAngles() {
	_horizontalAngle = fmodf(_horizontalAngle, 360.0f);
	//fmodf can return negative values, but this will make them all positive
	if (_horizontalAngle < 0.0f)
		_horizontalAngle += 360.0f;

	if (_verticalAngle > MaxVerticalAngle)
		_verticalAngle = MaxVerticalAngle;
	else if (_verticalAngle < -MaxVerticalAngle)
		_verticalAngle = -MaxVerticalAngle;
}

// added
void Camera::setDirection(const glm::vec3& direction) {
	_viewDirection = direction;
}
glm::mat4 Camera::getWorldToViewMatrix() const
{
	return glm::lookAt(_position, _position + _viewDirection, glm::vec3(0.0, 1.0, 0.0));
}

