#include "build_world_state.h"
#include "world_state.h"
#include "exit_state.h"

#include "state_factory.h"

CStateFactory* CStateFactory::mInstance = 0;

void CStateFactory::init(HWND hwnd)
{
	if (mInstance == 0) mInstance = new CStateFactory(hwnd);
}

void CStateFactory::release()
{
	delete mInstance;
	mInstance = 0;
}

CStateFactory::CStateFactory(HWND hwnd)
{
	mBuildWorldState = new CBuildWorldState();
	mWorldState = new CWorldState();
	mExitState = new CExitState(hwnd);
}

CStateFactory::~CStateFactory()
{
	delete mExitState;
	delete mWorldState;
	delete mBuildWorldState;
}

IState* CStateFactory::getBuildWorldState()
{
	return mBuildWorldState;
}

IState* CStateFactory::getWorldState()
{
	return mWorldState;
}

IState* CStateFactory::getExitState()
{
	return mExitState;
}