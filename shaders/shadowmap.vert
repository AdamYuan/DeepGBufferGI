#version 450 core
layout (location = 0) in vec3 aPosition;
layout (location = 2) in vec2 aTexcoords;
layout (location = 3) in int aTexture;

out vec2 vTexcoords;
flat out int vTexture;

void main()
{
	vTexcoords = aTexcoords;
	vTexture = aTexture;
	gl_Position = vec4(aPosition, 1.0f);
}
