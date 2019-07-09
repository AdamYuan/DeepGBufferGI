//
// Created by adamyuan on 19-7-9.
//

#ifndef DEEPGBUFFERGI_SHADERSETTINGS_HPP
#define DEEPGBUFFERGI_SHADERSETTINGS_HPP

#include <string>

class ShaderSettings
{
private:
	std::string m_deepgbuffer_frag_src, m_radiosity_src, m_radiosity_blur_src, m_radiosity_temporal_blend_src;
public:
	float m_deepgbuffer_min_separate = 0.05f;
	bool m_radiosity_use_y_normal_test = false;
	int m_radiosity_min_mip = 3;
	int m_radiosity_sample_cnt = 13;
	float m_radiosity_radius = 0.4f;
	int m_radiosity_blur_radius = 6;
	int m_radiosity_blur_scale = 3;
	float m_radiosity_blur_edge_sharpness = 1.0f;
	float m_radiosity_temporal_blend_alpha = 0.85f;
	void Initialize();
	void Reset();
	void SetRadiosityQuality(int quality); //0 - High Performance, 1 - Balanced, 2 - High Quality
	std::string GetDeepGBufferFragSrc() const;
	std::string GetRadiositySrc() const;
	std::string GetRadiosityBlurSrc() const;
	std::string GetRadiosityTemporalBlendSrc() const;
};

#endif //DEEPGBUFFERGI_SHADERSETTINGS_HPP
