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
	m_radiance.Storage(kWidth, kHeight, 2, GL_R11F_G11F_B10F, mygl3::Texture2DArray::GetLevelCount(kWidth, kHeight));
	m_radiance.SetSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_radiance.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_tmp_radiance.Initialize();
	m_tmp_radiance.Storage(kWidth, kHeight, GL_R11F_G11F_B10F);

	m_direct_light_fbo.Initialize();
	m_direct_light_fbo.AttachTexture(m_radiance, GL_COLOR_ATTACHMENT0);

	m_radiosity_fbo.Initialize();
	m_radiosity_fbo.AttachTexture(m_tmp_radiance, GL_COLOR_ATTACHMENT0);

	m_direct_light_shader.Initialize();
	m_direct_light_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_direct_light_shader.LoadFromFile("shaders/directlight.frag", GL_FRAGMENT_SHADER);
	m_direct_light_shader.LoadFromFile("shaders/directlight.geom", GL_GEOMETRY_SHADER);
	m_direct_light_unif_shadow_transform = m_direct_light_shader.GetUniform("uShadowTransform");
	m_direct_light_unif_light_dir = m_direct_light_shader.GetUniform("uLightDir");

	m_radiosity_shader.Initialize();
	m_radiosity_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_radiosity_shader.LoadFromFile("shaders/radiosity.frag", GL_FRAGMENT_SHADER);
}

void GIRenderer::DirectLight(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer, const ShadowMap &shadowmap)
{
	m_direct_light_fbo.Bind();
	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	shadowmap.GetTexture().Bind(kShadowMapSampler2D);
	gbuffer.GetAlbedo().Bind(kAlbedoSampler2DArray);
	gbuffer.GetNormal().Bind(kNormalSampler2DArray);
	gbuffer.GetDepth().Bind(kDepthSampler2DArray);

	m_direct_light_shader.Use();
	m_direct_light_shader.SetMat4(m_direct_light_unif_shadow_transform, glm::value_ptr(shadowmap.GetShadowTransform()));
	m_direct_light_shader.SetVec3(m_direct_light_unif_light_dir, glm::value_ptr(shadowmap.GetLightDir()));
	quad.Render();

	mygl3::FrameBuffer::Unbind();

	m_radiance.GenerateMipmap();
}

void GIRenderer::Radiosity(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer)
{
	m_radiosity_fbo.Bind();

	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	gbuffer.GetAlbedo().Bind(kAlbedoSampler2DArray);
	gbuffer.GetNormal().Bind(kNormalSampler2DArray);
	gbuffer.GetDepth().Bind(kDepthSampler2DArray);
	m_radiance.Bind(kRadianceSampler2DArray);

	m_radiosity_shader.Use();
	quad.Render();

	mygl3::FrameBuffer::Unbind();
}
