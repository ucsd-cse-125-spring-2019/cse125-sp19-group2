/**
 * Camera.hpp
 */

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <tuple>
#include <vector>

enum CameraMovement
{
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

namespace cameradefaults
{
  const glm::vec3 position = glm::vec3(0);
  const glm::vec3 up = glm::vec3(0, 1, 0);
  const float yaw = -90.0f;
  const float pitch = 0;
  
  const float fov = 45.0f;
  const float aspect = 1.33f;
  
  const float near_plane = 0.1f;
  const float far_plane  = 1000.0f;
  
  const float move_speed = 1.0f;
  const float mouse_sensitivity = 0.1f;
}

class Camera
{
 public:
  
  /**
   * @brief Creates a camera.
   */
  Camera();
  
  void Update();
  void Reset();

  /**
   * @brief Moves camera in a given direction.
   *
   * @param delta_time Time to render a single frame.
   */
  void CameraDolly(CameraMovement direction, float delta_time);
  
  /**
   * @brief Rotates camera by a given yaw and pitch amount.
   */
  void CameraPan(float delta_yaw, float delta_pitch, bool constrain_pitch = true);
  
  void CameraZoom(float delta_zoom);
  
  glm::mat4 projection_matrix() const;
  glm::mat4 view_matrix() const;
  glm::vec3 position() const;
  bool isInFrustum(glm::vec3 p, float radius) const;
  float fov() const;

  void set_position(const glm::vec3 & position);
  void set_position(float x, float y, float z);
  void set_pitch(float pitch, bool constrain_pitch = true);
  void set_yaw(float yaw);
  void set_aspect(float aspect);
  void set_fovy(float fovy);
  void set_movement_speed(float speed);
  void set_mouse_sensitivity(float sensitivity);
  
 private:
  
  glm::vec3 _position; ///< The camera's position in world space.
  glm::vec3 _forward;  ///< The camera's forward vector.
  glm::vec3 _up;       ///< The camera's up vector.
  glm::vec3 _world_up;
  glm::vec3 _right;    ///< The camera's right vector.

    glm::vec3 ftl;
    glm::vec3 ftr;
    glm::vec3 fbl;
    glm::vec3 fbr;
    glm::vec3 ntl;
    glm::vec3 ntr;
    glm::vec3 nbl;
    glm::vec3 nbr;

    std::vector<std::tuple<glm::vec3, glm::vec3, glm::vec3>> _planes; 
  
  float _pitch;        ///< The camera's pitch.
  float _yaw;          ///< The camera's yaw.
  
  float _fov;
  float _aspect;
  
  float _near; ///< Near plane
  float _far;  ///< Far plane
  
  float _movement_speed;
  float _sensitivity;
};

#endif /* CAMERA_HPP */
