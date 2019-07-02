#version 450 core
//Based on https://github.com/Jam3/glsl-fast-gaussian-blur/ (MIT licensed)

in vec2 vTexcoords;
layout (binding = 2) uniform sampler2D uShadowMap;
uniform vec2 uDirection;
out vec4 oBlured;

void main()
{
	vec2 texel_size = vec2(1.0f) / vec2(textureSize(uShadowMap, 0));
	vec4 color = vec4(0.0f);

	/*vec2 off1 = vec2(1.3846153846) * uDirection;
	  vec2 off2 = vec2(3.2307692308) * uDirection;
	  color += texture(uShadowMap, vTexcoords) * 0.2270270270;
	  color += texture(uShadowMap, vTexcoords + (off1 * texel_size)) * 0.3162162162;
	  color += texture(uShadowMap, vTexcoords - (off1 * texel_size)) * 0.3162162162;
	  color += texture(uShadowMap, vTexcoords + (off2 * texel_size)) * 0.0702702703;
	  color += texture(uShadowMap, vTexcoords - (off2 * texel_size)) * 0.0702702703;*/

	vec2 off1 = vec2(1.3333333333333333) * uDirection;
	color += texture(uShadowMap, vTexcoords) * 0.29411764705882354;
	color += texture(uShadowMap, vTexcoords + (off1 * texel_size)) * 0.35294117647058826;
	color += texture(uShadowMap, vTexcoords - (off1 * texel_size)) * 0.35294117647058826;

	oBlured = color;
}
