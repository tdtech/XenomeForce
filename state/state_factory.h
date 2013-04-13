#pragma once

#include <windows.h>

#include "state_manager.h"

class CStateFactory
{
public:
	static void init(HWND hwnd);
	static void release();
	static inline CStateFactory* instance() { return mInstance; }

	IState* getBuildWorldState();
	IState* getWorldState();
	IState* getExitState();

private:
	CStateFactory(HWND hwnd);
	~CStateFactory();

private:
	IState*               mBuildWorldState;
	IState*               mWorldState;
	IState*               mExitState;

	static CStateFactory* mInstance;
};