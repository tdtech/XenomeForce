#include "..\layer\layer_manager.h"
#include "landscape_builder.h"

class CCommonLandscape: public ILandscape, public IRenderDeviceListener
{
public:
	CCommonLandscape(int width, int height, SURFACE_REF surface);
	~CCommonLandscape();

	int getWidth();
	int getHeight();
	void draw(TRect& rect);

	void beginUpdate();
	void endUpdate();

	// IRenderDeviceListener implementation
	void onRenderDeviceLost();
	void onRenderDeviceReset();

private:
	int         mWidth;
	int         mHeight;

	CVector2D   mOldSpriteScale;

	SURFACE_REF mSurface;
	SURFACE_REF mBackup;

	int         mRenderDeviceListenerId;
};

CCommonLandscape::CCommonLandscape(int width, int height, SURFACE_REF surface):
mWidth(width),
mHeight(height),
mSurface(surface)
{
	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	// register to receive render device events
	mRenderDeviceListenerId = render->addDeviceListener(this);

	render->incSurfaceRef(mSurface);
	render->createSurface(mWidth, mHeight, RENDER_FLAG_SYSTEMMEM, mBackup);

	render->drawSurface(mSurface, mBackup);
}

CCommonLandscape::~CCommonLandscape()
{
	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	render->releaseSurface(mSurface);
	render->releaseSurface(mBackup);

	// deregister from receiving of render device events
	render->removeDeviceListener(mRenderDeviceListenerId);
}

int CCommonLandscape::getWidth()
{
	return mWidth;
}

int CCommonLandscape::getHeight()
{
	return mHeight;
}

void CCommonLandscape::draw(TRect& rect)
{
	CLayerManager::instance()->getRenderLayer()->drawSurface(mSurface, rect);
}

void CCommonLandscape::beginUpdate()
{
	CVector2D spriteScale(1.0f, 1.0f);

	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	render->getSpriteScale(mOldSpriteScale);
	render->setSpriteScale(spriteScale);
	render->setRenderTarget(mSurface);
}

void CCommonLandscape::endUpdate()
{
	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	render->setSpriteScale(mOldSpriteScale);
	render->setDefaultRenderTarget();

	render->drawSurface(mSurface, mBackup);
}

void CCommonLandscape::onRenderDeviceLost()
{
	CLayerManager::instance()->getRenderLayer()->releaseSurface(mSurface);
	mSurface = MAX_SURFACES;
}

void CCommonLandscape::onRenderDeviceReset()
{
	IRenderLayer* render = CLayerManager::instance()->getRenderLayer();

	render->createSurface(mWidth, mHeight, RENDER_FLAG_VIDEOMEM | RENDER_FLAG_RENDERTARGET, mSurface);
	render->drawSurface(mBackup, mSurface);
}

//---------------------------------------------------

CLandscapeBuilder::CLandscapeBuilder(int width, int height):
mWidth(width),
mHeight(height)
{
	CLayerManager::instance()->getRenderLayer()->createSurface(mWidth, mHeight,
	        RENDER_FLAG_VIDEOMEM | RENDER_FLAG_RENDERTARGET, mLandscapeSurface);
}

CLandscapeBuilder::~CLandscapeBuilder()
{
	CLayerManager::instance()->getRenderLayer()->releaseSurface(mLandscapeSurface);
}

int CLandscapeBuilder::addTile(SURFACE_REF tile, int posX, int posY)
{
	return CLayerManager::instance()->getRenderLayer()->drawSurface(tile, mLandscapeSurface, posX, posY);
}

ILandscape* CLandscapeBuilder::buildLandscape()
{
	return new CCommonLandscape(mWidth, mHeight, mLandscapeSurface);
}