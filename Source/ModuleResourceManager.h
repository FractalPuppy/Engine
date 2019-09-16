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
class ResourceAnimation;
class ResourceStateMachine;

enum class TYPE;
enum class FILETYPE;

class ModuleResourceManager :
	public Module
{
	public:
		ModuleResourceManager();
		~ModuleResourceManager();

	bool Init(JSON* config) override;
	bool CleanUp() override;

	// Shader functions
	Shader* GetProgram(std::string filename) const;
	std::list<Shader*>GetAllPrograms() const;
	void AddProgram(Shader* shader);
	void DeleteProgram(std::string filename);

	// ResourceManager functions
	unsigned FindByFileInAssets(const char* fileInAssets) const;						// Returns UID of resource by file variable 
	unsigned FindByFileInAssetsOfType(const char* fileInAssets, TYPE type) const;		// Returns UID of resource by file variable of the type given
	unsigned FindByFileInAssetsExcludingType(const char* fileInAssets, TYPE type) const;// Returns UID of resource by file variable, excludes files of the type given
	unsigned FindByExportedFile(const char* exportedFile, TYPE type) const;				// Returns UID of resource by exportedFile and type
	unsigned FindByExportedFile(const char* exportedFile) const;						// Returns UID of resource by exportedFile
	ENGINE_API unsigned FindByName(const char* name, TYPE type) const;								// Returns UID of resource by name and type
	bool ImportFile(const char* newFileInAssets, const char* filePath, TYPE type);
	bool ReImportFile(Resource* resource, const char* filePath, TYPE type);				// Imports again an already loaded resource
	unsigned GenerateNewUID();
	ENGINE_API Resource* Get(unsigned uid) const;													// Returns the resource using UID adding one to the references count and loads it to memory if not already
	Resource* Get(const char* file) const;												// Returns the resource using exportedFileName adding one to the references count and loads it to memory if not already
	Resource* Get(const char* file, TYPE type) const;									// Returns the resource using exportedFileName and type, adding one to the references count and loads it to memory if not already
	ENGINE_API Resource* GetByName(const char* name, TYPE type);									// Returns the resource using name variable.

	ENGINE_API Resource* GetWithoutLoad(unsigned uid) const;										// Returns the resource using UID and doesn't add one to the references count neither loads it to memory
	Resource* GetWithoutLoad(const char* file) const;									// Returns the resource using exportedFileName and doesn't add one to the references count neither loads it to memory
	Resource* GetWithoutLoad(const char* file, TYPE type) const;						// Returns the resource using exportedFileName and type, also doesn't add one to the references count neither loads it to memory
	Resource* CreateNewResource(TYPE type, unsigned forceUid = 0);
	ENGINE_API bool DeleteResource(unsigned uid);										// If references < 1 delete it from memory

	std::vector<Resource*> GetResourcesList();											// Get list with pointers to all resources
	std::vector<Resource*> GetResourcesList(TYPE type);									// Get list with pointers to all resources of type given
	std::vector<Resource*> GetResourcesList(bool loaded);								// Get list with pointers to all resources if they are loaded to memory
	std::vector<Resource*> GetResourcesList(TYPE type, bool loaded);					// Get list with pointers to all resources of type given and if they are loaded to memory

	ENGINE_API std::vector<std::string> GetResourceNamesList(TYPE resourceType, bool ordered);		// Returns a vector with the exportedFileName of every Resource of the type given.	

	TYPE GetResourceType(FILETYPE fileType) const;

	bool Exists(const char* exportedFile);													// Checks if a resource with the given exported filename already exist
	bool Exists(const char* exportedFile, TYPE type);										// Checks if a resource of the given type and exported filename already exist
	bool NameExists(const char* name, TYPE type);											// Checks if a resource of the given type and name already exist
	std::string GetAvailableName(const char* name, TYPE type);								// Based on the name given looks for a name that isn't already taken

	void LoadEngineResources();																// Loads resources needed by the engine (Skybox, white, no camera textures...)
	Resource* AddResource(const char* file, const char* directory, TYPE type);				// Adds a resource with the file information to the resources list
	Resource* AddResource(const char* file, const char* directory, TYPE type, unsigned uid);// Adds a resource with the file and uid information to the resources list
	Resource* AddResourceFromLibrary(const char* exportedFile, TYPE type);						// Adds a resource, by getting the info from the meta file in library, to the resources list
	Resource* ReplaceResource(unsigned oldResourceUID, Resource* newResource);	
	void DeleteResourceFromList(unsigned uid);

	unsigned GetUIDFromMeta(const char* metaFile, FILETYPE fileType) const;

	// Clean functions
	void CleanUnusedMetaFiles() const;			// Deletes all meta files that doesn't have a file assigned
	void CleanUnusedExportedFiles() const;		// Deletes all imported files that aren't registered on the Resource Manager

private:
	// Resources map (Textures, Models, Mehses, Materials, Skyboxes, Scenes...)
	std::map<unsigned, Resource*> resources;	// map<UID, pointer to resource>

	// Shaders map
	std::map<std::string, std::pair<unsigned, Shader*>> shaderResources; //filename , times used, shader
};

#endif __ModuleResourceManager_h__
