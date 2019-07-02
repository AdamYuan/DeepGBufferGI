#version 450 core

layout(std140, binding = 1) uniform uuCamera
{
	mat4 uProjection;
	mat4 uView;
	float uX, uY, uZ, uInvCosHalfFov;
};

layout (binding = 2) uniform sampler2DShadow uShadowMap;
layout (binding = 3) uniform sampler2DArray uAlbedo;
layout (binding = 4) uniform sampler2DArray uNormal;
layout (binding = 5) uniform sampler2DArray uDepth;

layout (location = 0) out vec3 oRadiance;

uniform mat4 uShadowTransform;

in vec2 gTexcoords;

const float kNear = 1.0f / 512.0f, kFar = 4.0f;
float LinearDepth(in const float depth) { return (2.0 * kNear * kFar) / (kFar + kNear - depth * (kFar - kNear)); }

//oct16 normal encoding :
// Returns ±1
vec2 SignNotZero(vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
// Assume normalized input. Output is on [-1, 1] for each component.
vec2 float32x3_to_oct(in vec3 v) 
{
	// Project the sphere onto the octahedron, and then onto the xy plane
	vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	// Reflect the folds of the lower hemisphere over the diagonals
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * SignNotZero(p)) : p;
}

vec3 ReconstructPosition(in const vec2 texcoords)
{
	vec4 clip = vec4(texcoords * 2.0f - 1.0f, texture(uDepth, vec3(texcoords, gl_Layer)).r * 2.0f - 1.0f, 1.0f);
	vec4 rec = inverse(uProjection * uView) * clip;
	return rec.xyz / rec.w;
}
float SampleShadow(in const vec3 position)
{
	vec4 transformed = uShadowTransform * vec4(position, 1.0f);
	vec3 coord = transformed.xyz / transformed.w;
	coord = coord * 0.5f + 0.5f;
	coord.z -= 1.0f / 256.0f;

	return texture(uShadowMap, coord).r;
}
float SampleShadowPCF(in const vec3 position)
{
	float shadow = 0.0;
	vec2 texel_size = 1.0 / textureSize(uShadowMap, 0);

	vec4 transformed = uShadowTransform * vec4(position, 1.0f);
	vec3 coord = transformed.xyz / transformed.w;
	coord = coord * 0.5f + 0.5f;
	coord.z -= 1.0f / 1024.0f; //bias
	shadow += texture(uShadowMap, vec3(coord.xy + vec2( texel_size.x,  texel_size.y), coord.z)).r;
	shadow += texture(uShadowMap, vec3(coord.xy + vec2( texel_size.x,             0), coord.z)).r;
	shadow += texture(uShadowMap, vec3(coord.xy + vec2( texel_size.x, -texel_size.y), coord.z)).r;
	shadow += texture(uShadowMap, vec3(coord.xy + vec2(            0,  texel_size.y), coord.z)).r;
	shadow += texture(uShadowMap, vec3(coord.xy + vec2(            0,             0), coord.z)).r;
	shadow += texture(uShadowMap, vec3(coord.xy + vec2(            0, -texel_size.y), coord.z)).r;
	shadow += texture(uShadowMap, vec3(coord.xy + vec2(-texel_size.x,  texel_size.y), coord.z)).r;
	shadow += texture(uShadowMap, vec3(coord.xy + vec2(-texel_size.x,             0), coord.z)).r;
	shadow += texture(uShadowMap, vec3(coord.xy + vec2(-texel_size.x, -texel_size.y), coord.z)).r;
	return shadow / 9.0f;
}

void main()
{
	vec3 position = ReconstructPosition( gTexcoords );
	vec3 albedo = texture(uAlbedo, vec3(gTexcoords, gl_Layer)).rgb;

	oRadiance = (vec3(5, 4, 4)*SampleShadowPCF(position) + vec3(0.2, 0.16, 0.16)) * albedo;
}
