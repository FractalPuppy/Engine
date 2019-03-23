#include "ResourceTexture.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleTextures.h"

#include "GL/glew.h"
#include "IL/ilut.h"
#include "JSON.h"

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

ResourceTexture::ResourceTexture(unsigned uid) : Resource(uid, TYPE::TEXTURE)
{
}

ResourceTexture::ResourceTexture(const ResourceTexture& resource) : Resource(resource)
{
	width = resource.width;
	height = resource.height;
	depth = resource.depth;
	mips = resource.mips;
	bytes = resource.bytes;
	gpuID = resource.gpuID;
	format = resource.format;
	imageType = resource.imageType;
	cubemapIndex = resource.cubemapIndex;
}


ResourceTexture::~ResourceTexture()
{
	DeleteFromMemory();
}

void ResourceTexture::Copy(const Resource& resource)
{
	Resource::Copy(resource);
	if (resource.GetType() == TYPE::TEXTURE)
	{
		ResourceTexture& texture = (ResourceTexture&)resource;
		width = texture.width;
		height = texture.height;
		depth = texture.depth;
		mips = texture.mips;
		bytes = texture.bytes;
		gpuID = texture.gpuID;
		format = texture.format;
		imageType = texture.imageType;
		cubemapIndex = texture.cubemapIndex;
	}
}

bool ResourceTexture::LoadInMemory()
{
	if (Resource::IsLoadedToMemory())
		return false;

	bool success = false;

	switch (imageType)
	{
	default:
	case IMAGE_TYPE::TEXTURE:
		success = LoadTexture();
		break;
	case IMAGE_TYPE::CUBEMAP:
		success =  LoadCubemap();
		break;
	}

	// Increase references
	if (success) ++loaded;

	return success;
}

bool ResourceTexture::LoadTexture()
{
	int format = 0;
	unsigned imageID;

	char* data;
	unsigned size;
	std::string filename(exportedFileName);

	// Load image file
	if (engineUsed)
	{
		size = App->fsystem->Load((IMPORTED_RESOURCES + filename + TEXTUREEXT).c_str(), &data);
	}
	else
	{
		size = App->fsystem->Load((TEXTURES + filename + TEXTUREEXT).c_str(), &data);
	}

	if (size == 0u)
	{
		LOG("Error loading image file.");
		return false;
	}

	ilGenImages(1, &imageID); 		// Generate the image ID
	ilBindImage(imageID); 			// Bind the image
	ILboolean success = ilLoadL(IL_DDS, data, size);

	if (success)
	{
		RELEASE_ARRAY(data);
		glGenTextures(1, &gpuID);

		glBindTexture(GL_TEXTURE_2D, gpuID);

		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);
		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		{
			iluFlipImage();
		}

		ILubyte* data = ilGetData();
		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
		depth = ilGetInteger(IL_IMAGE_DEPTH);
		format = ilGetInteger(IL_IMAGE_FORMAT);
		bytes = ilGetInteger(GL_UNSIGNED_BYTE);
		mips = 0u;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		if (App->textures->filter_type == FILTERTYPE::LINEAR)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else if (App->textures->filter_type == FILTERTYPE::NEAREST)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else if (App->textures->filter_type == FILTERTYPE::NEAREST_MIPMAP_NEAREST)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
			mips = 1u;
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
			mips = 2u;
		}

		ilDeleteImages(1, &gpuID);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		ILenum error = ilGetError();
		LOG("Error loading data: %s\n", iluErrorString(error));
		return false;
	}
}

bool ResourceTexture::LoadCubemap()
{
	int format = 0;
	unsigned imageID;

	char* data;
	unsigned size;
	std::string filename(exportedFileName);

	// Load image file
	if (engineUsed)
	{
		size = App->fsystem->Load((IMPORTED_RESOURCES + filename + TEXTUREEXT).c_str(), &data);
	}
	else
	{
		size = App->fsystem->Load((TEXTURES + filename + TEXTUREEXT).c_str(), &data);
	}

	if (size == 0u)
	{
		LOG("Error loading image file.");
		return false;
	}

	ilGenImages(1, &imageID); 		// Generate the image ID
	ilBindImage(imageID); 			// Bind the image
	ILboolean success = ilLoadL(IL_DDS, data, size);

	if (success)
	{
		RELEASE_ARRAY(data);
		glGenTextures(1, &gpuID);

		glBindTexture(GL_TEXTURE_2D, gpuID);

		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);
		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT && imageType == IMAGE_TYPE::TEXTURE)
		{
			iluFlipImage();
		}

		ILubyte* data = ilGetData();
		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
		depth = ilGetInteger(IL_IMAGE_DEPTH);
		format = ilGetInteger(IL_IMAGE_FORMAT);
		bytes = ilGetInteger(GL_UNSIGNED_BYTE);
		mips = 0u;

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubemapIndex, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		ILenum error = ilGetError();
		LOG("Error loading data: %s\n", iluErrorString(error));
		return false;
	}
}

void ResourceTexture::SetImageType(IMAGE_TYPE type)
{
	imageType = type;
	if (IsLoadedToMemory())
	{
		unsigned references = loaded;
		DeleteFromMemory();
		SetReferences(references);
	}
}

void ResourceTexture::DeleteFromMemory()
{
	Resource::DeleteFromMemory();
	if (gpuID != 0)
		glDeleteTextures(1, &gpuID);
}

void ResourceTexture::SaveMetafile(const char* file) const
{
	std::string filepath;
	filepath.append(file);
	JSON* json = new JSON();
	JSON_value* meta = json->CreateValue();
	struct stat statFile;
	stat(filepath.c_str(), &statFile);
	meta->AddUint("GUID", UID);
	meta->AddUint("timeCreated", statFile.st_ctime);
	meta->AddUint("height", height);
	meta->AddUint("width", width);
	meta->AddUint("depth", depth);
	meta->AddUint("mips", mips);
	meta->AddUint("format", format);
	meta->AddUint("DX compresion", ilGetInteger(IL_DXTC_FORMAT));
	meta->AddUint("mipmap", ilGetInteger(IL_ACTIVE_MIPMAP));
	json->AddValue("Texture", *meta);
	filepath += ".meta";
	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
}

void ResourceTexture::Load(const JSON_value& config)
{
	Resource::Load(config);

	width = config.GetUint("Width");
	height = config.GetUint("Height");
	depth = config.GetUint("Depth");
	mips = config.GetUint("Mips");
	bytes = config.GetUint("Bytes");
	gpuID = config.GetUint("GpuID");

	format = config.GetUint("Format");
}