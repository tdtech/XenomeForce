#pragma once

#include "..\sprite\defender_handgun_model.h"
#include "..\bullet\defender_bullet.h"
#include "weapon.h"

class CDefenderHandgun: public IWeapon
{
public:
	CDefenderHandgun();
	~CDefenderHandgun();

	void setPosition(CVector2D& position);
	void getPosition(CVector2D& position);

	void setAngle(float angle);
	float getAngle();

	void setActive(bool active);

	bool fire();

private:
	CDefenderHandgunModel* mModel;
	CDefenderBullet*       mBullet;
};