layout (location = 0) in vec3 vertex; 
layout (location = 1) in vec2 uvs; 
layout(location = 4) in ivec4 bone_indices;
layout(location = 5) in vec4 bone_weights;

uniform mat4 model;
uniform mat4 viewProjection;
uniform mat4 palette[64];


out vec2 tCoords;

void main()
{
	vec3 position;
#ifdef SKINNED
	mat4 skin_t = palette[bone_indices[0]]*bone_weights[0]+palette[bone_indices[1]]*bone_weights[1]+
	palette[bone_indices[2]]*bone_weights[2]+palette[bone_indices[3]]*bone_weights[3];
	position = (skin_t*vec4(vertex, 1.0)).xyz;
#else
	position = (model * vec4(vertex, 1.0)).xyz;
#endif
    gl_Position = viewProjection * vec4(position, 1.0f);
}  