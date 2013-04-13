#include "..\layer\layer_manager.h"
#include "fading_spot_light.h"

CFadingSpotLight::CFadingSpotLight(float radius, int amplitude, int fadeOutThreshold, int fadeInThreshold):
CFlickeringSpotLight(radius, amplitude),
mFadeStep(0),
mFadeOutThreshold(fadeOutThreshold),
mFadeInThreshold(fadeInThreshold),
mFading(false)
{
	setColor(DEFAULT_SPOT_LIGHT_BRIGHTNESS, 255, 255, 255);
}

CFadingSpotLight::~CFadingSpotLight() {}

void CFadingSpotLight::setColor(int brightness, int red, int green, int blue)
{
	CFlickeringSpotLight::setColor(brightness, red, green, blue);

	mBrightnessOrigin = brightness;
	mBrightness = (float)brightness;
}

void CFadingSpotLight::reset()
{
	setColor(mBrightnessOrigin, mRed, mGreen, mBlue);
}

void CFadingSpotLight::doAnimate()
{
	if (mFading)
	{
		if (mBrightness > mFadeOutThreshold && mBrightness < mFadeInThreshold)
		{
			mSpriteDef.mDiffuse = CLayerManager::instance()->getRenderLayer()->ARGB(xmath_round(mBrightness), mRed, mGreen, mBlue);
			mBrightness += mFadeStep;
		}
		else
		{
			int brightness;

			if (mBrightness < mFadeOutThreshold) brightness = mFadeOutThreshold;
			else brightness = mFadeInThreshold;

			CFlickeringSpotLight::setColor(brightness, mRed, mGreen, mBlue);

			setFading(false);
		}
	}
	else
	{
		CFlickeringSpotLight::doAnimate();
	}
}