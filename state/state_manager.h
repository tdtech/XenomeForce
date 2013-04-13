#pragma once

class IState
{
public:
	virtual ~IState() {}

	virtual void preProcess() = 0;
	virtual void process() = 0;
	virtual void postProcess() = 0;
};

class CStateManager
{
public:
	static void init();
	static void release();
	static inline CStateManager* instance() { return mInstance; }

	void setState(IState* state);
	void processState();

private:
	CStateManager();
	~CStateManager();

private:
	IState*               mState;

	static CStateManager* mInstance;
};