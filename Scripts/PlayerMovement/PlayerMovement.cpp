#include "PlayerMovement.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleTime.h"
#include "ModuleScene.h"
#include "ModuleNavigation.h"
#include "PlayerState.h"
#include "PlayerStateAttack.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateDeath.h"
#include "EnemyControllerScript.h"

#include "ComponentAnimation.h"
#include "ComponentBoxTrigger.h"
#include "ComponentTransform.h"
#include "ComponentImage.h"
#include "GameObject.h"

#include "DamageController.h"
#include "DamageFeedbackUI.h"
#include "EnemyControllerScript.h"

#include "BasicSkill.h"
#include "SliceSkill.h"
#include "ChainAttackSkill.h"
#include "DashSkill.h"

#include "JSON.h"
#include <assert.h>
#include <string>
#include "imgui.h"
#include "Globals.h"
#include "debugdraw.h"

#define SKILLS_SLOTS 5

PlayerMovement_API Script* CreateScript()
{
	PlayerMovement* instance = new PlayerMovement;
	return instance;
}

void PlayerMovement::Expose(ImGuiContext* context)
{
	ImGui::Text("Cooldowns");
	if (ImGui::Checkbox("Show Ability Cooldown", &showAbilityCooldowns)) ActivateHudCooldownMask(showAbilityCooldowns, HUB_BUTTON_Q, HUB_BUTTON_R);
	ImGui::DragFloat("General Ability Cooldown", &hubGeneralAbilityCooldown, 1.0F, 0.0F, 10.0F);
	ImGui::DragFloat("Q Cooldown", &hubCooldown[HUB_BUTTON_Q], 1.0F, 0.0F, 10.0F);
	ImGui::DragFloat("W Cooldown", &hubCooldown[HUB_BUTTON_W], 1.0F, 0.0F, 10.0F);
	ImGui::DragFloat("E Cooldown", &hubCooldown[HUB_BUTTON_E], 1.0F, 0.0F, 10.0F);
	ImGui::DragFloat("R Cooldown", &hubCooldown[HUB_BUTTON_R], 1.0F, 0.0F, 10.0F);

	if (ImGui::Checkbox("Show Item Cooldown", &showItemCooldowns)) ActivateHudCooldownMask(showItemCooldowns, HUB_BUTTON_1, HUB_BUTTON_4);

	ImGui::SetCurrentContext(context);

	//Exposing durations this should access to every class instead of allocating them in PlayerMovement, but for now scripts don't allow it
	ImGui::DragFloat("Walking speed", &walkingSpeed, 0.01f, 10.f, 500.0f);
	ImGui::DragFloat("Out of NavMesh pos correction XZ", &OutOfMeshCorrectionXZ, 1.f, 0.f, 1000.f);
	ImGui::DragFloat("Out of NavMesh pos correction Y", &OutOfMeshCorrectionY, 1.f, 0.f, 500.f);
	ImGui::DragFloat("Max walking distance", &maxWalkingDistance, 100.f, 0.f, 100000.0f);

	ImGui::DragFloat("Out of Combat time", &outCombatMaxTime, 1.f, 0.f, 10.f);

	float maxHP = stats.health;
	float maxMP = stats.mana;
	stats.Expose("Player Stats");
	if (maxHP != stats.health)
	{
		health += stats.health - maxHP;
		if (health > stats.health) health = stats.health;
		else if (health < 0) health = 0;
	}
	if (maxMP != stats.mana)
	{
		mana += stats.mana - maxMP;
		if (mana > stats.mana) mana = stats.mana;
		else if (mana < 0) mana = 0;
	}


	// Stats Debug
	ImGui::Text("Play Stats Debug");
	ImGui::Text("HP: %f / %f", health, stats.health);
	ImGui::Text("MP: %f / %f", mana, stats.mana);
	ImGui::Text("Strength: %i", stats.strength);
	ImGui::Text("Dexterity: %i", stats.dexterity);
	ImGui::Text("HP Regen: %f pts/s", stats.hpRegen);
	ImGui::Text("Dexterity: %f pts/s", stats.manaRegen);

	// Skills debug
	/*for (const auto& skill : playerSkills)
	{
		skill->Expose(context);
	}*/
}

