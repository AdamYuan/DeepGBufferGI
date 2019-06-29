//
// Created by adamyuan on 2/1/19.
//
#include "Camera.hpp"
#include "Config.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera::GetView() const
{
	glm::mat4 view = glm::rotate(glm::identity<glm::mat4>(), glm::radians(-m_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::rotate(view, glm::radians(-m_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::translate(view, -m_position);

	return view;
}

glm::mat4 Camera::GetProjection() const
{
	return glm::perspective(glm::radians(m_fov), kCamAspectRatio, kCamNear, kCamFar);
}

void Camera::Control(GLFWwindow *window, const mygl3::Framerate &fps)
{
	static glm::dvec2 last_mouse_pos;
	float speed = fps.GetDelta() * m_speed;
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		move_forward(speed, 0.0f);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		move_forward(speed, 90.0f);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		move_forward(speed, -90.0f);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		move_forward(speed, 180.0f);
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		m_position.y += speed;
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		m_position.y -= speed;

	glm::dvec2 cur_pos;
	glfwGetCursorPos(window, &cur_pos.x, &cur_pos.y);

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
	{
		glfwGetCursorPos(window, &cur_pos.x, &cur_pos.y);
		float offset_x = (cur_pos.x - last_mouse_pos.x) * m_sensitive;
		float offset_y = (cur_pos.y - last_mouse_pos.y) * m_sensitive;

		m_yaw -= offset_x;
		m_pitch -= offset_y;
		m_pitch = glm::clamp(m_pitch, -90.0f, 90.0f);
		m_yaw = glm::mod(m_yaw, 360.0f);
		last_mouse_pos = cur_pos;
	}

	last_mouse_pos = cur_pos;
}

