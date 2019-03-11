#include "PanelHardware.h"
#include "SDL_cpuinfo.h"
#include "GL/glew.h"
#include <string>
#include "imgui.h"

PanelHardware::PanelHardware()
{
	enabled = false;
}


PanelHardware::~PanelHardware()
{
}

void PanelHardware::Draw()
{
	if (!ImGui::Begin("Hardware", &enabled))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("CPU cores: %d", SDL_GetCPUCount());
	ImGui::Text("CPU cache line size : %d B", SDL_GetCPUCacheLineSize());
	ImGui::Separator();

	ImGui::Text("Total system RAM : %d MB", SDL_GetSystemRAM());
	ImGui::Separator();

	std::string extensions("Extensions: ");

	if (SDL_Has3DNow())
	{
		extensions += "3DNow, ";
	}
	if (SDL_HasAltiVec())
	{
		extensions += "AltiVec, ";
	}
	if (SDL_HasAVX())
	{
		extensions += "AVX, ";
	}
	if (SDL_HasAVX2())
	{
		extensions += "AVX2, ";
	}
	if (SDL_HasMMX())
	{
		extensions += "MMX, ";
	}
	if (SDL_HasRDTSC())
	{
		extensions += "RDTSC, ";
	}
	if (SDL_HasSSE())
	{
		extensions += "SSE, ";
	}
	if (SDL_HasSSE2())
	{
		extensions += "SSE2, ";
	}
	if (SDL_HasSSE3())
	{
		extensions += "SSE3, ";
	}
	if (SDL_HasSSE41())
	{
		extensions += "SSE4.1, ";
	}
	if (SDL_HasSSE42())
	{
		extensions += "SSE4.2, ";
	}
	ImGui::Text(extensions.c_str());
	ImGui::Separator();
	ImGui::Text("GPU: %s", glGetString(GL_RENDERER));
	ImGui::Text("GPU company: %s", glGetString(GL_VENDOR));

	ImGui::End();
}
