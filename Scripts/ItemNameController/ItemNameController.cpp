#include "ItemNameController.h"

#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"

//#include "Component.h"
#include "ComponentImage.h"
#include "ComponentRenderer.h"
#include "ComponentText.h"
#include "ComponentTransform.h"
#include "ComponentTransform2D.h"

ItemNameController_API Script* CreateScript()
{
	ItemNameController* instance = new ItemNameController;
	return instance;
}

void ItemNameController::Start()
{
	itemName = App->scene->FindGameObjectByName("ItemName");
	itemBackground = App->scene->FindGameObjectByName("ItemBackground", itemName);
	itemTypeName = App->scene->FindGameObjectByName("ItemTypeName", itemName);
	myText = itemTypeName->GetComponent<Text>();
}

void ItemNameController::Update()
{
	itemBackground->SetActive(false);
	itemTypeName->SetActive(false);
}

void ItemNameController::SetNameBar(std::string name, int rarity, ComponentTransform* transform)
{
	myText->text = name;
	itemName->transform = transform;
	math::float2 pos;
	pos = transform->GetScreenPosition();

	switch (rarity)
	{
	case 0:
		myText->color = math::float4(211.0f, 211.0f, 211.0f, 1.0f);
		break;
	case 1:
		myText->color = math::float4(0.0f, 255.0f, 0.0f,1.0f);
		break;
	case 2:
		myText->color = math::float4(255.0f, 69.0f, 0.0f,1.0f);
		break;
	case 3:
		myText->color = math::float4(148.0f, 0.0f, 211.0f,1.0f);
		break;
	}
	pos = pos + math::float2(0.0f, 50.0f);
	itemBackground->GetComponent<Transform2D>()->SetPositionUsingAligment(pos);
	pos = pos - math::float2(itemTypeName->GetComponent<Transform2D>()->getSize().x,0.0f);
	itemTypeName->GetComponent<Transform2D>()->SetPositionUsingAligment(pos);
	itemBackground->SetActive(true);
	itemTypeName->SetActive(true);
}