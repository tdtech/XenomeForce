#pragma once

#include "..\sprite\splash.h"
#include "immediate_bullet.h"

class CDefenderBullet: public CImmediateBullet
{
public:
	CDefenderBullet();
	~CDefenderBullet();

	void activate(CVector2D& position, float angle);

protected:
	bool onIntersection();

private:
	CSplash      mSparkSplash;

	static float mDirAngle;
};