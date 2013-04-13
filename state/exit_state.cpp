#include "..\content\content_provider.h"
#include "..\sprite\sprite_manager.h"

#include "exit_state.h"

CExitState::CExitState(HWND hwnd):
mHwnd(hwnd)
{}

CExitState::~CExitState() {}

void CExitState::preProcess()
{
	CSpriteManager::release();
	CContentProvider::release();

	PostMessage(mHwnd, WM_DESTROY, 0, 0);
}

void CExitState::process() {}

void CExitState::postProcess() {}