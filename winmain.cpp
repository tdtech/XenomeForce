#define WINDOW_CLASS_NAME "SoE Class"
#define WINDOW_TITLE "SoE"

#include "common\common.h"
#include "log\logger.h"
#include "layer\layer_manager.h"
#include "state\state_factory.h"
#include "state\state_manager.h"

HWND mMainHwnd = NULL;
HINSTANCE mMainInstance = NULL;

void SoEInit()
{
	LOGTEXT("SoE init...");
	srand(GetTickCount());
	CVector2D::initUnitVectors();

	TDisplayMode displayMode;
	displayMode.mWidth = 1024;
	displayMode.mHeight = 768;
	displayMode.mFormat = EFormatA8R8G8B8;
	displayMode.mRefreshRate = 75;

	CLayerManager::init(mMainHwnd, mMainInstance, displayMode);
	CStateFactory::init(mMainHwnd);
	CStateManager::init();
	CStateManager::instance()->setState(CStateFactory::instance()->getBuildWorldState());
}

void SoEMain()
{
	CStateManager::instance()->processState();
	if (CLayerManager::instance()->getInputLayer()->keyPressed(TKEY_ESCAPE))
	{
		CStateManager::instance()->setState(CStateFactory::instance()->getExitState());
	}
}

void SoEClose()
{
	LOGTEXT("SoE close...");
	CStateManager::release();
	CStateFactory::release();
	CLayerManager::release();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CREATE:
		{
			return 0;
		} break;
	case WM_PAINT:
		{
			return 0;
		} break;
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		} break;
	default: break;
	}
	return (DefWindowProc(hwnd, msg, wparam, lparam));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASS winclass;
	MSG msg = {0};
	winclass.style = CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hInstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	if (!RegisterClass(&winclass)) return 0;
	mMainHwnd = CreateWindow(WINDOW_CLASS_NAME, WINDOW_TITLE, 
		WS_POPUP | WS_VISIBLE, 0, 0, 10, 10, NULL, NULL, hInstance, NULL);
	mMainInstance = hInstance;
	ShowWindow(mMainHwnd, SW_SHOW);
	ShowCursor(FALSE);
	SoEInit();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		SoEMain();
	}
	SoEClose();
	return (msg.wParam);
}