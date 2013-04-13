#pragma once

#include <dinput.h>

#include "input_layer.h"

class CDirectXInputLayer: public IInputLayer
{
public:
	static CDirectXInputLayer* obtainInstance(HWND hwnd, HINSTANCE instance);

// Initialization
	int init();

// Device reading
	int readKeyBoard();
	int readMouse(TMouseState& mouseState);
// add more devices...

// Keys checking
	unsigned char keyPressed(unsigned int key);
// add more checkings...

	int release();

private:
	CDirectXInputLayer(HWND hwnd, HINSTANCE instance);
	~CDirectXInputLayer();

	void releaseResources();

private:
	LPDIRECTINPUT8             mDInputInterface;

	LPDIRECTINPUTDEVICE8       mDInputKeyBoard;
	LPDIRECTINPUTDEVICE8       mDInputMouse;

	unsigned char              mKeyBuffer[256];
	DIMOUSESTATE               mMouseState;

	HWND                       mHwnd;

	static CDirectXInputLayer* mInstance;
	static int                 mRefCount;
};