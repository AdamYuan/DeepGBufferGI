//
// Created by adamyuan on 19-6-28.
//

#ifndef DEEPGBUFFERGI_SHADOWMAP_HPP
#define DEEPGBUFFERGI_SHADOWMAP_HPP

#include <mygl3/texture.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/framebuffer.hpp>
#include <glm/glm.hpp>
#include "Scene.hpp"

class ShadowMap
{
private:
	mygl3::Texture2D m_texture;
	mygl3::Shader m_shader;
	mygl3::FrameBuffer m_fbo;
	GLint m_unif_transform;
	glm::mat4 m_transform;
public:
	void Initialize();
	void Update(const Scene &scene, const glm::mat4 &transform);
	void Update(const Scene &scene, const glm::vec3 &sun_pos);
	const mygl3::Texture2D &GetTexture() const { return m_texture; }
	const glm::mat4 &GetShadowTransform() const { return m_transform; }
};


#endif //DEEPGBUFFERGI_SHADOWMAP_HPP
