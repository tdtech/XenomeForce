#pragma once

/* World time which does not depend on a real system time.
   Its more like just some timer ticks.
   It can be used by world events to track time, for example. */
class CWorldTime
{
public:
	CWorldTime(long initialTime = 0);
	~CWorldTime();

	void setTime(long time);
	long getTime(); // returns current count of ticks
	void nextTime();

private:
	long mTime;
	long mThreshold;
};