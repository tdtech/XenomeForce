#pragma once

/* GetTickCount() value is typically updated every 16ms,
   so the max fps will be [(1000 / 16)], which is 62. */
#define MAX_FPS 62
#define DEFAULT_FPS 60

class CFPSController
{
public:
	CFPSController(int fps = DEFAULT_FPS);
	~CFPSController();

	void beginFrame();
	void endFrame();

private:
	unsigned long mBeginMillis;
	unsigned long mDeltaMillis;
};