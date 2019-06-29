//
// Created by adamyuan on 19-6-28.
//

#ifndef DEEPGBUFFERGI_SHADOWMAP_HPP
#define DEEPGBUFFERGI_SHADOWMAP_HPP

#include <mygl3/texture.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/framebuffer.hpp>

class ShadowMap
{
private:
	mygl3::Texture2D m_texture;
	//mygl3::Shader m_shader;
	mygl3::FrameBuffer m_fbo;
public:
	void Initialize();
};


#endif //DEEPGBUFFERGI_SHADOWMAP_HPP
