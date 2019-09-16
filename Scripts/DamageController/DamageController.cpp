#include "DamageController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentText.h"
#include "ComponentTransform2D.h"
#include "ComponentTransform.h"
#include <list>
#include <string>
#include "imgui.h"

DamageController_API Script* CreateScript()
{
	DamageController* instance = new DamageController;
	return instance;
}

void DamageController::Start()
{
	std::list<GameObject*> list = App->scene->FindGameObjectByName("Damage")->children;
	
	int k = 0;
	for (GameObject* const &i : list) {
		uiDamage[k++] = i;
	}

	for (int i = 0; i != 10; i++) {
		damageList[i] = Damage();
	}
}

void DamageController::Update()
{
	for (int i = 0; i != 10; ++i) {
		if (damageList[i].show)
		{
			damageList[i].distanceDone += (damageList[i].distanceNormalized * 100 * App->time->gameDeltaTime).y;
			damageList[i].actualPosition = damageList[i].transform->GetScreenPosition();
			damageList[i].actualPosition.y += damageList[i].distanceDone;
			uiDamage[i]->GetComponent<Transform2D>()->SetPositionUsingAligment(damageList[i].actualPosition);

			if (damageList[i].distanceDone > 100)
			{
				uiDamage[i]->SetActive(false);
				damageList[i].show = false;
			}
		}
	}
}

void DamageController::Expose(ImGuiContext* context)
{
	ImGui::Text("Damage UI Numbers Color:");
	ImGui::ColorEdit4("Nomal Color", (float*)&normalColor);
	ImGui::ColorEdit4("Critical Color", (float*)&criticalColor);
	ImGui::ColorEdit4("Pollometer Color", (float*)&pollometerColor);
	ImGui::ColorEdit4("Healing Color", (float*)&healingColor);
	ImGui::ColorEdit4("Received Color", (float*)&receivedColor);
	ImGui::ColorEdit4("Mana Color", (float*)&manaColor);

	
}

bool DamageController::AddDamage(ComponentTransform* transform, int damage, DamageType type)
{

	for (int i = 0; i != 10; i++) {
		if (!damageList[i].show)
		{
			damageList[i].transform = transform;
			damageList[i].damage = damage;
			damageList[i].actualPosition = transform->GetScreenPosition();
			damageList[i].type = (DamageType)type;
			damageList[i].show = true;
			damageList[i].distanceNormalized = (math::float2(damageList[i].actualPosition.x, damageList[i].actualPosition.y + 100) - damageList[i].actualPosition).Normalized();
			damageList[i].distanceDone = 0;
			
			uiDamage[i]->SetActive(true);
			uiDamage[i]->GetComponent<Text>()->text = std::to_string(damage);
			switch ((DamageType)type)
			{
			default:
			case DamageType::NORMAL:
				uiDamage[i]->GetComponent<Text>()->color = normalColor;
				break;
			case DamageType::CRITICAL:
				uiDamage[i]->GetComponent<Text>()->color = criticalColor;
				break;
			case DamageType::POLLOMETER:
				uiDamage[i]->GetComponent<Text>()->color = pollometerColor;
				break;
			case DamageType::HEALING:
				uiDamage[i]->GetComponent<Text>()->color = healingColor;
				break;
			case DamageType::RECIEVED:
				uiDamage[i]->GetComponent<Text>()->color = receivedColor;
				break;
			case DamageType::MANA:
				uiDamage[i]->GetComponent<Text>()->color = manaColor;
				break;
			}
			
			uiDamage[i]->GetComponent<Transform2D>()->SetPositionUsingAligment(damageList[i].actualPosition);

			return true;
		}
	}

	return false;
}