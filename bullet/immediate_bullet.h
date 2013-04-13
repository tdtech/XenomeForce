#pragma once

#include "..\world\world.h"
#include "bullet.h"

class CImmediateBullet: public IBullet
{
public:
	CImmediateBullet(TDamageType damageType, float lethalRange);
	virtual ~CImmediateBullet();

	virtual void activate(CVector2D& position, float angle);
	bool isActive();

	inline void setDamageValue(int value) { mDamageValue = value; }
	inline int  getDamageValue() { return mDamageValue; }

protected:
	/* Callback that will be called if bullet intersects with some object.
	   All information about intersection can be obtained from mRayCastResults.
	   Default implementation of this method simply takes specified damage from
	   intersected entity.
	   Returns TRUE if damage was applied. */
	virtual bool onIntersection();

protected:
	TWorldRayCastResults mRayCastResults;

private:
	TDamageType          mDamageType;
	int                  mDamageValue;

	float                mLethalRange;
};