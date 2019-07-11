//
// Created by adamyuan on 19-5-3.
//

#include <glm/gtc/matrix_transform.hpp>
#include "Application.hpp"
#include "Config.hpp"
#include "OglBindings.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <portable-file-dialogs.h>

Application::Application()
{
	//Initialize OpenGL and GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_window = glfwCreateWindow(1280, 720, "Deep G-Buffer Global Illumination", nullptr, nullptr);
	glfwMakeContextCurrent(m_window);
	glfwSetWindowUserPointer(m_window, (void*)this);
	glfwSetKeyCallback(m_window, glfw_key_callback);
	gl3wInit();

	//Initialize contents
	m_settings.Initialize();
	m_quad.Initialize();
	m_camera.Initialize();
	m_gbuffer.Initialize();
	m_shadowmap.Initialize();
	m_shadowmap_blurer.Initialize(m_shadowmap);
	m_renderer.Initialize();
	m_gi_blurer.Initialize(m_renderer);
	m_gi_temporal.Initialize(m_renderer);
	load_recompilable_shaders();
	m_final_pass_shader.Initialize();
	m_final_pass_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_final_pass_shader.LoadFromFile("shaders/finalpass.frag", GL_FRAGMENT_SHADER);


	//Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiStyle &st = ImGui::GetStyle();
	st.WindowBorderSize = 0.0f;
	st.Alpha = 0.7f;
	st.WindowRounding = 0.0f;
	st.ChildRounding = 0.0f;
	st.FrameRounding = 0.0f;
	st.ScrollbarRounding = 0.0f;
	st.GrabRounding = 0.0f;
	st.TabRounding = 0.0f;

	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");

	//Initialize portable-file-dialogs
	pfd::settings::verbose(true);
}

Application::~Application()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::Run()
{
	while(!glfwWindowShouldClose(m_window))
	{
		m_fps.Update();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glViewport(0, 0, kWidth, kHeight);
		glClear(GL_COLOR_BUFFER_BIT);

		if(m_show_ui) ui_control();

		if(m_scene)
		{
			sun_key_control();
			m_camera.Control(m_window, m_fps);
			m_camera.Update();

			m_gbuffer.Update(*m_scene, m_camera);
			m_gi_temporal.Reproject(m_quad, m_camera, m_gbuffer);

			m_renderer.PrepareInputRadiance(m_quad, m_camera, m_gbuffer, m_shadowmap, m_gi_temporal);
			m_renderer.SampleRadiosity(m_quad, m_camera, m_gbuffer);
			m_gi_blurer.Blur(m_quad, m_gbuffer);
			m_gi_temporal.Blend(m_quad);

			m_gbuffer.GetAlbedo().Bind(kAlbedoSampler2DArray);
			m_renderer.GetInputRadiance().Bind(kInputRadianceSampler2DArray);
			m_renderer.GetOutputRadiance().Bind(kOutputRadianceSampler2D);
			m_final_pass_shader.Use();
			m_quad.Render();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}
}

void Application::sun_key_control()
{
	float speed = m_fps.GetDelta();
	if(glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		m_sun_position.x -= speed;
		m_sun_moved = true;
	}
	if(glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		m_sun_position.x += speed;
		m_sun_moved = true;
	}
	if(glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		m_sun_position.z -= speed;
		m_sun_moved = true;
	}
	if(glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		m_sun_position.z += speed;
		m_sun_moved = true;
	}

	if(m_sun_moved)
	{
		m_shadowmap.Update(*m_scene, m_sun_position);
		m_shadowmap_blurer.Blur(m_quad);
		m_sun_moved = false;
	}
}

void Application::LoadScene(const char *filename)
{
	m_scene.reset(new Scene);
	if(!m_scene->Initialize(filename)) m_scene.reset();
	m_sun_moved = true;
}

void Application::load_recompilable_shaders()
{
	m_gbuffer.LoadShader(m_settings);
	m_renderer.LoadRadiosityShader(m_settings);
	m_gi_blurer.LoadShader(m_settings);
	m_gi_temporal.LoadBlendShader(m_settings);
}

void Application::ui_control()
{
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f),
							ImGuiCond_Always, ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(0.0f, kHeight), ImGuiCond_Always);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.4f)); // Transparent background
	if (ImGui::Begin("INFO", nullptr,
					 ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize
					 |ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove
					 |ImGuiWindowFlags_NoSavedSettings))
	{
		ui_load_scene();
		ImGui::Separator();

		ui_info_and_guide();
		ImGui::Separator();

		if(ImGui::Button("Apply Settings [C]")) { load_recompilable_shaders(); }

		if(ImGui::CollapsingHeader("Deep G-Buffer Settings", ImGuiTreeNodeFlags_DefaultOpen))
			ui_deepgbuffer_settings();

		if(ImGui::CollapsingHeader("Radiosity Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ui_radiosity_settings();
			ui_radiosity_blur_settings();
		}

		if(ImGui::CollapsingHeader("Temporal Blend Settings", ImGuiTreeNodeFlags_DefaultOpen))
			ui_radiosity_temporal_blend_settings();

		ImGui::End();
	}
	ImGui::PopStyleColor();
}

