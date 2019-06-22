#include "Application.h"

#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleUI.h"

#include "ComponentButton.h"
#include "ComponentImage.h"
#include "ComponentText.h"
#include "ComponentTransform2D.h"

#include "GameObject.h"
#include "Viewport.h"
#include "imgui.h"
#include "Math/float2.h"
#include "JSON.h"


Button::Button() : Component(nullptr, ComponentType::Button)
{
	text = new Text();
	buttonImage = new ComponentImage();
	highlightedImage = new ComponentImage();
	pressedImage = new ComponentImage();
	AssemblyButton();

	buttonImage->hoverDetectionMouse1 = false;
	buttonImage->hoverDetectionMouse3 = false;
	buttonImage->showHoverDetectInEditor = false;
	highlightedImage->hoverDetectionMouse1 = false;
	highlightedImage->hoverDetectionMouse3 = false;
	highlightedImage->showHoverDetectInEditor = false;
	pressedImage->hoverDetectionMouse1 = false;
	pressedImage->hoverDetectionMouse3 = false;
	pressedImage->showHoverDetectInEditor = false;
}

Button::Button(GameObject* gameobject) : Component(gameobject, ComponentType::Button)
{
	text = new Text();
	buttonImage = new ComponentImage();
	highlightedImage = new ComponentImage();
	pressedImage = new ComponentImage();
	AssemblyButton();
}

Button::Button(const Button& copy) : Component(copy)
{
	text = (Text*) copy.text->Clone();
	buttonImage = (ComponentImage*)copy.buttonImage->Clone();
	highlightedImage = (ComponentImage*)copy.highlightedImage->Clone();
	pressedImage = (ComponentImage*)copy.pressedImage->Clone();
	rectTransform = (Transform2D*)copy.rectTransform->Clone();
}


Button::~Button()
{
	RELEASE(text);
	RELEASE(buttonImage);
	RELEASE(highlightedImage);
	RELEASE(pressedImage);
}

Component* Button::Clone() const
{
	return new Button(*this);;
}

void Button::DrawProperties()
{
	ImGui::PushID(0);
	text->DrawProperties();
	ImGui::Text("Button image");
	ImGui::PushID(1);
	buttonImage->DrawProperties();	
	ImGui::Text("Highlighted image");
	ImGui::PushID(2);
	highlightedImage->DrawProperties();
	ImGui::Text("Pressed image");
	ImGui::PushID(3);
	pressedImage->DrawProperties();
	ImGui::Text("Button Hover Detection");
	ImGui::Checkbox("Hover Detection Mouse1", &hoverDetectionMouse1);
	ImGui::Checkbox("Hover Detection Mouse3", &hoverDetectionMouse3);
	ImGui::PopID();
	ImGui::PopID();
	ImGui::PopID();
	ImGui::PopID();
}

void Button::Save(JSON_value* value) const
{
	Component::Save(value);
	JSON_value* textValue = value->CreateValue(rapidjson::kObjectType);	
	text->Save(textValue);
	JSON_value* bValue = value->CreateValue(rapidjson::kObjectType);
	buttonImage->Save(bValue);
	JSON_value* hValue = value->CreateValue(rapidjson::kObjectType);
	highlightedImage->Save(hValue);
	JSON_value* pValue = value->CreateValue(rapidjson::kObjectType);
	pressedImage->Save(pValue);
	value->AddValue("text", *textValue);
	value->AddValue("buttonImage", *bValue);
	value->AddValue("highlightedImage", *hValue);
	value->AddValue("pressedImage", *pValue);
	value->AddInt("hoverDetectionMouse1", hoverDetectionMouse1);
	value->AddInt("hoverDetectionMouse3", hoverDetectionMouse3);
}

