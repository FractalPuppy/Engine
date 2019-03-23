#include "ModuleFileSystem.h"
#include "FileImporter.h"

#include "Application.h"
#include "ModuleResourceManager.h"

#include "Resource.h"

#include "JSON.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "physfs.h"
#include "SDL_timer.h"
#include "SDL_atomic.h"

#include <assert.h>
#include <stack>

#define MONITORIZE_TIME 1000
#define stat _stat

ModuleFileSystem::ModuleFileSystem()
{
	PHYSFS_init(NULL);
	PHYSFS_setWriteDir("../Game");
	baseDir = PHYSFS_getWriteDir();
	PHYSFS_addToSearchPath(baseDir.c_str(), 1);
	PHYSFS_setWriteDir(baseDir.c_str());

	PHYSFS_mount(SHADERS, nullptr, 1);
	PHYSFS_mount(RESOURCES, nullptr, 1);
	PHYSFS_mount(SCRIPTS, nullptr, 1);

	// Assets Folder
	if (!Exists(ASSETS))
	{
		MakeDirectory(ASSETS);
	}
	PHYSFS_mount(ASSETS, nullptr, 1);

	// Library Folder
	if (!Exists(LIBRARY))
	{
		MakeDirectory(LIBRARY);
	}
	PHYSFS_mount(LIBRARY, nullptr, 1);

	if (!Exists(IMPORTED_MATERIALS))
		MakeDirectory(IMPORTED_MATERIALS);
	if (!Exists(MESHES))
		MakeDirectory(MESHES);
	if (!Exists(TEXTURES))
		MakeDirectory(TEXTURES);

}


ModuleFileSystem::~ModuleFileSystem()
{
	PHYSFS_deinit();
}

bool ModuleFileSystem::Start()
{
	// Check files in Assets and add them to ResManager
	CheckResourcesInFolder(ASSETS);
	if (filesToImport.size() > 0) ImportFiles();

	// Set thread to monitorize Assets folder
	monitor_thread = std::thread(&ModuleFileSystem::Monitorize, this, ASSETS);
	monitor_thread.detach();
	return true;
}

update_status ModuleFileSystem::Update(float dt)
{
	if (filesToImport.size() > 0) ImportFiles();
	return UPDATE_CONTINUE;
}

bool ModuleFileSystem::CleanUp()
{
	monitorize = false;
	while (threadIsWorking)
	{
		SDL_Delay(100);
	}
	return true;
}

unsigned ModuleFileSystem::Load(const char* file, char** buffer) const
{
	PHYSFS_file* myfile = PHYSFS_openRead(file);
	if (myfile == nullptr)
	{
		LOG("Error: %s %s", file, PHYSFS_getLastError());
		return 0;
	}
	PHYSFS_sint32 fileSize = PHYSFS_fileLength(myfile);

	*buffer = new char[fileSize+1]();
	int readed =PHYSFS_read(myfile, *buffer, 1, fileSize);
	if (readed != fileSize)
	{
		LOG("Error reading from file %s, : %s",file, PHYSFS_getLastError());
		RELEASE(*buffer);
		return 0;
	}
	PHYSFS_close(myfile);

	return readed;
}

bool ModuleFileSystem::Save(const char* file, const char* buffer, unsigned size) const
{
	PHYSFS_file* myfile = PHYSFS_openWrite(file);
	if (myfile == nullptr)
	{
		LOG("Error: %s %s", file, PHYSFS_getLastError());
		return false;
	}
	int readedObj = PHYSFS_write(myfile, buffer, size, 1);
	if (readedObj < 1)
	{
		LOG("Error: %s %s", file, PHYSFS_getLastError());
		return false;
	}

	PHYSFS_close(myfile);

	return true;
}

bool ModuleFileSystem::Remove(const char* file) const
{
	assert(file != nullptr);
	if (file == nullptr) return false;
	if (PHYSFS_unmount(file) != 0)
	{
		LOG("Error: %s", PHYSFS_getLastError());
		return false;
	}
	return true;
}

bool ModuleFileSystem::Delete(const char* file) const
{
	assert(file != nullptr);
	if (file == nullptr) return false;

	if (PHYSFS_delete(file) != 0)
	{
		LOG("Error: %s", PHYSFS_getLastError());
		return false;
	}
	return true;
}

bool ModuleFileSystem::Exists(const char* file) const
{
	assert(file != nullptr);
	if (file == nullptr) return false;
	return PHYSFS_exists(file);
}

unsigned ModuleFileSystem::Size(const char* file) const
{
	assert(file != nullptr);
	if (file == nullptr) return 0;
	PHYSFS_file* myfile = PHYSFS_openRead(file);
	PHYSFS_sint64 file_size = PHYSFS_fileLength(myfile);
	PHYSFS_close(myfile);
	return file_size;
}

