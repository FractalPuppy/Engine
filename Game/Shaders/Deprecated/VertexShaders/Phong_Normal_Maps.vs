#version 330
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;
layout(location = 3) in vec3 vertex_tangent;

layout (std140) uniform Matrices
{
    mat4 proj;
    mat4 view;
};

uniform mat4 model;
out vec3 normalIn;
out vec3 position;
out vec2 uv0;
out vec3 tan;
out vec3 bitan;

void main()
{
	position = (model * vec4(vertex_position, 1.0)).xyz;
	gl_Position = proj*view*vec4(position, 1.0);

	normalIn = mat3(transpose(inverse(model))) * vertex_normal;
	//normalIn = (model * vec4(vertex_normal, 1.0)).xyz; 
	tan = (model * vec4(vertex_tangent, 1.0)).xyz;
	bitan = (model * vec4(cross(vertex_tangent, vertex_normal), 1.0)).xyz;

	uv0 = vertex_uv0;
}