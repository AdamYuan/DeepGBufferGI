//
// Created by adamyuan on 19-7-1.
//

#ifndef DEEPGBUFFERGI_GIRENDERER_HPP
#define DEEPGBUFFERGI_GIRENDERER_HPP

#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>
#include "ScreenQuad.hpp"
#include "ShadowMap.hpp"
#include "DeepGBuffer.hpp"

class GITemporalFilter;
class GIRenderer
{
private:
	mygl3::Texture2DArray m_input_radiance; //GL_R11F_G11F_B10F
	mygl3::Texture2D m_output_radiance;
	//1. use direct light shader to emmit m_input_radiance (2 layers),
	//   also combine the first layer with last radiosity result to do multi-bounce radiosity
	//2. use m_input_radiance to calculate radiosity and store it in m_output_radiance (the first layer)
	//3. apply bilateral filter to m_output_radiance (use the second layer of m_input_radiance as tmp texture)
	//4. apply temporal filter (store last radiosity result in another texture)

	mygl3::FrameBuffer m_radiosity_input_fbo, m_radiosity_fbo;
	mygl3::Shader m_radiosity_input_shader, m_radiosity_shader;
	GLint m_radiosity_input_unif_shadow_transform, m_radiosity_input_unif_light_dir;
	GLint m_radiosity_unif_time, m_radiosity_unif_resolution;
public:
	void Initialize();
	void LoadRadiosityShader(const ShaderSettings &settings);
	void PrepareInputRadiance(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer,
							  const ShadowMap &shadowmap, const GITemporalFilter &temporal);
	void SampleRadiosity(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer);
	const mygl3::Texture2DArray &GetInputRadiance() const { return m_input_radiance; }
	const mygl3::Texture2D &GetOutputRadiance() const { return m_output_radiance; }
};

class GIBlurer
{
private:
	mygl3::Texture2D m_tmp_texture;
	const mygl3::Texture2D *m_target;
	mygl3::Shader m_shader;
	GLint m_unif_direction, m_unif_resolution;
	mygl3::FrameBuffer m_blur_fbo[2];
public:
	void Initialize(const GIRenderer &renderer);
	void LoadShader(const ShaderSettings &settings);
	void Blur(const ScreenQuad &quad, const DeepGBuffer &gbuffer);
};

class GITemporalFilter
{
private:
	mygl3::Texture2D m_reprojected_texture; //store reprojected last radiosity result
	mygl3::Texture2D m_result_texture; //do filter on this tmp texture
	const mygl3::Texture2D *m_target;
	mygl3::Shader m_reproject_shader, m_blend_shader;
	GLint m_reproject_unif_last_view;
	mygl3::FrameBuffer m_reproject_fbo, m_blend_fbo;
public:
	void Initialize(const GIRenderer &renderer);
	void LoadBlendShader(const ShaderSettings &settings);
	void Reproject(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer);
	//reproject last frame's radiosity result to m_reprojected_texture
	void Blend(const ScreenQuad &quad);
	//blend this frame's result with m_reprojected_texture
	const mygl3::Texture2D &GetReprojectedRadiance() const { return m_reprojected_texture; }
};


#endif //DEEPGBUFFERGI_GIRENDERER_HPP
