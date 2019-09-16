#ifndef  __ItemNameController_h__
#define  __ItemNameController_h__

#include "BaseScript.h"
#include "ItemPicker.h"
#include "ItemName.h"
#include "Math\float4.h"

#ifdef ItemNameController_EXPORTS
#define ItemNameController_API __declspec(dllexport)
#else
#define ItemNameController_API __declspec(dllimport)
#endif
class GameObject;
class ComponentRenderer;
class ComponentTransform;
class ComponentImage;
class Text;
class ItemPicker;
class ItemName;

class ItemNameController_API ItemNameController : public Script
{
public:
	void Start() override;
	void Update() override;

	void SetNameBar(unsigned uid, ItemRarity rarity);
	void Hovered(unsigned uid, bool hover);
	void DisableName(unsigned uid);
	bool Intersection(unsigned uid);
	float4 GetColor(unsigned uid);

private:
	GameObject* itemName = nullptr;
	GameObject* uiName[10];
	GameObject* itemBackground = nullptr;
	GameObject* itemTypeName = nullptr;

	math::float4 grey = math::float4(147.0f, 147.0f, 147.0f, 255.0f);
	math::float4 white = math::float4(255.0f, 255.0f, 255.0f, 255.0f);
	math::float4 darkGreen = math::float4(0.0f, 100.0f, 0.0f, 255.0f);
	math::float4 green = math::float4(0.0f, 255.0f, 0.0f, 255.0f);
	math::float4 orange = math::float4(255.0f, 165.0f, 0.0f, 255.0f);
	math::float4 darkOrange = math::float4(255.0f, 140.0f, 0.0f, 255.0f);
	math::float4 purple = math::float4(255.0f, 0.0f, 255.0f, 255.0f);
	math::float4 darkPurple = math::float4(139.0f, 0.0f, 139.0f, 255.0f);

	ComponentRenderer* myRender = nullptr;
	Text* myText = nullptr;

	ItemName itemList[10];

	math::float4 color[10];

};

extern "C" ItemNameController_API Script* CreateScript();

#endif __ItemNameController_h__
