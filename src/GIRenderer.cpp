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
	m_input_radiance.Initialize();
	m_input_radiance.Storage(kWidth, kHeight, 2, GL_R11F_G11F_B10F, kMipmapLayers);
	m_input_radiance.SetSizeFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
	m_input_radiance.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_output_radiance.Initialize();
	m_output_radiance.Storage(kWidth, kHeight, GL_R11F_G11F_B10F);
	m_output_radiance.SetSizeFilter(GL_LINEAR, GL_LINEAR); //for temporal filtering
	m_output_radiance.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_radiosity_input_fbo.Initialize();
	m_radiosity_input_fbo.AttachTexture(m_input_radiance, GL_COLOR_ATTACHMENT0);

	m_radiosity_fbo.Initialize();
	m_radiosity_fbo.AttachTexture(m_output_radiance, GL_COLOR_ATTACHMENT0);

	m_radiosity_input_shader.Initialize();
	m_radiosity_input_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_radiosity_input_shader.LoadFromFile("shaders/radiosity_input.frag", GL_FRAGMENT_SHADER);
	m_radiosity_input_shader.LoadFromFile("shaders/quad.geom", GL_GEOMETRY_SHADER);
	m_radiosity_input_unif_shadow_transform = m_radiosity_input_shader.GetUniform("uShadowTransform");
	m_radiosity_input_unif_light_dir = m_radiosity_input_shader.GetUniform("uLightDir");
}

void GIRenderer::LoadRadiosityShader(const ShaderSettings &settings)
{
	m_radiosity_shader.~Shader();
	m_radiosity_shader.Initialize();
	m_radiosity_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_radiosity_shader.Load(settings.GetRadiositySrc().c_str(), GL_FRAGMENT_SHADER);
	m_radiosity_unif_time = m_radiosity_shader.GetUniform("uTime");
	m_radiosity_unif_resolution = m_radiosity_shader.GetUniform("uResolution");
	constexpr GLint kResolution[] = {kWidth, kHeight};
	m_radiosity_shader.SetIVec2(m_radiosity_unif_resolution, kResolution);
}


void GIRenderer::PrepareInputRadiance(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer,
									  const ShadowMap &shadowmap, const GITemporalFilter &temporal)
{
	m_radiosity_input_fbo.Bind();
	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	shadowmap.GetTexture().Bind(kShadowMapSampler2D);
	gbuffer.GetAlbedo().Bind(kAlbedoSampler2DArray);
	gbuffer.GetNormal().Bind(kNormalSampler2DArray);
	gbuffer.GetDepth().Bind(kDepthSampler2DArray);
	temporal.GetReprojectedRadiance().Bind(kReprojectedRadianceSampler2D);

	m_radiosity_input_shader.Use();
	m_radiosity_input_shader.SetMat4(m_radiosity_input_unif_shadow_transform, glm::value_ptr(shadowmap.GetShadowTransform()));
	m_radiosity_input_shader.SetVec3(m_radiosity_input_unif_light_dir, glm::value_ptr(shadowmap.GetLightDir()));
	quad.Render();

	mygl3::FrameBuffer::Unbind();

	m_input_radiance.GenerateMipmap();
}

void GIRenderer::SampleRadiosity(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer)
{
	m_radiosity_fbo.Bind();

	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	gbuffer.GetAlbedo().Bind(kAlbedoSampler2DArray);
	gbuffer.GetNormal().Bind(kNormalSampler2DArray);
	gbuffer.GetDepth().Bind(kDepthSampler2DArray);
	m_input_radiance.Bind(kInputRadianceSampler2DArray);

	m_radiosity_shader.Use();
	m_radiosity_shader.SetFloat(m_radiosity_unif_time, glfwGetTime() * 1000.0f);
	quad.Render();

	mygl3::FrameBuffer::Unbind();
}

void GIBlurer::Initialize(const GIRenderer &renderer)
{
	m_tmp_texture.InitializeWithoutTarget();
	glTextureView(m_tmp_texture.Get(), GL_TEXTURE_2D, renderer.GetInputRadiance().Get(), GL_R11F_G11F_B10F, 0, 1, 1, 1);
	//use the second layer of m_input_radiance as tmp texture
	m_tmp_texture.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_tmp_texture.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_target = &renderer.GetOutputRadiance();

	m_blur_fbo[0].Initialize();
	m_blur_fbo[0].AttachTexture(m_tmp_texture, GL_COLOR_ATTACHMENT0);
	m_blur_fbo[1].Initialize();
	m_blur_fbo[1].AttachTexture(*m_target, GL_COLOR_ATTACHMENT0);
}

