#include "..\layer\layer_manager.h"
#include "mouse_controller.h"

CMouseController::CMouseController(TRect& clipRect)
{
	mClipRect.mX0 = clipRect.mX0;
	mClipRect.mY0 = clipRect.mY0;
	mClipRect.mX1 = clipRect.mX1;
	mClipRect.mY1 = clipRect.mY1;

	mPosition.mX = (float)mClipRect.mX0;
	mPosition.mY = (float)mClipRect.mY0;

	mMouseState.mOffsetX = mMouseState.mOffsetY = 0;
	mMouseState.mButton[0] = mMouseState.mButton[1] = 0;

	// TODO: compute sensivity using clipRect and display resolution
	mSensivity = 1.0f;
}

CMouseController::~CMouseController() {}

unsigned char CMouseController::isButtonPressed(int button)
{
	return mMouseState.mButton[button];
}

void CMouseController::updateState()
{
	CLayerManager::instance()->getInputLayer()->readMouse(mMouseState);

	mPosition.mX += mMouseState.mOffsetX * mSensivity;
	mPosition.mY += mMouseState.mOffsetY * mSensivity;

	if (mPosition.mX < mClipRect.mX0) mOffScreenOffset.mX = mPosition.mX - mClipRect.mX0;
	else if (mPosition.mX > mClipRect.mX1) mOffScreenOffset.mX = mPosition.mX - mClipRect.mX1;
	else mOffScreenOffset.mX = 0.0f;

	if (mPosition.mY < mClipRect.mY0) mOffScreenOffset.mY = mPosition.mY - mClipRect.mY0;
	else if (mPosition.mY > mClipRect.mY1) mOffScreenOffset.mY = mPosition.mY - mClipRect.mY1;
	else mOffScreenOffset.mY = 0.0f;

	mPosition = mPosition - mOffScreenOffset;
}