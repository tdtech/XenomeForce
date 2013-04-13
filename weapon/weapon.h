#pragma once

#include "..\common\xmath.h"

class IWeapon
{
public:
	virtual ~IWeapon() {}

	virtual void setPosition(CVector2D& position) = 0;
	virtual void getPosition(CVector2D& position) = 0;

	virtual void setAngle(float angle) = 0;
	virtual float getAngle() = 0;

	virtual void setActive(bool active) = 0; // TODO: think how to avoid this method?

	virtual bool fire() = 0; // true if fire was successful
};