void Button::Load(JSON_value* value)
{
	RELEASE(text);
	RELEASE(buttonImage);
	RELEASE(highlightedImage);
	RELEASE(pressedImage);	
	Component::Load(value);
	text = new Text();
	text->Load(value->GetValue("text"));
	buttonImage = new ComponentImage();
	buttonImage->Load(value->GetValue("buttonImage"));
	buttonImage->showHoverDetectInEditor = false;
	highlightedImage = new ComponentImage();
	highlightedImage->Load(value->GetValue("highlightedImage"));
	highlightedImage->showHoverDetectInEditor = false;
	pressedImage = new ComponentImage();
	pressedImage->Load(value->GetValue("pressedImage"));	
	pressedImage->showHoverDetectInEditor = false;
	AssemblyButton();
	hoverDetectionMouse1 = value->GetInt("hoverDetectionMouse1", 1);
	hoverDetectionMouse3 = value->GetInt("hoverDetectionMouse3", 1);
}

void Button::Update()
{
	math::float2 mouse = reinterpret_cast<const float2&>(App->input->GetMousePosition());	
	float screenX = mouse.x - App->renderer->viewGame->winPos.x - (App->ui->currentWidth * .5f);
	float screenY = mouse.y - App->renderer->viewGame->winPos.y - (App->ui->currentHeight * .5f);
	math::float2 pos = rectTransform->getPosition();
	math::float2 size = rectTransform->getSize();
	float buttonX = pos.x;
	float buttonY = pos.y;
	
	math::float2 buttonMin = float2(buttonX - size.x *.5f, -buttonY - size.y *.5f);
	math::float2 buttonMax = float2(buttonX + size.x *.5f, -buttonY + size.y *.5f);
	if (screenX > buttonMin.x && screenX < buttonMax.x && screenY > buttonMin.y && screenY < buttonMax.y)
	{
		isHovered = true;
		
		if (hoverDetectionMouse1) App->ui->uiHoveredMouse1 = true;
		if (hoverDetectionMouse3) App->ui->uiHoveredMouse3 = true;

		buttonImage->enabled = false;
		highlightedImage->enabled = true;
		pressedImage->enabled = false;
		text->isHovered = true;
	}
	else
	{
		isHovered = false;
		buttonImage->enabled = true && !isSelected;
		highlightedImage->enabled = false || isSelected;
		pressedImage->enabled = false;
		text->isHovered = false;
	}

	if (isKeyDown && !isHovered)
	{
		isKeyDown = false;
		pressedImage->enabled = false;

	}

	isKeyUp = false;
	if (isHovered && isKeyDown && App->input->GetMouseButtonDown(1) == KEY_UP)
	{
		isKeyUp = true;
		isKeyDown = false;
		pressedImage->enabled = false;
	}

	if (isHovered && App->input->GetMouseButtonDown(1) == KEY_DOWN)
	{
		isKeyDown = true;
		buttonImage->enabled = false;
		highlightedImage->enabled = false;
		pressedImage->enabled = true;
	}
}

void Button::Enable(bool enable)
{
	Component::Enable(enable);
	isKeyDown = false;
	isKeyUp = false;
	isHovered = false;
	isSelected = false;
	highlightedImage->enabled = false;
	pressedImage->enabled = false;
	buttonImage->enabled = true;
}

void Button::AssemblyButton() 
{
	text->gameobject = gameobject;
	buttonImage->gameobject = gameobject;
	highlightedImage->gameobject = gameobject;
	pressedImage->gameobject = gameobject;
	rectTransform = (Transform2D*)gameobject->GetComponentOld(ComponentType::Transform2D);
	buttonImage->enabled = true;
	highlightedImage->enabled = false;
	pressedImage->enabled = false;
}

void Button::UpdateImageByName(std::string name)
{
	buttonImage->UpdateTexture(name);
	highlightedImage->UpdateTexture(name);
	pressedImage->UpdateTexture(name);
}

void Button::UpdateImageByResource(ResourceTexture* texture)
{
	buttonImage->texture = texture;
	highlightedImage->texture = texture;
	pressedImage->texture = texture;
}