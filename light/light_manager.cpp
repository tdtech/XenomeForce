#include "..\layer\layer_manager.h"
#include "light_manager.h"

#define LIGHTS_INITIAL_CAPACITY 64

class ILightLayer
{
public:
	virtual ~ILightLayer() {}

	virtual void setBackgroundColor(unsigned long color) = 0;

	virtual void begin() = 0;
	virtual void end() = 0;
};

//-----------DEFAULT LIGHT LAYER-----------

class CDefaultLightLayer: public ILightLayer, public IRenderDeviceListener
{
public:
	CDefaultLightLayer(CViewPort* viewPort, unsigned long maxTextureWidth, unsigned long maxTextureHeight);
	~CDefaultLightLayer();

	void setBackgroundColor(unsigned long color);

	void begin();
	void end();

	// IRenderDeviceListener implementation
	void onRenderDeviceLost();
	void onRenderDeviceReset();

private:
	unsigned long mMaxTextureWidth;
	unsigned long mMaxTextureHeight;

	unsigned long mBackgroundColor;
	TSpriteDef    mSpriteDef;

	CVector2D     mOldSpriteScale;
	CVector2D     mLightScale;
	CVector2D     mFinalScale;

	TEXTURE_REF   mTextureRef;
	SURFACE_REF   mSurfaceRef;

	int           mRenderDeviceListenerId;
};

CDefaultLightLayer::CDefaultLightLayer(CViewPort* viewPort, unsigned long maxTextureWidth, unsigned long maxTextureHeight):
mMaxTextureWidth(maxTextureWidth),
mMaxTextureHeight(maxTextureHeight),
mFinalScale(1.0f, 1.0f)
{
	unsigned long textureWidth, textureHeight;
	float aspectRatio = (float)viewPort->getWidth() / viewPort->getHeight();

	/* Light layer must has at least 1 pixel-size border like all sprite textures have
	   in order to avoid linear filtering issues during rasterization.
	   Also it must save aspect ratio. */
	if (maxTextureWidth < maxTextureHeight * aspectRatio)
	{
		textureWidth = (unsigned long)(maxTextureWidth - 1 * aspectRatio);
		textureHeight = (unsigned long)(maxTextureWidth / aspectRatio - 1);
	}
	else
	{
		textureWidth = (unsigned long)(maxTextureHeight * aspectRatio - 1);
		textureHeight = (unsigned long)(maxTextureHeight - 1 / aspectRatio);
	}

	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	// register to receive render device events
	mRenderDeviceListenerId = render->addDeviceListener(this);

	TDisplayMode displayMode;
	render->getDeviceDisplayMode(displayMode);

	// Fill Sprite Definition
	mSpriteDef.mLeftTop.mX = mSpriteDef.mLeftTop.mY = 0.0f;
	mSpriteDef.mWidth = (float)displayMode.mWidth;
	mSpriteDef.mHeight = (float)displayMode.mHeight;
	mSpriteDef.mAngle = 0.0f;
	mSpriteDef.mDiffuse = render->ARGB(255, 255, 255, 255);
	mSpriteDef.mTextureRect.mX0 = mSpriteDef.mTextureRect.mY0 = 0;
	mSpriteDef.mTextureRect.mX1 = (long)textureWidth;
	mSpriteDef.mTextureRect.mY1 = (long)textureHeight;
	mSpriteDef.mTexelToPixel = true;

	// Calculate lights scale
	mLightScale.mX = (float)textureWidth / viewPort->getWidth();
	mLightScale.mY = (float)textureHeight / viewPort->getHeight();

	render->createTexture(maxTextureWidth, maxTextureHeight, RENDER_FLAG_RENDERTARGET | RENDER_FLAG_VIDEOMEM, mTextureRef);
	render->getSurfaceFromTexture(mTextureRef, mSurfaceRef);

	mBackgroundColor = render->ARGB(235, 0, 0, 0);
	render->fillSurface(mSurfaceRef, mBackgroundColor);
}

CDefaultLightLayer::~CDefaultLightLayer()
{
	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	render->releaseSurface(mSurfaceRef);
	render->releaseTexture(mTextureRef);

	// deregister from receiving of render device events
	render->removeDeviceListener(mRenderDeviceListenerId);
}

void CDefaultLightLayer::setBackgroundColor(unsigned long color)
{
	mBackgroundColor = color;
}

