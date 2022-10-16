#include "pch.h"
#include "render/camera.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace SC;

Camera::Camera(const glm::vec3& origin /*= glm::vec3(0,0,0)*/) :
	m_position(origin),
	m_front(0.0f, 0.0f, -1.0f),
	m_up(0.0f, 1.0f, 0.0f),
	m_yaw(-90.0f),
	m_pitch(0.0f),
	m_lastX(0.0f),
	m_lastY(0.0f),
	m_fov(65.0f)
{
}

void Camera::Translate(const glm::vec3& translate)
{
	m_position += translate;
}

void Camera::MousePosition(float xpos, float ypos)
{
	float xoffset = xpos - m_lastX;
	float yoffset = m_lastY - ypos; // reversed since y-coordinates go from bottom to top
	m_lastX = xpos;
	m_lastY = ypos;

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	m_yaw += xoffset;
	m_pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front = glm::normalize(front);
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(m_position, m_position + m_front, m_up);
}

const glm::vec3& Camera::Position() const
{
	return m_position;
}

const glm::vec3& Camera::Front() const
{
	return m_front;
}

const glm::vec3& Camera::Up() const
{
	return m_up;
}

glm::vec3& Camera::Position()
{
	return m_position;
}

float& Camera::Yaw()
{
	return m_yaw;
}

const float& Camera::Yaw() const
{
	return m_yaw;
}

float& Camera::Pitch()
{
	return m_pitch;
}

const float& Camera::Pitch() const
{
	return m_pitch;
}

glm::vec3 Camera::Right() const
{
	return glm::cross(m_front, m_up);
}

float Camera::Fov() const
{
	return m_fov;
}
