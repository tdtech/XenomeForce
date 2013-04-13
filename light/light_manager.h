#pragma once

#include "..\common\collection.h"
#include "..\layer\render_layer.h"
#include "..\viewport\viewport.h"

class ILight
{
	friend class CLightManager;

public:
	virtual ~ILight() {}

	virtual void animate() = 0;
	virtual void draw() = 0;

private:
	int mId;
};

class ILightLayer;

class CLightManager
{
public:
	static void init(CViewPort* viewPort);
	static void release();
	static inline CLightManager* instance() { return mInstance; }

	void addLight(ILight* light);
	void removeLight(ILight* light);
	void animateLights();
	void drawLights();
	void releaseLights();

	void setBackgroundColor(unsigned long color);

private:
	CLightManager(CViewPort* viewPort);
	~CLightManager();

	CCollection<ILight>   mLights;

	ILightLayer*          mLightLayer;

	static CLightManager* mInstance;
};