void PlayerMovement::CreatePlayerStates()
{
	playerStates.reserve(5);

	playerStates.push_back(walk = new PlayerStateWalk(this, "Walk"));
	if (dustParticles == nullptr)
	{
		LOG("Dust Particles not found");
	}
	else
	{
		LOG("Dust Particles found");
		dustParticles->SetActive(false);
		walk->dustParticles = dustParticles;
	}
	playerStates.push_back(attack = new PlayerStateAttack(this, "Attack",
		math::float3(150.f, 100.f, 100.f), 0.7f, 0.9f));
	playerStates.push_back(idle = new PlayerStateIdle(this, "Idle"));
	playerStates.push_back(death = new PlayerStateDeath(this, "Death"));
}

void PlayerMovement::CreatePlayerSkills()
{
	playerSkills.reserve(SKILLS_SLOTS);
	playerSkills.push_back(chain = new ChainAttackSkill(this, "Chain1", attackBoxTrigger));
	playerSkills.push_back(dash = new DashSkill(this, "Dash", attackBoxTrigger));
	playerSkills.push_back(slice = new SliceSkill(this, "Slice", attackBoxTrigger));
	playerSkills.push_back(nullptr);
	playerSkills.push_back(nullptr);
}

void PlayerMovement::Start()
{
	dustParticles = App->scene->FindGameObjectByName("WalkingDust");
	dashFX = App->scene->FindGameObjectByName("DashFX");
	dashMesh = App->scene->FindGameObjectByName("DashMesh");

	GameObject* damageGO = App->scene->FindGameObjectByName("Damage");
	if (damageGO == nullptr)
	{
		LOG("Damage controller GO couldn't be found \n");
	}
	else
	{
		damageController = damageGO->GetComponent<DamageController>();
		if (damageController == nullptr)
		{
			LOG("Damage controller couldn't be found \n");
		}
	}

	GameObject* damageFeedback = App->scene->FindGameObjectByName("RedScreen");
	if (damageFeedback == nullptr)
	{
		LOG("Damage Feedback blood couldn't be found \n");
	}
	else
	{
		damageUIFeedback = damageFeedback->GetComponent<DamageFeedbackUI>();
		if (damageUIFeedback == nullptr)
		{
			LOG("Damage UI feedback script couldn't be found \n");
		}
	}
	
	CreatePlayerStates();

	currentState = idle;

	anim = gameobject->GetComponent<ComponentAnimation>();
	if (anim == nullptr)
	{
		LOG("The GameObject %s has no Animation component attached \n", gameobject->name);
	}

	GameObject* hitBoxAttackGameObject = App->scene->FindGameObjectByName("HitBoxAttack", gameobject);

	attackBoxTrigger = hitBoxAttackGameObject->GetComponent<ComponentBoxTrigger>();
	if (attackBoxTrigger == nullptr)
	{
		LOG("The GameObject %s has no boxTrigger component attached \n", hitBoxAttackGameObject->name);
	}

	hpHitBoxTrigger = gameobject->GetComponent<ComponentBoxTrigger>();
	if (hpHitBoxTrigger == nullptr)
	{
		LOG("The GameObject %s has no boxTrigger component attached \n", gameobject->name);
	}

	transform = gameobject->GetComponent<ComponentTransform>();
	if (transform == nullptr)
	{
		LOG("The GameObject %s has no transform component attached \n", gameobject->name);
	}

	GameObject* lifeUIGameObject = App->scene->FindGameObjectByName("Life");
	assert(lifeUIGameObject != nullptr);

	lifeUIComponent = lifeUIGameObject->GetComponent<ComponentImage>();
	assert(lifeUIComponent != nullptr);

	GameObject* manaUIGameObject = App->scene->FindGameObjectByName("Mana");
	assert(manaUIGameObject != nullptr);

	manaUIComponent = manaUIGameObject->GetComponent<ComponentImage>();
	assert(manaUIComponent != nullptr);

	GameObject* hubCooldownGO = nullptr;

	hubCooldownGO = App->scene->FindGameObjectByName("Q_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_Q] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_Q] != nullptr);
	}
	else
	{
		LOG("The Game Object 'Q_Cooldown' couldn't be found.");
	}


	hubCooldownGO = App->scene->FindGameObjectByName("W_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_W] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_W] != nullptr);
	}
	else
	{
		LOG("The Game Object 'W_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("E_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_E] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_E] != nullptr);
	}
	else
	{
		LOG("The Game Object 'E_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("R_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_R] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_R] != nullptr);
	}
	else
	{
		LOG("The Game Object 'R_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("One_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_1] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_1] != nullptr);
	}
	else
	{
		LOG("The Game Object '1_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("Two_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_2] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_2] != nullptr);
	}
	else
	{
		LOG("The Game Object '2_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("Three_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_3] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_3] != nullptr);
	}
	else
	{
		LOG("The Game Object '3_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("Four_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_4] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_4] != nullptr);
	}
	else
	{
		LOG("The Game Object '4_Cooldown' couldn't be found.");
	}

	CreatePlayerSkills();

	if (dashFX == nullptr)
	{
		LOG("DashFX Gameobject not found");
	}
	else
	{
		LOG("DashFX found");
		dashFX->SetActive(false);
		dash->dashFX = dashFX;
	}

	if (dashMesh == nullptr)
	{
		LOG("DashMesh Gameobject not found");
	}
	else
	{
		LOG("DashMesh found");
		dashMesh->SetActive(false);
		dash->meshOriginalScale = dashMesh->transform->scale;
		dash->dashMesh = dashMesh;
	}

	LOG("Started player movement script");
}

