#version 450 core

#define PERFORMANCE_MODE 0

layout(std140, binding = 1) uniform uuCamera
{
	mat4 uProjection;
	mat4 uView;
	vec4 uPosition;
};

layout (binding = 3) uniform sampler2DArray uAlbedo;
layout (binding = 4) uniform sampler2DArray uNormal;
layout (binding = 5) uniform sampler2DArray uDepth;
layout (binding = 6) uniform sampler2DArray uRadiance; //input radiance

uniform float uTime;
uniform ivec2 uResolution;

in vec2 vTexcoords;
out vec3 oRadiance;

#if PERFORMANCE_MODE == 0
const int kN = 13;
const int kMinMip = 3;
#elif PERFORMANCE_MODE == 1
const int kN = 14;
const int kMinMip = 2;
#else
const int kN = 30;
const int kMinMip = 0;
#endif
const int kMaxMip = 5;

const float kR = 0.25f; //world space sample radius
const float kR2 = kR * kR;
const float kQ = 64; //screen space radius which we first increase mip-level
const float kInvN = 1.0f / float(kN);
const float kTwoPi = 6.283185307179586f;
// tau[N-1] = optimal number of spiral turns for N samples
const int tau[] = {1, 1, 2, 3, 2, 5, 2, 3, 2, 3, 3, 5, 5, 3, 4,
	7, 5, 5, 7, 9, 8, 5, 5, 7, 7, 7, 8, 5, 8, 11, 12, 7, 10, 13, 8,
	11, 8, 7, 14, 11, 11, 13, 12, 13, 19, 17, 13, 11, 18, 19, 11, 11,
	14, 17, 21, 15, 16, 17, 18, 13, 17, 11, 17, 19, 18, 25, 18, 19,
	19, 29, 21, 19, 27, 31, 29, 21, 18, 17, 29, 31, 31, 23, 18, 25,
	26, 25, 23, 19, 34, 19, 27, 21, 25, 39, 29, 17, 21, 27};

float Hash(in const vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453); }

const float kNear = 1.0f / 512.0f, kFar = 4.0f;
float LinearDepth(in const float depth) { return (2.0 * kNear * kFar) / (kFar + kNear - depth * (kFar - kNear)); }
vec3 ReconstructPosition(in const vec2 texcoords, in float depth)
{
	vec4 clip = vec4(texcoords * 2.0f - 1.0f, depth * 2.0f - 1.0f, 1.0f);
	vec4 rec = inverse(uProjection * uView) * clip;
	return rec.xyz / rec.w;
}

