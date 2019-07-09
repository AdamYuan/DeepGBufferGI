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
	m_albedo.Storage(kWidth, kHeight, 2, GL_RGB8, 1);
	m_albedo.SetWrapFilter(GL_CLAMP_TO_BORDER);
	m_normal.Initialize();
	m_normal.Storage(kWidth, kHeight, 2, GL_RG8_SNORM, kMipmapLayers);
	m_albedo.SetWrapFilter(GL_CLAMP_TO_BORDER);
	m_depth.Initialize();
	m_depth.Storage(kWidth, kHeight, 2, GL_DEPTH_COMPONENT32, kMipmapLayers);
	m_albedo.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_last_depth.Initialize();
	m_last_depth.Storage(kWidth, kHeight, GL_DEPTH_COMPONENT32);
	m_last_depth.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_fbo.Initialize();
	m_fbo.AttachTexture(m_albedo, GL_COLOR_ATTACHMENT0);
	m_fbo.AttachTexture(m_normal, GL_COLOR_ATTACHMENT1);
	m_fbo.AttachTexture(m_depth, GL_DEPTH_ATTACHMENT);

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glNamedFramebufferDrawBuffers(m_fbo.Get(), 2, attachments);

}

void DeepGBuffer::LoadShader(const ShaderSettings &settings)
{
	m_shader.~Shader();
	m_shader.Initialize();
	m_shader.LoadFromFile("shaders/deepgbuffer.vert", GL_VERTEX_SHADER);
	m_shader.LoadFromFile("shaders/deepgbuffer.geom", GL_GEOMETRY_SHADER);
	m_shader.Load(settings.GetDeepGBufferFragSrc().c_str(), GL_FRAGMENT_SHADER);
	m_unif_last_view = m_shader.GetUniform("uLastView");
}

void DeepGBuffer::Update(const Scene &scene, const Camera &camera)
{
	static glm::mat4 last_view_mat;

	//copy depth texture
	glCopyImageSubData(m_depth.Get(), GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
			m_last_depth.Get(), GL_TEXTURE_2D, 0, 0, 0, 0,
			kWidth, kHeight, 1);
	m_last_depth.Bind(kLastDepthSampler2D);

	m_fbo.Bind();

	glViewport(0, 0, kWidth, kHeight);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_shader.Use();
	m_shader.SetMat4(m_unif_last_view, glm::value_ptr(last_view_mat));
	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	scene.Draw();

	mygl3::FrameBuffer::Unbind();

	last_view_mat = camera.m_view;
	m_normal.GenerateMipmap();
	m_depth.GenerateMipmap();
}