void PlayerMovement::Update()
{
	if (App->time->gameTimeScale == 0) return;

	if (health <= 0.f)
	{
		currentState = (PlayerState*)death;
		return;
	}

	PlayerState* previous = currentState;

	//Check input here and update the state!
	if (currentState != death)
	{
		// Update cooldowns
		for (unsigned i = 0; i < 8; ++i)
		{
			if (hubCooldownTimer[i] > 0)
			{
				hubCooldownTimer[i] -= App->time->gameDeltaTime;
				if (hubCooldownTimer[i] < 0) hubCooldownTimer[i] = 0;
			}

			if (hubCooldownMask != nullptr && hubCooldownMask[i] != nullptr && hubCooldownMask[i]->enabled)
			{
				hubCooldownMask[i]->SetMaskAmount((int)(100.0F * hubCooldownTimer[i] / hubCooldownMax[i]));
			}
		}

		currentState->UpdateTimer();

		currentState->CheckInput();

		currentState->Update();

		//if previous and current are different the functions Exit() and Enter() are called
		CheckStates(previous, currentState);	
	}

	ManaManagement();

	if (outCombatTimer > 0)
	{
		outCombatTimer -= App->time->gameDeltaTime;
	}
	else if (health < stats.health)
	{
		health += stats.hpRegen * App->time->gameDeltaTime;
		if (health > stats.health) health = stats.health;
		int healthPercentage = (health / stats.health) * 100;
		lifeUIComponent->SetMaskAmount(healthPercentage);
	}

	//Check for changes in the state to send triggers to animation SM
}

PlayerMovement_API void PlayerMovement::Damage(float amount)
{
	if (!isPlayerDead)
	{
		outCombatTimer = outCombatMaxTime;
		health -= amount;
		if (health < 0)
		{
			isPlayerDead = true;
		}

		damageController->AddDamage(gameobject->transform, amount, 5);
		if(damageUIFeedback != nullptr)
			damageUIFeedback->ActivateDamageUI();

		int healthPercentage = (health / stats.health) * 100;
		lifeUIComponent->SetMaskAmount(healthPercentage);
	}
}

void PlayerMovement::Equip(const PlayerStats & equipStats)
{
	this->stats += equipStats;

	int healthPercentage = (health / stats.health) * 100;
	lifeUIComponent->SetMaskAmount(healthPercentage);

	int manaPercentage = (mana / stats.mana) * 100;
	manaUIComponent->SetMaskAmount(manaPercentage);
}

void PlayerMovement::UnEquip(const PlayerStats & equipStats)
{
	this->stats -= equipStats;
	health = health > stats.health ? stats.health : health;
	mana = mana > stats.mana ? stats.mana : mana;

	int healthPercentage = (health / stats.health) * 100;
	lifeUIComponent->SetMaskAmount(healthPercentage);

	int manaPercentage = (mana / stats.mana) * 100;
	manaUIComponent->SetMaskAmount(manaPercentage);
}

