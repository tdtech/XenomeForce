#pragma once

#include "spot_light.h"

class CFlickeringSpotLight: public CSpotLight
{
public:
	CFlickeringSpotLight(float radius, int amplitude); // amplitude >= 0
	virtual ~CFlickeringSpotLight();

	virtual void setColor(int brightness, int red, int green, int blue);

protected:
	virtual void doAnimate();

protected:
	int mRed;
	int mGreen;
	int mBlue;

private:
	int mAmplitude;
	int mBrightnessOrigin;
};