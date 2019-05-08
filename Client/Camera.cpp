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

    float tanVal = 2.0 * (float)tan(glm::radians(_fov) * 0.5);
	float nh = _near * tanVal;
	float nw = nh * _aspect; 
	float fh = _far  * tanVal;
	float fw = fh * _aspect;

    glm::vec3 farCenter = _position + _forward * _far;
	ftl = farCenter + (_up * fh/2.0f) - (_right * fw/2.0f);
	ftr = farCenter + (_up * fh/2.0f) + (_right * fw/2.0f);
	fbl = farCenter - (_up * fh/2.0f) - (_right * fw/2.0f);
	fbr = farCenter - (_up * fh/2.0f) + (_right * fw/2.0f);
	glm::vec3 nc = _position + _forward * _near; 
	ntl = nc + (_up * nh/2.0f) - (_right * nw/2.0f);
	ntr = nc + (_up * nh/2.0f) + (_right * nw/2.0f);
	nbl = nc - (_up * nh/2.0f) - (_right * nw/2.0f);
	nbr = nc - (_up * nh/2.0f) + (_right * nw/2.0f);

    _planes[0] = {ntr,ntl,ftl};
	_planes[1] = {nbl,nbr,fbr};
	_planes[2] = {ntl,nbl,fbl};
	_planes[3] = {nbr,ntr,fbr};
	_planes[4] = {ntl,ntr,nbr};
	_planes[5] = {ftr,ftl,fbl};
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

  _planes.resize(6);
    
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

bool Camera::isInFrustum(glm::vec3 p, float radius) const {
	for(int i=0; i < 6; i++) {
        auto & [p0, p1, p2] = _planes[i];
        glm::vec3 normal = glm::normalize(glm::cross((p1-p0), (p2-p1)));
        glm::vec3 vec = p - p0;
        float dist = glm::dot(vec, normal);
		if (dist + radius < 0)
			return false;
	}
    return true;
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

void Camera::set_fovy(float fovy) {
  _fov = fovy;
}

void Camera::set_movement_speed(float speed)
{
  _movement_speed = speed;
}

void Camera::set_mouse_sensitivity(float sensitivity)
{
  _sensitivity = sensitivity;
}