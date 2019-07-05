#version 450 core
//#define TEST_SCENE
#define TEST_RADIOSITY
//#define TEST_SHADOW

#ifdef TEST_RADIOSITY
out vec4 oColor;
layout (binding = 3) uniform sampler2DArray uAlbedo;
layout (binding = 6) uniform sampler2DArray uRadiance;
layout (binding = 7) uniform sampler2D uGIRadiance;
in vec2 vTexcoords;

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 color = texelFetch(uRadiance, ivec3(coord, 0), 0).rgb 
		+ texelFetch(uGIRadiance, coord, 0).rgb * texelFetch(uAlbedo, ivec3(coord, 0), 0).rgb;
	oColor = vec4(pow(color, vec3(1.0f / 2.2f)) , 1);
}
#endif

#ifdef TEST_SCENE
out vec4 oColor;
layout (binding = 2) uniform sampler2DArray uTexture;
in vec2 vTexcoords;

void main()
{
	vec3 coord = vec3(vTexcoords, gl_FragCoord.x < 640 ? 0 : 1);
	oColor = vec4(
		pow(texture(uTexture, coord).rgb, vec3(1.0f / 2.2f))
		, 1);
}
#endif

#ifdef TEST_SHADOW
out vec4 oColor;
layout (binding = 2) uniform isampler2D uTexture;
in vec2 vTexcoords;

//copied from https://github.com/TheRealMJP/Shadows/blob/master/Shadows/MSM.hlsl (MIT License)
vec4 ConvertMoments(vec4 optimized_moments) {
	optimized_moments[0] -= 0.0359558848;
	const mat4 mat = mat4(0.2227744146, 0.1549679261, 0.1451988946, 0.163127443,
						  0.0771972861, 0.1394629426, 0.2120202157, 0.2591432266,
						  0.7926986636,0.7963415838, 0.7258694464, 0.6539092497,
						  0.0319417555,-0.1722823173,-0.2758014811,-0.3376131734);
	return mat * optimized_moments;
}

void main()
{
	vec4 moments = ConvertMoments( vec4( texture(uTexture, vTexcoords) ) / 32768.0f );
	oColor = vec4(vec3(moments.r), 1.0f);
}
#endif

/*#version 450 core
layout (binding = 2) uniform sampler2D uTexture;
layout (binding = 3, rgba8) uniform image2D uImage;
in vec2 vTexcoords;
out vec4 oColor;
void main()
{
	oColor = vec4(imageLoad(uImage, ivec2(gl_FragCoord.xy)).xyz + vec3(0.005f), 1.0f);
	imageStore(uImage, ivec2(gl_FragCoord.xy), oColor);
}*/
