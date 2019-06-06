#ifndef  __GameLoop_h__
#define  __GameLoop_h__

#include "BaseScript.h"

#include "Math/float3.h"

#ifdef GameLoop_EXPORTS
#define GameLoop_API __declspec(dllexport)
#else
#define GameLoop_API __declspec(dllimport)
#endif

#include <vector>

class Component;
class Button;
class Text;
class ComponentScript;
class ComponentCamera;
class PlayerMovement;
class EnemyMovementScript;
class IntroScript;
class CreditsScript;
class AABB;
class JSON_value;

class GameLoop_API GameLoop : public Script
{
	enum class GameState
	{		
		MENU,
		INTRO,
		PLAYING,
		DEAD,
		WIN,
		PAUSED,
		OPTIONS,
		CREDITS,
		CONTROLS,
		QUIT,
		LOADING
	};

	enum class GameScene
	{
		MENU,
		CEMENTERY,
		HUD
	};
public :
	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;
private:

	void Start() override;
	void Update() override;

	void LoadMenuScene();
	void LoadCementeryScene();
	void LoadHUDScene();

	void ManageDead();
	void ManageMenu();
	void ManageIntro();
	void ManagePlaying();
	void ManageWin();
	void ManagePaused();
	void ManageOptions();
	void ManageCredits();
	void ManageControls();
	void ManageQuit();
	void ManageLoading();

	void EnableMenuButtons(bool enable);

	void VolumeManagement();
	void SoundManagement();
	void VsyncManagement();
	void ResolutionManagement();

	void ResetPositions();

	void ChangeGameState(GameState newState); //Set initial conditions for each state here if required

	GameState gameState = GameState::MENU;
	GameScene gameScene = GameScene::MENU;
public:
	int volume = 10;

private:
	//UI Values
	int minVolume = 0;
	int maxVolume = 10;
	int sound = 6;
	int minSound = 0;
	int maxSound = 10;

	float loading = 0;

	//UI

	//Buttons
	std::vector<Component*> menuButtons;
	Button* optionButton = nullptr;
	Button* controlsButton = nullptr;
	Button* creditsButton = nullptr;
	Button* vsyncButton = nullptr;
	Button* exitButton = nullptr;
	Button* backOptionButton = nullptr;
	Button* backControlsButton = nullptr;
	Button* backCreditsButton = nullptr;
	Button* toTheAltarButton = nullptr;
	Button* hudBackToMenuButton = nullptr;
	Button* inventoryButton = nullptr;
	Button* missionsButton = nullptr;
	Button* skillsButton = nullptr;
	Button* closePlayerMenuButton = nullptr;
	std::vector<Component*> volumeButtons;
	std::vector<Component*> soundButtons;

	//GO
	GameObject* menu = nullptr;
	GameObject* options = nullptr;
	GameObject* controls = nullptr;
	GameObject* loseWindow = nullptr;
	GameObject* winWindow = nullptr;
	GameObject* hudGO = nullptr;
	GameObject* playerGO = nullptr;
	GameObject* enemyGO = nullptr;
	GameObject* creditsGO = nullptr;
	GameObject* vsyncGO = nullptr;
	GameObject* introCamera = nullptr;
	GameObject* loadingGO = nullptr;
	GameObject* playerMenuGO = nullptr;
	GameObject* inventoryMenuGO = nullptr;
	GameObject* missionsMenuGO = nullptr;
	GameObject* skillsMenuGO = nullptr;

	//BBOX
	math::AABB* playerBbox = nullptr;
	math::AABB* winBbox = nullptr;

	//TEXT
	Text* volumeText = nullptr;
	Text* soundText = nullptr;

	//Script
	PlayerMovement* playerScript = nullptr;
	EnemyMovementScript* enemyMovementScript = nullptr;
	IntroScript* introScript = nullptr;
	CreditsScript* creditsScript = nullptr;

	//Camera
	ComponentCamera* componentIntroCamera = nullptr;

	float3 playerStartPosition = float3::zero;
	float3 enemyStartPosition = float3::zero;

	bool runningCredits = false;
	bool runningIntro = false;
	bool vsync = false;

	std::string sceneToLoad = "";
	int actionAfterLoad = -1;
};

#endif __GameLoop_h__
