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

	void SetNameBar(unsigned uid, ItemRarity rarity, math::float4 color);
	void Hovered(unsigned uid, bool hover);
	void DisableName(unsigned uid);
	bool Intersection(unsigned uid);

private:
	GameObject* uiName;
	GameObject* itemBackground;
	GameObject* itemTypeName = nullptr;

	ComponentRenderer* myRender = nullptr;
	Text* myText = nullptr;

	ItemName itemList;

	math::float4 color;

	bool showAfterFristFrame = false;

};

extern "C" ItemNameController_API Script* CreateScript();

#endif __ItemNameController_h__
