#include "..\layer\layer_manager.h"
#include "gui_manager.h"

#define GUIELEMENTS_INITIAL_CAPACITY 32

CGUIManager* CGUIManager::mInstance = 0;

void CGUIManager::init()
{
	if (mInstance == 0) mInstance = new CGUIManager();
}

void CGUIManager::release()
{
	delete mInstance;
	mInstance = 0;
}

CGUIManager::CGUIManager():
mGUIElements(GUIELEMENTS_INITIAL_CAPACITY)
{}

CGUIManager::~CGUIManager()
{
	releaseGUIElements();
}

void CGUIManager::addGUIElement(IGUIElement* element)
{
	element->mId = mGUIElements.add(element);
}

void CGUIManager::removeGUIElement(IGUIElement* element)
{
	mGUIElements.remove(element->mId);
}

void CGUIManager::handleEvents()
{
	IGUIElement* element;
	mGUIElements.reset();
	while ((element = mGUIElements.next()) != 0) element->handleEvents();
}

void CGUIManager::drawGUIElements()
{
	CLayerManager::instance()->getRenderLayer()->beginSprite();

	IGUIElement* element;
	mGUIElements.reset();
	while ((element = mGUIElements.next()) != 0) element->draw();

	CLayerManager::instance()->getRenderLayer()->endSprite();
}

void CGUIManager::releaseGUIElements()
{
	IGUIElement* element;
	mGUIElements.reset();
	while ((element = mGUIElements.next()) != 0) delete element;
	mGUIElements.clear();
}