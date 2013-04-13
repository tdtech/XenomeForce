#pragma once

#include "..\world\world_builder.h"
#include "state_manager.h"

class CBuildWorldState: public IState, public IWorldBuilderListener
{
public:
	CBuildWorldState();
	~CBuildWorldState();

	void preProcess();
	void process();
	void postProcess();

	// IWorldBuilderListener implementation
	void onProgress(int progress);

private:
	CWorldBuilder* mWorldBuilder;
	char           mProgressText[64];
};