#ifndef  __AudioMusicTransition_h__
#define  __AudioMusicTransition_h__

#include "BaseScript.h"

#ifdef AudioMusicTransition_EXPORTS
#define AudioMusicTransition_API __declspec(dllexport)
#else
#define AudioMusicTransition_API __declspec(dllimport)
#endif

class AudioMusicTransition_API AudioMusicTransition : public Script
{
};

#endif __AudioMusicTransition_h__
