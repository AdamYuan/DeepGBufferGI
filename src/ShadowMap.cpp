//
// Created by adamyuan on 19-6-28.
//

#include "ShadowMap.hpp"
#include "Config.hpp"

void ShadowMap::Initialize()
{
	m_texture.Initialize();
	m_texture.Storage(kShadowMapSize, kShadowMapSize, GL_DEPTH_COMPONENT24);

	/*m_shader.Initialize();
	m_shader.LoadFromFile("shaders/shadowmap.vert", GL_VERTEX_SHADER);
	m_shader.LoadFromFile("shaders/shadowmap.frag", GL_FRAGMENT_SHADER);*/

	m_fbo.Initialize();
	m_fbo.AttachTexture(m_texture, GL_DEPTH_ATTACHMENT);
}
