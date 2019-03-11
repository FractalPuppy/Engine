#include "PanelResourcePreview.h"

#include "ModuleProgram.h"
#include "Viewport.h"

#include "GL/glew.h"
#include "imgui.h"

PanelResourcePreview::PanelResourcePreview()
{
	preview = new Viewport("Preview");
}

PanelResourcePreview::~PanelResourcePreview()
{
	delete preview;
}

void PanelResourcePreview::Draw()
{
	if (!ImGui::Begin("About", &enabled))
	{
		ImGui::End();
		return;
	}

	ImVec2 region_size = ImGui::GetContentRegionAvail();

	ImGui::Image((ImTextureID)preview->texture, region_size);

	ImGui::End();
}

void PanelResourcePreview::SetResource(unsigned uid)
{
}

void PanelResourcePreview::ReleaseResource()
{
}
