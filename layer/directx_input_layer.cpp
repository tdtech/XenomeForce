#include "..\common\common.h"
#include "..\log\logger.h"
#include "directx_input_layer.h"

// Key mapping table
const unsigned int sKeyTable[65] =
{
	 DIK_ESCAPE                     // TKEY_ESCAPE
	,DIK_F1                         // TKEY_F1
	,DIK_F2                         // TKEY_F2
	,DIK_F3                         // TKEY_F3
	,DIK_F4                         // TKEY_F4
	,DIK_F5                         // TKEY_F5
	,DIK_F6                         // TKEY_F6
	,DIK_F7                         // TKEY_F7
	,DIK_F8                         // TKEY_F8
	,DIK_F9                         // TKEY_F9
	,DIK_F10                        // TKEY_F10
	,DIK_F11                        // TKEY_F11
	,DIK_F12                        // TKEY_F12
	,DIK_0                          // TKEY_0
	,DIK_1                          // TKEY_1
	,DIK_2                          // TKEY_2
	,DIK_3                          // TKEY_3
	,DIK_4                          // TKEY_4
	,DIK_5                          // TKEY_5
	,DIK_6                          // TKEY_6
	,DIK_7                          // TKEY_7
	,DIK_8                          // TKEY_8
	,DIK_9                          // TKEY_9
	,DIK_MINUS                      // TKEY_MINUS
	,DIK_EQUALS                     // TKEY_EQUALS
	,DIK_BACK                       // TKEY_BACK
	,DIK_TAB                        // TKEY_TAB
	,DIK_Q                          // TKEY_Q
	,DIK_W                          // TKEY_W
	,DIK_E                          // TKEY_E
	,DIK_R                          // TKEY_R
	,DIK_T                          // TKEY_T
	,DIK_Y                          // TKEY_Y
	,DIK_U                          // TKEY_U
	,DIK_I                          // TKEY_I
	,DIK_O                          // TKEY_O
	,DIK_P                          // TKEY_P
	,DIK_RETURN                     // TKEY_ENTER
	,DIK_A                          // TKEY_A
	,DIK_S                          // TKEY_S
	,DIK_D                          // TKEY_D
	,DIK_F                          // TKEY_F
	,DIK_G                          // TKEY_G
	,DIK_H                          // TKEY_H
	,DIK_J                          // TKEY_J
	,DIK_K                          // TKEY_K
	,DIK_L                          // TKEY_L
	,DIK_LSHIFT                     // TKEY_LSHIFT
	,DIK_Z                          // TKEY_Z
	,DIK_X                          // TKEY_X
	,DIK_C                          // TKEY_C
	,DIK_V                          // TKEY_V
	,DIK_B                          // TKEY_B
	,DIK_N                          // TKEY_N
	,DIK_M                          // TKEY_M
	,DIK_RSHIFT                     // TKEY_RSHIFT
	,DIK_LCONTROL                   // TKEY_LCTRL
	,DIK_LMENU                      // TKEY_LALT
	,DIK_SPACE                      // TKEY_SPACE
	,DIK_RMENU                      // TKEY_RALT
	,DIK_RCONTROL                   // TKEY_RCTRL
	,DIK_UP                         // TKEY_UP
	,DIK_LEFT                       // TKEY_LEFT
	,DIK_DOWN                       // TKEY_DOWN
	,DIK_RIGHT                      // TKEY_RIGHT
	// add more keys...
};

CDirectXInputLayer* CDirectXInputLayer::mInstance = NULL;
int CDirectXInputLayer::mRefCount = 0;

CDirectXInputLayer* CDirectXInputLayer::obtainInstance(HWND hwnd, HINSTANCE instance)
{
	if (mInstance == NULL) mInstance = new CDirectXInputLayer(hwnd, instance);

	mRefCount++;
	return mInstance;
}

CDirectXInputLayer::CDirectXInputLayer(HWND hwnd, HINSTANCE instance):
mDInputInterface(NULL),
mDInputKeyBoard(NULL),
mDInputMouse(NULL),
mHwnd(hwnd)
{
	for (int i = 0; i < 256; i++)
		mKeyBuffer[i] = 0;
	DirectInput8Create(instance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&mDInputInterface, NULL);
	if (mDInputInterface == NULL)
	{
		LOGTEXT("CDirectXInputLayer: DirectInput8Create() failed");
	}
}

