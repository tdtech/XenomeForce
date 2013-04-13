#pragma once

#include <windows.h>

#include "render_layer.h"
#include "sound_layer.h"
#include "input_layer.h"
#include "physics_layer.h"

class CLayerManager
{
public:
	static void init(HWND hwnd, HINSTANCE instance, TDisplayMode& displayMode);
	static void release();
	static inline CLayerManager* instance() { return mInstance; }

	inline IRenderLayer*  getRenderLayer() { return mRenderLayer; }
	inline ISoundLayer*   getSoundLayer() { return mSoundLayer; }
	inline IInputLayer*   getInputLayer() { return mInputLayer; }
	inline IPhysicsLayer* getPhysicsLayer() { return mPhysicsLayer; }

private:
	CLayerManager(HWND hwnd, HINSTANCE instance, TDisplayMode& displayMode);
	~CLayerManager();

	IRenderLayer*         mRenderLayer;
	ISoundLayer*          mSoundLayer;
	IInputLayer*          mInputLayer;
	IPhysicsLayer*        mPhysicsLayer;

	static CLayerManager* mInstance;
};