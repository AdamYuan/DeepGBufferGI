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
#include "ScreenQuad.hpp"
#include "ShaderSettings.hpp"

class DeepGBuffer //2 layer deep g-buffer
{
private:
	mygl3::Texture2DArray
			m_normal, //16bit * 2, using RG8_SNORM, oct16 encoding
			m_albedo, //24bit * 2, using RGB8
			m_depth; //32bit * 2, using DEPTH_COMPONENT32
	mygl3::Texture2D m_last_depth;
	mygl3::FrameBuffer m_fbo;
	mygl3::Shader m_shader;
	GLint m_unif_last_view;

public:
	void Initialize();
	void LoadShader(const ShaderSettings &settings);
	void Update(const Scene &scene, const Camera &camera);
	const mygl3::Texture2DArray &GetNormal() const { return m_normal; }
	const mygl3::Texture2DArray &GetAlbedo() const { return m_albedo; }
	const mygl3::Texture2DArray &GetDepth() const { return m_depth; }
	const mygl3::Texture2D &GetLastDepth() const { return m_last_depth; }
};


#endif //DEEPGBUFFERGI_DEEPGBUFFER_HPP
