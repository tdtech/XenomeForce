#pragma once

#include "oneshot_sprite.h"

class CSelfRemovableSprite: public COneShotSprite
{
public:
	CSelfRemovableSprite(CTextureContent* content, int offsetU, int offsetV, int frameSize, int delimiterWidth = 0);
	virtual ~CSelfRemovableSprite();

	inline void setSelfRemovable(bool selfRemovable) { mSelfRemovable = selfRemovable; }

protected:
	virtual void doAnimate();

private:
	bool mSelfRemovable;
};