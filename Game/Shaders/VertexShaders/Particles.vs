#version 330 core
layout (location = 0) in vec3 vertex; 
layout (location = 1) in vec2 uvs; 
layout (location = 2) in vec4 r0;
layout (location = 3) in vec4 r1;
layout (location = 4) in vec4 r2;
layout (location = 5) in vec4 position;

uniform mat4 view;
uniform mat4 projection;

out vec2 tCoords;

void main()
{
	mat4 model;
	model[0] = r0;
	model[1] = r1;
	model[2] = r2;
	model[3] = position;
    gl_Position = projection * view * model * vec4(vertex, 1.0f);
	tCoords = uvs;
}  