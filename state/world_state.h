#pragma once

#include "state_manager.h"

class CWorldState: public IState
{
public:
	CWorldState();
	~CWorldState();

	void preProcess();
	void process();
	void postProcess();
};