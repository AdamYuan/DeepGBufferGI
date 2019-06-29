//
// Created by adamyuan on 19-6-26.
//

#include <glm/gtc/type_ptr.hpp>
#include "DeepGBuffer.hpp"
#include "Config.hpp"
#include "OglBindings.hpp"

void DeepGBuffer::Initialize()
{
	m_albedo.Initialize();
	m_albedo.Storage(kWidth, kHeight, 2, GL_RGB8);
	m_depth.Initialize();
	m_depth.Storage(kWidth, kHeight, 2, GL_DEPTH_COMPONENT24);
	m_normal.Initialize();
	m_normal.Storage(kWidth, kHeight, 2, GL_RG8_SNORM);
	m_radiance.Initialize();
	m_radiance.Storage(kWidth, kHeight, 2, GL_R11F_G11F_B10F);

	m_last_depth.Initialize();
	m_last_depth.Storage(kWidth, kHeight, GL_DEPTH_COMPONENT24);
	m_last_depth.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_fbo.Initialize();
	m_fbo.AttachTexture(m_albedo, GL_COLOR_ATTACHMENT0);
	m_fbo.AttachTexture(m_normal, GL_COLOR_ATTACHMENT1);
	m_fbo.AttachTexture(m_radiance, GL_COLOR_ATTACHMENT2);
	m_fbo.AttachTexture(m_depth, GL_DEPTH_ATTACHMENT);

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glNamedFramebufferDrawBuffers(m_fbo.Get(), 3, attachments);

	m_shader.Initialize();
	m_shader.LoadFromFile("shaders/deepgbuffer.vert", GL_VERTEX_SHADER);
	m_shader.LoadFromFile("shaders/deepgbuffer.geom", GL_GEOMETRY_SHADER);
	m_shader.LoadFromFile("shaders/deepgbuffer.frag", GL_FRAGMENT_SHADER);
	m_unif_view = m_shader.GetUniform("uView");
	m_unif_last_view = m_shader.GetUniform("uLastView");
	m_unif_projection = m_shader.GetUniform("uProjection");
}

void DeepGBuffer::Update(const Scene &scene, const Camera &camera)
{
	static glm::mat4 last_view_mat;

	//copy depth texture
	glCopyImageSubData(m_depth.Get(), GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
			m_last_depth.Get(), GL_TEXTURE_2D, 0, 0, 0, 0,
			kWidth, kHeight, 1);
	m_last_depth.Bind(kLastDepthTexture);

	m_fbo.Bind();

	glViewport(0, 0, kWidth, kHeight);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_shader.Use();
	glm::mat4 view_mat = camera.GetView();
	m_shader.SetMat4(m_unif_view, glm::value_ptr(view_mat));
	m_shader.SetMat4(m_unif_last_view, glm::value_ptr(last_view_mat));
	m_shader.SetMat4(m_unif_projection, glm::value_ptr(camera.GetProjection()));
	scene.Draw();

	mygl3::FrameBuffer::Unbind();

	last_view_mat = view_mat;
}
