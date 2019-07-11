//
// Created by adamyuan on 19-5-3.
//

#ifndef SPARSEVOXELOCTREE_APPLICATION_HPP
#define SPARSEVOXELOCTREE_APPLICATION_HPP

#include <mygl3/utils/framerate.hpp>
#include <memory>
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
	std::unique_ptr<Scene> m_scene;
	Camera m_camera;

	ShaderSettings m_settings;

	DeepGBuffer m_gbuffer;
	ShadowMap m_shadowmap;
	ShadowMapBlurer m_shadowmap_blurer;
	GIRenderer m_renderer;
	GIBlurer m_gi_blurer;
	GITemporalFilter m_gi_temporal;
	mygl3::Framerate m_fps;

	ScreenQuad m_quad;
	mygl3::Shader m_final_pass_shader;

	glm::vec3 m_sun_position{0.0f, 10.0f, 0.0f};
	bool m_sun_moved{true};
	void sun_key_control();

	static void glfw_key_callback(GLFWwindow *window, int key, int, int action, int);

	void load_recompilable_shaders();

	bool m_show_ui = true;
	void ui_control();
	void ui_load_scene();
	void ui_info_and_guide();
	void ui_deepgbuffer_settings();
	void ui_radiosity_settings();
	void ui_radiosity_blur_settings();
	bool ui_radiosity_temporal_blend_settings();
	static bool ui_file_open(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
					  const std::vector<std::string> &filters);
public:
	Application();
	~Application();
	void LoadScene(const char *filename);
	void Run();
};


#endif //SPARSEVOXELOCTREE_APPLICATION_HPP
