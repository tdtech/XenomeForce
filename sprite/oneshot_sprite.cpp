#include "oneshot_sprite.h"

COneShotSprite::COneShotSprite(CTextureContent* content, int offsetU, int offsetV, int frameSize, int delimiterWidth):
CBaseSprite(content, offsetU, offsetV, frameSize, delimiterWidth),
mOneShot(true)
{}

COneShotSprite::~COneShotSprite() {}

void COneShotSprite::reset()
{
	CBaseSprite::reset();
	mIsAnimationCompleted = false;
}

void COneShotSprite::doAnimate()
{
	if (!mIsAnimationCompleted)
	{
		CBaseSprite::doAnimate();
		mIsAnimationCompleted = (mOneShot && (mDelay == 0) && (mCurrentOffset == 0));
	}
}

void COneShotSprite::doDraw()
{
	// TODO: Decide, whether we need to draw sprite regardless of mIsAnimationCompleted flag or not?
	if (!mIsAnimationCompleted) CBaseSprite::doDraw();
}