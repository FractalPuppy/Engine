#ifndef  __WorldControllerScript_h__
#define  __WorldControllerScript_h__

#include "BaseScript.h"
#include <map>
#include <memory>
#include <vector>
#include "Math/float3.h"

#ifdef WorldControllerScript_EXPORTS
#define WorldControllerScript_API __declspec(dllexport)
#else
#define WorldControllerScript_API __declspec(dllimport)
#endif

class crowdTool;
class ComponentTransform;

typedef std::pair<GameObject*, float*> objVel;

class WorldControllerScript_API WorldControllerScript : public Script
{
public:
	WorldControllerScript();
	~WorldControllerScript();

	void resetWorldAgents();
	//checks if we dont have the player and creates a new agent if not
	void setPlayer(GameObject* player, float speed);

	//checks if we dont have the enemy and creates a new agent if not
	void addEnemy(GameObject* enemy);

	//move request processing
	bool PlayerMoveRequest(unsigned int agentId, math::float3 posY, math::float3 correction = defaultCorrection);
	void EnemyMoveRequest(unsigned int agentId, math::float3 dest, math::float3 correction = defaultCorrection);

	//the update calls detour crowds update which updates all the positions of the agents
	void Update() override;

	//function to change the velocity of a certain agent
	void changeVelocity(unsigned int goID, float newVel);

	//function to stop agent
	void StopAgent(unsigned goID);

	bool IsAgentRequesting(unsigned goID);

	//Function to detect if an agent is idling
	bool IsAgentIdle(unsigned goID);

	//Function to detect if an agent is stuck in a movement request
	bool IsAgentStuck(unsigned goID);

	//fuunction that deletes all agents and enemies, and returns the gameObject* of the player
	GameObject* DeleteEnemiesFromWorld();

	void RemoveEnemy(unsigned enemyID);

private:
	//explicitly disable copy constructor and copy assignment operator
	WorldControllerScript(const WorldControllerScript&);
	WorldControllerScript& operator=(const WorldControllerScript);

	//function that adds the agent to the crowd
	int addAgent(float* pos, float* vel, float speed = 200.f);

	//clear agent objects velocities
	void clearAgentsInfo();

	//private variables
	//we are gonna start with just one crowd and each agent (any enemy or player) moves alone
	//map UUID-agent index
	std::map<unsigned, int> objectAgentIndexMap;

	//vector of gameobjects of agents to update their position
	std::vector<objVel> agentObjectsVelocity;

	//pointer to the crowd
	std::unique_ptr<crowdTool> crowdToolPointer = std::make_unique<crowdTool>();

	static const math::float3 defaultCorrection;
	
	unsigned playerUID = 0u;
	
	int playerAgentsPosition = -1;
	float* playerPos = nullptr;

};

extern "C" WorldControllerScript_API Script* CreateScript();

#endif __WorldControllerScript_h__
