//
// Created by adamyuan on 19-5-3.
//

#ifndef SPARSEVOXELOCTREE_APPLICATION_HPP
#define SPARSEVOXELOCTREE_APPLICATION_HPP

#include <mygl3/utils/framerate.hpp>
#include "Scene.hpp"
#include "DeepGBuffer.hpp"
#include "ShadowMap.hpp"
#include "ScreenQuad.hpp"
#include "GIRenderer.hpp"
#include <GLFW/glfw3.h>

class Application
{
private:
	GLFWwindow *m_window;
	Camera m_camera;
	Scene m_scene;
	DeepGBuffer m_gbuffer;
	ShadowMap m_shadowmap;
	GIRenderer m_renderer;
	mygl3::Framerate m_fps;

	ScreenQuad m_quad;
	mygl3::Shader m_tester;

	//test area
	//mygl3::FrameBuffer m_test_fbo;
	//mygl3::Texture2D m_test_texture;
public:
	Application();
	~Application();
	void Run();
};


#endif //SPARSEVOXELOCTREE_APPLICATION_HPP
