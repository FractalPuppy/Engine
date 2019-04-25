#ifndef __Resource_h__
#define __Resource_h__

#include <string>

struct JSON_value;

enum class TYPE
{
	TEXTURE,
	MODEL,
	MESH,
	AUDIO,
	SCENE,
	ANIMATION,
	MATERIAL,
	SKYBOX,
	STATEMACHINE,
	UNKNOWN
};

/*	A Resource is any file with a known format dropped in /Assets/
	A copy of this will be automaticaly imported to /Library/ with 
	the engine own format and a .meta file saved next to the /Assets/ file.

	WARNING:
	- Do not delete the .meta file.
	- Do not delete the files saved in Library.
	- Deleting, moving or renaming of any file located on /Assets/ from 
	external file browsers is highly discouraged. Always use the engine
	to perform this actions.
*/

class Resource
{
public:
	Resource(unsigned uid, TYPE type);
	Resource(const Resource& resource);
	virtual ~Resource();

	virtual void Copy(const Resource& resource);	// Copies all variables from other Resource excluding UID

	inline TYPE			GetType() const					{ return type; };
	inline void			SetType(TYPE newType)			{ type = newType; }
	inline unsigned		GetUID() const					{ return UID; };
	inline const char*	GetFile() const					{ return file.c_str(); };
	inline void			SetFile(const char* newFile)	{ file = newFile; };
	inline const char*	GetExportedFile() const			{ return exportedFileName.c_str(); };
	inline void			SetExportedFile(const char* newExportedFile) { exportedFileName = newExportedFile; };
	inline bool			IsLoadedToMemory() const		{ return loaded > 0; };
	inline unsigned		GetReferences() const			{ return loaded; };
	void				SetReferences(unsigned references);
	bool				IsUsedByEngine()				{ return engineUsed; };
	void				SetUsedByEngine(bool used)		{ engineUsed = used; };

	virtual void SaveMetafile(const char* file) const;
	virtual void LoadConfigFromMeta() {};
	virtual bool LoadInMemory() {return true;};
	virtual void DeleteFromMemory() { loaded = 0; };

	// File in asset specific
	virtual void Rename(const char* newName);
	virtual void Delete();

	virtual void DrawImportConfiguration();

protected:
	unsigned UID = 0u;
	std::string file;				// The file located in /Assets/ (Path + filename + extension)
	std::string exportedFileName;	// The name of the file located in /Library/
	unsigned loaded = 0;			// Number of times this resource is being used
	bool engineUsed = false;		// True if the resource is used by default by the engine, false if it was added by the user

	TYPE type = TYPE::UNKNOWN;		// Type of resource
};

#endif __Resource_h__