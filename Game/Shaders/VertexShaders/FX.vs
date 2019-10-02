#version 330 core
layout (location = 0) in vec3 vertex; 
layout (location = 1) in vec2 uvs; 

uniform mat4 model;

layout (std140) uniform Matrices
{
    mat4 proj;
    mat4 view;
};

out vec2 tCoords;

void main()
{
    gl_Position = proj * view * model * vec4(vertex, 1.0f);
	tCoords = uvs;
}  