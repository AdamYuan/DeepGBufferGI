//
// Created by adamyuan on 2/1/19.
//

#ifndef ADYPT_CAMERA_HPP
#define ADYPT_CAMERA_HPP

#include <mygl3/utils/framerate.hpp>
#include <glm/glm.hpp>

struct GLFWwindow;
class Camera
{
public:
	glm::vec3 m_position{};
	float m_yaw{}, m_pitch{};
	float m_sensitive{0.3f}, m_speed{1.f / 16.f}, m_fov{60.0f};
private:
	void move_forward(float dist, float dir) //degrees
	{
		float rad = glm::radians(m_yaw + dir);
		m_position.x -= glm::sin(rad) * dist;
		m_position.z -= glm::cos(rad) * dist;
	}

public:
	glm::mat4 GetView() const;
	glm::mat4 GetProjection() const;

	void Control(GLFWwindow *window, const mygl3::Framerate &fps);
};

#endif //ADYPT_CAMERA_HPP
