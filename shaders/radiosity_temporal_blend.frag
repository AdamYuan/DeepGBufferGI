#version 450 core

in vec2 vTexcoords;
layout (binding = 7) uniform sampler2D uOutputRadiance;
layout (binding = 8) uniform sampler2D uReprojectedRadiance;

out vec3 oBlended;

void main()
{
	ivec2 frag_coord = ivec2(gl_FragCoord.xy);

	vec3 color_now = texelFetch(uOutputRadiance, frag_coord, 0).rgb;
	vec3 color_hist = texelFetch(uReprojectedRadiance, frag_coord, 0).rgb;
	oBlended = color_now*0.15f + color_hist*0.85f;
}
