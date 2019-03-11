#version 330 core
out vec4 Fragcolor;

flat in vec4 fcolor;

void main()
{
	Fragcolor = fcolor;
}
