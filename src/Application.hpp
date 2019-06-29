//
// Created by adamyuan on 19-5-3.
//

#ifndef SPARSEVOXELOCTREE_APPLICATION_HPP
#define SPARSEVOXELOCTREE_APPLICATION_HPP

#include <mygl3/utils/framerate.hpp>
#include "Scene.hpp"
#include "DeepGBuffer.hpp"
#include "ScreenQuad.hpp"
#include <GLFW/glfw3.h>

class Application
{
private:
	GLFWwindow *m_window;
	Camera m_camera;
	Scene m_scene;
	DeepGBuffer m_gbuffer;
	mygl3::Framerate m_fps;

	mygl3::Shader m_tester;
	ScreenQuad m_quad;
public:
	Application();
	~Application();
	void Run();
};


#endif //SPARSEVOXELOCTREE_APPLICATION_HPP
