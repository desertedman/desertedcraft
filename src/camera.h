#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

enum class Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

// Default camera values
constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM = 90.0f;

class Camera {
public:
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;

  float Yaw;
  float Pitch;

  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
         float pitch = PITCH);

  Camera(float posX = 0.f, float posY = 0.f, float posZ = 0.f, float upX = 0.f,
         float upY = 1.f, float upZ = 0.f, float yaw = YAW,
         float pitch = PITCH);

  glm::mat4 GetViewMatrix() const;

  void ProcessKeyboard(const Camera_Movement direction, float deltaTime);
  void ProcessMouseMovement(float xoffset, float yoffset,
                            const GLboolean constrainPitch = true);
  void ProcessMouseScroll(const float yoffset);

private:
  void updateCameraVectors();
};
