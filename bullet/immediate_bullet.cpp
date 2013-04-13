#include "immediate_bullet.h"

CImmediateBullet::CImmediateBullet(TDamageType damageType, float lethalRange):
mDamageType(damageType),
mDamageValue(0),
mLethalRange(lethalRange)
{}

CImmediateBullet::~CImmediateBullet() {}

void CImmediateBullet::activate(CVector2D& position, float angle)
{
	mRayCastResults.mEntity = 0;
	/* Ray-cast requires two points.
	   The initial intersection point is the most distant point. */
	mRayCastResults.mIntersection = position + (CVector2D::getUnitVector(angle) * mLethalRange);

	if (CWorld::instance()->rayCast(position, mRayCastResults.mIntersection, mRayCastResults, ~ENTITY_GROUP_DEBRIS))
	{
		onIntersection();
	}
}

bool CImmediateBullet::onIntersection()
{
	if (mRayCastResults.mEntity) // check if bullet intersected interactive entity, not just static wall
	{
		mRayCastResults.mEntity->applyDamage(&mRayCastResults.mIntersection, mDamageType, mDamageValue);
		return true;
	}

	return false;
}

bool CImmediateBullet::isActive()
{
	return false;
}