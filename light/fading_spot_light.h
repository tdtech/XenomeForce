#pragma once

#include "flickering_spot_light.h"

class CFadingSpotLight: public CFlickeringSpotLight
{
public:
	// 0 <= fadeOutThreshold <= fadeInThreshold <= 255
	CFadingSpotLight(float radius, int amplitude, int fadeOutThreshold, int fadeInThreshold);
	~CFadingSpotLight();

	void setColor(int brightness, int red, int green, int blue);

	inline void setFading(bool fading) { mFading = fading; }
	inline bool isFading() { return mFading; }

	/* step > 0  -  fade in
	   step < 0  -  fade out */
	inline void setFadeStep(float step) { mFadeStep = step; }

	void reset();

protected:
	void doAnimate();

private:
	int   mBrightnessOrigin;
	float mBrightness;
	float mFadeStep;

	int   mFadeOutThreshold;
	int   mFadeInThreshold;

	bool  mFading;
};