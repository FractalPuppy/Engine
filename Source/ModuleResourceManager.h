#ifndef __ModuleResourceManager_h__
#define __ModuleResourceManager_h__

#include "Module.h"

#include <map>
#include <list>
#include <vector>

struct Shader;
class Resource;
class ResourceMesh;
class ResourceMaterial;
enum class TYPE;

class ModuleResourceManager :
	public Module
{
public:
	ModuleResourceManager();
	~ModuleResourceManager();

	bool Init(JSON* config) override;
	bool Start() override;

	// Shader functions
	Shader* GetProgram(std::string filename) const;
	std::list<Shader*>GetAllPrograms() const;
	void AddProgram(Shader* shader);
	void DeleteProgram(std::string filename);

	// New ResourceManager functions
	unsigned FindByFileInAssets(const char* fileInAssets) const;		// Returns UID of resource by file variable 
	unsigned FindByExportedFile(const char* exportedFileName) const;	// Returns UID of resource by exportedFileName variable
	unsigned ImportFile(const char* newFileInAssets, const char* filePath, TYPE type, bool force = false);
	unsigned GenerateNewUID();
	Resource* Get(unsigned uid) const;									// Returns the resource using UID adding one to the references count and loads it to memory if not already
	Resource* Get(const char* file) const;								// Returns the resource using exportedFileName adding one to the references count and loads it to memory if not already
	Resource* GetWithoutLoad(unsigned uid) const;						// Returns the resource using UID and doesn't add one to the references count neither loads it to memory
	Resource* GetWithoutLoad(const char* file) const;					// Returns the resource using exportedFileName and doesn't add one to the references count neither loads it to memory
	Resource* CreateNewResource(TYPE type, unsigned forceUid = 0);
	bool DeleteResource(unsigned uid);									// If references < 1 delete it from memory

	std::vector<Resource*> GetResourcesList();
	void LoadEngineResources();											// Loads resources needed by the engine (Skybox, white, no camera textures...)
	Resource* AddResource(const char* file, const char* directory, TYPE type);
	void DeleteResourceFromList(unsigned uid);

private:
	// Resources map (Textures, Mehses, Materials, Skyboxes...)
	std::map<unsigned, Resource*> resources;	// map<UID, pointer to resource>

	// Shaders map
	std::map<std::string, std::pair<unsigned, Shader*>> shaderResources; //filename , times used, shader
};

#endif __ModuleResourceManager_h__
