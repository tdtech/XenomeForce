#pragma once

#include "base_sprite.h"

class COneShotSprite: public CBaseSprite
{
public:
	COneShotSprite(CTextureContent* content, int offsetU, int offsetV, int frameSize, int delimiterWidth = 0);
	virtual ~COneShotSprite();

	virtual void reset();

	inline void setOneShot(bool oneshot) { mOneShot = oneshot; }

protected:
	virtual void doAnimate();
	virtual void doDraw();

protected:
	bool mIsAnimationCompleted;

private:
	bool mOneShot;
};