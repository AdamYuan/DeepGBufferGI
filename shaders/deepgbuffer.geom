#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

in vec3 vNormal[];
in vec2 vTexcoords[];
flat in int vTexture[];
flat in vec3 vDiffuseColor[];

out vec2 gTexcoords;
out vec3 gNormal;
out vec4 gLastScreenPos;
flat out int gTexture;
flat out vec3 gDiffuseColor;

layout(std140, binding = 1) uniform uuCamera
{
	mat4 uProjection;
	mat4 uView;
	mat4 uInvPV;
};

uniform mat4 uLastView;

void main()
{
	gTexture = vTexture[0];
	gDiffuseColor = vDiffuseColor[0];

	vec4 pos0 = uProjection * uView * gl_in[0].gl_Position;
	vec4 pos1 = uProjection * uView * gl_in[1].gl_Position;
	vec4 pos2 = uProjection * uView * gl_in[2].gl_Position;
	vec4 last_pos0 = uProjection * uLastView * gl_in[0].gl_Position;
	vec4 last_pos1 = uProjection * uLastView * gl_in[1].gl_Position;
	vec4 last_pos2 = uProjection * uLastView * gl_in[2].gl_Position;


	gl_Layer = 0;
	{ //the first layer
		gTexcoords = vTexcoords[0];
		gNormal = vNormal[0];
		gl_Position = pos0;
		EmitVertex();
		gTexcoords = vTexcoords[1];
		gNormal = vNormal[1];
		gl_Position = pos1;
		EmitVertex();
		gTexcoords = vTexcoords[2];
		gNormal = vNormal[2];
		gl_Position = pos2;
		EmitVertex();
		EndPrimitive();
	}

	gl_Layer = 1;
	{ //the second layer
		gTexcoords = vTexcoords[0];
		gNormal = vNormal[0];
		gLastScreenPos = last_pos0;
		gl_Position = pos0;
		EmitVertex();
		gTexcoords = vTexcoords[1];
		gNormal = vNormal[1];
		gLastScreenPos = last_pos1;
		gl_Position = pos1;
		EmitVertex();
		gTexcoords = vTexcoords[2];
		gNormal = vNormal[2];
		gLastScreenPos = last_pos2;
		gl_Position = pos2;
		EmitVertex();
		EndPrimitive();
	}
}
