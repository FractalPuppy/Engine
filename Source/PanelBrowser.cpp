#include "PanelBrowser.h"
#include "Globals.h"
#include "SDL_version.h"
#include "IL/il.h"
#include "assimp/version.h"
#include "GL/glew.h"
#include "imgui.h"
#include "rapidjson/rapidjson.h"
#include "physfs.h"

PanelBrowser::PanelBrowser() 
{
	enabled = false;
}

PanelBrowser::~PanelBrowser() 
{

}

void PanelBrowser::Draw() 
{
	if (!ImGui::Begin("Assets", &enabled))
	{
		ImGui::End();
		return;
	}
	ImGui::End();
}