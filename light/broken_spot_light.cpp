#include "..\layer\layer_manager.h"
#include "broken_spot_light.h"

#define BROKEN_SPOT_LIGHT_STATE_SHINE    0
#define BROKEN_SPOT_LIGHT_STATE_NOTSHINE 1

CBrokenSpotLight::CBrokenSpotLight(float radius, int amplitude, int minPeriod, int maxPeriod, int minDelay, int maxDelay):
CFlickeringSpotLight(radius, amplitude),
mMinPeriod(minPeriod),
mPeriodMod(maxPeriod - minPeriod + 1),
mMinDelay(minDelay),
mDelayMod(maxDelay - minDelay + 1),
mState(BROKEN_SPOT_LIGHT_STATE_SHINE),
mTimer(0),
mBroken(false)
{
	mPeriod = mMinPeriod + (rand() % mPeriodMod) - 1;
}

CBrokenSpotLight::~CBrokenSpotLight() {}

void CBrokenSpotLight::doAnimate()
{
	if (!mBroken)
	{
		CFlickeringSpotLight::doAnimate();
	}
	else if (mState == BROKEN_SPOT_LIGHT_STATE_SHINE)
	{
		CFlickeringSpotLight::doAnimate();

		if (++mTimer > mPeriod)
		{
			mState = BROKEN_SPOT_LIGHT_STATE_NOTSHINE;
			mTimer = 0;
			mDelay = mMinDelay + (rand() % mDelayMod) - 1;
		}
	}
	else if (mState == BROKEN_SPOT_LIGHT_STATE_NOTSHINE)
	{
		if (++mTimer > mDelay)
		{
			mState = BROKEN_SPOT_LIGHT_STATE_SHINE;
			mTimer = 0;
			mPeriod = mMinPeriod + (rand() % mPeriodMod) - 1;
		}
	}
}

void CBrokenSpotLight::doDraw()
{
	if (mState == BROKEN_SPOT_LIGHT_STATE_SHINE) CFlickeringSpotLight::doDraw();
}