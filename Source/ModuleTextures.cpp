#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h" 

#include "GL/glew.h"
#include "IL/ilut.h"
#include "imgui.h"
#include "JSON.h"

Texture::~Texture()
{
	if (id != 0)
	{
		glDeleteTextures(1, &id);
	}
}

ModuleTextures::ModuleTextures()
{
}

// Destructor
ModuleTextures::~ModuleTextures()
{
	ilShutDown();
}

// Called before render is available
bool ModuleTextures::Init(JSON * config)
{
	LOG("Init Image library");
	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);

	JSON_value* textureConfig = config->GetValue("textures");
	if (textureConfig == nullptr) return true;

	filter_type = (FILTERTYPE)textureConfig->GetUint("filter");

	return true;
}

// Called before quitting
bool ModuleTextures::CleanUp()
{
	LOG("Freeing textures and Image library");
	return true;
}

void ModuleTextures::SaveConfig(JSON * config)
{
	JSON_value* textureConfig = config->CreateValue();

	textureConfig->AddUint("filter", (unsigned)filter_type);
	config->AddValue("textures", *textureConfig);
}

void ModuleTextures::DrawGUI()
{
	ImGui::Text("Filter type on load:");
	ImGui::RadioButton("Linear", (int*)&filter_type, (unsigned)FILTERTYPE::LINEAR);
	ImGui::RadioButton("Nearest", (int*)&filter_type, (unsigned)FILTERTYPE::NEAREST);
	ImGui::RadioButton("Nearest MipMap", (int*)&filter_type, (unsigned)FILTERTYPE::NEAREST_MIPMAP_NEAREST);
	ImGui::RadioButton("Linear MipMap", (int*)&filter_type, (unsigned)FILTERTYPE::LINEAR_MIPMAP_LINEAR);
}

Texture * ModuleTextures::GetTexture(const char * file) const
{
	assert(file != NULL);

	Texture* loadedText = App->resManager->GetTexture(file);
	if (loadedText != nullptr)
	{
		App->resManager->AddTexture(loadedText);
		return loadedText;
	}

	ILuint imageID;
	ILboolean success;
	ILenum error;
	unsigned width = 0;
	unsigned height = 0;
	unsigned pixelDepth = 0;
	int format = 0;

	char *data;
	std::string filename(file);
	unsigned size = App->fsystem->Load((TEXTURES + filename + TEXTUREEXT).c_str(), &data); 

	ilGenImages(1, &imageID); 		// Generate the image ID
	ilBindImage(imageID); 			// Bind the image
	success = ilLoadL(IL_TYPE_UNKNOWN, data, size); //Temporary!! Should use not compressed format for normal mapping!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//TODO: Resource manager load not compressed texture on normal mapping
	RELEASE_ARRAY(data);

	if (success)
	{
		GLuint textureID = 0;
		glGenTextures(1, &textureID);

		glBindTexture(GL_TEXTURE_2D, textureID);

		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);
		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		{
			iluFlipImage();
		}

		ILubyte* data = ilGetData();
		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
		pixelDepth = ilGetInteger(IL_IMAGE_DEPTH);
		format = ilGetInteger(IL_IMAGE_FORMAT);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		if (filter_type == FILTERTYPE::LINEAR)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else if(filter_type == FILTERTYPE::NEAREST)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else if (filter_type == FILTERTYPE::NEAREST_MIPMAP_NEAREST)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		ilDeleteImages(1, &imageID);

		glBindTexture(GL_TEXTURE_2D, 0);

		Texture* texture = new Texture(textureID, width, height, file);
		App->resManager->AddTexture(texture);
		return texture;
	}
	else
	{
		error = ilGetError();
		LOG("Error loading data: %s\n", iluErrorString(error));
	}
	return nullptr;
}

unsigned ModuleTextures::LoadCubeMap(const std::string faces[]) const
{
	unsigned textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	unsigned width = 0;
	unsigned height = 0;
	unsigned pixelDepth = 0;
	int format = 0;

	for (unsigned int i=0; i< NUMFACES; ++i)
	{
		ILuint imageID;
		ILboolean success;
		ILenum error;

		ilGenImages(1, &imageID); 		// Generate the image ID
		ilBindImage(imageID); 			// Bind the image

		char *data;
		unsigned size = App->fsystem->Load((TEXTURES + faces[i] + TEXTUREEXT).c_str(), &data);
		success = ilLoadL(IL_DDS, data, size);
		RELEASE_ARRAY(data);

		if (success)
		{

			ILubyte* ildata = ilGetData();
			width = ilGetInteger(IL_IMAGE_WIDTH);
			height = ilGetInteger(IL_IMAGE_HEIGHT);
			pixelDepth = ilGetInteger(IL_IMAGE_DEPTH);
			format = ilGetInteger(IL_IMAGE_FORMAT);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, format, width, height, 0, format, GL_UNSIGNED_BYTE, ildata);

			ilDeleteImages(1, &imageID);
		}
		else
		{
			error = ilGetError();
			LOG("Error file %s: %s\n", faces[i], iluErrorString(error));
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void ModuleTextures::ImportImage(const char * file, const char* folder) const
{
	ILuint imageID;
	ILboolean success;
	ILenum error;

	ilGenImages(1, &imageID); 		// Generate the image ID
	ilBindImage(imageID); 			// Bind the image
	std::string path(folder);
	success = ilLoadImage((path+file).c_str());
	if (success)
	{
		LOG("Imported image %s", file);
		ILuint size;
		ILubyte* data = ilGetData();
		ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);// To pick a specific DXT compression use
		size = ilSaveL(IL_DDS, NULL, 0);	// Get the size of the data buffer
		data = new ILubyte[size];// allocate data buffer
		if (ilSaveL(IL_DDS, data, size) > 0)
		{
			// Save to buffer with the ilSaveIL function
			std::string filepath(TEXTURES);
			filepath += App->fsystem->RemoveExtension(file);
			filepath += TEXTUREEXT;
			App->fsystem->Save(filepath.c_str(), (char*)data, size);
		}
		ilDeleteImages(1, &imageID);
		RELEASE_ARRAY(data);
	}
	else
	{
		error = ilGetError();
		LOG("Error loading file %s, error: %s\n", file, iluErrorString(error));
	}
}

