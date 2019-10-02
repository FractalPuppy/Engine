#ifndef  __DamageFeedbackUI_h__
#define  __DamageFeedbackUI_h__

#include "BaseScript.h"
#include "ComponentImage.h"
#include "Timer.h"

#ifdef DamageFeedbackUI_EXPORTS
#define DamageFeedbackUI_API __declspec(dllexport)
#else
#define DamageFeedbackUI_API __declspec(dllimport)
#endif

class DamageFeedbackUI_API DamageFeedbackUI : public Script
{
public:
	void Awake() override;
	void Update() override;
	void Expose(ImGuiContext* context) override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;
	inline virtual DamageFeedbackUI* Clone() const
	{
		return new DamageFeedbackUI(*this);
	}

	void ActivateDamageUI();


private:
	bool isDamaged = false;
	enum FADE
	{
		IN,
		OUT
	};
	FADE fade = IN;
	float timer = 0.0f;
	float fadeInTime = 0.2f;
	float fadeOutTime = 0.5f;
	ComponentImage* image = nullptr;
};

extern "C" DamageFeedbackUI_API Script* CreateScript();

#endif __DamageFeedbackUI_h__
