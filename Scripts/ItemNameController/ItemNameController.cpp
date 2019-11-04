#include "ItemNameController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleInput.h"

#include "GameObject.h"

//#include "Component.h"
#include "ComponentImage.h"
#include "ComponentRenderer.h"
#include "ComponentText.h"
#include "ComponentTransform.h"
#include "ComponentTransform2D.h"
#include "ComponentImage.h"

#include "Item.h"

ItemNameController_API Script* CreateScript()
{
	ItemNameController* instance = new ItemNameController;
	return instance;
}

void ItemNameController::Start()
{
	uiName = App->scene->FindGameObjectByName("ItemName")->children.front();
	itemList = ItemName();

}

void ItemNameController::Update()
{

	if (itemList.show)
	{

		itemList.actualPosition = itemList.transform->GetScreenPosition();
		itemList.actualPosition += math::float2(0.0f, 30.0f);
		uiName->children.front()->GetComponent<Transform2D>()->SetPositionUsingAligment(itemList.actualPosition);
		math::float2 pos = itemList.actualPosition - math::float2((itemBackground->GetComponent<Transform2D>()->getSize().x / 2) - 10.0f, 5.0f);
		uiName->children.back()->GetComponent<Transform2D>()->SetPositionUsingAligment(pos);
		myText = uiName->children.back()->GetComponent<Text>();
		itemBackground->GetComponent<Transform2D>()->SetSize(math::float2((myText->text.size() * 10)+35 , 30));
		myText->color = color;

		if (showAfterFristFrame)
		{
			itemBackground->SetActive(true);
			itemTypeName->SetActive(true);
			uiName->SetActive(true);
		}

		showAfterFristFrame = true;
	}
}


void ItemNameController::SetNameBar(unsigned uid, ItemRarity rarity, math::float4 rarityColor)
{

	if (!itemList.show)
	{
		itemList.UID = uid;
		GameObject* go = App->scene->FindGameObjectByUID(uid);
		itemList.transform = go->transform;
		itemList.actualPosition = itemList.transform->GetScreenPosition();
		itemList.actualPosition += math::float2(0.0f, 30.0f);
		itemList.show = true;
		uiName->children.front()->GetComponent<Transform2D>()->SetPositionUsingAligment(itemList.actualPosition);
		itemBackground = (uiName->children).front();
		itemTypeName = (uiName->children).back();
		myText = itemTypeName->GetComponent<Text>();
		itemList.rarity = rarity;
		myText->text = go->GetComponent<ItemPicker>()->name;
		itemBackground->GetComponent<Transform2D>()->SetPositionUsingAligment(itemList.actualPosition);
		math::float2 pos = itemList.actualPosition - math::float2((itemBackground->GetComponent<Transform2D>()->getSize().x / 2) - 10.0f, 5.0f);
		itemTypeName->GetComponent<Transform2D>()->SetPositionUsingAligment(pos);
		color = rarityColor;
		showAfterFristFrame = false;
		return;
	}
}


void ItemNameController::Hovered(unsigned uid, bool hover)
{
	if (itemList.UID == uid)
	{
		itemList.hovered = hover;
	}
}

void ItemNameController::DisableName(unsigned uid)
{
	if (itemList.UID == uid)
	{
		itemList.show = false;
		uiName->SetActive(false);
		myText->text = "";
	}
}

bool ItemNameController::Intersection(unsigned uid)
{
	if (itemList.UID == uid)
	{
		GameObject* box = (uiName->children).front();
		return(box->GetComponent<ComponentImage>()->isHovered);
	}
	return false;
}