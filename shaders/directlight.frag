#version 450 core

layout(std140, binding = 1) uniform uuCamera
{
	mat4 uProjection;
	mat4 uView;
	float uX, uY, uZ, uInvCosHalfFov;
};

layout (binding = 2) uniform sampler2D uShadowMap;
layout (binding = 3) uniform sampler2DArray uAlbedo;
layout (binding = 4) uniform sampler2DArray uNormal;
layout (binding = 5) uniform sampler2DArray uDepth;

layout (location = 0) out vec3 oRadiance;

uniform mat4 uShadowTransform;
uniform vec3 uLightDir;

in vec2 gTexcoords;

vec3 ReconstructPosition(in const vec2 texcoords)
{
	vec4 clip = vec4(texcoords * 2.0f - 1.0f, texture(uDepth, vec3(texcoords, gl_Layer)).r * 2.0f - 1.0f, 1.0f);
	vec4 rec = inverse(uProjection * uView) * clip;
	return rec.xyz / rec.w;
}

//copied from https://github.com/TheRealMJP/Shadows/blob/master/Shadows/MSM.hlsl (MIT License)
vec4 ConvertMoments(vec4 optimized_moments) {
	optimized_moments[0] -= 0.0359558848;
	const mat4 mat = mat4(0.2227744146, 0.1549679261, 0.1451988946, 0.163127443,
						  0.0771972861, 0.1394629426, 0.2120202157, 0.2591432266,
						  0.7926986636,0.7963415838, 0.7258694464, 0.6539092497,
						  0.0319417555,-0.1722823173,-0.2758014811,-0.3376131734);
	return mat * optimized_moments;
}

float ComputeMSMHamburger(in vec4 moments, in float depth, in float depth_bias, in float moment_bias)
{
	// Bias input data to avoid artifacts
	vec4 b = mix(moments, vec4(0.5f, 0.5f, 0.5f, 0.5f), moment_bias);
	vec3 z;
	z[0] = depth - depth_bias;

	// Compute a Cholesky factorization of the Hankel matrix B storing only non-
	// trivial entries or related products
	float L32D22 = fma(-b[0], b[1], b[2]);
	float D22 = fma(-b[0], b[0], b[1]);
	float squaredDepthVariance = fma(-b[1], b[1], b[3]);
	float D33D22 = dot(vec2(squaredDepthVariance, -L32D22), vec2(D22, L32D22));
	float InvD22 = 1.0f / D22;
	float L32 = L32D22 * InvD22;

	// Obtain a scaled inverse image of bz = (1,z[0],z[0]*z[0])^T
	vec3 c = vec3(1.0f, z[0], z[0] * z[0]);

	// Forward substitution to solve L*c1=bz
	c[1] -= b.x;
	c[2] -= b.y + L32 * c[1];

	// Scaling to solve D*c2=c1
	c[1] *= InvD22;
	c[2] *= D22 / D33D22;

	// Backward substitution to solve L^T*c3=c2
	c[1] -= L32 * c[2];
	c[0] -= dot(c.yz, b.xy);

	// Solve the quadratic equation c[0]+c[1]*z+c[2]*z^2 to obtain solutions
	// z[1] and z[2]
	float p = c[1] / c[2];
	float q = c[0] / c[2];
	float D = (p * p * 0.25f) - q;
	float r = sqrt(D);
	z[1] =- p * 0.5f - r;
	z[2] =- p * 0.5f + r;

	// Compute the shadow intensity by summing the appropriate weights
	vec4 switch_val = (z[2] < z[0]) ? vec4(z[1], z[0], 1.0f, 1.0f) :
		((z[1] < z[0]) ? vec4(z[0], z[1], 0.0f, 1.0f) :
		 vec4(0.0f,0.0f,0.0f,0.0f));
	float quotient = (switch_val[0] * z[2] - b[0] * (switch_val[0] + z[2]) + b[1])/((z[2] - switch_val[1]) * (z[0] - z[1]));
	float shadow_intensity = switch_val[2] + switch_val[3] * quotient;
	return 1.0f - clamp(shadow_intensity, 0.0, 1.0);
}


float SampleShadow(in const vec3 position)
{
	vec4 transformed = uShadowTransform * vec4(position, 1.0f);
	vec3 coord = transformed.xyz / transformed.w;
	coord = coord * 0.5f + 0.5f;

	vec4 moments = ConvertMoments(texture2D(uShadowMap, coord.xy));

	float shadow = ComputeMSMHamburger(moments, coord.z, 0.0f, 3e-5f);
	shadow = smoothstep(0.65f, 1.0f, shadow);
	return shadow;
}

//oct16 normal decoding :
// Returns ±1
vec2 SignNotZero(vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
vec3 oct_to_float32x3(vec2 e)
{
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * SignNotZero(v.xy);
	return normalize(v);
}

void main()
{
	vec3 position = ReconstructPosition( gTexcoords );
	vec3 albedo = texture(uAlbedo, vec3(gTexcoords, gl_Layer)).rgb;
	vec3 normal = oct_to_float32x3( texture(uNormal, vec3(gTexcoords, gl_Layer)).rg );

	oRadiance = max( dot(normal, -uLightDir), 0.0f ) * vec3(5, 4, 4) * SampleShadow(position) * albedo;
	oRadiance += albedo * vec3(0.2, 0.16, 0.16);
}
