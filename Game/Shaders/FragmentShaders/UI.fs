#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D fontTexture;
uniform vec4 textColor;

void main()
{    
    vec4 FragColor = texture2D(fontTexture, TexCoords);
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(fontTexture, TexCoords).r);
    color = textColor * sampled * FragColor;
	color.a = min(FragColor.a, textColor.a);
}  