#pragma once

#include <windows.h>

#include "state_manager.h"

class CExitState: public IState
{
public:
	CExitState(HWND hwnd);
	~CExitState();

	void preProcess();
	void process();
	void postProcess();

private:
	HWND mHwnd;
};