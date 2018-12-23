#ifndef __ModuleProgram_h__
#define __ModuleProgram_h__

#include "Module.h"
#include <string>

struct Shader
{
	unsigned id;
	std::string file;
	Shader(unsigned program, std::string file) : id(program), file(file) {}
};

class ModuleProgram : public Module
{
public:
	ModuleProgram();
	~ModuleProgram();

	bool Init();
	bool CleanUp();

	Shader* CreateProgram(const char *name);
	Shader* GetProgram(const char * name); //creates new one if it doesn't exist

private:
	void ShaderLog(unsigned int shader, char* type) const;
	unsigned CreateVertexShader(const char *name);
	unsigned CreateFragmentShader(const char *name);

public:

	Shader * defaultShader = nullptr; //Deallocated memory in resourcemanager

};

#endif /* __ModuleProgram_h__ */
