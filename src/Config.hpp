//
// Created by adamyuan on 19-5-10.
//

#ifndef SPARSEVOXELOCTREE_CONFIG_HPP
#define SPARSEVOXELOCTREE_CONFIG_HPP

constexpr int kWidth = 1280, kHeight = 720;
constexpr int kMaxMip = 6; //max mipmap level of screen buffers
constexpr float kCamNear = 1.0f / 512.0f, kCamFar = 4.0f;
constexpr float kCamAspectRatio = kWidth / (float)kHeight;
constexpr int kShadowMapSize = 2048;

#endif //SPARSEVOXELOCTREE_CONFIG_HPP
