#version 450

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_tangent;
layout (location = 3) in vec2 vertex_texcoord;
layout (location = 6) in uint instance_material_ID;
layout (location = 7) in mat4 instance_model_xform;

uniform mat4 view_xform;
uniform mat4 projection_xform;

out vec3 varying_position;
out vec3 varying_normal;
out vec3 varying_tangent;
out vec2 varying_texcoord;
flat out uint material_ID;


void main(void)
{
	varying_position = vec3(instance_model_xform * vec4(vertex_position, 1.0f));
	varying_normal = transpose(inverse(mat3(instance_model_xform))) * vertex_normal;
	varying_tangent = transpose(inverse(mat3(instance_model_xform))) * vertex_tangent;
	varying_texcoord = vertex_texcoord;
	material_ID = instance_material_ID;
	mat4 mv = view_xform * instance_model_xform;
	mat4 mvp = projection_xform * view_xform * instance_model_xform;

	gl_Position = mvp * vec4(vertex_position, 1.0);
}