#ifndef __MODULENAVIGATION_H__
#define __MODULENAVIGATION_H__

#include "Module.h"

#include <vector>
#include <memory>
#include "Math/float3.h"
#include "MathGeoLib/Geometry/AABB.h"
#include "Math/MathConstants.h"

#define MAX_DETOUR_PATH 1024
#define MAX_POLYS 256
#define MAX_SMOOTH 512

//#include "DebugUtils/DetourDebugDraw.h"
//#include "DebugUtils/DebugDraw.h"
//<>
//fwd declarations
class GameObject;
class ComponentRenderer;
class ComponentTransform;
//from recast
class rcConfig;
class rcContext;
class rcCompactHeightfield;
class rcHeightfield;
class rcContourSet;
class rcPolyMesh;
class rcPolyMeshDetail;
//class DrawVertex;
//from detour
class dtNavMesh;
class dtNavMeshQuery;
class DetourDebugInterface;
//from detour crowd
class dtCrowd;
struct dtCrowdAgentDebugInfo;
//from obstacle avoidance
class dtObstacleAvoidanceDebugData;

namespace dd
{
	union DrawVertex;
}

/// These are just sample areas to use consistent values across the samples.
/// The use should specify these base on his needs.
enum SamplePolyAreas
{
	SAMPLE_POLYAREA_GROUND,
	SAMPLE_POLYAREA_WATER,
	SAMPLE_POLYAREA_ROAD,
	SAMPLE_POLYAREA_DOOR,
	SAMPLE_POLYAREA_GRASS,
	SAMPLE_POLYAREA_JUMP,
};
enum SamplePolyFlags
{
	SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
	SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
	SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
	SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
	SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
	SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
};

enum SamplePartitionType
{
	SAMPLE_PARTITION_WATERSHED,
	SAMPLE_PARTITION_MONOTONE,
	SAMPLE_PARTITION_LAYERS,
};

ENGINE_API enum class PathFindType
{
	FOLLOW,
	STRAIGHT,
	NODODGE
};

