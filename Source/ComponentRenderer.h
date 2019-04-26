#ifndef __ComponentRenderer_h__
#define __ComponentRenderer_h__

#include "Component.h"

class ResourceMesh;
class ResourceMaterial;

class ComponentRenderer :
	public Component
{
public:
	ComponentRenderer(GameObject* gameobject);
	ComponentRenderer(const ComponentRenderer& component);
	~ComponentRenderer();


	Component* Clone() const override;
	void DrawProperties() override;
	bool CleanUp() override;

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	void SetMaterial(const char* material);
	void SetMesh(const char* meshfile);
	void UpdateGameObject();
	void LinkBones() const;

public:
	ResourceMesh* mesh = nullptr;
	ResourceMaterial* material = nullptr;
	bool castShadows = false;
	bool useAlpha = false;

private:
	std::vector<std::string> guiMaterials;
	std::vector<std::string> guiMeshes;
};

#endif __ComponentRenderer_h__