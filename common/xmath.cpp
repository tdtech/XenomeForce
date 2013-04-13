#include "xmath.h"

CVector2D CVector2D::mUnitVectors[361];

void CVector2D::initUnitVectors()
{
	float radians = 0.0f;
	for (int angle = 0; angle < 361; angle++)
	{
		mUnitVectors[angle].mX = cosf(radians);
		mUnitVectors[angle].mY = -sinf(radians);
		radians += RADIANS_PER_DEGREE;
	}
}

CVector2D& CVector2D::getUnitVector(int angle)
{
	return mUnitVectors[angle];
}

CVector2D& CVector2D::getUnitVector(float angle)
{
	return mUnitVectors[xmath_round(angle)];
}

float xmath_wrap_angle(float angle)
{
	angle -= 360 * ((int)(angle / 360.0f));
	if (angle < 0) angle += 360.0f;
	return angle;
}