class ModuleNavigation :
	public Module
{
public:
	ModuleNavigation();
	~ModuleNavigation();

	bool Init(JSON* config);
	void SaveConfig(JSON* config) override;
	void sceneLoaded(JSON* config);
	void sceneSaved(JSON* config);

	void DrawGUI()override;
	
	void renderNavMesh();

	void AddNavigableMeshFromObject(GameObject* obj);

	ENGINE_API bool FindPath(	math::float3 start, math::float3 end, std::vector<math::float3> &path, 
								PathFindType type = PathFindType::FOLLOW, math::float3 diff = math::float3(0.f, 0.f, 0.f),
								float maxDist = 10000.0f, float ignoreDist = floatMax) const;
	//there is a default really big limitating path distance for the calls that are not supposed to be limitated
	ENGINE_API bool NavigateTowardsCursor(math::float3 start, std::vector<math::float3>& path, 
										  math::float3 positionCorrection, math::float3& intersectionPos, 
										  float maxPathDistance = 10000.0f, PathFindType type = PathFindType::FOLLOW,
										  float ignoreDist = floatMax) const;
	ENGINE_API bool FindIntersectionPoint(math::float3 start, math::float3& intersectionPoint) const;
	ENGINE_API bool FindClosestPoint2D(math::float3& initial) const;
	ENGINE_API bool IsValidPosition(math::float3& position) const;

	ENGINE_API bool HighQualityMouseDetection(math::float3* intersection) const;
	ENGINE_API bool NavMeshPolygonQuery(unsigned int* targetRef, math::float3* endPos, math::float3 correction) const;

	ENGINE_API bool IsCursorPointingToNavigableZone(float xPickingCorrection = 0.0f, float yPickingCorrection = 0.0f, float zPickingCorrection = 0.0f) const;

	ENGINE_API void setPlayerBB(math::AABB bbox);

	ENGINE_API void GenerateNavigabilityFromGOs(std::vector<GameObject*>& vectorGOs);

	void RecalcPath(math::float3 point);

	//Constants
	//static const int ERROR = -1;
	static const int ERROR_NEARESTPOLY = -2;

	//info from other sources
	std::string sceneName = "";

private:
	// Explicitly-disabled copy constructor and copy assignment operator.
	ModuleNavigation(const ModuleNavigation&);
	ModuleNavigation& operator=(const ModuleNavigation);

	void generateNavigability(bool render);
	void addNavigableMesh();
	void CleanValuesPRE();
	void CleanValuesPOST();

	void fillVertices();
	void fillIndices();
	void fillNormals();
	void fillDrawPoints();

	bool getSteerTarget(dtNavMeshQuery * navQuery, const float * startPos,
		const float * endPos, const float minTargetDist, 
		const unsigned * path, const int pathSize, 
		float * steerPos, unsigned char & steerPosFlag, unsigned & steerPosRef,
		float * outPoints, int * outPointCount) const;

	bool inRange(const float * v1, const float * v2, const float r, const float h) const;

	float GetXZDistance(float3 a, float3 b) const;
	float GetXZDistanceWithoutSQ(float3 a, float3 b) const;

	float3 getNextStraightPoint(float3 current, float3 pathDirection, float3 end, bool* destination) const;
	bool checkNododgePathPoints(int numPoints) const;
	
private:
	//variables

	friend class crowdTool;

	dtNavMesh* navMesh = nullptr;
	dtNavMeshQuery* navQuery = nullptr;

	math::AABB playerBB;
	//char newCharacter[64] = "New Character";//implementation postponed, possibly aborted
	float characterMaxRadius = 0.6f;
	float characterMaxHeight = 5.0f;
	float characterMaxSlopeScaling = 50.0f;
	float characterMaxStepHeightScaling = 5.0f;
	
	//UI modificiators
	const float sliderIncreaseSpeed = 0.03f;
	const float minSliderValue = 0.01f;
	const float maxSliderValue = 100.0f;
	const float maxSlopeValue = 60.0f;
	const float cellIncreaseSpeed = 0.25f;
	const float minCellSize = 0.1f;
	const float maxCellSize = 50.0f;
	int minRegionSize = 8;
	int mergedRegionSize = 20;
	float edgeMaxLength = 20.f;
	float edgeMaxError = 1.3f;
	int vertexPerPoly = 6;

	//newer config
	float cellWidth = 20.250f;
	float cellHeight = 5.f;
	
	float sampleDistance = 6;
	float sampleMaxError = 1;

	unsigned minNododgePathPoints = 10u;

	//filters
	bool filterLowHangingObstacles = true;
	bool filterLedgeSpans = true;
	bool filterWalkableLowHeightSpans = true;

	//partition type
	int partitionType = 0;

	//load info
	int navDataSize = 0;

	//navigation mesh properties
	bool meshGenerated = false;
	bool renderMesh = false;

	enum DrawMode
	{
		DRAWMODE_NAVMESH,
		DRAWMODE_NAVMESH_TRANS,
		DRAWMODE_NAVMESH_BVTREE,
		DRAWMODE_NAVMESH_NODES,
		DRAWMODE_NAVMESH_INVIS,
		DRAWMODE_MESH,
		DRAWMODE_VOXELS,
		DRAWMODE_VOXELS_WALKABLE,
		DRAWMODE_COMPACT,
		DRAWMODE_COMPACT_DISTANCE,
		DRAWMODE_COMPACT_REGIONS,
		DRAWMODE_REGION_CONNECTIONS,
		DRAWMODE_RAW_CONTOURS,
		DRAWMODE_BOTH_CONTOURS,
		DRAWMODE_CONTOURS,
		DRAWMODE_POLYMESH,
		DRAWMODE_POLYMESH_DETAIL,
		MAX_DRAWMODE
	};

	DrawMode m_drawMode;

	std::vector < const ComponentRenderer*> meshComponents;
	std::vector < const ComponentTransform*> transformComponents;
	std::vector <bool> unwalkableVerts;
	std::vector <bool> isObstacle;

	rcConfig* cfg = nullptr;
	rcContext* ctx = nullptr;
	rcCompactHeightfield* chf = nullptr;
	rcHeightfield* heightField = nullptr;
	rcContourSet* cset = nullptr;
	rcPolyMesh* pmesh = nullptr;
	rcPolyMeshDetail* dmesh = nullptr;

	DetourDebugInterface* ddi = nullptr;

	unsigned char* m_triareas = nullptr;
	bool m_keepInterResults = true;

	int* tris = nullptr;
	float* verts = nullptr;
	float* normals = nullptr;
	int nverts = 0;
	int ntris = 0;
	bool pointsUpdated = false;
	dd::DrawVertex* points = nullptr;
	std::vector<const AABB*> meshboxes;
	float* bmin = nullptr;
	float* bmax = nullptr;
	const AABB* meshbox = nullptr;

	//Debugging
	mutable bool pathGenerated = false;
	mutable std::vector<std::pair<math::float3, float>> pathDist;
	mutable std::vector<math::float3> path;
	math::float3 start = math::float3::inf;
	math::float3 end = math::float3::inf;

	bool startPoint = true; //defines if we are going to select start or end point in debug mode
	bool drawNavMesh = true;

	bool logDebugPathing = false;
};

ENGINE_API class crowdTool
{
public:
	//public functions
	ENGINE_API crowdTool();
	ENGINE_API ~crowdTool();

	ENGINE_API int AddNewAgent(float* pos, float* vel, float speed = 200.f);
	ENGINE_API void UpdateCrowd(float dtime);
	ENGINE_API void MoveRequest(int idAgent, unsigned int targetRef, float* endPos);
	ENGINE_API void ChangeVelocity(int idAgent, float velocity);

	//public variables
	static const int MAX_AGENTS = 50;

private:
	//private functions
	static void calcVel(float* vel, const float* pos, const float* tgt, const float speed);

	//private variables
	dtNavMeshQuery* m_navQuery = nullptr;
	dtNavMesh* m_nav = nullptr;
	dtCrowd* m_crowd = nullptr;

	float m_targetPos[3];

	dtObstacleAvoidanceDebugData* m_vod = nullptr;

	dtCrowdAgentDebugInfo* debug;

	//struct that should be declared close by if needed, its for debug draw
	//CrowdToolParams 

	bool m_run = true;
};

#endif __MODULENAVIGATION_H__
