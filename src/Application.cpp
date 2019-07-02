//
// Created by adamyuan on 19-5-3.
//

#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>
#include "Application.hpp"
#include "Config.hpp"

Application::Application()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_window = glfwCreateWindow(1280, 720, "SparseVoxelOctree", nullptr, nullptr);
	glfwMakeContextCurrent(m_window);
	glfwSetWindowUserPointer(m_window, (void*)this);

	gl3wInit();

	m_camera.Initialize();

	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/sibenik/sibenik.obj");
	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/living_room/living_room.obj");
	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/San_Miguel/san-miguel-low-poly.obj");
	m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/sponza/sponza.obj");
	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/vokselia_spawn/vokselia_spawn.obj");
	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/fireplace_room/fireplace_room.obj");

	m_gbuffer.Initialize();

	m_quad.Initialize();
	m_tester.Initialize();
	m_tester.LoadFromFile("shaders/tester.vert", GL_VERTEX_SHADER);
	m_tester.LoadFromFile("shaders/tester.frag", GL_FRAGMENT_SHADER);

	m_shadowmap.Initialize();
	m_shadowmap.Update(m_scene, {-24.6f, 50.0f, 12.0f});

	m_renderer.Initialize();
}

Application::~Application()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::Run()
{
	char title[64];
	while(!glfwWindowShouldClose(m_window))
	{
		glViewport(0, 0, kWidth, kHeight);
		m_fps.Update();
		m_camera.Control(m_window, m_fps);
		m_camera.Update();

		sprintf(title, "fps: %f", m_fps.GetFps());
		glfwSetWindowTitle(m_window, title);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_gbuffer.Update(m_scene, m_camera);
		m_renderer.DirectLight(m_quad, m_camera, m_gbuffer, m_shadowmap);

		m_renderer.GetRadiance().Bind(2);
		m_tester.Use();
		m_quad.Render();

		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}
}
