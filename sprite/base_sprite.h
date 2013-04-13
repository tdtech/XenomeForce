#pragma once

#include "..\content\texture_content.h"
#include "sprite_depth.h"
#include "sprite_manager.h"

class CBaseSprite: public ISprite
{
public:
	CBaseSprite(CTextureContent* content, int offsetU, int offsetV, int frameSize, int delimiterWidth = 0);
	virtual ~CBaseSprite();

	void animate();
	void draw();

	virtual void reset();
	virtual TSpriteDepth getDefaultDepth(); // each particular sprite should override this method

	inline void setPosition(CVector2D& pos) { mPosition = pos; }
	inline CVector2D& getPosition() { return mPosition; }
	inline void setAngle(float angle) { mSpriteDef.mAngle = angle; }
	inline float getAngle() { return mSpriteDef.mAngle; }
	inline void setDiffuse(unsigned long diffuse) { mSpriteDef.mDiffuse = diffuse; }

	inline void setEnabled(bool enabled) { mEnabled = enabled; }
	inline bool isEnabled() { return mEnabled; }

protected:
	void setAnimation(int startFrame, int frameCount);
	void setAnimationLine(int line);

	virtual void doAnimate();
	virtual void doDraw();

protected:
	TSpriteDef       mSpriteDef;
	int              mDelay; // made protected for oneshot sprites
	long             mCurrentOffset; // made protected for oneshot sprites

private:
	CVector2D        mPosition;
	int              mOffsetU;
	int              mOffsetV;
	long             mStartOffset;
	long             mOffsetTreshold;
	int              mFrameSize;
	long             mFrameOffset;
	float            mRadius;
	bool             mEnabled;

	CTextureContent* mContent;
};