void CDefaultLightLayer::begin()
{
	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	render->getSpriteScale(mOldSpriteScale);
	render->setSpriteScale(mLightScale);

	render->fillSurface(mSurfaceRef, mBackgroundColor);
	render->setRenderTarget(mSurfaceRef, mSpriteDef.mTextureRect);
}

void CDefaultLightLayer::end()
{
	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	render->setDefaultRenderTarget();
	render->setSpriteScale(mFinalScale);

	render->beginSprite();
	render->drawSprite(mTextureRef, mSpriteDef);
	render->endSprite();

	render->setSpriteScale(mOldSpriteScale);
}

void CDefaultLightLayer::onRenderDeviceLost()
{
	CLayerManager::instance()->getRenderLayer()->releaseSurface(mSurfaceRef);
	CLayerManager::instance()->getRenderLayer()->releaseTexture(mTextureRef);

	// fill refs with invalid values
	mSurfaceRef = MAX_SURFACES;
	mTextureRef = MAX_TEXTURES;
}

void CDefaultLightLayer::onRenderDeviceReset()
{
	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	render->createTexture(mMaxTextureWidth, mMaxTextureHeight, RENDER_FLAG_RENDERTARGET | RENDER_FLAG_VIDEOMEM, mTextureRef);
	render->getSurfaceFromTexture(mTextureRef, mSurfaceRef);
}

//--------------LIGHT MANAGER--------------

CLightManager* CLightManager::mInstance = 0;

void CLightManager::init(CViewPort* viewPort)
{
	if (mInstance == 0) mInstance = new CLightManager(viewPort);
}

void CLightManager::release()
{
	delete mInstance;
	mInstance = 0;
}

CLightManager::CLightManager(CViewPort* viewPort):
mLights(LIGHTS_INITIAL_CAPACITY)
{
	unsigned long maxTextureWidth, maxTextureHeight;
	// Retrieve maximum allowed texture dimensions
	CLayerManager::instance()->getRenderLayer()->getMaxTextureSize(maxTextureWidth, maxTextureHeight);

	unsigned long preferredTextureWidth, preferredTextureHeight;
	// Calculate preferred texture dimensions
	preferredTextureWidth = NEAREST_POW_2(viewPort->getWidth());
	preferredTextureHeight = NEAREST_POW_2(viewPort->getHeight());

	/* TODO: Seems there is no appreciable difference between good and poor quality layers
	   except video memory usage. So the question is: to use good quality layers or not to use?
	   Maybe even just use hard-coded 512x512 light layer dimensions? */
	if (maxTextureWidth < preferredTextureWidth || maxTextureHeight < preferredTextureHeight)
	{
		/* Poor quality layer allocates texture with maximum allowed
		   dimensions saving the aspect ratio of view port dimensions.
		   Lights will look worse than while using good quality layer
		   but this algorythm should work on the old video cards,
		   which does not support big texture size. */
		mLightLayer = new CDefaultLightLayer(viewPort, maxTextureWidth, maxTextureHeight);
	}
	else
	{
		/* Good quality layer uses big texture to render lights.
		   This texture almost fully covers the view port,
		   that's why to use such layer device MUST support pretty
		   big texture size. */
		mLightLayer = new CDefaultLightLayer(viewPort, preferredTextureWidth, preferredTextureHeight);
	}
}

CLightManager::~CLightManager()
{
	releaseLights();
	delete mLightLayer;
}

void CLightManager::addLight(ILight* light)
{
	light->mId = mLights.add(light);
}

void CLightManager::removeLight(ILight* light)
{
	mLights.remove(light->mId);
}

void CLightManager::animateLights()
{
	ILight* light;
	mLights.reset();
	while ((light = mLights.next()) != 0) light->animate();
}

void CLightManager::drawLights()
{
	mLightLayer->begin();
	CLayerManager::instance()->getRenderLayer()->beginSprite();

	ILight* light;
	mLights.reset();
	while ((light = mLights.next()) != 0) light->draw();

	CLayerManager::instance()->getRenderLayer()->endSprite();
	mLightLayer->end();
}

void CLightManager::releaseLights()
{
	ILight* light;
	mLights.reset();
	while ((light = mLights.next()) != 0) delete light;
	mLights.clear();
}

void CLightManager::setBackgroundColor(unsigned long color)
{
	mLightLayer->setBackgroundColor(color);
}