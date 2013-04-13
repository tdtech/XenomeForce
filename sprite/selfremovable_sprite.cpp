#include "selfremovable_sprite.h"

CSelfRemovableSprite::CSelfRemovableSprite(CTextureContent* content, int offsetU, int offsetV, int frameSize, int delimiterWidth):
COneShotSprite(content, offsetU, offsetV, frameSize, delimiterWidth),
mSelfRemovable(true)
{}

CSelfRemovableSprite::~CSelfRemovableSprite() {}

void CSelfRemovableSprite::doAnimate()
{
	COneShotSprite::doAnimate();

	if (mIsAnimationCompleted && mSelfRemovable)
	{
		CSpriteManager::instance()->removeSprite(this);
		delete this; // TODO: maybe end-user sprites will delete themselves if they want?
	}
}