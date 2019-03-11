#ifndef __MaterialEditor_h__
#define __MaterialEditor_h__
#define None "None Selected"

class Material;

class MaterialEditor
{
public:
	MaterialEditor();
	~MaterialEditor();

	void Draw();
	void ShaderSelector(std::string &current_shader);
	void TextureSelector(unsigned i, std::string &current_texture, int id);
	void SetCurrentTextures();	// Sets current textures strings with the corresponding texture file string
	void NewMaterial();
	bool Exists(const std::string& material) const;
	void Save();
	void CleanUp();

public:
	bool open = false;

	char newNamePrev[64] = "";
	char newName[64] = "New Material";
	bool newMaterial = false;
	bool newMatExists = false;

	Material* material = nullptr;
	Material* previous = nullptr;

	std::string currentShader = None;
	std::string currentDiffuse = None;
	std::string currentSpecular = None;
	std::string currentOcclusion = None;
	std::string currentEmissive = None;
	std::string currentNormal = None;

	std::vector<std::string> textureFiles;
	std::vector<std::string> shaders;
};

#endif __MaterialEditor_h__