void PlayerMovement::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddFloat("General_Ability_Cooldown", hubGeneralAbilityCooldown);
	json->AddFloat("Q_Cooldown", hubCooldown[HUB_BUTTON_Q]);
	json->AddFloat("W_Cooldown", hubCooldown[HUB_BUTTON_W]);
	json->AddFloat("E_Cooldown", hubCooldown[HUB_BUTTON_E]);
	json->AddFloat("R_Cooldown", hubCooldown[HUB_BUTTON_R]);
	json->AddUint("Show_Ability_Cooldown", showAbilityCooldowns ? 1 : 0);
	json->AddUint("Show_Items_Cooldown", showItemCooldowns ? 1 : 0);
	json->AddFloat("Out_of_combat_timer", outCombatMaxTime);

	json->AddFloat("walkingSpeed", walkingSpeed);
	json->AddFloat("MeshCorrectionXZ", OutOfMeshCorrectionXZ);
	json->AddFloat("MeshCorrectionY", OutOfMeshCorrectionY);
	json->AddFloat("MaxWalkDistance", maxWalkingDistance);

	stats.Serialize(json);
}

void PlayerMovement::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	hubGeneralAbilityCooldown = json->GetFloat("General_Ability_Cooldown", 0.5F);
	hubCooldown[HUB_BUTTON_Q] = json->GetFloat("Q_Cooldown", 1.0F);
	hubCooldown[HUB_BUTTON_W] = json->GetFloat("W_Cooldown", 1.0F);
	hubCooldown[HUB_BUTTON_E] = json->GetFloat("E_Cooldown", 1.0F);
	hubCooldown[HUB_BUTTON_R] = json->GetFloat("R_Cooldown", 1.0F);
	showAbilityCooldowns = json->GetUint("Show_Ability_Cooldown", 1U) == 1;
	showItemCooldowns = json->GetUint("Show_Items_Cooldown", 1U) == 1;

	walkingSpeed = json->GetFloat("walkingSpeed", 300.0f);
	OutOfMeshCorrectionXZ = json->GetFloat("MeshCorrectionXZ", 500.f);
	OutOfMeshCorrectionY = json->GetFloat("MeshCorrectionY", 300.f);
	maxWalkingDistance = json->GetFloat("MaxWalkDistance", 10000.0f);

	outCombatMaxTime = json->GetFloat("Out_of_combat_timer", 3.f);

	stats.DeSerialize(json);
}

void PlayerMovement::OnTriggerExit(GameObject* go)
{
	//if (go->name == "HitBoxAttack")
	//{
	//	Damage(10);
	//}
}

bool PlayerMovement::IsAtacking() const
{
	return canInteract && App->input->GetMouseButtonDown(1) == KEY_DOWN; //Left button
}

bool PlayerMovement::IsMoving() const
{
	math::float3 temp;
	return (App->input->GetMouseButtonDown(3) == KEY_DOWN && canInteract ||
		currentState->playerWalking ||
		(App->input->GetMouseButtonDown(3) == KEY_REPEAT && !App->scene->Intersects("PlayerMesh", false, temp))); //right button, the player is still walking or movement button is pressed and can get close to mouse pos
}

bool PlayerMovement::IsUsingFirstSkill() const
{
	return hubCooldownTimer[HUB_BUTTON_Q] <= 0 && App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN;
}

bool PlayerMovement::IsUsingSecondSkill() const
{
	return hubCooldownTimer[HUB_BUTTON_W] <= 0 && App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN;
}

bool PlayerMovement::IsUsingThirdSkill() const
{
	return hubCooldownTimer[HUB_BUTTON_E] <= 0 && App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN;
}

bool PlayerMovement::IsUsingFourthSkill() const
{
	return hubCooldownTimer[HUB_BUTTON_R] <= 0 && App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN;
}

bool PlayerMovement::IsUsingFirstItem() const
{
	return hubCooldownTimer[HUB_BUTTON_1] <= 0 && App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN;
}

bool PlayerMovement::IsUsingSecondItem() const
{
	return hubCooldownTimer[HUB_BUTTON_2] <= 0 && App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN;
}

