#pragma once

#include "..\common\xmath.h"

class IBullet
{
public:
	virtual ~IBullet() {}

	virtual void activate(CVector2D& position, float angle) = 0;
	virtual bool isActive() = 0;
};