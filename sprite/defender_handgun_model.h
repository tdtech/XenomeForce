#pragma once

#include "oneshot_sprite.h"

enum TDefenderHandgunAction
{
	 EDefenderHandgunActionStay
	,EDefenderHandgunActionShoot
	,EDefenderHandgunActionDeath
	// add more actions...
	,EDefenderHandgunActionCount // always should be the last
};

class CDefenderHandgunModel: public COneShotSprite
{
public:
	CDefenderHandgunModel();
	~CDefenderHandgunModel();

	void setAction(int action);
	int getAction();

	TSpriteDepth getDefaultDepth();

protected:
	void doAnimate();

private:
	int mDefenderAction;
};