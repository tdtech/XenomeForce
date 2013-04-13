#include <windows.h>

#include "fps_controller.h"

CFPSController::CFPSController(int fps):
mBeginMillis(0)
{
	if ((fps <= 0) || (fps > MAX_FPS))
	{
		// invalid fps, defaulting...
		fps = DEFAULT_FPS;
	}
	mDeltaMillis = 1000 / fps;
}

CFPSController::~CFPSController() {}

void CFPSController::beginFrame()
{
	mBeginMillis = GetTickCount();
}

void CFPSController::endFrame()
{
	// Do not play game for 49.7 days, otherwise deadlock may occur ;)
	while ((GetTickCount() - mBeginMillis) < mDeltaMillis);
}