#version 450 core

in vec2 vTexcoords;
layout (binding = 2) uniform sampler2D uShadowMap;
uniform vec2 uDirection;
out vec4 oBlured;

void main()
{
	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.3846153846) * uDirection;
	vec2 off2 = vec2(3.2307692308) * uDirection;
	vec2 texel_size = vec2(1.0f) / vec2(textureSize(uShadowMap, 0));
	color += texture(uShadowMap, vTexcoords) * 0.2270270270;
	color += texture(uShadowMap, vTexcoords + (off1 * texel_size)) * 0.3162162162;
	color += texture(uShadowMap, vTexcoords - (off1 * texel_size)) * 0.3162162162;
	color += texture(uShadowMap, vTexcoords + (off2 * texel_size)) * 0.0702702703;
	color += texture(uShadowMap, vTexcoords - (off2 * texel_size)) * 0.0702702703;
	oBlured = color;
}
