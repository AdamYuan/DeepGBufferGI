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
	mygl3::FrameBuffer m_fbo;
	mygl3::Shader m_direct_light_shader, m_radiosity_shader;
	GLint m_diret_light_unif_shadow_transform;
public:
	void Initialize();
	void DirectLight(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer, const ShadowMap &shadowmap);
	void Radiosity(const ScreenQuad &quad, const Camera &camera, const DeepGBuffer &gbuffer);
	const mygl3::Texture2DArray &GetRadiance() const { return m_radiance; }
};


#endif //DEEPGBUFFERGI_GIRENDERER_HPP
