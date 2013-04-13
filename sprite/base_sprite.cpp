#include "..\world\world.h"
#include "base_sprite.h"

#define FRAME_DELAY 4 // TODO: find the more appropriate value

CBaseSprite::CBaseSprite(CTextureContent* content, int offsetU, int offsetV, int frameSize, int delimiterWidth):
mOffsetU(offsetU),
mOffsetV(offsetV),
mFrameSize(frameSize),
mEnabled(true),
mContent(content)
{
	mFrameOffset = mFrameSize + delimiterWidth;

	// Fill Sprite Definition
	mSpriteDef.mWidth = mSpriteDef.mHeight = (float)mFrameSize;
	mSpriteDef.mAngle = 0.0f;
	mSpriteDef.mDiffuse = CLayerManager::instance()->getRenderLayer()->ARGB(255, 255, 255, 255);
	mSpriteDef.mTexelToPixel = false;

	mRadius = mFrameSize / 2.0f;

	setAnimation(0, 1);
	setAnimationLine(0);
}

CBaseSprite::~CBaseSprite()
{
	delete mContent;
}

void CBaseSprite::animate()
{
	if (mEnabled) doAnimate();
}

void CBaseSprite::draw()
{
	if (mEnabled) doDraw();
}

void CBaseSprite::reset()
{
	mCurrentOffset = 0;

	mSpriteDef.mTextureRect.mX0 = mOffsetU + mStartOffset;
	mSpriteDef.mTextureRect.mX1 = mSpriteDef.mTextureRect.mX0 + mFrameSize;

	mDelay = 0;
}

TSpriteDepth CBaseSprite::getDefaultDepth()
{
	return ESpriteDepthModels; // by default sprite has lowest depth
}

void CBaseSprite::setAnimation(int startFrame, int frameCount)
{
	mStartOffset = startFrame * mFrameOffset;
	mCurrentOffset = 0;
	mOffsetTreshold = frameCount * mFrameOffset;
}

void CBaseSprite::setAnimationLine(int line)
{
	mSpriteDef.mTextureRect.mY0 = mOffsetV + mFrameOffset * line;
	mSpriteDef.mTextureRect.mY1 = mSpriteDef.mTextureRect.mY0 + mFrameSize;

	reset();
}

void CBaseSprite::doAnimate()
{
	if (++mDelay > FRAME_DELAY)
	{
		mCurrentOffset = (mCurrentOffset + mFrameOffset) % mOffsetTreshold;

		mSpriteDef.mTextureRect.mX0 = mOffsetU + mStartOffset + mCurrentOffset;
		mSpriteDef.mTextureRect.mX1 = mSpriteDef.mTextureRect.mX0 + mFrameSize;

		mDelay = 0;
	}
}

void CBaseSprite::doDraw()
{
	if (CWorld::instance()->getViewPort()->inView(mPosition, mRadius))
	{
		mSpriteDef.mLeftTop = mPosition - CWorld::instance()->getViewPort()->getLeftTop() - mRadius;
		CLayerManager::instance()->getRenderLayer()->drawSprite(mContent->getContent(), mSpriteDef);
	}
}