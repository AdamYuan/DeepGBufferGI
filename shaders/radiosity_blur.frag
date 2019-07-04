#version 450 core
//Based on https://github.com/Jam3/glsl-fast-gaussian-blur/ (MIT licensed)

in vec2 vTexcoords;
layout (binding = 7) uniform sampler2D uGIRadiance;
uniform vec2 uDirection;
out vec3 oBlured;

void main()
{
	vec2 texel_size = vec2(1.0f) / vec2(textureSize(uGIRadiance, 0));
	vec3 color = vec3(0.0f);

	/*vec2 off1 = vec2(1.3846153846) * uDirection;
	vec2 off2 = vec2(3.2307692308) * uDirection;
	color += texture(uGIRadiance, vTexcoords).xyz * 0.2270270270;
	color += texture(uGIRadiance, vTexcoords + (off1 * texel_size)).xyz * 0.3162162162;
	color += texture(uGIRadiance, vTexcoords - (off1 * texel_size)).xyz * 0.3162162162;
	color += texture(uGIRadiance, vTexcoords + (off2 * texel_size)).xyz * 0.0702702703;
	color += texture(uGIRadiance, vTexcoords - (off2 * texel_size)).xyz * 0.0702702703;*/

	vec2 off1 = vec2(1.411764705882353) * uDirection;
	vec2 off2 = vec2(3.2941176470588234) * uDirection;
	vec2 off3 = vec2(5.176470588235294) * uDirection;
	color += texture2D(uGIRadiance, vTexcoords).xyz * 0.1964825501511404;
	color += texture2D(uGIRadiance, vTexcoords + (off1 * texel_size)).xyz * 0.2969069646728344;
	color += texture2D(uGIRadiance, vTexcoords - (off1 * texel_size)).xyz * 0.2969069646728344;
	color += texture2D(uGIRadiance, vTexcoords + (off2 * texel_size)).xyz * 0.09447039785044732;
	color += texture2D(uGIRadiance, vTexcoords - (off2 * texel_size)).xyz * 0.09447039785044732;
	color += texture2D(uGIRadiance, vTexcoords + (off3 * texel_size)).xyz * 0.010381362401148057;
	color += texture2D(uGIRadiance, vTexcoords - (off3 * texel_size)).xyz * 0.010381362401148057;

	/*vec2 off1 = vec2(1.3333333333333333) * uDirection;
	  color += texture(uGIRadiance, vTexcoords) * 0.29411764705882354;
	  color += texture(uGIRadiance, vTexcoords + (off1 * texel_size)) * 0.35294117647058826;
	  color += texture(uGIRadiance, vTexcoords - (off1 * texel_size)) * 0.35294117647058826;*/

	oBlured = color;
}
