//
// Created by adamyuan on 19-7-9.
//

#include "ShaderSettings.hpp"
#include <fstream>

void ShaderSettings::Initialize()
{
	std::ifstream fin;

	fin.open("shaders/deepgbuffer.frag");
	m_deepgbuffer_frag_src = {std::istreambuf_iterator<char>{fin}, std::istreambuf_iterator<char>{}};
	fin.close();

	fin.open("shaders/radiosity.frag");
	m_radiosity_src = {std::istreambuf_iterator<char>{fin}, std::istreambuf_iterator<char>{}};
	fin.close();

	fin.open("shaders/radiosity_blur.frag");
	m_radiosity_blur_src = {std::istreambuf_iterator<char>{fin}, std::istreambuf_iterator<char>{}};
	fin.close();

	fin.open("shaders/radiosity_temporal_blend.frag");
	m_radiosity_temporal_blend_src = {std::istreambuf_iterator<char>{fin}, std::istreambuf_iterator<char>{}};
	fin.close();
}

std::string ShaderSettings::GetDeepGBufferFragSrc() const
{
	return std::string{"#version 450 core\n"}
		   + "#define MIN_SEPARATE " + std::to_string(m_deepgbuffer_min_separate) + "\n"
		   + m_deepgbuffer_frag_src;
}

std::string ShaderSettings::GetRadiositySrc() const
{
	constexpr int kTau[] = {1, 1, 2, 3, 2, 5, 2, 3, 2, 3, 3, 5, 5, 3, 4,
							7, 5, 5, 7, 9, 8, 5, 5, 7, 7, 7, 8, 5, 8, 11, 12, 7, 10, 13, 8,
							11, 8, 7, 14, 11, 11, 13, 12, 13, 19, 17, 13, 11, 18, 19, 11, 11,
							14, 17, 21, 15, 16, 17, 18, 13, 17, 11, 17, 19, 18, 25, 18, 19,
							19, 29, 21, 19, 27, 31, 29, 21, 18, 17, 29, 31, 31, 23, 18, 25,
							26, 25, 23, 19, 34, 19, 27, 21, 25, 39, 29, 17, 21, 27};
	return std::string{"#version 450 core\n"}
		   + "#define SAMPLE_CNT " + std::to_string(m_radiosity_sample_cnt) + "\n"
		   + "#define MIN_MIP " + std::to_string(m_radiosity_min_mip) + "\n"
		   + "#define USE_Y_NORMAL_TEST " + std::to_string((int)m_radiosity_use_y_normal_test) + "\n"
		   + "#define TAU " + std::to_string(kTau[ m_radiosity_sample_cnt - 1 ]) + "\n"
		   + "#define R " + std::to_string(m_radiosity_radius) + "\n"
		   + m_radiosity_src;
}

std::string ShaderSettings::GetRadiosityBlurSrc() const
{
	return std::string{"#version 450 core\n"}
		   + "#define R " + std::to_string(m_radiosity_blur_radius) + "\n"
		   + "#define SCALE " + std::to_string(m_radiosity_blur_scale) + "\n"
		   + "#define EDGE_SHARPNESS " + std::to_string(m_radiosity_blur_edge_sharpness) + "\n"
		   + m_radiosity_blur_src;
}

std::string ShaderSettings::GetRadiosityTemporalBlendSrc() const
{
	return std::string{"#version 450 core\n"}
		   + "#define ALPHA " + std::to_string(m_radiosity_temporal_blend_alpha) + "\n"
		   + m_radiosity_temporal_blend_src;
}

void ShaderSettings::Reset()
{
	m_deepgbuffer_min_separate = 0.05f;
	m_radiosity_use_y_normal_test = false;
	m_radiosity_min_mip = 3;
	m_radiosity_sample_cnt = 13;
	m_radiosity_radius = 0.4f;
	m_radiosity_blur_radius = 4;
	m_radiosity_blur_scale = 3;
	m_radiosity_blur_edge_sharpness = 1.0f;
	m_radiosity_temporal_blend_alpha = 0.85f;
}

void ShaderSettings::SetRadiosityQuality(int quality)
{
	if(quality == 0)
	{
		m_radiosity_min_mip = 3;
		m_radiosity_sample_cnt = 13;
		m_radiosity_use_y_normal_test = false;
		m_radiosity_blur_radius = 4;
	}
	else if(quality == 1)
	{
		m_radiosity_min_mip = 2;
		m_radiosity_sample_cnt = 14;
		m_radiosity_use_y_normal_test = true;
		m_radiosity_blur_radius = 5;
	}
	else
	{
		m_radiosity_min_mip = 0;
		m_radiosity_sample_cnt = 30;
		m_radiosity_use_y_normal_test = true;
		m_radiosity_blur_radius = 6;
	}
}
