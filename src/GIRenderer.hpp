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

class GIRenderer
{
private:
	mygl3::Texture2DArray m_radiance; //GL_R11F_G11F_B10F
	mygl3::Texture2D m_gi_radiance;
	//0. copy m_gi_radiance to the first layer of m_radiance and apply temporal filter (with something like a compute shader)
	//1. use direct light shader to emmit m_radiance (2 layers) (maybe temporal filter later)
	//2. use m_radiance to calculate radiosity and store it in m_gi_radiance (the first layer)
	//3. blur m_gi_radiance (use the second layer of m_radiance as tmp buffer) IMPLEMENTED IN GIBlurer class

	mygl3::FrameBuffer m_direct_light_fbo, m_radiosity_fbo;
	mygl3::Shader m_direct_light_shader, m_radiosity_shader;
	GLint m_direct_light_unif_shadow_transform, m_direct_light_unif_light_dir;
	GLint m_radiosity_unif_time;
public:
	void Initialize();
	void DirectLight(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer, const ShadowMap &shadowmap);
	void Radiosity(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer);
	const mygl3::Texture2DArray &GetRadiance() const { return m_radiance; }
	const mygl3::Texture2D &GetGIRadiance() const { return m_gi_radiance; }
};

class GIBlurer
{
private:
	mygl3::Texture2D m_tmp_texture;
	const mygl3::Texture2D *m_target;
	mygl3::Shader m_shader;
	GLint m_unif_direction;
	mygl3::FrameBuffer m_blur_fbo[2];
public:
	void Initialize(const GIRenderer &renderer);
	void Blur(const ScreenQuad &quad);
};


#endif //DEEPGBUFFERGI_GIRENDERER_HPP
