#pragma once

#include "oneshot_sprite.h"

enum TSpiderWebAction
{
	 ESpiderWebActionInlace = 0
	,ESpiderWebActionFly
	,ESpiderWebActionDeath
	// add more action...
	,ESpiderWebActionCount // always should be the last
};

class CSpiderWeb: public COneShotSprite
{
public:
	CSpiderWeb();
	~CSpiderWeb();

	void setAction(int action);
	int getAction();

	TSpriteDepth getDefaultDepth();

protected:
	void doAnimate();

private:
	int mSpiderWebAction;
};