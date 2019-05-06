#version 330 core
out vec4 color;

uniform sampler2D texture;

in vec2 tCoords;

void main()
{   
	color = texture2D(texture, tCoords);
}  