#version 330 core

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 highlight;
layout (location = 2) out vec4 brightColor;


uniform vec4 colorU;

uniform sampler2D texture;
uniform float bloomIntensity;

in vec2 tCoords;

void main()
{ 
	color = texture2D(texture, tCoords) * colorU * bloomIntensity;
	highlight = vec4(0);
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1)
        brightColor = color;
    else
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}  