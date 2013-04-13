#include "defender_bullet.h"

#define DEFENDER_LETHAL_RANGE (CWorld::instance()->getViewPort()->getDiagonalLength() * 1.25f) // viewport diagonal length +25%
#define DEFENDER_DAMAGE_VALUE 1

float CDefenderBullet::mDirAngle = 0.0f;

CDefenderBullet::CDefenderBullet():
CImmediateBullet(EDamageCommon, DEFENDER_LETHAL_RANGE),
mSparkSplash(ESparkSplash)
{
	// TODO: maybe set damage value somewhere outside?
	setDamageValue(DEFENDER_DAMAGE_VALUE);

	mSparkSplash.setEnabled(false);
	CSpriteManager::instance()->addSprite(&mSparkSplash, mSparkSplash.getDefaultDepth());
}

CDefenderBullet::~CDefenderBullet()
{
	CSpriteManager::instance()->removeSprite(&mSparkSplash);
}

void CDefenderBullet::activate(CVector2D& position, float angle)
{
	mDirAngle = angle;

	CImmediateBullet::activate(position, angle);
}

bool CDefenderBullet::onIntersection()
{
	if (!CImmediateBullet::onIntersection())
	{
		mSparkSplash.setEnabled(true);
		mSparkSplash.reset();
		mSparkSplash.setPosition(mRayCastResults.mIntersection);
		mSparkSplash.setAngle(mDirAngle);
		// TODO: add light flash

		return false;
	}

	return true;
}