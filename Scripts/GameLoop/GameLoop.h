#ifndef  __GameLoop_h__
#define  __GameLoop_h__

#include "BaseScript.h"
#include "Application.h"
#include "Math/float3.h"

#ifdef GameLoop_EXPORTS
#define GameLoop_API __declspec(dllexport)
#else
#define GameLoop_API __declspec(dllimport)
#endif

#define MENU_SCENE "MenuScene"

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
class InventoryScript;
class EquipPopupController;
class SkillTreeController;
class ExperienceController;
class JSON_value;
class ComponentAudioSource;
class WorldControllerScript;
class ComponentImage;

class LoopState;
class LoopStateControls;
class LoopStateCredits;
class LoopStateDead;
class LoopStateIntro;
class LoopStateLoading;
class LoopStateMenu;
class LoopStateOptions;
class LoopStatePaused;
class LoopStatePlaying;
class LoopStateQuit;
class LoopStateWin;

enum class GameScene
{
	MENU,
	CEMENTERY,
	TEMPLE,
	HUD
};

class GameLoop_API GameLoop : public Script
{

public :
	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;
	inline virtual GameLoop* Clone() const
	{
		return new GameLoop(*this);
	}

public:

	void Start() override;
	void Update() override;

	void LoadMenuScene();
	void LoadCementeryScene();
	void LoadTempleScene();
	void LoadHUDScene();
	void LoadLvl(unsigned lvl);

	void CreateGameStates();
	void CheckStates(LoopState* previous);

	void EnableMenuButtons(bool enable);

	void GetStateAfterLoad();
	void VolumeManagement();
	void SoundManagement();
	void VsyncManagement();
	void ResolutionManagement();

	void ResetPositions();

	bool HasImageHoveredInChildren(const GameObject* go) const;

	//world controller management
	GameObject* DeleteAllEnemies();
	void AddPlayerToWorld(GameObject* Player);
	void navMeshReloaded();


	LoopState* currentLoopState = nullptr;

	LoopStateControls* controlsState = nullptr;
	LoopStateCredits* creditsState = nullptr;
	LoopStateDead* deadState = nullptr;
	LoopStateIntro* introState = nullptr;
	LoopStateLoading* loadingState = nullptr;
	LoopStateMenu* menuState = nullptr;
	LoopStateOptions* optionsState = nullptr;
	LoopStatePaused* pausedState = nullptr;
	LoopStatePlaying* playingState = nullptr;
	LoopStateQuit* quitState = nullptr;
	LoopStateWin* winState = nullptr;

	GameScene gameScene = GameScene::MENU;
	std::vector<LoopState*> loopStates;

private:
	void ManageNavigationCursorIcon();

public:
	int volume = 10;

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
	Button* skillsButton = nullptr;
	Button* closePlayerMenuButton = nullptr;
	Button* pauseResume = nullptr;
	Button* pauseExit = nullptr;
	Button* pauseControls = nullptr;
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
	GameObject* worldControllerGO = nullptr;
	GameObject* enemyGO = nullptr;
	GameObject* creditsGO = nullptr;
	GameObject* vsyncGO = nullptr;
	GameObject* introCamera = nullptr;
	GameObject* loadingGO = nullptr;
	GameObject* playerMenuGO = nullptr;
	GameObject* inventoryMenuGO = nullptr;
	GameObject* skillsMenuGO = nullptr;
	GameObject* pauseMenuGO = nullptr;
	GameObject* audioGO = nullptr;
	GameObject* introVideoGO = nullptr;
	GameObject* outroVideoGO = nullptr;
	GameObject* introSkipTextGO = nullptr;
	GameObject* outroSkipTextGO = nullptr;
	std::vector<GameObject*> sunHoverGO;

	//BBOX
	math::AABB* playerBbox = nullptr;
	math::AABB* winBbox = nullptr;

	//TEXT
	Text* volumeText = nullptr;
	Text* soundText = nullptr;

	//Script
	PlayerMovement* playerScript = nullptr;
	EnemyMovementScript* enemyMovementScript = nullptr;
	WorldControllerScript* worldController = nullptr;
	IntroScript* introScript = nullptr;
	CreditsScript* creditsScript = nullptr;
	InventoryScript* inventoryScript = nullptr;
	EquipPopupController* equipPopUpScript = nullptr;
	SkillTreeController* skillTreeScript = nullptr;
	ExperienceController* experienceScript = nullptr;

	//Camera
	ComponentCamera* componentIntroCamera = nullptr;

	//Audio
	ComponentAudioSource* menuButtonsSound = nullptr;

	//Credits video
	GameObject* menuMusic = nullptr;
	GameObject* creditsAudio = nullptr;
	ComponentImage* creditsVideo = nullptr;
	GameObject* creditsVideoGO = nullptr;


	float3 playerStartPosition = float3::zero;
	float3 enemyStartPosition = float3::zero;

	bool runningCredits = false;
	bool runningIntro = false;
	bool introvideoPlaying = false;
	bool vsync = false;

	std::string sceneToLoad = "";
	LoopState* stateAfterLoad = nullptr;
	bool actionAfterLoad = false;
	bool bossDeath = false;

	float xPickingCorrection = 0.0f;
	float yPickingCorrection = 0.0f;
	float zPickingCorrection = 0.0f;
	bool changeNavigationCursorIcon = true;
	bool changeStandarCursorIcon = true;
};

extern "C" GameLoop_API Script* CreateScript();

#endif __GameLoop_h__