void GIBlurer::LoadShader(const ShaderSettings &settings)
{
	m_shader.~Shader();
	m_shader.Initialize();
	m_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_shader.Load(settings.GetRadiosityBlurSrc().c_str(), GL_FRAGMENT_SHADER);
	m_unif_direction = m_shader.GetUniform("uDirection");
	m_unif_resolution = m_shader.GetUniform("uResolution");
	constexpr GLint kResolution[] = {kWidth, kHeight};
	m_shader.SetIVec2(m_unif_resolution, kResolution);
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
	m_target->Bind(kOutputRadianceSampler2D);
	m_shader.SetIVec2(m_unif_direction, dir0);
	quad.Render();
	mygl3::FrameBuffer::Unbind();

	m_blur_fbo[1].Bind();
	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	m_tmp_texture.Bind(kOutputRadianceSampler2D);
	m_shader.SetIVec2(m_unif_direction, dir1);
	quad.Render();
	mygl3::FrameBuffer::Unbind();
}

void GITemporalFilter::Initialize(const GIRenderer &renderer)
{
	m_reprojected_texture.Initialize();
	m_reprojected_texture.Storage(kWidth, kHeight, GL_R11F_G11F_B10F);
	m_reprojected_texture.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_reprojected_texture.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_result_texture.InitializeWithoutTarget();
	glTextureView(m_result_texture.Get(), GL_TEXTURE_2D, renderer.GetInputRadiance().Get(), GL_R11F_G11F_B10F, 0, 1, 1, 1);
	//use the second layer of m_input_radiance as tmp texture
	//m_result_texture.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	//m_result_texture.SetWrapFilter(GL_CLAMP_TO_EDGE);


	m_target = &renderer.GetOutputRadiance();

	m_reproject_shader.Initialize();
	m_reproject_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_reproject_shader.LoadFromFile("shaders/radiosity_temporal_reproject.frag", GL_FRAGMENT_SHADER);
	m_reproject_unif_last_view = m_reproject_shader.GetUniform("uLastView");

	m_reproject_fbo.Initialize();
	m_reproject_fbo.AttachTexture(m_reprojected_texture, GL_COLOR_ATTACHMENT0);

	m_blend_fbo.Initialize();
	m_blend_fbo.AttachTexture(m_result_texture, GL_COLOR_ATTACHMENT0);
}

void GITemporalFilter::LoadBlendShader(const ShaderSettings &settings)
{
	m_blend_shader.~Shader();
	m_blend_shader.Initialize();
	m_blend_shader.LoadFromFile("shaders/quad.vert", GL_VERTEX_SHADER);
	m_blend_shader.Load(settings.GetRadiosityTemporalBlendSrc().c_str(), GL_FRAGMENT_SHADER);
}

void GITemporalFilter::Reproject(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer)
{
	static glm::mat4 last_view_mat;

	m_reproject_shader.Use();
	m_reproject_shader.SetMat4(m_reproject_unif_last_view, glm::value_ptr(last_view_mat));

	camera.GetBuffer().BindBase(GL_UNIFORM_BUFFER, kCameraUBO);
	m_target->Bind(kOutputRadianceSampler2D);
	gbuffer.GetDepth().Bind(kDepthSampler2DArray);
	gbuffer.GetLastDepth().Bind(kLastDepthSampler2D);

	m_reproject_fbo.Bind();

	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	quad.Render();

	mygl3::FrameBuffer::Unbind();

	last_view_mat = camera.m_view;
}

void GITemporalFilter::Blend(const ScreenQuad &quad)
{
	m_blend_shader.Use();

	m_target->Bind(kOutputRadianceSampler2D);
	m_reprojected_texture.Bind(kReprojectedRadianceSampler2D);

	m_blend_fbo.Bind();

	glViewport(0, 0, kWidth, kHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	quad.Render();

	mygl3::FrameBuffer::Unbind();
	glCopyImageSubData(m_result_texture.Get(), GL_TEXTURE_2D, 0, 0, 0, 0,
					   m_target->Get(), GL_TEXTURE_2D, 0, 0, 0, 0,
					   kWidth, kHeight, 1);
}
