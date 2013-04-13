#include "..\layer\layer_manager.h"
#include "flickering_spot_light.h"

CFlickeringSpotLight::CFlickeringSpotLight(float radius, int amplitude):
CSpotLight(radius),
mAmplitude(amplitude + 1)
{
	setColor(DEFAULT_SPOT_LIGHT_BRIGHTNESS, 255, 255, 255);
}

CFlickeringSpotLight::~CFlickeringSpotLight() {}

void CFlickeringSpotLight::setColor(int brightness, int red, int green, int blue)
{
	mBrightnessOrigin = brightness - (mAmplitude >> 1);

	if (mBrightnessOrigin < 0)
	{
		mAmplitude += mBrightnessOrigin;
		mBrightnessOrigin = 0;
	}

	if (mBrightnessOrigin + mAmplitude > 255)
	{
		mAmplitude = 255 - mBrightnessOrigin;
	}

	mRed = red;
	mGreen = green;
	mBlue = blue;

	mSpriteDef.mDiffuse = CLayerManager::instance()->getRenderLayer()->ARGB(brightness, red, green, blue);
}

void CFlickeringSpotLight::doAnimate()
{
	// TODO: maybe do nothing if amplitude is 1 ?
	mSpriteDef.mDiffuse = CLayerManager::instance()->getRenderLayer()->ARGB(mBrightnessOrigin + rand() % mAmplitude, mRed, mGreen, mBlue);
}