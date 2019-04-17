#define MAX_DIRECTIONAL_LIGHTS 2
#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4

const float PI = 3.14159265359f; 

struct Material
{
    sampler2D diffuse_texture;
    vec4      diffuse_color;

    sampler2D specular_texture;
    float     shininess;

    sampler2D occlusion_texture;
    sampler2D normal_texture;

    sampler2D emissive_texture;
    vec3      emissive_color;

    float roughness;
	float metallic;
};

struct DirLight
{
	vec3 direction;
	vec3 color;
	float intensity;
};

struct PointLight
{
	vec3  position;
	vec3  color;
	float radius;
	float intensity;
};

struct SpotLight
{
	vec3  position;
	vec3  direction;
	vec3  color;
	float inner;
	float outer;
	float radius;
	float intensity;
};

struct Lights
{
	vec3        ambient_color; 
	DirLight    directional[MAX_DIRECTIONAL_LIGHTS];
	int			num_directionals;
	PointLight  points[MAX_POINT_LIGHTS];
	int         num_points;
	SpotLight   spots[MAX_SPOT_LIGHTS];
	int         num_spots;
};


layout (std140) uniform Matrices
{
    mat4 proj;
    mat4 view;
};

in vec3 normalIn;
in vec3 position;
in vec2 uv0;
in vec3 viewPos;


in vec3 pointPositions[MAX_POINT_LIGHTS]; //positions in tangent space
in vec3 spotPositions[MAX_SPOT_LIGHTS];   //positions in tangent space
in vec3 spotDirections[MAX_SPOT_LIGHTS];  //directions in tangent space
in vec3 directionalDirections[MAX_DIRECTIONAL_LIGHTS]; //directions in tangent space

out vec4 Fragcolor;

uniform Material material;
uniform Lights lights;
uniform int hasNormalMap;

vec4 textureGammaCorrected(sampler2D tex)
{
	vec4 texRaw = texture2D(tex, uv0);
	return vec4(pow(texRaw.r, 2.2), pow(texRaw.g, 2.2), pow(texRaw.b, 2.2), texRaw.a);
}


vec4 get_albedo()
{
	return textureGammaCorrected(material.diffuse_texture) * material.diffuse_color;
}

vec3 get_occlusion_color()
{
	return textureGammaCorrected(material.occlusion_texture).rgb;
}

vec3 get_emissive_color()
{
	return textureGammaCorrected(material.emissive_texture).rgb * material.emissive_color;
}

float get_attenuation(float distance, float radius, float intensity)
{	
	float att = intensity / pow((max(distance, radius) / radius + 1), 2);
	return mix(att, 0, distance / radius);
}

float D(vec3 H, vec3 N)
{
	float r2 = pow(material.roughness, 2);
	float NdotH = dot(N,H);
	float NdotH2 = pow(NdotH, 2);
	float num = pow(r2, 2);
	float den = NdotH2 * (num - 1.f) + 1.f;
	return num / max(den, 0.001f);
}

float GGX(vec3 M, vec3 N)
{
	float NdotM = max(dot(N,M), 0.001f);

	float r = (material.roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotM;
    float denom = max(NdotM * (1.0 - k) + k, 0.001f);
	
    return num / denom;
}

float GSmith(vec3 L, vec3 V, vec3 N)
{
	return GGX(L, N) * GGX(V, N);
}

vec3 BRDF(vec3 F, vec3 L, vec3 V, vec3 N, vec3 H)
{
	vec3 num = F * GSmith(L, V, N) * D(H, N);
	float den = 4 * dot(N,L) * dot(N,V);
	return num / max(den, 0.001);
}

vec3 FSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
} 


vec3 CalculateNormal()
{
	vec3 normalM = texture(material.normal_texture, uv0).xyz;
	normalM = normalize(normalM * 2.0 - 1.0);
	return hasNormalMap * normalM + (1 - hasNormalMap) * normalIn;	
}

void main()
{
	vec3 normal = CalculateNormal();	
	vec4 albedo = get_albedo();
	
	vec3 F0 = vec3(.04f);
	F0 = mix(F0, albedo.rgb, material.metallic);

	vec3 color = vec3(0); 
	
	vec3 N = normal;
	vec3 V = normalize(viewPos - position);
	for(int i=0; i < lights.num_directionals; ++i)
	{
		vec3 L = directionalDirections[i];
		vec3 H = normalize(V + L);	

		vec3 radiance = lights.directional[i].color * lights.directional[i].intensity;				
		
		vec3 F = FSchlick(max(dot(H, V), 0.0), F0);   

		vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - material.metallic; //albedo amount		

		float NdotL = max(dot(N, L), 0.0);        
		color += (kD * albedo.rgb / PI + BRDF(F, L, V, N, H)) * radiance * NdotL;  
	}
	for(int i=0; i < lights.num_points; ++i)
	{	
		vec3 lightPos = pointPositions[i];
		vec3 L = normalize(lightPos - position);
		vec3 H = normalize(V + L);
		float distance = length(lightPos - position);

		float att = max(get_attenuation(distance, lights.points[i].radius, lights.points[i].intensity), 0);

		vec3 radiance = lights.points[i].color * att;				
		
		vec3 F = FSchlick(max(dot(H, V), 0.0), F0);   

		vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - material.metallic; //albedo amount		

		float NdotL = max(dot(N, L), 0.0);        
		color += (kD * albedo.rgb / PI + BRDF(F, L, V, N, H)) * radiance * NdotL;  
	}
	
	for(int i=0; i < lights.num_spots; ++i)
	{
		vec3 lightPos = spotPositions[i];
		vec3 L = normalize(lightPos - position);
		vec3 H = normalize(V + L);
		float distance = length(lightPos - position);

		float theta = dot(normalize(L), normalize(-(spotDirections[i])));
		float epsilon = max(0.0001, lights.spots[i].inner - lights.spots[i].outer);
		float cone = clamp((theta - lights.spots[i].outer) / epsilon, 0.0, 1.0); 
	
		float att = max(get_attenuation(distance, lights.spots[i].radius, lights.spots[i].intensity), 0);
		vec3 radiance = lights.spots[i].color * att * cone;
		
		vec3 F = FSchlick(max(dot(H, V), 0.0), F0);   

		vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - material.metallic; //albedo amount		

		float NdotL = max(dot(N, L), 0.0);        
		color += (kD * albedo.rgb / PI + BRDF(F, L, V, N, H)) * radiance * NdotL;
	}
	
	
	color *= lights.ambient_color;
	color *= get_occlusion_color();
	color += get_emissive_color();
	color = vec3(pow(color.r, (1.0 / 2.2)), pow(color.g, (1.0 / 2.2)), pow(color.b, (1.0 / 2.2)));
	
	Fragcolor = vec4(color, albedo.a);
}
