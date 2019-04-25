#include "PanelConfiguration.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include "ModuleCamera.h"
#include "ModuleInput.h"
#include "ModuleTextures.h"
#include "ModuleScene.h"
#include "ModuleUI.h"
#include "ModuleAudioManager.h"
#include "imgui.h"
#include "Math/MathConstants.h"

#define NUMFPS 100

PanelConfiguration::PanelConfiguration()
{
	fps.reserve(NUMFPS);
	ms.reserve(NUMFPS);
}


PanelConfiguration::~PanelConfiguration()
{
}


void PanelConfiguration::Draw()
{
	if (!ImGui::Begin("Configuration", &enabled))
	{
		ImGui::End();
		return;
	}
	if (ImGui::CollapsingHeader("Application"))
	{
		UpdateExtrems();
		DrawFPSgraph();
		DrawMSgraph();
		DrawMemoryStats();
	}
	if (ImGui::CollapsingHeader("Window"))
	{
		App->window->DrawGUI();
	}
	if (ImGui::CollapsingHeader("Camera"))
	{
		App->camera->DrawGUI();
	}
	if (ImGui::CollapsingHeader("Render"))
	{
		App->renderer->DrawGUI();
	}
	if (ImGui::CollapsingHeader("Input"))
	{
		App->input->DrawGUI();
	}
	if (ImGui::CollapsingHeader("Textures"))
	{
		App->textures->DrawGUI();
	}
	if (ImGui::CollapsingHeader("Scene"))
	{
		App->scene->DrawGUI();
	}
	if (ImGui::CollapsingHeader("UI"))
	{
		App->ui->DrawGUI();
	}
	if (ImGui::CollapsingHeader("Audio"))
	{
		App->audioManager->DrawGUI();
	}
	ImGui::End();
}

void PanelConfiguration::DrawFPSgraph() const
{
	if (fps.size() == 0) return;

	char avg[32];
	char showmin[32];
	sprintf_s(avg, "%s%.2f", "avg:", totalfps / fps.size());
	sprintf_s(showmin, "%s%.2f", "min:", minfps);
	ImGui::PlotHistogram("FPS", &fps[0], fps.size(), 0, avg,  0.0f, 300.0f, ImVec2(0, 80));
	ImGui::Text(showmin);
}

void PanelConfiguration::DrawMSgraph() const
{
	if (ms.size() == 0) return;


	char avg[32];
	char showmax[32];
	sprintf_s(avg, "%s%.2f", "avg:", totalms / ms.size());
	sprintf_s(showmax, "%s%.2f", "max:", maxms);
	ImGui::PlotHistogram("MS", &ms[0], ms.size(), 0, avg, 0.0f, 20.0f, ImVec2(0, 80));
	ImGui::Text(showmax);
}

void PanelConfiguration::AddFps(float dt)
{
	if (dt == 0.0f) return;
	if (fps.size() == NUMFPS)
	{
		totalfps -= fps.back();
		fps.pop_back();
		totalms -= ms.back();
		ms.pop_back();
	}
	
	float newfps = 1 / dt;
	float newms = dt * 1000;
	fps.insert(fps.begin(), newfps);
	ms.insert(ms.begin(), newms);

	totalfps += newfps;
	totalms += newms;
}

void PanelConfiguration::UpdateExtrems()
{
	minfps = 1000.f;
	maxms = 0.f;
	for (unsigned i = 0; i < fps.size(); i++)
	{
		minfps = MIN(minfps, fps[i]);
		maxms = MAX(maxms, ms[i]);
	}
}

void PanelConfiguration::DrawMemoryStats() const
{
	//sMStats stats = m_getMemoryStatistics();
	//ImGui::Text("Total Reported Mem: %u", stats.totalReportedMemory);
	//ImGui::Text("Total Actual Mem: %u", stats.totalActualMemory);
	//ImGui::Text("Peak Reported Mem: %u", stats.peakReportedMemory);
	//ImGui::Text("Peak Actual Mem: %u", stats.peakActualMemory);
	//ImGui::Text("Accumulated Reported Mem: %u", stats.accumulatedReportedMemory);
	//ImGui::Text("Accumulated Actual Mem: %u", stats.accumulatedActualMemory);
	//ImGui::Text("Accumulated Alloc Unit Count: %u", stats.accumulatedAllocUnitCount);
	//ImGui::Text("Total Alloc Unit Count: %u", stats.totalAllocUnitCount);
	//ImGui::Text("Peak Alloc Unit Count: %u", stats.peakAllocUnitCount);
}


