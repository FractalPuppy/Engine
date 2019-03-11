#ifndef __ModuleProgram_h__
#define __ModuleProgram_h__

#include "Module.h"
#include <string>
#define DEFAULTPROGRAM "Default"

struct Shader
{
	unsigned id = 0;
	std::string file;
	Shader(unsigned program, std::string file) : id(program), file(file) {}
	~Shader();
};

class ModuleProgram : public Module
{
public:
	ModuleProgram();
	~ModuleProgram();

	bool Init(JSON * config) override;
	bool CleanUp() override;

	Shader* CreateProgram(const char *name);
	Shader* GetProgram(const char * name); //Creates new one if it doesn't exist

private:
	void ShaderLog(unsigned int shader, char* type) const;
	unsigned CreateVertexShader(const char *name);
	unsigned CreateFragmentShader(const char *name);

public:
	Shader * defaultShader = nullptr; //Deallocated in resourcemanager

};
#endif /* __ModuleProgram_h__ */
