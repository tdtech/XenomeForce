#include "..\content\content_provider.h"
#include "..\gui\gui_manager.h"
#include "..\light\light_manager.h"
#include "..\sprite\sprite_manager.h"
#include "..\world\world.h"
#include "world_state.h"

CWorldState::CWorldState() {}

CWorldState::~CWorldState() {}

void CWorldState::preProcess() {}

void CWorldState::process()
{
	CWorld::instance()->simulate();
}

void CWorldState::postProcess()
{
	CWorld::release();
	CLightManager::release();
	CGUIManager::release();
	CSpriteManager::instance()->releaseSprites();
	CContentProvider::instance()->releasePrimaryContent();
}