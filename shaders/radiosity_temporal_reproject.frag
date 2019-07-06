#version 450 core

in vec2 vTexcoords;
layout(std140, binding = 1) uniform uuCamera
{
	mat4 uProjection;
	mat4 uView;
	mat4 uInvPV;
};
layout (binding = 1) uniform sampler2D uLastDepth;
layout (binding = 5) uniform sampler2DArray uDepth;
layout (binding = 7) uniform sampler2D uOutputRadiance;

uniform mat4 uLastView;
out vec3 oReprojected;

vec3 ReconstructPosition(in const vec2 texcoords, in const float depth, in const mat4 inv_pv)
{
	vec4 clip = vec4(texcoords * 2.0f - 1.0f, depth * 2.0f - 1.0f, 1.0f);
	vec4 rec = inv_pv * clip;
	return rec.xyz / rec.w;
}

vec2 ReprojectTexcoord(in const vec3 position, in const mat4 pv)
{
	vec4 rec = pv * vec4(position, 1.0);
	return (rec.xy / rec.w) * 0.5f + 0.5f;
}

void main()
{
	ivec2 frag_coord = ivec2(gl_FragCoord.xy);
	float depth = texelFetch(uDepth, ivec3(frag_coord, 0), 0).r;
	vec3 position = ReconstructPosition(vTexcoords, depth, uInvPV);

	mat4 last_pv = uProjection * uLastView;
	vec2 last_texcoords = ReprojectTexcoord(position, last_pv);
	//float last_depth = texture(uLastDepth, last_texcoords).r;
	//vec3 last_position = ReconstructPosition(last_texcoords, last_depth, inverse(last_pv));

	//vec3 dist = position - last_position;
	//if(dot(dist, dist) < 0.00001f) 
	oReprojected = texture(uOutputRadiance, last_texcoords).rgb;
}
