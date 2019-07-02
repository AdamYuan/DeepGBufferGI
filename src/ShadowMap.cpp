//
// Created by adamyuan on 19-6-28.
//

#include <glm/gtc/type_ptr.hpp>
#include "ShadowMap.hpp"
#include "Config.hpp"

void ShadowMap::Initialize()
{
	m_texture.Initialize();
	m_texture.Storage(kShadowMapSize, kShadowMapSize, GL_DEPTH_COMPONENT32);
	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(m_texture.Get(), GL_TEXTURE_BORDER_COLOR, border_color);
	m_texture.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_texture.SetWrapFilter(GL_CLAMP_TO_BORDER);
	glTextureParameteri(m_texture.Get(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	m_shader.Initialize();
	m_shader.LoadFromFile("shaders/shadowmap.vert", GL_VERTEX_SHADER);
	m_shader.LoadFromFile("shaders/shadowmap.frag", GL_FRAGMENT_SHADER);
	m_unif_transform = m_shader.GetUniform("uTransform");

	m_fbo.Initialize();
	m_fbo.AttachTexture(m_texture, GL_DEPTH_ATTACHMENT);
}

void ShadowMap::Update(const Scene &scene, const glm::mat4 &transform)
{
	m_transform = transform;
	m_fbo.Bind();
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_shader.Use();
	m_shader.SetMat4(m_unif_transform, glm::value_ptr(transform));
	scene.Draw();

	mygl3::FrameBuffer::Unbind();
}

void ShadowMap::Update(const Scene &scene, const glm::vec3 &sun_pos)
{
	glm::vec3 light_dir = glm::normalize(-sun_pos);
	glm::vec3 up{0.0f, 1.0f, 0.0f};
	if(light_dir.x == 0.0f && light_dir.z == 0.0f)
		up = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::mat4 light_view = glm::lookAt(glm::vec3(0.0f), light_dir, up);
	glm::mat4 light_projection = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, -1.5f, 1.5f);

	glm::mat4 transform = light_projection * light_view;
	Update(scene, transform);
}
