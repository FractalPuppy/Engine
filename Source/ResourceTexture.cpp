#include "ResourceTexture.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleTextures.h"
#include "ModuleResourceManager.h"

#include "GL/glew.h"
#include "IL/ilut.h"
#include "JSON.h"
#include "imgui.h"

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

	// Load image file
	if (engineUsed)
	{
		size = App->fsystem->Load(exportedFile.c_str(), &data);
	}
	else
	{
		size = App->fsystem->Load(exportedFile.c_str(), &data);
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		ilDeleteImages(1, &imageID);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		ILenum error = ilGetError();
		LOG("Error loading data: %s\n", iluErrorString(error));
		return false;
	}
	return true;
}

bool ResourceTexture::LoadCubemap()
{
	int format = 0;
	unsigned imageID;

	char* data;
	unsigned size;

	// Load image file
	if (engineUsed)
	{
		size = App->fsystem->Load(exportedFile.c_str(), &data);
	}
	else
	{
		size = App->fsystem->Load(exportedFile.c_str(), &data);
	}

	if (size == 0u)
	{
		LOG("Error loading image file.");
		RELEASE_ARRAY(data);
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
		ilDeleteImages(1, &imageID);
	}
	else
	{
		ILenum error = ilGetError();
		LOG("Error loading data: %s\n", iluErrorString(error));
		return false;
	}
	return true;
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
	filepath += METAEXT;
	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
}

void ResourceTexture::LoadConfigFromMeta()
{
	std::string metaFile(file);
	metaFile += ".meta";

	// Check if meta file exists
	if (!App->fsystem->Exists(metaFile.c_str()))
		return;

	char* data = nullptr;
	unsigned oldUID = GetUID();

	if (App->fsystem->Load(metaFile.c_str(), &data) == 0)
	{
		LOG("Warning: %s couldn't be loaded", metaFile.c_str());
		RELEASE_ARRAY(data);
		return;
	}
	JSON* json = new JSON(data);
	JSON_value* value = json->GetValue("Texture");

	// Make sure the UID from meta is the same
	unsigned checkUID = value->GetUint("GUID");
	if (oldUID != checkUID)
	{
		UID = checkUID;
		// Update resource UID on resource list
		App->resManager->ReplaceResource(oldUID, this);
		exportedFile = TEXTURES + std::to_string(UID) + TEXTUREEXT;
	}

	dxtFormat = (DXT)value->GetInt("DX compresion");

	switch (dxtFormat)
	{
	case DXT::DXT1:	compression = 0; break;
	//case DXT::DXT2:	compression = 1; break;
	case DXT::DXT3:	compression = 1; break;
	//case DXT::DXT4:	compression = 3; break;
	case DXT::DXT5:	compression = 2; break;
	//case DXT::DXT_NO_COMP:	compression = 5; break;
	//case DXT::KEEP_DXTC_DATA:	compression = 3; break;
	//case DXT::DXTC_DATA_FORMAT:	compression = 4; break;
	case DXT::THREE_DC:	compression = 3; break;
	case DXT::RXGB:	compression = 4; break;
	case DXT::ATI1N:	compression = 5; break;
	case DXT::DXT1A:	compression = 6; break;
	}
}

void ResourceTexture::DrawImportConfiguration()
{
	const char* compressionTypes[] = { "DXT1", /*"DXT2",*/ "DXT3", /*"DXT4",*/ "DXT5", /*"DXT_NO_COMP", "KEEP_DXTC_DATA", "DXTC_DATA_FORMAT",*/ "THREE_DC", "RXGB", "ATI1N", "DXT1A" };
	if (ImGui::Combo("Compression type", &compression, compressionTypes, IM_ARRAYSIZE(compressionTypes)))
	{
		switch (compression)
		{
		case 0:	dxtFormat = DXT::DXT1; break;
			//case 1:	resource->dxtFormat = DXT::DXT2; break;
		case 1:	dxtFormat = DXT::DXT3; break;
			//case 3:	resource->dxtFormat = DXT::DXT4; break;
		case 2:	dxtFormat = DXT::DXT5; break;
			//case 5: resource->dxtFormat = DXT::DXT_NO_COMP; break;
			//case 3:	resource->dxtFormat = DXT::KEEP_DXTC_DATA; break;
			//case 4:	resource->dxtFormat = DXT::DXTC_DATA_FORMAT; break;
		case 3:	dxtFormat = DXT::THREE_DC; break;
		case 4:	dxtFormat = DXT::RXGB; break;
		case 5:	dxtFormat = DXT::ATI1N; break;
		case 6:	dxtFormat = DXT::DXT1A; break;
		}
	}
}