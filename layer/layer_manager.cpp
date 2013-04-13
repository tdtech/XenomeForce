#include "layer_manager.h"

CLayerManager* CLayerManager::mInstance = NULL;

void CLayerManager::init(HWND hwnd, HINSTANCE instance, TDisplayMode& displayMode)
{
	if (mInstance == NULL) mInstance = new CLayerManager(hwnd, instance, displayMode);
}

void CLayerManager::release()
{
	delete mInstance;
	mInstance = NULL;
}

CLayerManager::CLayerManager(HWND hwnd, HINSTANCE instance, TDisplayMode& displayMode)
{
	mSoundLayer = obtainDirectXSoundLayer(hwnd);
	mSoundLayer->init();

	mInputLayer = obtainDirectXInputLayer(hwnd, instance);
	mInputLayer->init();

	mRenderLayer = obtainDirectXRenderLayer(hwnd);
	mRenderLayer->validateDeviceCapabilities(0, ERasterizationTypeHardware);
	mRenderLayer->initDevice(/* TODO: pass the choosen adapter, not default */ 0, ERasterizationTypeHardware, displayMode);

	mPhysicsLayer = obtainBox2DPhysicsLayer();
}

CLayerManager::~CLayerManager()
{
	mRenderLayer->release();
	mInputLayer->release();
	mSoundLayer->release();
	mPhysicsLayer->release();
}
