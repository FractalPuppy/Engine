#ifndef __ModuleTextures_h__
#define __ModuleTextures_h__

#include "Module.h"
#include <vector>
#include <string>

class ResourceTexture;

enum class FILTERTYPE
{
	LINEAR,
	NEAREST,
	NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_LINEAR
};

class ModuleTextures : public Module
{
public:
	ModuleTextures();
	~ModuleTextures();

	bool Init(JSON * config) override;
	bool CleanUp() override;
	void SaveConfig(JSON * config) override;

	void DrawGUI() override;
	bool ImportImage(const char* file, const char* folder, ResourceTexture* resource) const;

public:
	FILTERTYPE filter_type = FILTERTYPE::LINEAR;
};

#endif /* __ModuleTextures_h__ */