#version 450

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormalRoughness;
layout(location = 2) out vec4 gAlbedogMetal;


layout (binding=0) uniform sampler2DArray textureArray;

in vec3 varying_position;
in vec3 varying_normal;
in vec3 varying_tangent;
in vec2 varying_texcoord;

flat in uint material_ID;

struct Material
{
	uint diffuse_texture_index;
	uint normal_texture_index;
	uint roughness_texture_index;
	uint metalic_texture_index;
};

layout(std140, binding=1) uniform MaterialBuffer{
	Material materials[7];
};


vec3 calculateNormal()
{
	vec3 normal = normalize(varying_normal);
	vec3 tangent = normalize(varying_tangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(tangent,normal);
	vec3 bumpMapNormal = texture(textureArray, vec3(varying_texcoord,materials[material_ID].normal_texture_index)).rgb;
	bumpMapNormal = 2.0 * bumpMapNormal - vec3(1.0,1.0,1.0);
	vec3 newNormal;
	mat3 TBN = mat3(tangent,bitangent,normal);
	newNormal = TBN * bumpMapNormal;
	newNormal = normalize(newNormal);
	return newNormal;
}

vec2 packNormal(vec3 n)
{
	vec2 enc = normalize(n.xy) * (sqrt(-n.z*0.5+0.5));
	enc = enc*0.5+0.5;
	return enc;
}

void main(void)
{
	gPosition = varying_position;
	vec3 normal = calculateNormal();
	gNormalRoughness.rg = packNormal(normal);
	gNormalRoughness.b = texture(textureArray, vec3(varying_texcoord,materials[material_ID].roughness_texture_index)).r;
	gAlbedogMetal.rgb = texture(textureArray, vec3(varying_texcoord,materials[material_ID].diffuse_texture_index)).rgb;
	gAlbedogMetal.a = 1-texture(textureArray, vec3(varying_texcoord,materials[material_ID].metalic_texture_index)).r;
}
