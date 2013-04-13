#pragma once

#include "oneshot_sprite.h"

enum THumanAction
{
	 EHumanActionHandgunStay = 0
	,EHumanActionHandgunWalk
	,EHumanActionHandgunRightStrafe
	,EHumanActionHandgunLeftStrafe
	//,EHumanActionHandgunReload // TODO: implement handgun reload
	,EHumanActionRifleStay
	,EHumanActionRifleWalk
	,EHumanActionRifleRightStrafe
	,EHumanActionRifleLeftStrafe
	,EHumanActionDeath
	// add more actions...
	,EHumanActionCount // always should be the last
};

class CHumanModel: public COneShotSprite
{
public:
	CHumanModel(CTextureContent* content, int frameSize, int delimiterWidth = 0);
	~CHumanModel();

	void setAction(int action);
	int getAction();

	TSpriteDepth getDefaultDepth();

private:
	int mHumanAction;
};