bool
Application::ui_file_open(const char *label, const char *btn, char *buf, size_t buf_size, const char *title,
						  const std::vector<std::string> &filters)
{
	bool ret = ImGui::InputText(label, buf, buf_size);
	ImGui::SameLine();

	if(ImGui::Button(btn))
	{
		auto file_dialog = pfd::open_file(title, "", filters, false);
		if(!file_dialog.result().empty()) strcpy(buf, file_dialog.result().front().c_str());
		ret = true;
	}
	return ret;
}

void Application::ui_deepgbuffer_settings()
{
	ImGui::DragFloat("Min Separation", &m_settings.m_deepgbuffer_min_separate, 0.01f, 0.0f, 2.0f);
}

void Application::ui_radiosity_settings()
{
	ImGui::Text("Load pattern: ");
	ImGui::SameLine();
	if(ImGui::Button("High Performance"))
		m_settings.SetRadiosityQuality(0);
	ImGui::SameLine();
	if(ImGui::Button("Balanced"))
		m_settings.SetRadiosityQuality(1);
	ImGui::SameLine();
	if(ImGui::Button("High Quality"))
		m_settings.SetRadiosityQuality(2);
	ImGui::DragFloat("Sample Radius", &m_settings.m_radiosity_radius, 0.01f, 0.0f, 2.0f);
	ImGui::DragInt("Sample Count", &m_settings.m_radiosity_sample_cnt, 1, 1, 90);
	ImGui::DragInt("Min Mip-Level", &m_settings.m_radiosity_min_mip, 1, 0, kMipmapLayers - 1);
	ImGui::Checkbox("Y Normal Test", &m_settings.m_radiosity_use_y_normal_test);
}

void Application::ui_radiosity_blur_settings()
{
	ImGui::DragInt("Blur Radius", &m_settings.m_radiosity_blur_radius, 1, 1, 6);
	ImGui::DragInt("Blur Scale", &m_settings.m_radiosity_blur_scale, 1, 1, 6);
	ImGui::DragFloat("Edge Sharpness", &m_settings.m_radiosity_blur_edge_sharpness, 0.1f, 0.0f, 2.0f);
}

bool Application::ui_radiosity_temporal_blend_settings()
{
	return ImGui::DragFloat("Alpha", &m_settings.m_radiosity_temporal_blend_alpha, 0.01f, 0.0f, 1.0f);
}

void Application::ui_load_scene()
{
	bool open_popup = false;

	if(ImGui::Button("Load Scene"))
		open_popup = true;

	if(open_popup)
		ImGui::OpenPopup("Load Scene##1");

	if (ImGui::BeginPopupModal("Load Scene##1", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		constexpr size_t kFilenameBufSize = 512;
		static char name_buf[kFilenameBufSize];

		ui_file_open("OBJ Filename", "...##5", name_buf, kFilenameBufSize, "OBJ Filename",
					   {"OBJ File (.obj)", "*.obj", "All Files", "*"});

		{
			if (ImGui::Button("Load", ImVec2(256, 0)))
			{
				LoadScene(name_buf);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(256, 0)))
				ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Application::ui_info_and_guide()
{
	ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
	ImGui::Text("FPS: %f", m_fps.GetFps());
	ImGui::Text("Drag Mouse to change perspective");
	ImGui::Text("Press [WASD] to move around");
	ImGui::Text("Press [Arrow Keys] to change light direction");
	ImGui::Text("Press [X] to toggle GUI");
}

void Application::glfw_key_callback(GLFWwindow *window, int key, int, int action, int)
{
	auto *app = (Application *)glfwGetWindowUserPointer(window);
	if(action == GLFW_PRESS)
	{
		if(key == GLFW_KEY_X)
			app->m_show_ui = !app->m_show_ui;
		if(key == GLFW_KEY_C)
			app->load_recompilable_shaders();
	}
}
