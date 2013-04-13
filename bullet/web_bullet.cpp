#include "web_bullet.h"

// Web bullet states
#define WEB_BULLET_STATE_INACTIVE 0
#define WEB_BULLET_STATE_FLYING 1
#define WEB_BULLET_STATE_INLACE 2
#define WEB_BULLET_STATE_INLACING 3
#define WEB_BULLET_STATE_ANNIHILATING 4

// Web bullet parameters
#define WEB_BULLET_SPEED 3.6f
#define WEB_BULLET_RADIUS 16.0f
#define WEB_BULLET_MASS 0
#define WEB_BULLET_INLACE_DELAY 500

CWebBullet::CWebBullet():
IEntity(ENTITY_GROUP_BULLET),
mWebBulletState(WEB_BULLET_STATE_INACTIVE),
mVictim(0)
{
	mSpiderWebSprite = new CSpiderWeb();
	mSpiderWebSprite->setEnabled(false);
	CSpriteManager::instance()->addSprite(mSpiderWebSprite, mSpiderWebSprite->getDefaultDepth());
}

CWebBullet::~CWebBullet()
{
	CSpriteManager::instance()->removeSprite(mSpiderWebSprite);
	delete mSpiderWebSprite;
}

void CWebBullet::registerToWorld(CVector2D& position)
{
	TCircleShape shape(WEB_BULLET_RADIUS);
	TEntityDef def(shape);
	def.mPosition = position;
	def.mFrameRadius = WEB_BULLET_RADIUS;
	def.mGroupFilter = ~ENTITY_GROUP_DEBRIS; // do not contact with debris

	CWorld::instance()->registerEntity(this, def);
}

CVector2D& CWebBullet::action()
{
	if (mWebBulletState == WEB_BULLET_STATE_FLYING)
	{
		mSpiderWebSprite->setPosition(getPosition());
	}
	else if (mWebBulletState == WEB_BULLET_STATE_ANNIHILATING)
	{
		mSpiderWebSprite->setAction(ESpiderWebActionDeath);
		CWorld::instance()->setActive(this, false);
		mWebBulletState = WEB_BULLET_STATE_INACTIVE;
	}
	else if (mWebBulletState == WEB_BULLET_STATE_INLACE)
	{
		mSpiderWebSprite->setPosition(mVictim->getPosition());
		mSpiderWebSprite->setAction(ESpiderWebActionInlace);
		CWorld::instance()->setActive(this, false);
		mVictim->applyDamage(&getPosition(), EDamageImmobilization, /* TODO: pass the duration of immobilization */ 0);
		mInlaceTimer = 0;
		mWebBulletState = WEB_BULLET_STATE_INLACING;
	}
	else if (mWebBulletState == WEB_BULLET_STATE_INLACING)
	{
		if (mInlaceTimer++ == WEB_BULLET_INLACE_DELAY)
		{
			mSpiderWebSprite->setAction(ESpiderWebActionDeath);
			mWebBulletState = WEB_BULLET_STATE_INACTIVE;
		}
	}

	return mVelocity;
}

void CWebBullet::applyDamage(CVector2D* position, TDamageType damageType, int damageValue)
{
	if (mWebBulletState == WEB_BULLET_STATE_FLYING) mWebBulletState = WEB_BULLET_STATE_ANNIHILATING;
}

bool CWebBullet::onContact(IEntity* entity)
{
	if (mWebBulletState == WEB_BULLET_STATE_FLYING)
	{
		mVictim = entity;
		if ((mVictim != 0) && ((mVictim->getGroup() & (ENTITY_GROUP_HUMAN | ENTITY_GROUP_MECHANISM)) > 0)) mWebBulletState = WEB_BULLET_STATE_INLACE;
		else mWebBulletState = WEB_BULLET_STATE_ANNIHILATING;
		return true;
	}
	return false;
}

float CWebBullet::getRadius()
{
	return WEB_BULLET_RADIUS;
}

float CWebBullet::getMass()
{
	return WEB_BULLET_MASS;
}

void CWebBullet::onDeregistered()
{
	// TODO: delete this?
}

void CWebBullet::activate(CVector2D& position, float angle)
{
	mSpiderWebSprite->setEnabled(true);
	mSpiderWebSprite->setPosition(position);
	mSpiderWebSprite->setAngle(angle);
	mSpiderWebSprite->setAction(ESpiderWebActionFly);

	setTransform(position, angle);
	CWorld::instance()->setActive(this, true);
	mVelocity = CVector2D::getUnitVector(angle) * WEB_BULLET_SPEED;

	mWebBulletState = WEB_BULLET_STATE_FLYING;
}

bool CWebBullet::isActive()
{
	return mSpiderWebSprite->isEnabled(); // TODO: using mWebBulletState would be more correct
}