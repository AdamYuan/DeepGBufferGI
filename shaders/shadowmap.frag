#version 450 core
#extension GL_ARB_bindless_texture : require

layout (binding = 0) uniform uuDiffuseTextures { sampler2D uDiffuseTextures[1024]; };
layout (location = 0) out vec4 oShadowColor;

in vec2 vTexcoords;
flat in int vTexture;

vec4 ComputeMoments(in const float depth)
{
	float square = depth * depth;
	vec4 mu = vec4(depth, square, square * depth, square * square);
	const mat4 mat = mat4(-2.07224649,    13.7948857237,  0.105877704,   9.7924062118,
						   32.23703778,  -59.4683975703, -1.9077466311, -33.7652110555,
						   -68.571074599,  82.0359750338,  9.3496555107,  47.9456096605,
						   39.3703274134,-35.364903257,  -6.6543490743, -23.9728048165);
	vec4 ret = mat * mu;
	ret[0] += 0.0359558848;
	return ret;
}

void main() 
{
	if(vTexture != -1 && texture(uDiffuseTextures[vTexture], vTexcoords).a < 0.5f) discard;
	oShadowColor = ComputeMoments(gl_FragCoord.z);
}
