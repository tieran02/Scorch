#pragma once
#include "glm/glm.hpp"

namespace SC 
{
	class Camera
	{
	public:
		Camera(const glm::vec3& origin = glm::vec3(0,0,0));

		void Translate(const glm::vec3& translate);

		void MousePosition(float xpos, float ypos);

		glm::mat4 GetViewMatrix() const;

		glm::vec3& Position();
		const glm::vec3& Position() const;
		const glm::vec3& Front() const;
		const glm::vec3& Up() const;
		glm::vec3 Right() const;

		float Fov() const;

		float& Yaw();
		const float& Yaw() const;

		float& Pitch();
		const float& Pitch() const;
	private:
		glm::vec3 m_position;
		glm::vec3 m_front;
		glm::vec3 m_up;

		float m_yaw;
		float m_pitch;
		float m_lastX;
		float m_lastY;
		float m_fov;
	};
}