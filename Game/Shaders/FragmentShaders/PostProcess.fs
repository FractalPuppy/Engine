
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 hlight;

uniform sampler2D gColor;
uniform sampler2D gHighlight;
uniform sampler2D gBrightness;
uniform sampler2D gDepth;

uniform float gammaCorrector;
uniform float exposure;

uniform float fogFalloff;
uniform float fogQuadratic;
uniform float maxFog;
uniform vec3 fogColor;
uniform float zNear;
uniform float zFar;

in vec2 UV0;

vec4 ProcessHighlights(vec4 hColorIn)
{
	vec4 hColor = hColorIn;
	vec2 offset = 1.0 / textureSize(gHighlight, 0);
	vec4 N = texture2D(gHighlight, vec2(UV0.x , UV0.y + offset.y));
	vec4 NE = texture2D(gHighlight, vec2(UV0.x + offset.x, UV0.y + offset.y));
	vec4 NO = texture2D(gHighlight, vec2(UV0.x - offset.x, UV0.y + offset.y));
	vec4 S = texture2D(gHighlight, vec2(UV0.x , UV0.y - offset.y));
	vec4 SE = texture2D(gHighlight, vec2(UV0.x + offset.x , UV0.y - offset));
	vec4 SO = texture2D(gHighlight, vec2(UV0.x - offset.x , UV0.y - offset.y));
	vec4 E = texture2D(gHighlight, vec2(UV0.x + offset.x, UV0.y));
	vec4 O = texture2D(gHighlight, vec2(UV0.x - offset.x, UV0.y));
	vec4 C = texture2D(gHighlight, UV0);
	
	hlight = C;

	hColor = E - C;
	hColor = hColor + O - C;
	hColor = hColor + N - C;
	hColor = hColor + NE - C;
	hColor = hColor + NO - C;
	hColor = hColor + S - C;
	hColor = hColor + SE - C;
	hColor = hColor + SO - C;
	hColor.a = 1;
	
	hColor.r = max(hColor.r, 0.0f);
	hColor.g = max(hColor.g, 0.0f);
	hColor.b = max(hColor.b, 0.0f);
	hColor.a = max(hColor.a, 0.0f);
	
	float lC = length(hColor.rgb);	
	return (1 - lC) * hColorIn + lC * hColor;
}

vec4 GetTexel(in vec2 uv) //MSAA
{
	ivec2 vp = textureSize(gColor, 0);
	vp = ivec2(vec2(vp)*uv);
	vec4 sample1 = texelFetch(gColor, vp, 0);
	vec4 sample2 = texelFetch(gColor, vp, 1);
	vec4 sample3 = texelFetch(gColor, vp, 2);
	vec4 sample4 = texelFetch(gColor, vp, 3);
	return (sample1 + sample2 + sample3 + sample4) / 4.0f;
}

void main()
{
#ifdef FOG
	float depth = max(texture2D(gDepth, UV0).x, 0.0001f);

    depth = (2.0f * zNear) / (zFar + zNear - depth * (zFar - zNear));
#endif
	color = GetTexel(UV0);	

	vec3 bloomColor = texture2D(gBrightness, UV0).rgb;
	
	color += vec4(bloomColor, 1);
#ifdef FOG		
	float fogAmount = min(fogFalloff * depth + fogQuadratic * pow(depth, 6), maxFog);

	color = color + vec4(vec3(fogAmount, fogAmount, fogAmount) * fogColor, 0.f);	
#endif
	vec4 mapped = vec4(1.0) - exp(-color * exposure); //Tone mapping
	
	color = pow(mapped, vec4(1.0 / gammaCorrector)); // gamma correction
	
	color = ProcessHighlights(color);  //Draw highlights
	

	//color.rgb = texture2D(gBrightness, UV0).rgb;

}
