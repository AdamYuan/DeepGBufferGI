#version 450 core

in vec2 vTexcoords;
layout(std140, binding = 1) uniform uuCamera
{
	mat4 uProjection;
	mat4 uView;
	mat4 uInvPV;
};
layout (binding = 5) uniform sampler2DArray uDepth;
layout (binding = 7) uniform sampler2D uOutputRadiance;

uniform mat4 uLastView;
out vec3 oReprojected;

vec2 Reproject(in const vec2 texcoords, in const float depth)
{
	vec4 clip = vec4(texcoords * 2.0f - 1.0f, depth * 2.0f - 1.0f, 1.0f);
	vec4 rec = uInvPV * clip;
	rec = (uProjection * uLastView) * vec4(rec.xyz / rec.w, 1.0);
	rec.xy /= rec.w;
	return rec.xy * 0.5f + 0.5f;
}

vec3 ClipAABB(in const vec3 aabb_min, in const vec3 aabb_max, in const vec3 p, in const vec3 q)
{
	vec3 p_clip = (aabb_max + aabb_min) * 0.5f;
	vec3 e_clip = (aabb_max - aabb_min) * 0.5f;

	vec3 v_clip = q - p_clip;
	vec3 v_unit = v_clip / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if(ma_unit > 1.0f) 
		return p_clip + v_clip / ma_unit;
	return q;
}

void main()
{
	ivec2 frag_coord = ivec2(gl_FragCoord.xy);
	float depth = texelFetch(uDepth, ivec3(frag_coord, 0), 0).r;
	vec2 last_texcoords = Reproject(vTexcoords, depth);

	/*vec3 samp, aabb_min, aabb_max, color_in;
	color_in = aabb_max = aabb_min = texelFetch(uOutputRadiance, frag_coord, 0).rgb;

	samp = texelFetch(uOutputRadiance, ivec2(frag_coord.x - 1, frag_coord.y), 0).rgb;
	aabb_min = min(aabb_min, samp); aabb_max = max(aabb_max, samp);
	samp = texelFetch(uOutputRadiance, ivec2(frag_coord.x + 1, frag_coord.y), 0).rgb;
	aabb_min = min(aabb_min, samp); aabb_max = max(aabb_max, samp);
	samp = texelFetch(uOutputRadiance, ivec2(frag_coord.x, frag_coord.y - 1), 0).rgb;
	aabb_min = min(aabb_min, samp); aabb_max = max(aabb_max, samp);
	samp = texelFetch(uOutputRadiance, ivec2(frag_coord.x, frag_coord.y + 1), 0).rgb;
	aabb_min = min(aabb_min, samp); aabb_max = max(aabb_max, samp);*/

	vec3 color_hist = texture(uOutputRadiance, last_texcoords).rgb;
	oReprojected = color_hist;
}
