#ifndef __ResourceTexture_h__
#define __ResourceTexture_h__

#include "Resource.h"

enum class IMAGE_TYPE {TEXTURE, CUBEMAP};

enum class DXT
{
	DXT1 = 0x0706,				// Working
	//DXT2 = 0x0707,			// Error
	DXT3 = 0x0708,				// Working
	//DXT4 = 0x0709,			// Error
	DXT5 = 0x070A,				// Working
	//DXT_NO_COMP = 0x070B,		// Error
	//KEEP_DXTC_DATA = 0x070C,	// Error
	//DXTC_DATA_FORMAT = 0x070D,// Error
	THREE_DC = 0x070E,			// Working
	RXGB = 0x070F,				// Working
	ATI1N = 0x0710,				// Working
	DXT1A = 0x0711,				// Working
	NO_COMPRESSION = 0			// Our implementation
};

class ResourceTexture : public Resource
{
public:
	ResourceTexture(unsigned uid);
	ResourceTexture(const ResourceTexture& resource);
	virtual ~ResourceTexture();

	void Copy(const Resource& resource) override;

	bool LoadInMemory() override;
	void DeleteFromMemory() override;
	void SaveMetafile(const char* file) const override;
	void LoadConfigFromMeta() override;
	void LoadConfigFromLibraryMeta() override;

	void DrawImportConfiguration() override;

	inline IMAGE_TYPE GetImageType() { return imageType; };
	void SetImageType(IMAGE_TYPE type);
	inline unsigned GetCubemapIndex() { return cubemapIndex; };
	inline void SetCubemapIndex(unsigned index) { cubemapIndex = index; };

private:
	bool LoadTexture();
	bool LoadCubemap();

public:
	unsigned width = 0u;
	unsigned height = 0u;
	unsigned depth = 0u;
	unsigned mips = 0u;			// Type of mipmaps (None = 0, Nearest = 1, Linear = 2)
	unsigned bytes = 0u;
	unsigned gpuID = 0u;
	unsigned format = 0u;

	// Import configuration
	DXT dxtFormat = DXT::DXT5;	// Compression type
	int compression = 3;		// Compression enum number

private:
	IMAGE_TYPE imageType = IMAGE_TYPE::TEXTURE;
	unsigned cubemapIndex = 0u;	// If the image is a cubemap this variable will tell the index ( 0=right, 1=left, 2=top, 3=bottom, 4=front, 5=back)
};

#endif __ResourceTexture_h__