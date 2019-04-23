/**
 * Camera.cpp
 */

#include "Camera.hpp"

#include <iostream>

Camera::Camera()
{
  Reset();
}

void Camera::Update()
{
	glm::vec3 forward;
	forward.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	forward.y = sin(glm::radians(_pitch));
	forward.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	_forward = glm::normalize(forward);
	_right = glm::normalize(glm::cross(_forward, _world_up));
	_up = glm::normalize(glm::cross(_right, _forward));
}

void Camera::Reset()
{
  _fov    = cameradefaults::fov;
  _aspect = cameradefaults::aspect;
  _near   = cameradefaults::near_plane;
  _far    = cameradefaults::far_plane;
  
  _world_up = cameradefaults::up;
  
  _position = cameradefaults::position;
  _up       = cameradefaults::up;
  _pitch    = cameradefaults::pitch;
  _yaw      = cameradefaults::yaw;
  
  _movement_speed = cameradefaults::move_speed;
  _sensitivity    = cameradefaults::mouse_sensitivity;
}

void Camera::CameraDolly(CameraMovement direction, float delta_time)
{
  float velocity = _movement_speed * delta_time;
  switch (direction)
  {
    case FORWARD:
      _position += _forward * velocity;
      break;
      
    case BACKWARD:
      _position -= _forward * velocity;
      break;
      
    case RIGHT:
      _position += _right * velocity;
      break;
      
    case LEFT:
      _position -= _right * velocity;
      break;
      
    case UP:
      _position += _up * velocity;
      break;
      
    case DOWN:
      _position -= _up * velocity;
      break;
  }
}

void Camera::CameraPan(float delta_yaw, float delta_pitch, bool constrain_pitch)
{
  // Adjust for mouse sensitivity
  delta_yaw   *= _sensitivity;
  delta_pitch *= _sensitivity;
  
  _yaw   += delta_yaw;
  _pitch += delta_pitch;
  
  if (constrain_pitch)
  {
    if (_pitch > 89.0f)
    {
      _pitch = 89.0f;
    }
    else if (_pitch < -89.0f)
    {
      _pitch = -89.0f;
    }
  }
}

void Camera::CameraZoom(float delta_zoom)
{
  if (_fov >= 1.0f && _fov <= cameradefaults::fov)
    _fov -= delta_zoom;
  if (_fov <= 1.0f)
    _fov = 1.0f;
  if (_fov >= cameradefaults::fov)
    _fov = cameradefaults::fov;
}

glm::mat4 Camera::projection_matrix() const
{
	return glm::perspective(glm::radians(_fov), _aspect, _near, _far);
}

glm::mat4 Camera::view_matrix() const
{
	return glm::lookAt(_position, _position + _forward, _up);
}

glm::vec3 Camera::position() const
{
	return _position;
}

float Camera::fov() const
{
	return _fov;
}


void Camera::set_position(const glm::vec3 & position)
{
  _position = position;
}

void Camera::set_position(float x, float y, float z)
{
  _position.x = x;
  _position.y = y;
  _position.z = z;
}

void Camera::set_pitch(float pitch, bool constrain_pitch) {
	_pitch = pitch;
	if (constrain_pitch)
	{
		if (_pitch > 89.0f)
		{
			_pitch = 89.0f;
		}
		else if (_pitch < -89.0f)
		{
			_pitch = -89.0f;
		}
	}
}

void Camera::set_yaw(float yaw) {
	_yaw = yaw;
}

void Camera::set_aspect(float aspect)
{
  _aspect = aspect;
}

void Camera::set_movement_speed(float speed)
{
  _movement_speed = speed;
}

void Camera::set_mouse_sensitivity(float sensitivity)
{
  _sensitivity = sensitivity;
}