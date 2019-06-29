#version 450 core
#extension GL_ARB_bindless_texture : require

layout (binding = 0) uniform uuDiffuseTextures { sampler2D uDiffuseTextures[1024]; };

in vec2 vTexcoords;
flat in int vTexture;

void main() { if(vTexture != -1 && texture(uDiffuseTextures[vTexture], vTexcoords).a < 0.5f) discard; }
