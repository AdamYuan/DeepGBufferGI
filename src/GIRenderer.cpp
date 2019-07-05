//
// Created by adamyuan on 19-7-1.
//

#include <glm/gtc/type_ptr.hpp>
#include "GIRenderer.hpp"
#include "Config.hpp"
#include "OglBindings.hpp"

#include <GLFW/glfw3.h>

void GIRenderer::Initialize()
{
	m_radiance.Initialize();
	m_radiance.Storage(kWidth, kHeight, 2, GL_R11F_G11F_B10F, kMaxMip);
	m_radiance.SetSizeFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
	m_radiance.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_gi_radiance.Initialize();
	m_gi_radiance.Storage(kWidth, kHeight, GL_R11F_G11F_B10F);

	m_direct_light_fbo.Initialize();
	m_direct_light_fbo.AttachTexture(m_radiance, GL_COLOR_ATTACHMENT0);

	m_radiosity_fbo.Initialize();
	m_radiosity_fbo.AttachTexture(m_gi_radiance, GL_COLOR_ATTACHMENT0);

	m_direct_light_shader.Initialize();
	m_direct_light_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_direct_light_shader.LoadFromFile("shaders/directlight.frag", GL_FRAGMENT_SHADER);
	m_direct_light_shader.LoadFromFile("shaders/directlight.geom", GL_GEOMETRY_SHADER);
	m_direct_light_unif_shadow_transform = m_direct_light_shader.GetUniform("uShadowTransform");
	m_direct_light_unif_light_dir = m_direct_light_shader.GetUniform("uLightDir");

	m_radiosity_shader.Initialize();
	m_radiosity_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_radiosity_shader.LoadFromFile("shaders/radiosity.frag", GL_FRAGMENT_SHADER);
	m_radiosity_unif_time = m_radiosity_shader.GetUniform("uTime");
	m_radiosity_unif_resolution = m_radiosity_shader.GetUniform("uResolution");
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
	constexpr GLint kResolution[] = {kWidth, kHeight};
	m_radiosity_fbo.Bind();

	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	gbuffer.GetAlbedo().Bind(kAlbedoSampler2DArray);
	gbuffer.GetNormal().Bind(kNormalSampler2DArray);
	gbuffer.GetDepth().Bind(kDepthSampler2DArray);
	m_radiance.Bind(kRadianceSampler2DArray);

	m_radiosity_shader.Use();
	m_radiosity_shader.SetFloat(m_radiosity_unif_time, glfwGetTime());
	m_radiosity_shader.SetIVec2(m_radiosity_unif_resolution, kResolution);
	quad.Render();

	mygl3::FrameBuffer::Unbind();
}

void GIBlurer::Initialize(const GIRenderer &renderer)
{
	m_tmp_texture.InitializeWithoutTarget();
	glTextureView(m_tmp_texture.Get(), GL_TEXTURE_2D, renderer.GetRadiance().Get(), GL_R11F_G11F_B10F, 0, 1, 1, 1);
	//use the second layer of m_radiance as tmp texture
	m_tmp_texture.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_tmp_texture.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_target = &renderer.GetGIRadiance();

	m_shader.Initialize();
	m_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_shader.LoadFromFile("shaders/radiosity_blur.frag", GL_FRAGMENT_SHADER);
	m_unif_direction = m_shader.GetUniform("uDirection");

	m_blur_fbo[0].Initialize();
	m_blur_fbo[0].AttachTexture(m_tmp_texture, GL_COLOR_ATTACHMENT0);
	m_blur_fbo[1].Initialize();
	m_blur_fbo[1].AttachTexture(*m_target, GL_COLOR_ATTACHMENT0);
}

void GIBlurer::Blur(const ScreenQuad &quad, const DeepGBuffer &gbuffer)
{
	constexpr GLint dir0[] = {1, 0}, dir1[] = {0, 1};
	m_shader.Use();

	gbuffer.GetNormal().Bind(kNormalSampler2DArray);
	gbuffer.GetDepth().Bind(kDepthSampler2DArray);

	m_blur_fbo[0].Bind();
	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	m_target->Bind(kGIRadianceSampler2D);
	m_shader.SetIVec2(m_unif_direction, dir0);
	quad.Render();
	mygl3::FrameBuffer::Unbind();

	m_blur_fbo[1].Bind();
	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	m_tmp_texture.Bind(kGIRadianceSampler2D);
	m_shader.SetIVec2(m_unif_direction, dir1);
	quad.Render();
	mygl3::FrameBuffer::Unbind();
}
