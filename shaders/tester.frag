#version 450 core
out vec4 oColor;
layout (binding = 2) uniform sampler2DArray uTexture;
in vec2 vPosition;

const float kNear = 1.0f / 512.0f, kFar = 4.0f;
float LinearDepth(in const float depth) 
{
	float z = depth * 2.0 - 1.0; // back to NDC 
	return (2.0 * kNear * kFar) / (kFar + kNear - z * (kFar - kNear));
}
void main()
{
	vec3 coord = vec3(vPosition*0.5f + 0.5f, gl_FragCoord.x < 640 ? 0 : 1);
	oColor = vec4(
		pow(texture(uTexture, coord).rgb, vec3(1.0f / 2.2f))
		, 1);

	/*vec3 coord = vec3(vPosition*0.5f + 0.5f, gl_FragCoord.x < 640 ? 0 : 1);
	oColor = vec4(
		vec3(LinearDepth(texture(uTexture, coord).r))
		, 1);*/
}

//shadow map test
/*#version 450 core
out vec4 oColor;
layout (binding = 2) uniform sampler2D uTexture;
in vec2 vPosition;

const float kNear = 1.0f / 512.0f, kFar = 4.0f;
float LinearDepth(in const float depth) 
{
	float z = depth * 2.0 - 1.0; // back to NDC 
	return (2.0 * kNear * kFar) / (kFar + kNear - z * (kFar - kNear));
}
void main()
{
	vec2 coord = vec2(vPosition*0.5f + 0.5f);
	oColor = vec4( vec3(texture(uTexture, coord).r), 1);
}*/
