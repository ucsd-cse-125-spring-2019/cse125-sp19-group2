/**
 * Camera.cpp
 */

#include "Camera.hpp"

#include <iostream>
#include <algorithm>
#include <glm/gtx/matrix_transform_2d.hpp> 

Camera::Camera()
{
  Reset();
}

void Camera::Update()
{
    // Make sure pitch is within limit
    this->constrain_pitch();

	// Update distance and related parameter
	this->update_distance();

    // Calculate three axis
	glm::vec3 forward;
	forward.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	forward.y = sin(glm::radians(_pitch));
	forward.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
	_forward = glm::normalize(forward);
	_right = glm::normalize(glm::cross(_forward, _world_up));
	_up = glm::normalize(glm::cross(_right, _forward));

    // Update _position based on _forward, _lookAtPosition and _distance
    _position = _lookAtPosition - _forward * _distance;

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
  set_distance(14.0f, false);
  set_pitch(45.0f);
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
  
  this->constrain_pitch();
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

float Camera::getTransparency(glm::vec3 p, float radius) const
{
	float dist = glm::length(p - _position);
	return std::min(std::max(0.0f, (dist - 0.7f) / (radius)), 1.0f);
}

float Camera::fov() const
{
	return _fov;
}


void Camera::set_position(const glm::vec3 & position)
{
  _lookAtPosition = position;
}

void Camera::set_position(float x, float y, float z)
{
  _lookAtPosition.x = x;
  _lookAtPosition.y = y;
  _lookAtPosition.z = z;
}

void Camera::set_pitch(float pitch, bool constrain_pitch) {
	_pitch = pitch;
	if (constrain_pitch)
	{
		this->constrain_pitch();
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

void Camera::set_distance(float distance, bool delta) {
    if(delta){
        _targetDistance += distance * 0.5;
	}else {
	    _targetDistance = distance;
	}
	
	// Clamp _targetDistance to 0-30 units
	_targetDistance = std::max(std::min(_targetDistance, 30.0f), 0.0f);
}

void Camera::update_distance()
{
	// Smoothly change _distance toward _targetDistance
	const auto delta = _targetDistance - _distance;
	if(abs(delta) > 0.01){
		_distance += delta / _smoothness;
	}

	// Set _pitchLimit
	// 0 -> 89.0, 1 -> 0, 30 -> -89.0, lerp
	if (_distance <= 1)
	{
		_pitchLimit = 89.0f - (89.0f * _distance);
	}
	else
	{
		_pitchLimit = 0 - (89.0f / 30.0f * _distance);
	}
}

void Camera::resize(int x, int y) {
    const float aspect = float(x) / y;
    static float limit = glm::degrees(2 * atan(tan(glm::radians(45 * 0.5)) * aspect));
    float fovx = glm::degrees(2 * atan(tan(glm::radians(45 * 0.5)) * aspect));
    if (fovx >= limit) {
        // update fovy as well
        fovx = limit;
        const float newFovy = glm::degrees(2 * atan(tan(glm::radians(limit * 0.5)) / aspect));
        set_fovy(newFovy);
    }
    else {
        set_fovy(45.0f);
    }
    set_aspect(aspect);
}

void Camera::move_camera(glm::vec2 v, bool isMouse) {
    const auto delta = v * _mouseSensitivity;
    CameraPan(delta.x, -delta.y);
}

void Camera::constrain_pitch() {
    if (_pitch > _pitchLimit) {
        _pitch = _pitchLimit;
    }
    else if (_pitch < -89.0f) {
        _pitch = -89.0f;
    }
}

glm::vec2 Camera::convert_direction(glm::vec2 input) {
    static glm::vec2 initial = glm::vec2(0, -1);
    const glm::vec2 rotVec = glm::normalize(glm::vec2(_forward.x, _forward.z));
    const float angle = glm::atan(rotVec.y - initial.y,rotVec.x - initial.x);
    glm::vec2 out = glm::rotate(glm::mat3(1.0f), angle * 2.0f) * glm::vec3(input, 0);
	return glm::normalize(out);
}
