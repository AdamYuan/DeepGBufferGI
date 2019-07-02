//
// Created by adamyuan on 19-5-4.
//

#ifndef SPARSEVOXELOCTREE_OGLBINDINGS_HPP
#define SPARSEVOXELOCTREE_OGLBINDINGS_HPP

#include <GL/gl3w.h>

constexpr GLuint kTextureUBO = 0;
constexpr GLuint kCameraUBO = 1;
constexpr GLuint kLastDepthSampler2D = 1;
constexpr GLuint kShadowMapSampler2D = 2;
constexpr GLuint kGBufferAlbedoSampler2D = 3;
constexpr GLuint kGBufferNormalSampler2D = 4;
constexpr GLuint kGBufferDepthSampler2D = 5;

#endif //SPARSEVOXELOCTREE_OGLBINDINGS_HPP
