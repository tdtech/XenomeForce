#pragma once

#include <windows.h>

// List of keyboard keys:
#define TKEY_ESCAPE           0
#define TKEY_F1               1
#define TKEY_F2               2
#define TKEY_F3               3
#define TKEY_F4               4
#define TKEY_F5               5
#define TKEY_F6               6
#define TKEY_F7               7
#define TKEY_F8               8
#define TKEY_F9               9
#define TKEY_F10              10
#define TKEY_F11              11
#define TKEY_F12              12
#define TKEY_0                13
#define TKEY_1                14
#define TKEY_2                15
#define TKEY_3                16
#define TKEY_4                17
#define TKEY_5                18
#define TKEY_6                19
#define TKEY_7                20
#define TKEY_8                21
#define TKEY_9                22
#define TKEY_MINUS            23
#define TKEY_EQUALS           24
#define TKEY_BACK             25
#define TKEY_TAB              26
#define TKEY_Q                27
#define TKEY_W                28
#define TKEY_E                29
#define TKEY_R                30
#define TKEY_T                31
#define TKEY_Y                32
#define TKEY_U                33
#define TKEY_I                34
#define TKEY_O                35
#define TKEY_P                36
#define TKEY_ENTER            37
#define TKEY_A                38
#define TKEY_S                39
#define TKEY_D                40
#define TKEY_F                41
#define TKEY_G                42
#define TKEY_H                43
#define TKEY_J                44
#define TKEY_K                45
#define TKEY_L                46
#define TKEY_LSHIFT           47
#define TKEY_Z                48
#define TKEY_X                49
#define TKEY_C                50
#define TKEY_V                51
#define TKEY_B                52
#define TKEY_N                53
#define TKEY_M                54
#define TKEY_RSHIFT           55
#define TKEY_LCTRL            56
#define TKEY_LALT             57
#define TKEY_SPACE            58
#define TKEY_RALT             59
#define TKEY_RCTRL            60
#define TKEY_UP               61
#define TKEY_LEFT             62
#define TKEY_DOWN             63
#define TKEY_RIGHT            64
// add more keys...

/* Mouse state structure
   mButton[0] - Left mouse button
   mButton[1] - Right mouse button */
struct TMouseState
{
	long mOffsetX;
	long mOffsetY;
	unsigned char mButton[2];
};

class IInputLayer
{
public:

// Initialization
	virtual int init() = 0;

// Device reading
	virtual int readKeyBoard() = 0;
	virtual int readMouse(TMouseState& mouseState) = 0;
// add more devices...

// Keys checking
	virtual unsigned char keyPressed(unsigned int key) = 0;
// add more checkings...

	virtual int release() = 0;

protected:
	virtual ~IInputLayer() {}
};

// ==========================================================

// Methods to obtain input layers
IInputLayer* obtainDirectXInputLayer(HWND hwnd, HINSTANCE instance);