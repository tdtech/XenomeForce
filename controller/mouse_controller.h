#pragma once

#include "..\common\xmath.h"
#include "..\layer\input_layer.h"

#define LEFT_MOUSE_BUTTON 0
#define RIGHT_MOUSE_BUTTON 1

class CMouseController
{
public:
	CMouseController(TRect& clipRect);
	~CMouseController();

	inline void setSensivity(float sensivity) { mSensivity = sensivity; }
	inline float getSensivity() { return mSensivity; }
	inline CVector2D& getPosition() { return mPosition; }
	inline CVector2D& getOffScreenOffset() { return mOffScreenOffset; }
	unsigned char isButtonPressed(int button);

	void updateState();

private:
	TRect       mClipRect;

	CVector2D   mPosition;
	CVector2D   mOffScreenOffset;
	float       mSensivity;

	TMouseState mMouseState;
};