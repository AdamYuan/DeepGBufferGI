#version 450 core

#define PERFORMANCE_MODE 2

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

out vec3 oRadiance;

#if PERFORMANCE_MODE == 0
const int kN = 26;
#elif PERFORMANCE_MODE == 1
const int kN = 28;
#else
const int kN = 60;
#endif

const float kR = 0.25f; //world space sample radius
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

in vec2 vTexcoords;

vec3 ReconstructPosition(in const vec2 texcoords, in float depth)
{
	vec4 clip = vec4(texcoords * 2.0f - 1.0f, depth * 2.0f - 1.0f, 1.0f);
	vec4 rec = inverse(uProjection * uView) * clip;
	return rec.xyz / rec.w;
}
float Hash(in const vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453); }

//oct16 normal decoding :
// Returns ±1
vec2 SignNotZero(vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
vec3 oct_to_float32x3(vec2 e)
{
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * SignNotZero(v.xy);
	return normalize(v);
}

const float kNear = 1.0f / 512.0f, kFar = 4.0f;
float LinearDepth(in const float depth) { return (2.0 * kNear * kFar) / (kFar + kNear - depth * (kFar - kNear)); }

void main()
{
	ivec2 resolution = textureSize(uDepth, 0).xy;
	ivec2 icoord = ivec2(gl_FragCoord.xy);
	ivec3 x_coord = ivec3(icoord, 0);
	float x_depth = texelFetch(uDepth, x_coord, 0).r;
	vec3 x_position = ReconstructPosition(vec2(x_coord.xy) / vec2(resolution), x_depth);
	vec3 x_albedo = texelFetch(uAlbedo, x_coord, 0).rgb;
	vec3 x_normal = oct_to_float32x3( texelFetch(uNormal, x_coord, 0).rg );
	vec3 x_radiance = texelFetch(uRadiance, x_coord, 0).xyz;

	float rp = kR * 500 / LinearDepth(x_depth * 2.0f - 1.0f);

	vec3 radiance = vec3(0);
	int sample_used = 0;

	float hash = (3 * icoord.x ^ icoord.y + icoord.x * icoord.y) * 10.0f + uTime;
    float radial_jitter = fract(sin(gl_FragCoord.x * 1e2 + 
            uTime +
        gl_FragCoord.y) * 1e5 + sin(gl_FragCoord.y * 1e3) * 1e3) * 0.8 + 0.1;

	for(int i = 0; i < kN; ++i)
	{
		float k = (float(i) + radial_jitter) * kInvN;
		float theta = kTwoPi*tau[kN - 1]*k + hash;
		float h = k * rp;
		vec2 u = vec2(cos(theta), sin(theta));

		int mip = min(int(floor(log2(h / kQ))), 5);
#if PERFORMANCE_MODE == 0
		mip = max(mip, 3);
#elif PERFORMANCE_MODE == 1
		mip = max(mip, 2);
#endif
		ivec3 y_coord = ivec3(icoord + ivec2(u * h), (i & 1));
		if(y_coord.xy != clamp(y_coord.xy, ivec2(0), resolution)) continue; //skip out of range part

		float y_depth = texelFetch(uDepth, y_coord, 0).r;
		vec3 y_position = ReconstructPosition(vec2(y_coord.xy) / vec2(resolution), y_depth);
		vec3 y_normal = oct_to_float32x3( texelFetch(uNormal, y_coord, 0).rg );

		vec3 omega = y_position - x_position;
#if PERFORMANCE_MODE != 0
		int weight = (dot(omega, x_normal) > 0 && dot(-omega, y_normal) > 0) ? 1 : 0;
#else
		int weight = 1;
#endif

		if(dot(omega, omega) > kR*kR)
			weight = 0;

		sample_used += weight;
		if(weight == 1)
		{
			vec3 y_radiance = texelFetch(uRadiance, ivec3(y_coord.xy >> mip, y_coord.z), mip).xyz;
			float vmax = max(y_radiance.x, max(y_radiance.y, y_radiance.z));
			float vmin = min(y_radiance.x, min(y_radiance.y, y_radiance.z));
			float boost = (vmax - vmin) / max(vmax, 1e-9);
			radiance += weight * y_radiance * boost * max(0.0f, dot( normalize(omega), x_normal));
		}
	}
	radiance *= kTwoPi / (float(sample_used) + 1e-4f);
	oRadiance = radiance * x_albedo + x_albedo * 0.01f;
}
