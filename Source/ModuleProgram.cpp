#include "Application.h"

#include "ModuleProgram.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"

#include "GL/glew.h"
#include <assert.h>

Shader::~Shader()
{
	if (id != 0)
	{
		glDeleteProgram(id);
	}
}

ModuleProgram::ModuleProgram()
{
}


ModuleProgram::~ModuleProgram()
{
}

bool ModuleProgram::Init(JSON * config)
{
	defaultShader = CreateProgram(DEFAULTPROGRAM);
	return true;
}



Shader* ModuleProgram::CreateProgram(const char * name)
{
	assert(name != nullptr);
	if (name == nullptr) return nullptr;

	unsigned int vertexShader = CreateVertexShader(name);
	unsigned int fragmentShader = CreateFragmentShader(name);

	unsigned int program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	int  success;
	char infoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		LOG("ERROR::PROGRAM::CREATION_FAILED\n");
		LOG("ERROR: %s\n", infoLog);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	Shader* shader = new Shader(program, name);
	App->resManager->AddProgram(shader);
	return shader;
}

Shader* ModuleProgram::GetProgram(const char * name)
{
	Shader* shader = App->resManager->GetProgram(name);
	if (shader != nullptr)
	{
		App->resManager->AddProgram(shader); //Add to number of usages
		return shader;
	}

	return CreateProgram(name);
}

unsigned ModuleProgram::CreateVertexShader(const char *name)
{
	assert(name != NULL);
	std::string file(name);
	file += ".vs";
	char * vertexShaderSource = nullptr;
	App->fsystem->Load((VERTEXSHADERS + file).c_str(), &vertexShaderSource);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	RELEASE_ARRAY(vertexShaderSource);

	ShaderLog(vertexShader, "VERTEX");
	return vertexShader;
}

unsigned ModuleProgram::CreateFragmentShader(const char *name)
{
	assert(name != NULL);
	std::string file(name);
	file += ".fs";
	char * fragmentShaderSource = nullptr;
	App->fsystem->Load((FRAGSHADERS+file).c_str(), &fragmentShaderSource);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	RELEASE_ARRAY(fragmentShaderSource);

	ShaderLog(fragmentShader, "FRAGMENT");
	return fragmentShader;
}

void ModuleProgram::ShaderLog(unsigned int shader, char * type) const
{
	assert(type != NULL);
	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		LOG("ERROR::SHADER::%s::COMPILATION_FAILED\n", type);
		LOG("ERROR: %s\n", infoLog);
	}
}

bool ModuleProgram::CleanUp()
{
	if (defaultShader != nullptr)
	{
		App->resManager->DeleteProgram(defaultShader->file);
		defaultShader == nullptr;
	}
	return true;
}
