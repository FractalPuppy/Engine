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
	void UpdateMesh(const char * meshData, unsigned uid);
	void UpdateGameObject();

public:
	ResourceMesh* mesh = nullptr;
	ResourceMaterial* material = nullptr;

private:
	std::vector<std::string> guiMaterials;
};

#endif __ComponentRenderer_h__