#ifndef __Skybox_h__
#define __Skybox_h__

#include "Geometry/Frustum.h"

struct Shader;

class Skybox
{
public:
	Skybox();
	~Skybox();
	void Draw(const Frustum& frustum) const;

public:
	bool enabled = true;
	
private:
	Shader* shader = nullptr; //Deallocated in ResManager
	unsigned skybox_cubemap = 0;
	unsigned skyboxVAO = 0; 
	unsigned skyboxVBO = 0;
};

#endif __Skybox_h__
