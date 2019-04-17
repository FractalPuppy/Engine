#include "Application.h"
#include "GameObject.h"

#include "ModuleFontLoader.h"
#include "ModuleProgram.h"
#include "ModuleResourceManager.h"

#include <assert.h>
#include <string>

#include "ComponentText.h"
#include "ComponentTransform2D.h"
//fonts lib
#include <ft2build.h>
#include "MathGeoLib/include/Math/float4x4.h"
#include FT_FREETYPE_H 

ModuleFontLoader::ModuleFontLoader()
{
}


ModuleFontLoader::~ModuleFontLoader()
{
}

const float ModuleFontLoader::FontScaleFactor = 0.0001;

void ModuleFontLoader::LoadFonts(const char* newFont)
{
	//check if we already have this font loaded
	std::map<std::string, std::vector<Character>>::iterator it = fonts.find(newFont);
	if (it != fonts.end())
	{
		LOG("This font was already loaded");
		return;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//float4x4 projection = glm::ortho(0.0f, static_cast<GLfloat>(WIDTH), 0.0f, static_cast<GLfloat>(HEIGHT));
	//load font
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		LOG("ERROR::FREETYPE: Could not init FreeType Library");

	FT_Face face;
	if (FT_New_Face(ft, newFont, 0, &face))
		LOG("ERROR::FREETYPE: Failed to load font");

	//set size
	//Setting the width to 0 lets the face dynamically calculate the width based on the given height.
	FT_Set_Pixel_Sizes(face, 0, 48);

	//we load the characters and put them into our map
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

	//create the map of characters of the font
	std::vector<Character> Characters(128);

	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			LOG("ERROR::FREETYTPE: Failed to load Glyph");
			continue;
		}
		// Generate texture
		GLuint textureFonts;
		glGenTextures(1, &textureFonts);
		glBindTexture(GL_TEXTURE_2D, textureFonts);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character =
		{
			textureFonts,
			float2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			float2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		//Characters.insert(std::pair<GLchar, Character>(c, character));
		Characters[c] = character;
	}
	fonts.insert(std::pair<const char*, std::vector<Character>>(newFont, Characters) );
	//when we are done processing glyphs
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &VAOText);
	glGenBuffers(1, &VBOText);
	glBindVertexArray(VAOText);

	glBindBuffer(GL_ARRAY_BUFFER, VBOText);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

bool ModuleFontLoader::Init(JSON* json)
{
	shaderFonts = App->program->GetProgram(shaderFontsFile);
	LoadFonts(defaultFont);
	LoadFonts("Fonts/MATADOR.ttf");
	LoadFonts("Fonts/TACOM.ttf");
	LoadFonts("Fonts/TACOSRG.ttf");
	return true;
}

bool ModuleFontLoader::CleanUp()
{
	if (shaderFonts != nullptr)
	{
		App->resManager->DeleteProgram(shaderFonts->file);
		shaderFonts = nullptr;
	}
	//free maps and data structures.
	return true;
}

void ModuleFontLoader::RenderText(const ComponentText& compText, int currentWidth, int currentHeight)
{
	if (shaderFonts == nullptr)return;
	const char* font = compText.font.c_str();
	assert(fonts.find(font) != fonts.end());
	std::string text = compText.text;

	//now we get the component transform 2D and the transf matrices
	ComponentTransform2D* transform2D = (ComponentTransform2D*)compText.gameobject->GetComponent(ComponentType::Transform2D);
	float x = 0;
	float y = 0;
	math::float2 pos = transform2D->getPosition();
	math::float2 size = transform2D->getSize();
	if (transform2D != nullptr)
	{
		x = (compText.offset.x + pos.x) / (size.x * compText.scaleOffset.x);
		y = (compText.offset.y + pos.y) / (size.y * compText.scaleOffset.y);
	}

	math::float4x4 model = math::float4x4::identity;
	math::float4x4 projection = math::float4x4::D3DOrthoProjRH(-1.0f, 1.0f, currentWidth, currentHeight);
	math::float3 scl = math::float3(size.x * compText.scaleOffset.x, size.y * compText.scaleOffset.y, 1.0f);
	math::float3 center = math::float3(x, y, 0.0f);
	model = model.Scale(scl, center);
	model.SetTranslatePart(center);

	// Activate corresponding render state	
	glUseProgram(shaderFonts->id[0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderFonts->id[0], "projection"), 1, GL_TRUE, (const float*)&projection);
	glUniformMatrix4fv(glGetUniformLocation(shaderFonts->id[0], "model"), 1, GL_TRUE, (const float*)&model);
	if (compText.isHovered)
	{
		glUniform4f(glGetUniformLocation(shaderFonts->id[0], "textColor"), compText.colorHovered.x, compText.colorHovered.y, compText.colorHovered.z, compText.colorHovered.w);
	}
	else
	{
		glUniform4f(glGetUniformLocation(shaderFonts->id[0], "textColor"), compText.color.x, compText.color.y, compText.color.z, compText.color.w);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAOText);

	// Iterate through all characters
	float scale = compText.fontSize*FontScaleFactor;//this scale does not change with the window size, its something else
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); ++c)
	{
		Character ch = fonts[font][static_cast<int>(*c)];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		//glUniform1i(glGetUniformLocation(shaderFonts->id, "fontTexture"), 0);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBOText);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}