//from https://casual-effects.com/research/Mara2016DeepGBuffer/
#version 450 core

#define R 6

#define SCALE 2
#define EDGE_SHARPNESS 1.0f

//predefine kernels
#       if R == 1 
float kKernel[R + 1] = {0.5,      0.25};
#       elif R == 2 
float kKernel[R + 1] = {0.153170, 0.144893, 0.122649};
#       elif R == 3 
float kKernel[R + 1] = {0.153170, 0.144893, 0.122649, 0.092902};
#       elif R == 4 
float kKernel[R + 1] = {0.153170, 0.144893, 0.122649, 0.092902, 0.062970};
#       elif R == 5 
float kKernel[R + 1] = {0.111220, 0.107798, 0.098151, 0.083953, 0.067458, 0.050920};
#       elif R == 6
float kKernel[R + 1] = {0.111220, 0.107798, 0.098151, 0.083953, 0.067458, 0.050920, 0.036108};
#       endif

in vec2 vTexcoords;
layout(std140, binding = 1) uniform uuCamera
{
	mat4 uProjection;
	mat4 uView;
	mat4 uInvPV;
};
layout (binding = 4) uniform sampler2DArray uNormal;
layout (binding = 5) uniform sampler2DArray uDepth;
layout (binding = 7) uniform sampler2D uOutputRadiance;
uniform ivec2 uDirection; //(1, 0) or (0, 1)
uniform ivec2 uResolution;
out vec3 oBlured;

vec3 ReconstructPosition(in const ivec2 coord, in float depth)
{
	vec2 texcoords = vec2(coord) / vec2(uResolution);
	vec4 clip = vec4(texcoords * 2.0f - 1.0f, depth * 2.0f - 1.0f, 1.0f);
	vec4 rec = uInvPV * clip;
	return rec.xyz / rec.w;
}

const float kNear = 1.0f / 512.0f, kFar = 4.0f;
float LinearDepth(in const float depth) { return (2.0 * kNear * kFar) / (kFar + kNear - depth * (kFar - kNear)); }
float GetDepth(in const ivec2 coord)  { return texelFetch(uDepth, ivec3(coord, 0), 0).r; }
//oct16 normal decoding :
// Returns ±1
vec2 SignNotZero(vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
vec3 oct_to_float32x3(vec2 e)
{
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * SignNotZero(v.xy);
	return normalize(v);
}
vec3 GetNormal(in const ivec2 coord) 
{
	return oct_to_float32x3( texelFetch(uNormal, ivec3(coord, 0), 0).rg );
}

float GetBilateralWeight(in const vec3 normal, in const vec3 samp_normal, in const vec3 position, in const vec3 samp_position)
{
	const float kKNormal = 40.0f, kKPlane = 5.0f;
	const float kLowDistanceThreshold2 = 0.01f;

	float normal_error = (1.0f - dot(normal, samp_normal)) * kKNormal;
	float normal_weight = max(1.0 - EDGE_SHARPNESS * normal_error, 0.0f);

	vec3 dq = position - samp_position;
	float dist2 = dot(dq, dq);

	float plane_error = max( abs(dot(dq, samp_normal)), abs(dot(dq, normal)) );

	float plane_weight = (dist2 < kLowDistanceThreshold2) ? 1.0f : 
		pow( max(0.0f, 1.0f - EDGE_SHARPNESS * 2.0f * kKPlane * plane_error / sqrt(dist2)), 2.0f );

	return normal_weight * plane_weight;
}

void main()
{
	ivec2 frag_coord = ivec2(gl_FragCoord.xy);

	vec3 samp = texelFetch(uOutputRadiance, frag_coord, 0).rgb;

	vec3 sum = samp * kKernel[0];
	float total_weight = kKernel[0];
	float depth = GetDepth(frag_coord);
	vec3 normal = GetNormal(frag_coord);
	vec3 position = ReconstructPosition(frag_coord, depth);

	for(int r = -R; r <= R; ++r)
	{
		if(r == 0) continue;

		ivec2 samp_coord = frag_coord + uDirection * (r * SCALE);
		samp = texelFetch(uOutputRadiance, samp_coord, 0).rgb;
		float samp_depth = GetDepth(samp_coord);
		vec3 samp_normal = GetNormal(samp_coord);
		vec3 samp_position = ReconstructPosition(samp_coord, samp_depth);

		// spatial domain: offset kKernel tap
		float weight = 0.3 + kKernel[abs(r)];

		// range domain (the "bilateral" weight). As depth difference increases, decrease weight.
		float bilateral_weight = GetBilateralWeight(normal, samp_normal, position, samp_position);

		weight *= bilateral_weight;
		sum += samp * weight;
		total_weight += weight;
	}

	oBlured = sum / (total_weight + 0.00001f);
}
