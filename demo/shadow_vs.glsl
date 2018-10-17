#version 450

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_tangent;
layout (location = 3) in vec2 vertex_texcoord;
layout (location = 7) in mat4 instance_model_xform;

uniform mat4 lightSpaceMat;

void main(void)
{
	gl_Position = lightSpaceMat * instance_model_xform * vec4(vertex_position, 1.0);
}