bool PlayerMovement::IsUsingThirdItem() const
{
	return hubCooldownTimer[HUB_BUTTON_3] <= 0 && App->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN;
}

bool PlayerMovement::IsUsingFourthItem() const
{
	return hubCooldownTimer[HUB_BUTTON_4] <= 0 && App->input->GetKey(SDL_SCANCODE_4) == KEY_DOWN;
}

bool PlayerMovement::IsUsingSkill() const
{
	return (IsUsingFirstSkill() || IsUsingSecondSkill() || IsUsingThirdSkill() || IsUsingFourthSkill());
}

void PlayerMovement::ResetCooldown(unsigned int hubButtonID)
{
	if (hubButtonID <= HUB_BUTTON_R)
	{
		for (unsigned i = HUB_BUTTON_Q; i <= HUB_BUTTON_R; ++i)
		{
			hubCooldownTimer[i] = hubGeneralAbilityCooldown;
			hubCooldownMax[i] = hubGeneralAbilityCooldown;
		}
	}

	if (hubCooldownTimer[hubButtonID] < hubCooldown[hubButtonID])
	{
		hubCooldownTimer[hubButtonID] = hubCooldown[hubButtonID];
		hubCooldownMax[hubButtonID] = hubCooldown[hubButtonID];
	}
}

void PlayerMovement::CheckStates(PlayerState * previous, PlayerState * current)
{
	if (previous != current)
	{
		previous->ResetTimer();

		previous->Exit();
		current->Enter();
		current->duration = anim->GetDurationFromClip();

		if (anim != nullptr)
		{
			anim->SendTriggerToStateMachine(current->trigger.c_str());
		}
	}
}

void PlayerMovement::ManaManagement()
{
	IsManaUsed = false;

	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP && mana > 25)
	{
		mana = mana - 25;
		IsManaUsed = true;
	}

	if (!IsManaUsed && mana < stats.mana)
	{
		mana += stats.manaRegen * App->time->gameDeltaTime;
		if (mana > stats.mana) mana = stats.mana;
	}

	int manaPercentage = (mana / stats.mana) * 100;
	manaUIComponent->SetMaskAmount(manaPercentage);
}

void PlayerStats::Serialize(JSON_value * json) const
{
	JSON_value* statsValue = json->CreateValue();

	statsValue->AddFloat("health", health);
	statsValue->AddFloat("mana", mana);
	statsValue->AddInt("strength", strength);
	statsValue->AddInt("dexterity", dexterity);
	statsValue->AddFloat("hp_regen", hpRegen);
	statsValue->AddFloat("mana_regen", manaRegen);

	json->AddValue("stats", *statsValue);
}

void PlayerStats::DeSerialize(JSON_value * json)
{

	JSON_value* statsValue = json->GetValue("stats");
	if (!statsValue) return;

	health = statsValue->GetFloat("health", 100.0F);
	mana = statsValue->GetFloat("mana", 100.0F);
	strength = statsValue->GetInt("strength", 10);
	dexterity = statsValue->GetInt("dexterity", 10);
	hpRegen = statsValue->GetFloat("hp_regen", 5.0F);
	manaRegen = statsValue->GetFloat("mana_regen", 5.0F);
}

void PlayerStats::Expose(const char* sectionTitle)
{
	ImGui::Text(sectionTitle);
	ImGui::InputFloat("Health", &health);
	ImGui::InputFloat("Mana", &mana);

	int uiStrength = (int)strength;
	if (ImGui::InputInt("Strength", &uiStrength)) strength = uiStrength < 0 ? 0 : uiStrength;

	int uiDexterity = (int)dexterity;
	if (ImGui::InputInt("Strength", &uiDexterity)) dexterity = uiDexterity < 0 ? 0 : uiDexterity;

	ImGui::DragFloat("HP regen", &hpRegen, 1.0F, 0.0F, 10.0F);
	ImGui::DragFloat("Mana regen", &manaRegen, 1.0F, 0.0F, 10.0F);
}

void PlayerMovement::ActivateHudCooldownMask(bool activate, unsigned first, unsigned last)
{
	for (unsigned i = first; i <= last; ++i) hubCooldownMask[i]->gameobject->SetActive(activate);
}