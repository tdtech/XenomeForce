#include "cursor_sprite.h"

CCursorSprite::CCursorSprite():
CBaseSprite((CTextureContent*)CContentProvider::instance()->querySecondaryContent(ETextureContentCursor), 0, 0, 8)
{
	mSpriteDef.mTexelToPixel = true;
}

CCursorSprite::~CCursorSprite() {}

TSpriteDepth CCursorSprite::getDefaultDepth()
{
	return ESpriteDepthGUI;
}