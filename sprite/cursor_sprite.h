#pragma once

#include "base_sprite.h"

class CCursorSprite: public CBaseSprite
{
public:
	CCursorSprite();
	~CCursorSprite();

	TSpriteDepth getDefaultDepth();
};