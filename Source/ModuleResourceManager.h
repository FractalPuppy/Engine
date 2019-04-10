#ifndef __ModuleResourceManager_h__
#define __ModuleResourceManager_h__

#include "Module.h"

#include <map>
#include <list>
#include <vector>

struct Shader;
class Resource;
class ResourceTexture;
class ResourceMaterial;
class ResourceMesh;

enum class TYPE;

class ModuleResourceManager :
	public Module
{
public:
	ModuleResourceManager();
	~ModuleResourceManager();

	bool Init(JSON* config) override;

	// Shader functions
	Shader* GetProgram(std::string filename) const;
	std::list<Shader*>GetAllPrograms() const;
	void AddProgram(Shader* shader);
	void DeleteProgram(std::string filename);

	// ResourceManager functions
	unsigned FindByFileInAssets(const char* fileInAssets) const;						// Returns UID of resource by file variable 
	unsigned FindByFileInAssetsExcludingType(const char* fileInAssets, TYPE type) const;// Returns UID of resource by file variable, excludes files of the type given
	unsigned FindByExportedFile(const char* exportedFileName, TYPE type) const;			// Returns UID of resource by exportedFileName and type
	unsigned FindByExportedFile(const char* exportedFileName) const;					// Returns UID of resource by exportedFileName 
	bool ImportFile(const char* newFileInAssets, const char* filePath, TYPE type);
	bool ReImportFile(Resource* resource, const char* filePath, TYPE type);				// Imports again an already loaded resource
	unsigned GenerateNewUID();
	Resource* Get(unsigned uid) const;													// Returns the resource using UID adding one to the references count and loads it to memory if not already
	Resource* Get(const char* file) const;												// Returns the resource using exportedFileName adding one to the references count and loads it to memory if not already
	Resource* Get(const char* file, TYPE type) const;									// Returns the resource using exportedFileName and type, adding one to the references count and loads it to memory if not already
	ResourceMesh* GetMeshByName(const char* name);										// To be deprecated: returns mesh by name.
	Resource* GetWithoutLoad(unsigned uid) const;										// Returns the resource using UID and doesn't add one to the references count neither loads it to memory
	Resource* GetWithoutLoad(const char* file) const;									// Returns the resource using exportedFileName and doesn't add one to the references count neither loads it to memory
	Resource* GetWithoutLoad(const char* file, TYPE type) const;						// Returns the resource using exportedFileName and type, also doesn't add one to the references count neither loads it to memory
	Resource* CreateNewResource(TYPE type, unsigned forceUid = 0);
	bool DeleteResource(unsigned uid);													// If references < 1 delete it from memory

	std::vector<Resource*> GetResourcesList();
	std::vector<ResourceTexture*> GetTexturesList();
	std::vector<ResourceMaterial*> GetMaterialsList();
	std::vector<std::string> GetResourceNamesList(TYPE resourceType, bool ordered); // Returns a vector with the exportedFileName of every Resource of the type given.	
	std::vector<std::string> GetMeshesNamesList(bool ordered);						// To be deprecated: Returns a vector with the name of every mesh orederer or not.		

	bool Exists(const char* exportedFileName);										// Checks if a resource with the given exported filename already exist
	bool Exists(const char* exportedFileName, TYPE type);							// Checks if a resource of the given type and exported filename already exist

	void LoadEngineResources();														// Loads resources needed by the engine (Skybox, white, no camera textures...)
	Resource* AddResource(const char* file, const char* directory, TYPE type);
	Resource* ReplaceResource(unsigned oldResourceUID, Resource* newResource);	
	void DeleteResourceFromList(unsigned uid);

private:
	// Resources map (Textures, Mehses, Materials, Skyboxes...)
	std::map<unsigned, Resource*> resources;	// map<UID, pointer to resource>

	// Shaders map
	std::map<std::string, std::pair<unsigned, Shader*>> shaderResources; //filename , times used, shader
};

#endif __ModuleResourceManager_h__
