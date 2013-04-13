#pragma once

#include "..\world\world.h"
#include "..\sprite\spider_web.h"
#include "bullet.h"

class CWebBullet: public IEntity,
                  public IBullet
{
public:
	CWebBullet();
	~CWebBullet();

	void registerToWorld(CVector2D& position);

	// IEntity implementation
	CVector2D& action();
	void applyDamage(CVector2D* position, TDamageType damageType, int damageValue);
	bool onContact(IEntity* entity);
	float getRadius();
	float getMass();

	void onDeregistered();

	// IBullet implementation
	void activate(CVector2D& position, float angle);
	bool isActive();

private:
	int         mWebBulletState;
	CVector2D   mVelocity;
	IEntity*    mVictim;
	int         mInlaceTimer;

	CSpiderWeb* mSpiderWebSprite;
};