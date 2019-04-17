#ifndef __ModuleFontLoader_H__
#define __ModuleFontLoader_H__

#include "Module.h"
#include "GL/glew.h"
#include "Math/float2.h"
#include "Math/float4.h"
#include <map>
#include <list>

class ComponentText;
struct Shader;
class ModuleFontLoader :
	public Module
{
public:

	//font structures
	struct Character {
		GLuint		TextureID;  // ID handle of the glyph texture
		float2		Size;       // Size of glyph
		float2		Bearing;    // Offset from baseline to left/top of glyph
		GLuint		Advance;    // Offset to advance to next glyph
	};

	ModuleFontLoader();
	~ModuleFontLoader();
	bool Init(JSON* json) override;
	bool CleanUp() override;
	void LoadFonts(const char* newFont);
	void RenderText(const ComponentText& compText, int currentWidth, int currentHeight);
public:
	std::map<std::string, std::vector<Character>> fonts;
	const char* defaultFont = "Fonts/DroidSans.ttf";
	
private:
	static const float FontScaleFactor;
	Shader* shaderFonts = nullptr;
	const char* shaderFontsFile = "Fonts";
	unsigned VAOText = 0;
	unsigned VBOText = 0;
};

#endif // __ModuleFontLoader_H__