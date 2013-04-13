#pragma once

#include "oneshot_sprite.h"

enum TSpiderAction
{
	 ESpiderActionStay = 0
	,ESpiderActionWalk
	,ESpiderActionWebAttack
	,ESpiderActionSoftDeath
	,ESpiderActionCruelDeath
	// add more actions...
	,ESpiderActionCount // always should be the last
};

class CSpiderModel: public COneShotSprite
{
public:
	CSpiderModel();
	~CSpiderModel();

	void setAction(int action);
	int getAction();

	TSpriteDepth getDefaultDepth();

protected:
	void doAnimate();

private:
	int mSpiderAction;
};