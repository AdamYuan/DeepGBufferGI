//
// Created by adamyuan on 19-6-26.
//

#ifndef DEEPGBUFFERGI_DEEPGBUFFER_HPP
#define DEEPGBUFFERGI_DEEPGBUFFER_HPP

#include <mygl3/texture.hpp>
#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include "Camera.hpp"
#include "Scene.hpp"

class DeepGBuffer //2 layer deep g-buffer
{
private:
	mygl3::Texture2DArray
			m_normal, //16bit * 2, using RG8_SNORM, oct16 encoding
			m_albedo, //24bit * 2, using RGB8
			m_radiance, //32bit * 2, using R11G11B10
			m_depth; //24bit * 2, using DEPTH_COMPONENT24
	//totally (96 * 2 = 192) bit per pixel
	mygl3::Texture2D m_last_depth;
	mygl3::FrameBuffer m_fbo;
	mygl3::Shader m_shader;
	GLint m_unif_view, m_unif_last_view, m_unif_projection;

public:
	void Initialize();
	void Update(const Scene &scene, const Camera &camera);
	const mygl3::Texture2DArray &GetNormal() { return m_normal; }
	const mygl3::Texture2DArray &GetAlbedo() { return m_albedo; }
	const mygl3::Texture2DArray &GetDepth() { return m_depth; }
	const mygl3::Texture2D &GetLastDepth() { return m_last_depth; }
};


#endif //DEEPGBUFFERGI_DEEPGBUFFER_HPP
