#include <stdio.h>

#include "..\content\content_builder.h"
#include "..\layer\layer_manager.h"
#include "..\sprite\sprite_manager.h"
#include "..\sprite\sprite_depth.h"
#include "state_factory.h"
#include "world_state.h"
#include "build_world_state.h"

CBuildWorldState::CBuildWorldState():
mWorldBuilder(0)
{}

CBuildWorldState::~CBuildWorldState()
{
	delete mWorldBuilder;
}

void CBuildWorldState::preProcess()
{
	CContentProvider::init();
	CSpriteManager::init(ESpriteDepthCount);

	CLayerManager::instance()->getRenderLayer()->setFont(8, 12, "Arial");
	mWorldBuilder = new CWorldBuilder("test_world", this); // TODO: obtain world filename from global settings
	mWorldBuilder->buildWorld();
}

void CBuildWorldState::process()
{
	if (!mWorldBuilder->isBuilding())
	{
		CStateManager::instance()->setState(CStateFactory::instance()->getWorldState());
	}
}

void CBuildWorldState::postProcess()
{
	delete mWorldBuilder;
	mWorldBuilder = 0;
}

void CBuildWorldState::onProgress(int progress)
{
	static TRect textRect;
	textRect.mX0 = textRect.mY0 = 0;
	textRect.mX1 = 512;
	textRect.mY1 = 16;
	sprintf(mProgressText, "loading world... %d%% completed", progress);

	CLayerManager::instance()->getRenderLayer()->beginScene();
	CLayerManager::instance()->getRenderLayer()->beginText();
	CLayerManager::instance()->getRenderLayer()->outText(mProgressText, textRect, CLayerManager::instance()->getRenderLayer()->ARGB(255, 0, 255, 0));
	CLayerManager::instance()->getRenderLayer()->endText();
	CLayerManager::instance()->getRenderLayer()->endScene();
	CLayerManager::instance()->getRenderLayer()->presentScene();
}