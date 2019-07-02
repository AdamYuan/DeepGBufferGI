//
// Created by adamyuan on 19-6-28.
//

#include <glm/gtc/type_ptr.hpp>
#include "ShadowMap.hpp"
#include "Config.hpp"
#include "OglBindings.hpp"

void ShadowMap::Initialize()
{
	m_texture.Initialize();
	m_texture.Storage(kShadowMapSize, kShadowMapSize, GL_RGBA32F);
	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(m_texture.Get(), GL_TEXTURE_BORDER_COLOR, border_color);
	m_texture.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_texture.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_shader.Initialize();
	m_shader.LoadFromFile("shaders/shadowmap.vert", GL_VERTEX_SHADER);
	m_shader.LoadFromFile("shaders/shadowmap.frag", GL_FRAGMENT_SHADER);
	m_unif_transform = m_shader.GetUniform("uTransform");

	m_rbo.Initialize();
	m_rbo.Load(GL_DEPTH_COMPONENT, kShadowMapSize, kShadowMapSize);

	m_fbo.Initialize();
	m_fbo.AttachTexture(m_texture, GL_COLOR_ATTACHMENT0);
	m_fbo.AttachRenderbuffer(m_rbo, GL_DEPTH_ATTACHMENT);
}

void ShadowMap::Update(const Scene &scene, const glm::vec3 &sun_pos)
{
	m_light_dir = glm::normalize(-sun_pos);

	//calculate transformation
	glm::vec3 up{0.0f, 1.0f, 0.0f};
	if(m_light_dir.x == 0.0f && m_light_dir.z == 0.0f)
		up = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::mat4 light_view = glm::lookAt(glm::vec3(0.0f), m_light_dir, up);
	glm::mat4 light_projection = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, -1.5f, 1.5f);
	m_transform = light_projection * light_view;

	//render
	m_fbo.Bind();
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_shader.Use();
	m_shader.SetMat4(m_unif_transform, glm::value_ptr(m_transform));
	scene.Draw();

	mygl3::FrameBuffer::Unbind();
}

void ShadowMapBlurer::Initialize()
{
	m_tmp_texture.Initialize();
	m_tmp_texture.Storage(kShadowMapSize, kShadowMapSize, GL_RGBA32F);
	m_tmp_texture.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_tmp_texture.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_shader.Initialize();
	m_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_shader.LoadFromFile("shaders/shadowmap_blur.frag", GL_FRAGMENT_SHADER);
	m_unif_direction = m_shader.GetUniform("uDirection");

	m_blur_fbo[0].Initialize();
	m_blur_fbo[0].AttachTexture(m_tmp_texture, GL_COLOR_ATTACHMENT0);
	m_blur_fbo[1].Initialize();
}

void ShadowMapBlurer::Blur(const ScreenQuad &quad, const ShadowMap &shadowmap)
{
	constexpr GLfloat dir0[] = {1.0f, 0.0f}, dir1[] = {0.0f, 1.0f};
	m_shader.Use();

	m_blur_fbo[0].Bind();
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);
	glClear(GL_COLOR_BUFFER_BIT);
	shadowmap.GetTexture().Bind(kShadowMapSampler2D);
	m_shader.SetVec2(m_unif_direction, dir0);
	quad.Render();
	mygl3::FrameBuffer::Unbind();

	m_blur_fbo[1].AttachTexture(shadowmap.GetTexture(), GL_COLOR_ATTACHMENT0);
	m_blur_fbo[1].Bind();
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);
	glClear(GL_COLOR_BUFFER_BIT);
	m_tmp_texture.Bind(kShadowMapSampler2D);
	m_shader.SetVec2(m_unif_direction, dir1);
	quad.Render();
	mygl3::FrameBuffer::Unbind();
}
