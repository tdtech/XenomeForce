#pragma once

#include "flickering_spot_light.h"

class CBrokenSpotLight: public CFlickeringSpotLight
{
public:
	/* period - how many frames spot light shines, should be greater than 0
	   delay - how many frames spot light doesn't shine, should be greater than 0 */
	CBrokenSpotLight(float radius, int amplitude, int minPeriod, int maxPeriod, int minDelay, int maxDelay);
	~CBrokenSpotLight();

	inline void setBroken(bool broken) { mBroken = broken; }

protected:
	void doAnimate();
	void doDraw();

private:
	int  mMinPeriod;
	int  mPeriodMod;
	int  mPeriod;

	int  mMinDelay;
	int  mDelayMod;
	int  mDelay;

	int  mState;
	int  mTimer;

	bool mBroken;
};