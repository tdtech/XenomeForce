#include "state_manager.h"

CStateManager* CStateManager::mInstance = 0;

void CStateManager::init()
{
	if (mInstance == 0) mInstance = new CStateManager();
}

void CStateManager::release()
{
	delete mInstance;
	mInstance = 0;
}

CStateManager::CStateManager():
mState(0)
{}

CStateManager::~CStateManager()
{
	if (mState) mState->postProcess();
}

void CStateManager::setState(IState* state)
{
	if (mState != state) {
		if (mState) mState->postProcess();
		if (state) state->preProcess();

		mState = state;
	}
}

void CStateManager::processState()
{
	if (mState) mState->process();
}