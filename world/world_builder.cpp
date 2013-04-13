#include <stdio.h>
#include <string.h>

#include "..\landscape\landscape_builder.h"
#include "..\viewport\viewport.h"
#include "..\content\content_builder.h"
#include "..\content\content_provider.h"
#include "world.h"
#include "world_builder.h"

#include "..\layer\layer_manager.h"
#include "player_entity.h"
#include "..\log\logger.h"

CWorldBuilder::CWorldBuilder(const char* fileName, IWorldBuilderListener* listener):
mListener(listener),
mIsBuilding(false)
{
	mFileName = new char[strlen(fileName) + 1];
	strcpy(mFileName, fileName);
}

CWorldBuilder::~CWorldBuilder()
{
	delete [] mFileName;
}

bool CWorldBuilder::buildWorld()
{
	if (!mIsBuilding) {
		mIsBuilding = true;
		CreateThread(NULL, 65536, buildWorldThreadFunc, this, 0, NULL);
		return true;
	}
	return false;
}

bool CWorldBuilder::isBuilding()
{
	return mIsBuilding;
}

void CWorldBuilder::notifyProgress(int progress)
{
	if (mListener) mListener->onProgress(progress);
}

DWORD CWorldBuilder::buildWorldThreadFunc(void* data)
{
	CWorldBuilder* worldBuilder = (CWorldBuilder*)data;
	worldBuilder->notifyProgress(0);

	// TODO: implement building of world from file

	// <=== THE CODE BELOW IS JUST FOR TEST PURPOSES, IT WILL BE REMOVED LATER ===>
	CLandscapeBuilder landscapeBuilder(2048, 2048);
	// add tiles to landscape builder...
	SURFACE_REF tile;
	TRect srcRect;
	srcRect.mX0 = srcRect.mY0 = 0;
	srcRect.mX1 = srcRect.mY1 = 256;
	CLayerManager::instance()->getRenderLayer()->createSurfaceFromFile("tiles\\sand01.jpg", 256, 256, srcRect, RENDER_FLAG_SYSTEMMEM, tile);
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			landscapeBuilder.addTile(tile, i * 256, j * 256);
	CLayerManager::instance()->getRenderLayer()->releaseSurface(tile);

	ILandscape* landscape = landscapeBuilder.buildLandscape();

	TRect viewBound;
	viewBound.mX0 = viewBound.mY0 = 128;
	viewBound.mX1 = landscape->getWidth() - 128;
	viewBound.mY1 = landscape->getHeight() - 128;

	CViewPort* viewPort = new CViewPort(960, 720, viewBound, landscape);
	CGUIManager::init();
	CLightManager::init(viewPort);
	CWorld::init(viewPort);

	// create primary content
	CContentBuilder* contentBuilder = new CContentBuilder();
	contentBuilder->addContent(ETextureContentCursor);
	contentBuilder->addContent(ETextureContentSpotLights);
	contentBuilder->addContent(ETextureContentHuman01);
	contentBuilder->addContent(ETextureContentDefenderHandgun);
	contentBuilder->buildContent();
	delete contentBuilder;

	// create entities, events and etc...
	CVector2D chain[4];
	chain[0].mX = viewBound.mX0;
	chain[0].mY = viewBound.mY0;
	chain[1].mX = viewBound.mX1;
	chain[1].mY = viewBound.mY0;
	chain[2].mX = viewBound.mX1;
	chain[2].mY = viewBound.mY1;
	chain[3].mX = viewBound.mX0;
	chain[3].mY = viewBound.mY1;
	TBodyDef chainDef;
	chainDef.mFriction = 0.3f;
	BODY_REF chainRef;
	CLayerManager::instance()->getPhysicsLayer()->createChainBody(chainDef, chain, 4, true, 0, chainRef);

	TVertex vertices[3];
	vertices[0].mX = 800.0f;
	vertices[0].mY = 800.0f;
	vertices[0].mU = 0.0f;
	vertices[0].mV = 0.0f;
	vertices[1].mX = 1056.0f;
	vertices[1].mY = 624.0f;
	vertices[1].mU = 1.0f;
	vertices[1].mV = 0.0f;
	vertices[2].mX = 936.0f;
	vertices[2].mY = 768.0f;
	vertices[2].mU = 1.0f;
	vertices[2].mV = 1.0f;
	TEXTURE_REF wall;
	unsigned int width, height;
	CLayerManager::instance()->getRenderLayer()->createTextureFromFile("tiles\\rock01.jpg", 0, RENDER_FLAG_VIDEOMEM, width, height, wall);
	CLayerManager::instance()->getRenderLayer()->beginScene();
	landscape->beginUpdate();
	CLayerManager::instance()->getRenderLayer()->beginPrimitive();
	CLayerManager::instance()->getRenderLayer()->drawPolygon(wall, vertices, 3);
	CLayerManager::instance()->getRenderLayer()->endPrimitive();
	landscape->endUpdate();
	CLayerManager::instance()->getRenderLayer()->endScene();
	CLayerManager::instance()->getRenderLayer()->releaseTexture(wall);

	chain[0].mX = 800.0f;
	chain[0].mY = 800.0f;
	chain[1].mX = 1056.0f;
	chain[1].mY = 624.0f;
	chain[2].mX = 936.0f;
	chain[2].mY = 768.0f;
	CLayerManager::instance()->getPhysicsLayer()->createPolygonBody(chainDef, chain, 3, 0, chainRef);

	CVector2D pos(512.0f, 512.0f);
	CPlayerEntity* player = new CPlayerEntity();
	player->registerToWorld(pos);
	CWorld::instance()->setActive(player, true);

	worldBuilder->notifyProgress(100);
	worldBuilder->mIsBuilding = false;

	return 0;
}