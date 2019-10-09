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
		itemBackground->GetComponent<Transform2D>()->SetSize(math::float2((myText->text.size() * 9.5)+45 , 30));
		switch (itemList.rarity)
		{
		case ItemRarity::BASIC:
			if (itemList.hovered)
			{
				myText->color = white / 255;
			}
			else
			{
				myText->color = grey / 255;
			}
			break;
		case ItemRarity::RARE:
			if (itemList.hovered)
			{
				myText->color = green / 255;
			}
			else
			{
				myText->color = darkGreen / 255;
			}
			break;
		case ItemRarity::EPIC:
			if (itemList.hovered)
			{
				myText->color = orange / 255;
			}
			else
			{
				myText->color = darkOrange / 255;
			}
			break;
		case ItemRarity::LEGENDARY:
			if (itemList.hovered)
			{
				myText->color = purple / 255;
			}
			else
			{
				myText->color = darkPurple / 255;
			}
			break;
		}
		color = myText->color;
	}
}


void ItemNameController::SetNameBar(unsigned uid, ItemRarity rarity)
{

	if (!itemList.show)
	{
		itemList.UID = uid;
		GameObject* go = App->scene->FindGameObjectByUID(uid);
		itemList.transform = go->transform;
		itemList.actualPosition = go->transform->GetScreenPosition();
		itemList.show = true;
		itemList.distanceNormalized = (math::float2(itemList.actualPosition.x, itemList.actualPosition.y + 100) - itemList.actualPosition).Normalized();
		uiName->SetActive(true);
		itemBackground = (uiName->children).front();
		itemTypeName = (uiName->children).back();
		myText = itemTypeName->GetComponent<Text>();
		itemList.rarity = rarity;
		myText->text = go->GetComponent<ItemPicker>()->name;
		itemBackground->GetComponent<Transform2D>()->SetPositionUsingAligment(itemList.actualPosition);
		math::float2 textPos = itemList.actualPosition + math::float2(60, 0);
		itemTypeName->GetComponent<Transform2D>()->SetPositionUsingAligment(textPos);
		itemBackground->SetActive(true);
		itemTypeName->SetActive(true);
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

math::float4 ItemNameController::GetColor(unsigned uid)
{

	if (itemList.UID == uid)
	{
		return color;
	}
	return math::float4(0, 0, 0, 0);
}