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
constexpr GLuint kAlbedoSampler2DArray = 3;
constexpr GLuint kNormalSampler2DArray = 4;
constexpr GLuint kDepthSampler2DArray = 5;
constexpr GLuint kRadianceSampler2DArray = 6;
constexpr GLuint kGIRadianceSampler2D = 7;
constexpr GLuint kLastGIRadianceSampler2D = 8;

#endif //SPARSEVOXELOCTREE_OGLBINDINGS_HPP