CDirectXInputLayer::~CDirectXInputLayer()
{
	releaseResources();
	if (mDInputInterface)
	{
		mDInputInterface->Release();
		mDInputInterface = NULL;
	}
}

int CDirectXInputLayer::init()
{
	releaseResources();
	if (mDInputInterface)
	{
		// Creating keyboard...
		if (FAILED(mDInputInterface->CreateDevice(GUID_SysKeyboard, &mDInputKeyBoard, NULL)))
		{
			LOGTEXT("CDirectXInputLayer: CreateDevice() for KeyBoard failed");
			return E_FAILED;
		}
		if (FAILED(mDInputKeyBoard->SetCooperativeLevel(mHwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
		{
			LOGTEXT("CDirectXInputLayer: SetCooperativeLevel() for KeyBoard failed");
			releaseResources();
			return E_FAILED;
		}
		if (FAILED(mDInputKeyBoard->SetDataFormat(&c_dfDIKeyboard)))
		{
			LOGTEXT("CDirectXInputLayer: SetDataFormat() for KeyBoard failed");
			releaseResources();
			return E_FAILED;
		}
		if (FAILED(mDInputKeyBoard->Acquire()))
		{
			LOGTEXT("CDirectXInputLayer: Acquire() for KeyBoard failed");
			releaseResources();
			return E_FAILED;
		}

		// Creating mouse...
		if (FAILED(mDInputInterface->CreateDevice(GUID_SysMouse, &mDInputMouse, NULL)))
		{
			LOGTEXT("CDirectXInputLayer: CreateDevice() for Mouse failed");
			releaseResources();
			return E_FAILED;
		}
		if (FAILED(mDInputMouse->SetCooperativeLevel(mHwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
		{
			LOGTEXT("CDirectXInputLayer: SetCooperativeLevel() for Mouse failed");
			releaseResources();
			return E_FAILED;
		}
		if (FAILED(mDInputMouse->SetDataFormat(&c_dfDIMouse)))
		{
			LOGTEXT("CDirectXInputLayer: SetDataFormat() for Mouse failed");
			releaseResources();
			return E_FAILED;
		}
		if (FAILED(mDInputMouse->Acquire()))
		{
			LOGTEXT("CDirectXInputLayer: Acquire() for Mouse failed");
			releaseResources();
			return E_FAILED;
		}
		return E_SUCCESS;
	}
	return E_FAILED;
}

int CDirectXInputLayer::readKeyBoard()
{
	if (mDInputKeyBoard)
		return SUCCEEDED(mDInputKeyBoard->GetDeviceState(256, (LPVOID)mKeyBuffer));
	return E_FAILED;
}

int CDirectXInputLayer::readMouse(TMouseState& mouseState)
{
	if (mDInputMouse)
	{
		if (SUCCEEDED(mDInputMouse->GetDeviceState(sizeof(mMouseState), (LPVOID)&mMouseState)))
		{
			mouseState.mOffsetX = mMouseState.lX;
			mouseState.mOffsetY = mMouseState.lY;
			mouseState.mButton[0] = mMouseState.rgbButtons[0];
			mouseState.mButton[1] = mMouseState.rgbButtons[1];
			return E_SUCCESS;
		}
	}
	return E_FAILED;
}

unsigned char CDirectXInputLayer::keyPressed(unsigned int key)
{
	return mKeyBuffer[sKeyTable[key]];
}

int CDirectXInputLayer::release()
{
	mRefCount--;

	if (mRefCount == 0)
	{
		delete mInstance;
		mInstance = NULL;
	}

	return E_SUCCESS;
}

void CDirectXInputLayer::releaseResources()
{
	if (mDInputMouse)
	{
		mDInputMouse->Unacquire();
		mDInputMouse->Release();
		mDInputMouse = NULL;
	}
	if (mDInputKeyBoard)
	{
		mDInputKeyBoard->Unacquire();
		mDInputKeyBoard->Release();
		mDInputKeyBoard = NULL;
	}
}

// ==========================================================

IInputLayer* obtainDirectXInputLayer(HWND hwnd, HINSTANCE instance)
{
	return CDirectXInputLayer::obtainInstance(hwnd, instance);
}