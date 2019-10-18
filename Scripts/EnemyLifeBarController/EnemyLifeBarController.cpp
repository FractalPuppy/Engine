#include "EnemyLifeBarController.h"

#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentImage.h"
#include "ComponentText.h"

EnemyLifeBarController_API Script* CreateScript()
{
	EnemyLifeBarController* instance = new EnemyLifeBarController;
	return instance;
}

void EnemyLifeBarController::Start()
{
	enemyLife = App->scene->FindGameObjectByName("EnemyLife");
	lifeImage = App->scene->FindGameObjectByName("HPbar", enemyLife)->GetComponent<ComponentImage>();
	enemyTypeName = App->scene->FindGameObjectByName("EnemyTypeName", enemyLife)->GetComponent<Text>();
	skull = App->scene->FindGameObjectByName("Skull", enemyLife)->GetComponent<ComponentImage>();
}

void EnemyLifeBarController::Update()
{
	if (draw)
	{
		enemyLife->SetActive(true);
		draw = false;
	}
	else
	{
		enemyLife->SetActive(false);
	}
}

void EnemyLifeBarController::SetLifeBar(int maxHP, int actualHP, EnemyLifeBarType type, std::string name)
{
	draw = true;
	enemyTypeName->text = name;

	switch (type)
	{
	case EnemyLifeBarType::NORMAL:
		skull->UpdateTexture("Basic_cemetery_enemy");
		break;
	case EnemyLifeBarType::NORMAL_TEMPLE:
		skull->UpdateTexture("Basic_Temple_enemy");
		break;
	case EnemyLifeBarType::ELITE_GRAVEYARD:
		skull->UpdateTexture("Elite_cemetery_enemy");
		break;
	case EnemyLifeBarType::ELITE_TEMPLE:
		skull->UpdateTexture("Elite_Temple_Enemy");
		break;
	case EnemyLifeBarType::BOSS:
		skull->UpdateTexture("Boos_temple_enemy");
		break;
	default:
		break;
	}

	lifeImage->SetMaskAmount((actualHP * 100) / maxHP);
}