bool ModuleFileSystem::MakeDirectory(const char* directory) const
{
	assert(directory != nullptr);
	if (directory == nullptr) return false;
	int result = PHYSFS_mkdir(directory);
	if (result != 0)
	{
		LOG("Error: %s",PHYSFS_getLastError());
		return false;
	}

	return true;
}

bool ModuleFileSystem::IsDirectory(const char* file) const
{
	assert(file != nullptr);
	if (file == nullptr) return false;
	return PHYSFS_isDirectory(file);
}

std::vector<std::string> ModuleFileSystem::GetFolderContent(const char* dir, bool extension) const
{
	std::vector<std::string> files;
	char **rc = PHYSFS_enumerateFiles(dir);
	char **i;

	for (i = rc; *i != NULL; i++)
	{
		if (!extension)
		{
			files.emplace_back(RemoveExtension(*i));
		}
		else
		{
			files.emplace_back(*i);
		}
	}

	PHYSFS_freeList(rc);
	return files;
}

void ModuleFileSystem::ListFolderContent(const char* dir, std::vector<std::string>& files, std::vector<std::string>& dirs) const
{
	char** filesList = PHYSFS_enumerateFiles(dir);
	char **i;

	for (i = filesList; *i != nullptr; i++)
	{
		std::string completePath = dir;
		completePath += "/";
		completePath += *i;
		if (IsDirectory(completePath.c_str()))
		{
			dirs.push_back(*i);
		}
		else
		{
			files.push_back(*i);
		}
	}

	PHYSFS_freeList(filesList);
}

void ModuleFileSystem::ListFiles(const char* dir, std::set<std::string>& files)
{
	files.clear();
	std::vector<std::string> foundFiles;
	std::stack<std::string> folderStack;
	folderStack.push(dir);
	std::string currentFolder;
	while (!folderStack.empty())
	{
		currentFolder = folderStack.top();
		folderStack.pop();

		foundFiles = GetFolderContent(currentFolder.c_str());
		for (auto& file : foundFiles)
		{
			std::string filefolder(currentFolder);
			filefolder += file;
			if (IsDirectory((currentFolder + file).c_str()))
			{
				folderStack.push(dir + file + "/");
			}
			else
			{
				files.insert(RemoveExtension(file));
			}
		}
	}
}

bool ModuleFileSystem::CopyFromOutsideFS(const char* source, const char* destination) const
{
	char *data;
	FILE* fp = fopen(source, "rb");
	if (!fp) {
		LOG("File opening failed");
		return false;
	}
	fseek(fp, 0, SEEK_END);
	unsigned size = ftell(fp);
	rewind(fp);

	data = new char[size];
	if (size != fread_s(data, size,1 , size, fp))
	{
		LOG("Error reading from file %s, : %s", source, PHYSFS_getLastError());
		RELEASE(data);
		return false;
	}
	Save(destination, data, size);
	RELEASE(data);
	return true;
}

bool ModuleFileSystem::Copy(const char* source, const char* destination, const char* file) const
{
	char * data = nullptr;
	std::string filepath(source);
	filepath += file;
	unsigned size = Load(filepath.c_str(), &data);
	std::string filedest(destination);
	filedest += file;
	Save(filedest.c_str(), data, size);
	RELEASE_ARRAY(data);

	return true;
}

void ModuleFileSystem::Monitorize(const char* folder)
{
	while (monitorize)
	{
		threadIsWorking = true;
		LookForNewResourceFiles(folder);
		threadIsWorking = false;
		SDL_Delay(MONITORIZE_TIME);
	}
}

