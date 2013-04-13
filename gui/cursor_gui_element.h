#pragma once

#include "..\sprite\cursor_sprite.h"
#include "gui_manager.h"

#include "..\light\fading_spot_light.h"

class CCursorGUIElement: public IGUIElement
{
public:
	CCursorGUIElement();
	~CCursorGUIElement();

	CVector2D& getPosition();

	void handleEvents();
	void draw();

private:
	CCursorSprite* mCursorSprite;

	CFadingSpotLight* mSpotLight;
};