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
	std::list<GameObject*> list = App->scene->FindGameObjectByName("ItemName")->children;

	int k = 0;
	for (GameObject* const &i : list) {
		uiName[k++] = i;
	}

	for (int i = 0; i != 2; i++) {
		itemList[i] = ItemName();
	}
}

void ItemNameController::Update()
{
	for (int i = 0; i != 10; ++i) {
		if (itemList[i].show)
		{
			itemList[i].actualPosition = itemList[i].transform->GetScreenPosition();
			itemList[i].actualPosition += math::float2(0.0f, 30.0f);
			uiName[i]->children.front()->GetComponent<Transform2D>()->SetPositionUsingAligment(itemList[i].actualPosition);
			math::float2 pos = itemList[i].actualPosition - math::float2((itemBackground->GetComponent<Transform2D>()->getSize().x / 2) - 10.0f, 5.0f);
			uiName[i]->children.back()->GetComponent<Transform2D>()->SetPositionUsingAligment(pos);
			myText = uiName[i]->children.back()->GetComponent<Text>();
			switch (itemList[i].rarity)
			{
			case ItemRarity::BASIC:
				if (itemList[i].hovered)
				{
					myText->color = white / 255;
				}
				else
				{
					myText->color = grey / 255;
				}
				break;
			case ItemRarity::RARE:
				if (itemList[i].hovered)
				{
					myText->color = green / 255;
				}
				else
				{
					myText->color = darkGreen / 255;
				}
				break;
			case ItemRarity::EPIC:
				if (itemList[i].hovered)
				{
					myText->color = orange / 255;
				}
				else
				{
					myText->color = darkOrange / 255;
				}
				break;
			case ItemRarity::LEGENDARY:
				if (itemList[i].hovered)
				{
					myText->color = purple / 255;
				}
				else
				{
					myText->color = darkPurple / 255;
				}
				break;
			}
			color[i] = myText->color;
		}
	}
}

void ItemNameController::SetNameBar(unsigned uid, ItemRarity rarity)
{
	for (int i = 0; i != 10; i++) 
	{
		if (!itemList[i].show)
		{
			itemList[i].UID = uid;
			GameObject* go = App->scene->FindGameObjectByUID(uid);
			itemList[i].transform = go->transform;
			itemList[i].actualPosition = go->transform->GetScreenPosition();
			itemList[i].show = true;
			itemList[i].distanceNormalized = (math::float2(itemList[i].actualPosition.x, itemList[i].actualPosition.y + 100) - itemList[i].actualPosition).Normalized();
			uiName[i]->SetActive(true);
			itemBackground = (uiName[i]->children).front();
			itemTypeName = (uiName[i]->children).back();
			myText = itemTypeName->GetComponent<Text>();
			itemList[i].rarity = rarity;
			myText->text = go->name;
			itemBackground->GetComponent<Transform2D>()->SetPositionUsingAligment(itemList[i].actualPosition);
			math::float2 textPos = itemList[i].actualPosition + math::float2(60, 0);
			itemTypeName->GetComponent<Transform2D>()->SetPositionUsingAligment(textPos);
			itemBackground->SetActive(true);
			itemTypeName->SetActive(true);
			return;
		}
	}
}

void ItemNameController::Hovered(unsigned uid, bool hover)
{
	for (int i = 0; i<10; i++)
	{
		if (itemList[i].UID == uid)
		{
			itemList[i].hovered = hover;
		}
	}
}

void ItemNameController::DisableName(unsigned uid)
{
	for (int i = 0; i<10; i++)
	{
		if (itemList[i].UID == uid)
		{
			itemList[i].show = false;
			uiName[i]->SetActive(false);
		}
	}
}

bool ItemNameController::Intersection(unsigned uid)
{
	for (int i = 0; i < 10; i++)
	{
		if (itemList[i].UID == uid)
		{
			GameObject* box = (uiName[i]->children).front();
			return(box->GetComponent<ComponentImage>()->isHovered);
		}
	}
	return false;
}

math::float4 ItemNameController::GetColor(unsigned uid)
{
	for (int i = 0; i < 10; i++)
	{
		if (itemList[i].UID == uid)
		{
			return color[i];
		}
	}
	return math::float4(0, 0, 0, 0);
}