//oct16 normal decoding :
// Returns ±1
vec2 SignNotZero(vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
vec3 oct_to_float32x3(vec2 e)
{
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * SignNotZero(v.xy);
	return normalize(v);
}

void GetPositionNormal(in const ivec3 coord, in const int mip, out vec3 position, out vec3 normal)
{
	ivec3 coord_mip = ivec3(coord.xy >> mip, coord.z);
	float depth = texelFetch(uDepth, coord_mip, mip).r;
	position = ReconstructPosition(vec2(coord.xy) / vec2(uResolution), depth);
	normal = oct_to_float32x3( texelFetch(uNormal, coord_mip, mip).rg );
}
void GetPositionNormalDepthAlbedoRadiance(in const ivec3 coord, out vec3 position, out vec3 normal, inout float depth, out vec3 albedo, out vec3 radiance)
{
	depth = texelFetch(uDepth, coord, 0).r;
	position = ReconstructPosition(vec2(coord.xy) / vec2(uResolution), depth);
	normal = oct_to_float32x3( texelFetch(uNormal, coord, 0).rg );
	albedo = texelFetch(uAlbedo, coord, 0).rgb;
	radiance = texelFetch(uRadiance, coord, 0).rgb;
}

vec3 GetRadiance(in const ivec3 coord, in const int mip)
{
	ivec3 coord_mip = ivec3(coord.xy >> mip, coord.z);
	return texelFetch(uRadiance, coord_mip, mip).rgb;
}

void GetSampleLocationAndMip(in const int sample_index, in const float radius, 
							 in const float random_rotation, in const float radial_jitter, out ivec2 loc, inout int mip)
{
	float k = (float(sample_index) + radial_jitter) * kInvN;
	float theta = kTwoPi * tau[kN - 1] * k + random_rotation;
	float h = k * radius;
	vec2 u = vec2(cos(theta), sin(theta));
	loc = ivec2(u * h);

	mip = int(floor(log2(h / kQ)));
	mip = clamp(mip, kMinMip, kMaxMip);
}

void GetSampleWeight(in const vec3 x_position, in const vec3 x_normal, in const vec3 y_position, in const vec3 y_normal, inout int weight, inout vec3 omega)
{
	omega = y_position - x_position;
#if PERFORMANCE_MODE != 0
	weight = (dot(omega, x_normal) > 0 && dot(-omega, y_normal) > 0) ? 1 : 0;
#else
	weight = 1;
#endif
	if(dot(omega, omega) > kR2)
		weight = 0;

	omega = normalize(omega);
}

void AcculumateRadiance(in const float normal_dot_omega, in const ivec3 coord, in const int mip, inout int sample_used, inout vec3 radiance_sum)
{
	++sample_used;
	vec3 radiance = GetRadiance(coord, mip);
	float vmax = max(radiance.x, max(radiance.y, radiance.z));
	float vmin = min(radiance.x, min(radiance.y, radiance.z));
	float boost = (vmax - vmin) / max(vmax, 1e-9);
	radiance_sum += radiance * boost * max(0.0f, normal_dot_omega);
}

void main()
{
	ivec2 frag_coord = ivec2(gl_FragCoord.xy);
	ivec3 x_coord = ivec3(frag_coord, 0);

	vec3 x_position, x_normal, x_radiance, x_albedo;
	float x_depth;
	GetPositionNormalDepthAlbedoRadiance(x_coord, x_position, x_normal, x_depth, x_albedo, x_radiance);

	int sample_used = 0;
	vec3 radiance_sum = vec3(0);

	float radius = kR * 500 / LinearDepth(x_depth * 2.0f - 1.0f); //screen space sample radius
	float random_rotation = Hash(vTexcoords + uTime) * kTwoPi;
	float radial_jitter = fract(sin(gl_FragCoord.x * 1e2 + uTime + gl_FragCoord.y) * 1e5 + sin(gl_FragCoord.y * 1e3) * 1e3) * 0.8 + 0.1;

	for(int i = 0; i < kN; ++i)
	{
		ivec2 relative_loc; int mip;
		GetSampleLocationAndMip(i, radius, random_rotation, radial_jitter, relative_loc, mip);
		ivec2 samp_coord = frag_coord + relative_loc;
		if(samp_coord.xy != clamp(samp_coord.xy, ivec2(0), uResolution - 1)) continue; //skip out of range part

		ivec3 y_coord			= ivec3(samp_coord, 0);
		ivec3 y_coord_peeled	= ivec3(samp_coord, 1);

		vec3 y_position,		y_normal;
		vec3 y_position_peeled,	y_normal_peeled;

		GetPositionNormal(y_coord,			mip, y_position,		y_normal		);
		GetPositionNormal(y_coord_peeled,	mip, y_position_peeled,	y_normal_peeled	);

		int  y_weight,  y_weight_peeled;
		vec3 y_omega,   y_omega_peeled;

		GetSampleWeight(x_position, x_normal, y_position,			y_normal,			y_weight,			y_omega);
		GetSampleWeight(x_position, x_normal, y_position_peeled,	y_normal_peeled,	y_weight_peeled,	y_omega_peeled);

		if(y_weight        == 1) AcculumateRadiance(dot(x_normal, y_omega)       , y_coord,        mip, sample_used, radiance_sum);
		if(y_weight_peeled == 1) AcculumateRadiance(dot(x_normal, y_omega_peeled), y_coord_peeled, mip, sample_used, radiance_sum);
	}
	radiance_sum *= kTwoPi / (float(sample_used) + 1e-4f);
	oRadiance = radiance_sum * x_albedo;
}
