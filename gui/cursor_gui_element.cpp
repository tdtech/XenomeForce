#include "..\world\world.h"
#include "cursor_gui_element.h"

#include "..\light\light_manager.h"

CCursorGUIElement::CCursorGUIElement()
{
	mCursorSprite = new CCursorSprite();
	mSpotLight = new CFadingSpotLight(512, 30, 0, 255);
	CLightManager::instance()->addLight(mSpotLight);
}

CCursorGUIElement::~CCursorGUIElement()
{
	CLightManager::instance()->removeLight(mSpotLight);
	delete mCursorSprite;
	delete mSpotLight;
}

CVector2D& CCursorGUIElement::getPosition()
{
	return mCursorSprite->getPosition();
}

void CCursorGUIElement::handleEvents()
{
	CWorld* world = CWorld::instance();
	mCursorSprite->animate();
	mCursorSprite->setPosition(world->getViewPort()->toWorld(world->getMouseController()->getPosition()));
	mSpotLight->setPosition(mCursorSprite->getPosition());
	if (world->getMouseController()->isButtonPressed(LEFT_MOUSE_BUTTON)) mSpotLight->reset();
}

void CCursorGUIElement::draw()
{
	mCursorSprite->draw();
}