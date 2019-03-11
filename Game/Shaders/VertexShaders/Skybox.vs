#version 330 core
layout (location = 0) in vec3 aPos;

layout (std140) uniform Matrices
{
    mat4 proj;
    mat4 view;
};

uniform mat4 model;
out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    gl_Position = proj * view* model* vec4(aPos, 1.0);
	gl_Position.z = gl_Position.w -0.00001; //fix to far plane.
}  