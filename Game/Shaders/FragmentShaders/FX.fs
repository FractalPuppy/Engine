#version 330 core
in vec2 TexCoords;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 highlight;
layout (location = 2) out vec4 brightColor;


uniform sampler2D texture;
uniform int xTiles;
uniform int yTiles;

uniform int f1Xpos;
uniform int f1Ypos;
uniform int f2Xpos;
uniform int f2Ypos;
uniform float mixAmount;
uniform vec4 colorIn;
uniform vec2 uvSpeed;
uniform float time;
uniform float bloomIntensity;

in vec2 tCoords;


void main()
{   
	vec2 tCoordsPanned = tCoords + uvSpeed * time;
	float cellS = tCoordsPanned.s / float(xTiles);
	float cellT = 1.0f - (tCoordsPanned.t / float(yTiles));
	float invX = 1 / float(xTiles);
	float invY = 1 / float(yTiles);
	vec2 tC1 = vec2(cellS + (invX * f1Xpos), cellT - (invY * f1Ypos));
	vec2 tC2 = vec2(cellS + (invX * f2Xpos), cellT - (invY * f2Ypos));
	color = mix(texture2D(texture, tC1), texture2D(texture, tC2) , mixAmount) * bloomIntensity * colorIn;
	//highlight = vec4(0);
	if (color.a < 0.1f)
		discard;
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1)
        brightColor = color;
    else
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}  