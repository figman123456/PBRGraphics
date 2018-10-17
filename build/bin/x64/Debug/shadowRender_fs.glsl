#version 450

out vec4 fragment_colour;

uniform sampler2D gPosition;
uniform sampler2D gNormalRoughness;
uniform sampler2D gAlbedoMetal;
uniform sampler2D shadowMap;

uniform vec3 camera_position;

void main(void)
{
	fragment_colour = vec4(1, 1, 1, 1);
}
