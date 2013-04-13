#include "..\content\content_provider.h"
#include "..\layer\layer_manager.h"
#include "..\world\world.h"
#include "spot_light.h"

CSpotLight::CSpotLight(float radius):
mRadius(radius),
mEnabled(true)
{
	mContent = (CTextureContent*)CContentProvider::instance()->querySecondaryContent(ETextureContentSpotLights);

	mSpriteDef.mWidth = mRadius * 2;
	mSpriteDef.mHeight = mSpriteDef.mWidth;
	mSpriteDef.mAngle = 0.0f;
	mSpriteDef.mTexelToPixel = false;
	setColor(DEFAULT_SPOT_LIGHT_BRIGHTNESS, 255, 255, 255);

	mSpriteDef.mTextureRect.mX0 = 0;
	mSpriteDef.mTextureRect.mY0 = 0;
	mSpriteDef.mTextureRect.mX1 = 65;
	mSpriteDef.mTextureRect.mY1 = 65;
}

CSpotLight::~CSpotLight()
{
	delete mContent;
}

void CSpotLight::setColor(int brightness, int red, int green, int blue)
{
	mSpriteDef.mDiffuse = CLayerManager::instance()->getRenderLayer()->ARGB(brightness, red, green, blue);
}

void CSpotLight::animate()
{
	if (mEnabled) doAnimate();
}

void CSpotLight::draw()
{
	if (mEnabled) doDraw();
}

void CSpotLight::doAnimate() {}

void CSpotLight::doDraw()
{
	if (CWorld::instance()->getViewPort()->inView(mPosition, mRadius))
	{
		mSpriteDef.mLeftTop = mPosition - CWorld::instance()->getViewPort()->getLeftTop() - mRadius;
		CLayerManager::instance()->getRenderLayer()->drawSprite(mContent->getContent(), mSpriteDef);
	}
}