#include "PanelAbout.h"
#include "Globals.h"
#include "SDL_version.h"
#include <windows.h>
#include "IL/il.h"
#include "assimp/version.h"
#include "GL/glew.h"
#include "imgui.h"
#include "rapidjson/rapidjson.h"
#include "physfs.h"

#define ENGINE_DESCRIPTION "Engine created during the Master of Videogames in UPC School."
#define AUTHOR "Albert Val Vila"
#define REPOSITORY "https://github.com/AlbertVVila/Engine"
#define LICENSE "https://github.com/AlbertVVila/Engine/blob/master/LICENSE"

PanelAbout::PanelAbout()
{
	enabled = false;
}


PanelAbout::~PanelAbout()
{
}

void PanelAbout::Draw()
{
	if (!ImGui::Begin("About", &enabled))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Name:"); ImGui::SameLine();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), TITLE);
	ImGui::Separator();
	
	ImGui::Text("Description:"); ImGui::SameLine();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), ENGINE_DESCRIPTION);
	ImGui::Separator();
	
	if (ImGui::SmallButton("Github Repository")) {
		ShellExecuteA(NULL, "open", REPOSITORY, NULL, NULL, SW_SHOWNORMAL);
	};
	ImGui::Separator();
	
	ImGui::Text("Author:"); ImGui::SameLine();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), AUTHOR);
	ImGui::Separator();
	
	SDL_version sdlVersion;
	SDL_GetVersion(&sdlVersion);

	if (ImGui::TreeNode("Libraries"))
	{
		ImGui::BulletText("Assimp (v%d.%d.%d)", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
		ImGui::BulletText("Brofiler (v1.1.2)");
		ImGui::BulletText("DevIL (v%d)", IL_VERSION);
		ImGui::BulletText("glew (v%s)", glewGetString(GLEW_VERSION));
		ImGui::BulletText("Imgui (v%s)", ImGui::GetVersion());
		ImGui::BulletText("Imguizmo");
		ImGui::BulletText("MathGeoLib (v1.5)");
		ImGui::BulletText("ParShapes");
		ImGui::BulletText("Pcg (v0.98)");
		ImGui::BulletText("Physfs (v%d.%d.%d)", PHYSFS_VER_MAJOR, PHYSFS_VER_MINOR, PHYSFS_VER_PATCH);
		ImGui::BulletText("RapidJson (v%d)", RAPIDJSON_VERSION_STRING);
		ImGui::BulletText("SDL (v%d.%d.%d)", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);

		ImGui::TreePop();
	}
	ImGui::Separator();
	if (ImGui::SmallButton("License")) {
		ShellExecuteA(NULL, "open", LICENSE, NULL, NULL, SW_SHOWNORMAL);
	};
	ImGui::Separator();
	ImGui::End();
}
