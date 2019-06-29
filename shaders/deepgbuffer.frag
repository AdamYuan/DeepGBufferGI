#version 450 core
#extension GL_ARB_bindless_texture : require

layout (binding = 0) uniform uuDiffuseTextures { sampler2D uDiffuseTextures[1024]; };
layout (binding = 1) uniform sampler2D uLastDepthTexture;

layout (location = 0) out vec3 oAlbedo;
layout (location = 1) out vec2 oNormal;
layout (location = 2) out vec3 oRadiance;

in vec3 gNormal;
in vec2 gTexcoords;
in vec4 gLastScreenPos;
flat in int gTexture;
flat in vec3 gDiffuseColor;

const float kNear = 1.0f / 512.0f, kFar = 4.0f, kMinSeparate = 0.05f;
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

void main()
{
	vec4 samp = gTexture == -1 ? vec4(gDiffuseColor, 1.0) : texture(uDiffuseTextures[gTexture], gTexcoords);
	if(samp.w < 0.5f) discard;
	if(gl_Layer == 1 && 
	   LinearDepth( gLastScreenPos.z / gLastScreenPos.w ) <= 
	   LinearDepth( texture(uLastDepthTexture, (gLastScreenPos.xy / gLastScreenPos.w) * 0.5f + 0.5f ).r * 2.0f - 1.0f ) + kMinSeparate)
		discard;
	oAlbedo = samp.rgb;
	oNormal = float32x3_to_oct(normalize(gNormal));
}
