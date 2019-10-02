#define MAX_DIRECTIONAL_LIGHTS 2
#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4


layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv0;
layout(location = 2) in vec3 vertex_normal;
layout(location = 3) in vec3 vertex_tangent;
layout(location = 4) in ivec4 bone_indices;
layout(location = 5) in vec4 bone_weights;

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

	sampler2D dissolve_texture;
	vec3 dissolve_color;

    float roughness;
	float bloomIntensity;
	vec3 specular;
};

layout (std140) uniform Matrices
{
    mat4 proj;
    mat4 view;
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


uniform Lights lights;
uniform mat4 model;
uniform mat4 palette[64];

uniform mat4 lightProjView;
uniform float time;

//Water stuff
uniform float waterAmplitude1;
uniform float frequency1;
uniform float decay1;
uniform vec3 source1;

uniform float waterAmplitude2;
uniform float frequency2;
uniform float decay2;
uniform	vec2 uvScaler;

uniform vec3 source2;
uniform vec3 eyePosUniform;

uniform Material material;
//

out vec3 normalIn;
out vec3 position;
out vec3 positionWorld;
out vec2 uv0;
out vec3 viewPos;
out vec3 pointPositions[MAX_POINT_LIGHTS];
out vec3 spotPositions[MAX_SPOT_LIGHTS];
out vec3 spotDirections[MAX_SPOT_LIGHTS];
out vec3 directionalDirections[MAX_DIRECTIONAL_LIGHTS];
out vec4 shadow_coord;
out vec3 eye_pos;

void main()
{

	eye_pos = eyePosUniform;	

#ifndef WATER	
	#ifdef SKINNED
		mat4 skin_t = palette[bone_indices[0]]*bone_weights[0]+palette[bone_indices[1]]*bone_weights[1]+
		palette[bone_indices[2]]*bone_weights[2]+palette[bone_indices[3]]*bone_weights[3];
		positionWorld = (skin_t*vec4(vertex_position, 1.0)).xyz;	
		normalIn = (skin_t*vec4(vertex_normal, 0.0)).xyz; // 0.0 avoid translation
		vec3 tan = (skin_t*vec4(vertex_tangent, 0.0)).xyz; // 0.0 avoid translation
	#else
		positionWorld = (model * vec4(vertex_position, 1.0)).xyz;
		//normalIn = mat3(model) * vertex_normal;
		normalIn = mat3(transpose(inverse(model))) * vertex_normal; //Temporary
		vec3 tan = mat3(model) * vertex_tangent;
	#endif
	#ifdef SHADOWS_ENABLED
		shadow_coord = lightProjView * vec4(positionWorld, 1.0);
	#endif
	
	gl_Position = proj*view*vec4(positionWorld, 1.0);
	vec3 bitan = cross(vertex_tangent, vertex_normal);
	vec3 N = normalize(normalIn);
	tan = normalize(tan - N * dot(N, tan));
	vec3 T = normalize(tan);
	vec3 B = cross(T, normalIn);	
	mat3 TBNMat = transpose(mat3(T, B, N));
	position = TBNMat * positionWorld;
	viewPos = TBNMat * eye_pos;
	
	uv0 = vertex_uv0;

	normalIn = TBNMat * normalIn;

	//transform lights to tangent space

	for(int i=0; i < lights.num_directionals; ++i)
	{
		directionalDirections[i] = TBNMat * lights.directional[i].direction;
	}

	for(int i=0; i < lights.num_points; ++i)
	{
		pointPositions[i] = TBNMat * lights.points[i].position;
	}

		for(int i=0; i < lights.num_spots; ++i)
		{
			spotPositions[i] = TBNMat * lights.spots[i].position;
			spotDirections[i] = TBNMat * lights.spots[i].direction;
		}
#else	
	position = vertex_position;
	uv0 = vec2(abs(vertex_position.x * uvScaler.x), abs(vertex_position.y * uvScaler.y));

	float dist1 = sqrt(pow(position.x - source1.x,2) + pow(position.y - source1.y,2));
	float dist2 = sqrt(pow(position.x - source2.x,2) + pow(position.y - source2.y,2));
	dist1 = max(dist1, 20);
	dist2 = max(dist2, 20);
	float ang1 = (dist1 / frequency1) - time;
	float ang2 = (dist2 / frequency2) - time;
	float att1 = (waterAmplitude1 / (dist1 * decay1));
	float att2 = (waterAmplitude2 / (dist2 * decay2));
	float s1 = sin(ang1);
	float s2 = sin(ang2);
	float offset = s1 * att1 + s2 * att2;
	float c1= cos(-time + (dist1 / frequency1)) / (dist1) * frequency1;
	float c2= cos(-time + (dist2 / frequency2)) / (dist2) * frequency2;
	
	position.z += offset;

	normalIn = mat3(transpose(inverse(model))) * vertex_normal;
	positionWorld = position = (model*vec4(position, 1.0)).xyz;
	gl_Position = proj*view*vec4(positionWorld, 1.0);	

	viewPos = eye_pos;

#endif	
}
