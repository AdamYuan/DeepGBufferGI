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

out vec3 oRadiance;

#if PERFORMANCE_MODE == 0
const int kN = 13;
#elif PERFORMANCE_MODE == 1
const int kN = 14;
#else
const int kN = 30;
#endif

const float kR = 0.4; //world space sample radius
const float kQ = 0.05; //screen space radius which we first increase mip-level
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
	vec3 x_coord = vec3(vTexcoords, 0.0f);
	float x_depth = texture(uDepth, x_coord).r;
	vec3 x_position = ReconstructPosition(vTexcoords, x_depth);
	vec3 x_albedo = texture(uAlbedo, x_coord).rgb;
	vec3 x_normal = oct_to_float32x3( texture(uNormal, x_coord).rg );
	vec3 x_radiance = texture(uRadiance, x_coord).xyz;

	float rp = kR * 0.25f / LinearDepth(x_depth * 2.0f - 1.0f);

	float hash = Hash(vTexcoords) * kTwoPi;

	vec3 radiance = vec3(0);
	int m = 0;
	for(int i = 0; i < kN; ++i)
	{
		float k = (float(i) + 0.5f) / float(kN);
		float theta = kTwoPi*tau[kN - 1]*k + hash;
		float h = k * rp;
		vec2 u = vec2(cos(theta), sin(theta));
		float mip = floor(log2(h / kQ));

#if PERFORMANCE_MODE == 0
		mip = max(mip, 3.0f);
#elif PERFORMANCE_MODE == 1
		mip = max(mip, 2.0f);
#endif

		vec3 y_coord = vec3(vTexcoords + u*h, float(i & 1));
		float y_depth = texture(uDepth, y_coord).r;
		vec3 y_position = ReconstructPosition(y_coord.xy, y_depth);
		vec3 y_normal = oct_to_float32x3( texture(uNormal, y_coord).rg );

		vec3 omega = normalize(y_position - x_position);
#if PERFORMANCE_MODE != 0
		if(dot(omega, x_normal) > 0 && dot(omega, y_normal) < 0)
#endif
		{
			++m;
			vec3 samp = textureLod(uRadiance, y_coord, mip).xyz;
			radiance += samp * max(0.0f, dot(omega, x_normal));
		}
	}
	if(m > 0) radiance *= kTwoPi / float(m);
	oRadiance = radiance * x_albedo;
}
