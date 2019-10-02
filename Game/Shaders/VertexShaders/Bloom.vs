#version 330

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_uv0;

out vec2 UV0;

void main()
{
   	UV0 = vertex_uv0;
	gl_Position =  vec4(vertex_position, 1.0);
}