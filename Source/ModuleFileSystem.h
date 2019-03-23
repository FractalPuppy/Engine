#ifndef __ModuleFileSystem_h__
#define __ModuleFileSystem_h__

#include "Module.h"
#include "Timer.h"
#include <vector>
#include <list>
#include <set>
#include <thread>
#include "FileImporter.h"

#define ASSETS "Assets/"
#define LIBRARY "Library/"
#define RESOURCES "Resources/"
#define IMPORTED_RESOURCES RESOURCES "Imported/"
#define IMPORTED_MATERIALS LIBRARY "Materials/"
#define MATERIALS ASSETS "Materials/"
#define MESHES LIBRARY "Meshes/"
#define TEXTURES LIBRARY "Textures/"
#define SCENES ASSETS "Scenes/"
#define SHADERS "Shaders/"
#define SCRIPTS "Scripts/"
#define VERTEXSHADERS SHADERS "VertexShaders/"
#define FRAGSHADERS SHADERS "FragmentShaders/"

#define FBXEXTENSION ".fbx"
#define FBXCAPITAL ".FBX"
#define MESHEXTENSION ".m3sh"
#define PNG ".png"
#define TIF	".tif"
#define JPG	".jpg"
#define TGA ".tga"
#define TEXTUREEXT ".dds"
#define MATERIALEXT ".m4t"
#define JSONEXT ".json"
#define DLL ".dll"

#define CHECKERS "checkersTexture"
#define NOCAMERA "nocamera"

enum class FILETYPE
{
	TEXTURE,
	IMPORTED_TEXTURE,
	MODEL,
	IMPORTED_MESH,
	SCENE,
	MATERIAL,
	NONE
};

class ModuleFileSystem :
	public Module
{
public:
	ModuleFileSystem();
	~ModuleFileSystem();

	bool Start() override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	unsigned Load(const char * file, char ** buffer) const;
	bool Save(const char* file, const char* buffer, unsigned size) const;
	bool Remove(const char* file) const;
	bool Delete(const char* file) const;
	bool Exists(const char* file) const;
	unsigned Size(const char* file) const;
	bool MakeDirectory(const char* directory) const;
	bool IsDirectory(const char* file) const;
	std::vector<std::string> GetFolderContent(const char* dir, bool extension=true) const;								// Returns a vector with all the files and directories found on dir
	void ListFolderContent(const char * dir, std::vector<std::string>& files, std::vector<std::string>& dirs) const;	// Saves all files found on dir on files vector and all directories on dirs vector
	void ListFiles(const char* dir, std::set<std::string>& files);														// Saves all files found on dir and subdirs on a set
	bool CopyFromOutsideFS(const char* source, const char* destination) const;
	bool Copy(const char* source, const char* destination, const char* file) const;

	int GetModTime(const char* file) const;
	std::string GetExtension(std::string file) const;
	std::string RemoveExtension(std::string file) const;
	std::string GetFilename(std::string file) const;
	FILETYPE GetFileType(std::string) const;

private:
	// Resources Management
	void Monitorize(const char* folder);				// Monitorizes the files in the selected folder to import the new ones. 
	void CheckResourcesInFolder(const char* folder);	// Compares the resource files located in the folder to the ones already imported, these files are added to the Resource Manager and imported if not already. (Used on App start)
	void LookForNewResourceFiles(const char* folder);	// Compares the resource files located in the folder to the ones in the Resource Manager list, if a new or modfied file is found it gets imported.
	void ImportFiles();									// Imports files stored on filesToImport list

public:
	std::string baseDir = "";
	FileImporter importer;

private:
	bool monitorize = true;
	bool threadIsWorking = false;
	Timer importTimer;
	std::list<std::pair<std::string, std::string>> filesToImport; //File and folder
	std::thread monitor_thread;
};

#endif __ModuleFileSystem_h__