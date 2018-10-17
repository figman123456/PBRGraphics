#version 450 core

out vec4 fragment_colour;

struct DirectionalLight
{
	vec3 direction;
	vec3 intensity;
};

layout(std140, binding=2) uniform DirectionalLightBuffer{
	DirectionalLight lights[2];
};


uniform sampler2D gPosition;
uniform sampler2D gNormalRoughness;
uniform sampler2D gAlbedoMetal;

uniform vec3 camera_position;

const float PI = 3.14159265359;

vec3 unpackNormal(vec2 enc)
{
    vec2 fenc = enc*4-2;
    float f = dot(fenc,fenc);
    float g = sqrt(1-f/4);
    vec3 n;
    n.xy = fenc*g;
    n.z = 1-f/2;
    return n;
}

vec3 EnvBRDFApprox( vec3 SpecularColor, float Roughness, float NoV ) {
    const vec4 c0 = vec4(-1, -0.0275, -0.572, 0.022 );
    const vec4 c1 = vec4( 1, 0.0425, 1.04, -0.04 );
    vec4 r = Roughness * c0 + c1;
    float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
    vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
    return SpecularColor * AB.x + AB.y;
}

float saturate(float f) {
    return clamp(f, 0.0, 1.0);
}

vec3 saturate(vec3 v) {
    return clamp(v, vec3(0.0), vec3(1.0));
}

float chiGGX(float v) {
    return v > 0 ? 1 : 0;
}

float GGX_Distribution(vec3 n, vec3 h, float alpha) {
    float NoH = dot(n,h);
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float den = NoH2 * alpha2 + (1.0 - NoH2);
    return (chiGGX(NoH) * alpha2) / ( PI * den * den );
}

float GGX_PartialGeometryTerm(vec3 v, vec3 n, vec3 h, float alpha)
{
    float VoH2 = saturate(dot(v,h));
    float chi = chiGGX( VoH2 / saturate(dot(v,n)) );
    VoH2 = VoH2 * VoH2;
    float tan2 = ( 1 - VoH2 ) / VoH2;
    return (chi * 2) / ( 1 + sqrt( 1 + alpha * alpha * tan2 ) );
}

vec3 Fresnel_Schlick(float cosT, vec3 F0)
{
  return F0 + (1-F0) * pow( 1 - cosT, 5);
}

vec3 F_Schlick(vec3 F0, float fd90, float cosT)
{
  return F0 + fd90 * pow( 1 - cosT, 5);
}

float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    float energyBias = mix(0.0, 0.5, linearRoughness);
    float energyFactor = mix(1.0, 1.0 / 1.51, linearRoughness);
    float fd90 = energyBias + 2.0 * LdotH*LdotH * linearRoughness;
    vec3 f0 = vec3(1.0f, 1.0f, 1.0f);
    float lightScatter = F_Schlick(f0, fd90, NdotL).r;
    float viewScatter = F_Schlick(f0, fd90, NdotV).r;
    return lightScatter * viewScatter * energyFactor;
}

void main()
{
	vec3 FragPos = texelFetch(gPosition, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 NormalRoughness = texelFetch(gNormalRoughness, ivec2(gl_FragCoord.xy), 0).rgb;
	vec2 packedNormal = NormalRoughness.rg;
	vec3 Normal = unpackNormal(packedNormal);
	vec3 albedo = pow(texelFetch(gAlbedoMetal, ivec2(gl_FragCoord.xy), 0).rgb,vec3(2.2));
	float metallic = texelFetch(gAlbedoMetal,ivec2(gl_FragCoord.xy), 0).a;
	float roughness = NormalRoughness.b;

	vec3 colour = vec3(0.0);

	float ior = 1.000277;

	vec3 N = normalize(Normal);
	vec3 V = normalize(camera_position-FragPos);

	vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
	float NoV = clamp(dot(N,V), 0.0,1.0);
	vec3 reflectance = EnvBRDFApprox( F0, roughness, NoV );

	vec3 kS = vec3(0.0);
	vec3 kD = vec3(1.0 - kS) * (1.0 - metallic);

	colour = kD * albedo * vec3(.3, .3, .3) * vec3(.3, .3, .3) * reflectance;

	for(int i = 0; i < 2; ++i) 
	{
		vec3 L = normalize(lights[i].direction);
		vec3 H = normalize(V+L);

		float VdotH = saturate(dot(V, H));
		float NdotL = saturate(dot(N, L));
		float NdotH = saturate(dot(N, H));
		float NdotV = saturate(dot(N, V));
		float LdotH = saturate(dot(L, H));
   

		float alpha = roughness * roughness;
		float D = GGX_Distribution(N,H,alpha);
		float G = GGX_PartialGeometryTerm(V, N, L, alpha);
		vec3 F = Fresnel_Schlick(VdotH, F0);
		float Fd = Fr_DisneyDiffuse(NdotV, NdotL, LdotH, roughness);
		float denom = saturate(4 * (NdotV * NdotH + 0.01));

		vec3 light_colour = lights[i].intensity;

		vec3 directSpecular = light_colour * NdotL * D * G * F / denom;
		vec3 directDiffuse = NdotL * light_colour * albedo / PI;
		colour += directDiffuse * Fd + directSpecular;
	}
	colour = colour / (colour + vec3(1.0));
	colour = pow(colour, vec3(1.0/2.2));

	fragment_colour = vec4(colour,1.0);
}