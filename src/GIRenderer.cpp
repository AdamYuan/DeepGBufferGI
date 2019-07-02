//
// Created by adamyuan on 19-7-1.
//

#include <glm/gtc/type_ptr.hpp>
#include "GIRenderer.hpp"
#include "Config.hpp"
#include "OglBindings.hpp"

void GIRenderer::Initialize()
{
	m_radiance.Initialize();
	m_radiance.Storage(kWidth, kHeight, 2, GL_R11F_G11F_B10F);
	m_fbo.Initialize();
	m_fbo.AttachTexture(m_radiance, GL_COLOR_ATTACHMENT0);

	m_direct_light_shader.Initialize();
	m_direct_light_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_direct_light_shader.LoadFromFile("shaders/directlight.frag", GL_FRAGMENT_SHADER);
	m_direct_light_shader.LoadFromFile("shaders/directlight.geom", GL_GEOMETRY_SHADER);
	m_diret_light_unif_shadow_transform = m_direct_light_shader.GetUniform("uShadowTransform");
}

void GIRenderer::DirectLight(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer, const ShadowMap &shadowmap)
{
	m_fbo.Bind();

	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	shadowmap.GetTexture().Bind(kShadowMapSampler2D);
	gbuffer.GetAlbedo().Bind(kGBufferAlbedoSampler2D);
	gbuffer.GetNormal().Bind(kGBufferNormalSampler2D);
	gbuffer.GetDepth().Bind(kGBufferDepthSampler2D);

	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	m_direct_light_shader.Use();
	m_direct_light_shader.SetMat4(m_diret_light_unif_shadow_transform, glm::value_ptr(shadowmap.GetShadowTransform()));
	quad.Render();

	mygl3::FrameBuffer::Unbind();
}

void GIRenderer::Radiosity(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer)
{

}
