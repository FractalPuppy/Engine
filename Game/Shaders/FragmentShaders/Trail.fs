#version 330 core

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 highlight;

uniform vec4 colorU;

uniform sampler2D texture;

in vec2 tCoords;

void main()
{ 
	color = texture2D(texture, tCoords) * colorU;
	highlight = vec4(0);

}  