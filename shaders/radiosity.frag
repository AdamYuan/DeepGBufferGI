//#version 450 core

//#define R (0.4f)
//#define SAMPLE_CNT (13)
//#define MIN_MIP (3)
//#define TAU (5)
//#define USE_Y_NORMAL_TEST (0)
//world space radius of samples

layout(std140, binding = 1) uniform uuCamera
{
	mat4 uProjection;
	mat4 uView;
	mat4 uInvPV;
};

layout (binding = 3) uniform sampler2DArray uAlbedo;
layout (binding = 4) uniform sampler2DArray uNormal;
layout (binding = 5) uniform sampler2DArray uDepth;
layout (binding = 6) uniform sampler2DArray uInputRadiance; //input radiance

uniform float uTime;
uniform ivec2 uResolution;

in vec2 vTexcoords;
out vec3 oRadiance;

#define MAX_MIP 5

const float R2 = R * R;
const float kQ = 32.0; //screen space radius which we first increase mip-level
const float kInvSampleCnt = 1.0f / float(SAMPLE_CNT);
const float kTwoPi = 6.283185307179586f;

const float kNear = 1.0f / 512.0f, kFar = 4.0f;
float LinearDepth(in const float depth) { return (2.0 * kNear * kFar) / (kFar + kNear - depth * (kFar - kNear)); }
vec3 ReconstructPosition(in const vec2 texcoords, in float depth)
{
	vec4 clip = vec4(texcoords * 2.0f - 1.0f, depth * 2.0f - 1.0f, 1.0f);
	vec4 rec = uInvPV * clip;
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
void GetPositionNormalDepthRadiance(in const ivec3 coord, out vec3 position, out vec3 normal, inout float depth, out vec3 radiance)
{
	depth = texelFetch(uDepth, coord, 0).r;
	position = ReconstructPosition(vec2(coord.xy) / vec2(uResolution), depth);
	normal = oct_to_float32x3( texelFetch(uNormal, coord, 0).rg );
	radiance = texelFetch(uInputRadiance, coord, 0).rgb;
}

vec3 GetRadiance(in const ivec3 coord, in const int mip)
{
	ivec3 coord_mip = ivec3(coord.xy >> mip, coord.z);
	return texelFetch(uInputRadiance, coord_mip, mip).rgb;
}

void GetSampleLocationAndMip(in const int sample_index, in const float radius, 
							 in const float random_rotation, in const float radial_jitter, out ivec2 loc, out int mip)
{
	float k = (float(sample_index) + radial_jitter) * kInvSampleCnt;
	float theta = kTwoPi * TAU * k + random_rotation;
	float h = k * radius;
	vec2 u = vec2(cos(theta), sin(theta));
	loc = ivec2(u * h);

	mip = clamp(findMSB(int(h / kQ)), MIN_MIP, MAX_MIP);
}

void GetSampleWeight(in const vec3 x_position, in const vec3 x_normal, in const vec3 y_position, in const vec3 y_normal, inout int weight, inout vec3 omega)
{
	omega = y_position - x_position;
	weight = (dot(omega, x_normal) > 0
#if USE_Y_NORMAL_TEST == 1
			  && dot(-omega, y_normal) > 0.01f
#endif
			 ) ? 1 : 0;
	if(dot(omega, omega) > R2) weight = 0;

	omega = normalize(omega);
}

void CalculateRadiance(in const float normal_dot_omega, in const ivec3 coord, in const int mip, inout vec3 radiance)
{
	radiance = GetRadiance(coord, mip) * max(0.0f, normal_dot_omega);
}

void main()
{
	ivec2 frag_coord = ivec2(gl_FragCoord.xy);
	ivec3 x_coord = ivec3(frag_coord, 0);

	vec3 x_position, x_normal, x_radiance;
	float x_depth;
	GetPositionNormalDepthRadiance(x_coord, x_position, x_normal, x_depth, x_radiance);

	int sample_used = 0;
	vec3 radiance_sum = vec3(0);

	float radius = R * 500 / LinearDepth(x_depth * 2.0f - 1.0f); //screen space sample radius
	float random_rotation = (3 * frag_coord.x ^ frag_coord.y + frag_coord.x * frag_coord.y) * 10.0f + uTime;
	float radial_jitter = fract(sin(gl_FragCoord.x * 1e2 + uTime + gl_FragCoord.y) * 1e5 + sin(gl_FragCoord.y * 1e3) * 1e3) * 0.8 + 0.1;

	for(int i = 0; i < SAMPLE_CNT; ++i)
	{
		ivec2 relative_loc; int mip;
		GetSampleLocationAndMip(i, radius, random_rotation, radial_jitter, relative_loc, mip);
		ivec2 samp_coord = frag_coord + relative_loc;
		if(samp_coord.xy != clamp(samp_coord.xy, ivec2(0), uResolution - 1)) continue; //skip out of range part

		ivec3 y_coord        = ivec3(samp_coord, 0);
		ivec3 y_coord_peeled = ivec3(samp_coord, 1);

		vec3 y_position,        y_normal;
		vec3 y_position_peeled, y_normal_peeled;

		GetPositionNormal(y_coord,        mip, y_position,        y_normal       );
		GetPositionNormal(y_coord_peeled, mip, y_position_peeled, y_normal_peeled);

		int  y_weight,  y_weight_peeled;
		vec3 y_omega,   y_omega_peeled;

		GetSampleWeight(x_position, x_normal, y_position,        y_normal,        y_weight,        y_omega);
		GetSampleWeight(x_position, x_normal, y_position_peeled, y_normal_peeled, y_weight_peeled, y_omega_peeled);

		vec3 y_radiance = vec3(0), y_radiance_peeled = vec3(0);

		if(y_weight        == 1) CalculateRadiance(dot(x_normal, y_omega)       , y_coord,        mip, y_radiance);
		if(y_weight_peeled == 1) CalculateRadiance(dot(x_normal, y_omega_peeled), y_coord_peeled, mip, y_radiance_peeled);

		float y_adj_weight = dot(y_radiance, y_radiance) + float(y_weight);
		float y_adj_weight_peeled = dot(y_radiance_peeled, y_radiance_peeled) + float(y_weight_peeled);

		sample_used += y_adj_weight > y_adj_weight_peeled ? y_weight : y_weight_peeled;
		radiance_sum += y_adj_weight > y_adj_weight_peeled ? y_radiance : y_radiance_peeled;
	}
	radiance_sum *= kTwoPi / (float(sample_used) + 0.01f);

	float vmax = max(radiance_sum.x, max(radiance_sum.y, radiance_sum.z));
	float vmin = min(radiance_sum.x, min(radiance_sum.y, radiance_sum.z));
	float boost = (vmax - vmin) / max(vmax, 1e-9);
	oRadiance = radiance_sum * boost;
}