void ModuleFileSystem::CheckResourcesInFolder(const char* folder)
{
	// Get lists with all imported resources
	std::set<std::string> importedTextures;
	std::set<std::string> importedModels;
	std::set<std::string> importedMaterials;
	ListFiles(TEXTURES, importedTextures);
	ListFiles(SCENES, importedModels);
	ListFiles(IMPORTED_MATERIALS, importedMaterials);

	// Look for files in folder passed as argument
	std::vector<std::string> files;
	std::stack<std::string> folderStack;
	folderStack.push(folder);
	std::string currentFolder;
	struct stat statFile;
	struct stat statMeta;

	while (!folderStack.empty())
	{
		currentFolder = folderStack.top();
		folderStack.pop();

		files = GetFolderContent(currentFolder.c_str());
		for (auto& file : files)
		{
			// Check if file is directory instead
			if (IsDirectory((currentFolder + file).c_str()))
			{
				folderStack.push(currentFolder + file + "/");
			}
			else
			{
				stat((currentFolder + file).c_str(), &statFile);
				stat((currentFolder + file + ".meta").c_str(), &statMeta);
				FILETYPE type = GetFileType(GetExtension(file));
				if (type == FILETYPE::TEXTURE) // PNG, TIF, LO QUE SEA	
				{
					std::set<std::string>::iterator it = importedTextures.find(RemoveExtension(file));
					if (it == importedTextures.end() || statFile.st_mtime > statMeta.st_mtime)
					{
						// File modified or not imported, send it to import
						filesToImport.push_back(std::pair<std::string, std::string>(file, currentFolder));
					}
					else
					{
						// File already imported, add it to the resources list
						App->resManager->AddResource(file.c_str(), currentFolder.c_str(), TYPE::TEXTURE);
					}
				}
				else if (type == FILETYPE::MODEL) //FBX
				{
					//TODO: Get UID from metafile 
					/*unsigned uid = GetMetaUID((current_folder + file + ".meta").c_str());
					std::set<std::string>::iterator it = importedModels.find(std::to_string(uid));
					if (it == importedModels.end() || statFile.st_mtime > statMeta.st_mtime)
					{
						filesToImport.push_back(std::pair<std::string, std::string>(file, current_folder));
					}*/
				}
				else if (type == FILETYPE::MATERIAL)
				{
					std::set<std::string>::iterator it = importedMaterials.find(RemoveExtension(file));
					if (it == importedMaterials.end() || statFile.st_mtime > statMeta.st_mtime)
					{
						// File modified or not imported, send it to import
						filesToImport.push_back(std::pair<std::string, std::string>(file, currentFolder));
					}
					else
					{
						// File already imported, add it to the resources list
						App->resManager->AddResource(file.c_str(), currentFolder.c_str(), TYPE::MATERIAL);
					}
				}
			}
		}
	}
	return;
}

void ModuleFileSystem::LookForNewResourceFiles(const char* folder)
{
	std::vector<std::string> files;
	std::stack<std::string> watchfolder;
	watchfolder.push(folder);
	std::string current_folder;
	struct stat statFile;
	struct stat statMeta;

	while (!watchfolder.empty())
	{
		current_folder = watchfolder.top();
		watchfolder.pop();

		files = GetFolderContent(current_folder.c_str());
		for (auto& file : files)
		{
			if (IsDirectory((current_folder + file).c_str()))
			{
				watchfolder.push(current_folder + file + "/");
			}
			else
			{
				if (GetExtension(file) == ".meta")
					continue;
				stat((current_folder + file).c_str(), &statFile);
				stat((current_folder + file + ".meta").c_str(), &statMeta);
				std::vector<Resource*> resources = App->resManager->GetResourcesList();
				bool found = false;
				for (std::vector<Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
				{
					if (strcmp((*it)->GetFile(), (current_folder + file).c_str()) == 0)
					{
						found = true;
						break;
					}
				}
				if (!found || statFile.st_mtime > statMeta.st_mtime)
				{
					// TODO: Enable FBX also
					FILETYPE type = GetFileType(GetExtension(file));
					if(type != FILETYPE::MODEL && type != FILETYPE::SCENE)
						filesToImport.push_back(std::pair<std::string, std::string>(file, current_folder));
				}
			}
		}
	}
}

void ModuleFileSystem::ImportFiles()
{
	for (auto & file : filesToImport)
	{
		importer.ImportAsset(file.first.c_str(), file.second.c_str());
	}
	filesToImport.clear();
}

int ModuleFileSystem::GetModTime(const char* file) const
{
	return PHYSFS_getLastModTime(file);
}

std::string ModuleFileSystem::GetExtension(std::string filename) const
{
	std::size_t found = filename.find_last_of(".");
	if (std::string::npos != found)
	{
		filename.erase(0, found);
	}
	return filename;
}

std::string ModuleFileSystem::RemoveExtension(std::string filename) const
{
	std::size_t found = filename.find_last_of(".");
	if (std::string::npos != found)
	{
		filename.erase(found, filename.size());
	}
	return filename;
}

std::string ModuleFileSystem::GetFilename(std::string filename) const
{
	std::size_t found = filename.find_last_of(".");
	if (std::string::npos != found)
	{
		filename.erase(found, filename.size());
	}

	found = filename.find_last_of(PHYSFS_getDirSeparator());
	if (std::string::npos != found)
	{
		filename.erase(0, found+1);
	}
	return filename;
}

FILETYPE ModuleFileSystem::GetFileType(std::string extension) const
{
	if (extension == PNG || extension == TIF || extension == JPG || extension == TGA)
	{
		return FILETYPE::TEXTURE;
	}
	if (extension == TEXTUREEXT)
	{
		return FILETYPE::IMPORTED_TEXTURE;
	}
	if (extension == FBXEXTENSION || extension == FBXCAPITAL)
	{
		return FILETYPE::MODEL;
	}
	if (extension == MESHEXTENSION)
	{
		return FILETYPE::IMPORTED_MESH;
	}
	if (extension == JSONEXT)
	{
		return FILETYPE::SCENE;
	}
	if (extension == MATERIALEXT)
	{
		return FILETYPE::MATERIAL;
	}
	return FILETYPE::NONE;
}