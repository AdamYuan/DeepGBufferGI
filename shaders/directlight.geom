#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

in vec2 vTexcoords[];
out vec2 gTexcoords;

void main()
{
	gl_Layer = 0;

	gl_Position = gl_in[0].gl_Position;
	gTexcoords = vTexcoords[0];
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	gTexcoords = vTexcoords[1];
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	gTexcoords = vTexcoords[2];
	EmitVertex();
	EndPrimitive();

	gl_Layer = 1;

	gl_Position = gl_in[0].gl_Position;
	gTexcoords = vTexcoords[0];
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	gTexcoords = vTexcoords[1];
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	gTexcoords = vTexcoords[2];
	EmitVertex();
	EndPrimitive();
}
