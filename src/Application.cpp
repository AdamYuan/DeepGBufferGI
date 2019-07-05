//
// Created by adamyuan on 19-5-3.
//

#include <glm/gtc/matrix_transform.hpp>
#include "Application.hpp"
#include "Config.hpp"
#include "OglBindings.hpp"

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

	m_quad.Initialize();

	m_camera.Initialize();

	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/sibenik/sibenik.obj");
	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/living_room/living_room.obj");
	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/San_Miguel/san-miguel-low-poly.obj");
	m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/sponza/sponza.obj");
	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/CornellBox/CornellBox-Original.obj");
	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/vokselia_spawn/vokselia_spawn.obj");
	//m_scene.Initialize("/home/adamyuan/Projects/Adypt/models/fireplace_room/fireplace_room.obj");

	m_gbuffer.Initialize();
	m_tester.Initialize();
	m_tester.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_tester.LoadFromFile("shaders/tester.frag", GL_FRAGMENT_SHADER);

	m_shadowmap.Initialize();
	m_shadowmap.Update(m_scene, {-24.6f, 50.0f, 12.0f});

	ShadowMapBlurer shadowmap_blurer;
	shadowmap_blurer.Initialize(m_shadowmap);
	shadowmap_blurer.Blur(m_quad);

	m_renderer.Initialize();
	m_gi_blurer.Initialize(m_renderer);

	/*m_test_texture.Initialize();
	m_test_texture.Storage(kWidth, kHeight, GL_RGBA8);
	m_test_fbo.Initialize();
	m_test_fbo.AttachTexture(m_test_texture, GL_COLOR_ATTACHMENT0);

	m_test_fbo.Bind();
	glClear(GL_COLOR_BUFFER_BIT);
	mygl3::FrameBuffer::Unbind();

	m_test_texture.Bind(2);
	glBindImageTexture(3, m_test_texture.Get(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

	m_tester.Use();
	m_quad.Render();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);*/
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

		m_gbuffer.Update(m_scene, m_camera);
		m_renderer.DirectLight(m_quad, m_camera, m_gbuffer, m_shadowmap);
		m_renderer.Radiosity(m_quad, m_camera, m_gbuffer);
		//m_gi_blurer.Blur(m_quad);

		m_renderer.GetRadiance().Bind(kRadianceSampler2DArray);
		m_renderer.GetGIRadiance().Bind(kGIRadianceSampler2D);
		//m_shadowmap.GetTexture().Bind(2);
		m_tester.Use();
		m_quad.Render();

		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}
}
