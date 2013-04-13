#include "world_time.h"

#define MAX_THRESHOLD 60

CWorldTime::CWorldTime(long initialTime)
{
	setTime(initialTime);
}

CWorldTime::~CWorldTime() {}

void CWorldTime::setTime(long time)
{
	mTime = time;
	mThreshold = 0;
}

long CWorldTime::getTime()
{
	return mTime;
}

void CWorldTime::nextTime()
{
	if (++mThreshold > MAX_THRESHOLD)
	{
		mTime++;
		mThreshold = 0;
	}
}