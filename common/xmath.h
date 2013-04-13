#pragma once

#include <math.h>

#define XMATH_PI 3.14159265359f

#define RADIANS_PER_DEGREE XMATH_PI / 180
#define DEGREES_PER_RADIAN 180 / XMATH_PI

#define RADIANS_TO_DEGREES(x) x * DEGREES_PER_RADIAN
#define DEGREES_TO_RADIANS(x) x * RADIANS_PER_DEGREE

#define XMATH_MAX(x, y) x < y ? y : x
#define XMATH_MIN(x, y) y < x ? y : x

#define XMATH_WRAP_LOWER(x) (x < 0) ? (x + 360) : x
#define XMATH_WRAP_UPPER(x) (x > 360) ? (x - 360) : x

#define NEAREST_POW_2(x) (long)(pow(2, ceil(log((double)x) / log((double)2))));

// Rectangle
struct TRect
{
	long mX0;
	long mY0;
	long mX1;
	long mY1;
};

// Vector 2D

class CVector2D
{
public:
	CVector2D():
	mX(0),
	mY(0)
	{}

	CVector2D(float x, float y):
	mX(x),
	mY(y)
	{}

	CVector2D& operator = (const CVector2D& vector)
	{
		this->mX = vector.mX;
		this->mY = vector.mY;
		return *this;
	}

	CVector2D operator + (float value)
	{
		CVector2D result;
		result.mX = this->mX + value;
		result.mY = this->mY + value;
		return result;
	}

	CVector2D operator + (const CVector2D& vector)
	{
		CVector2D result;
		result.mX = this->mX + vector.mX;
		result.mY = this->mY + vector.mY;
		return result;
	}

	CVector2D operator - (float value)
	{
		CVector2D result;
		result.mX = this->mX - value;
		result.mY = this->mY - value;
		return result;
	}

	CVector2D operator - (const CVector2D& vector)
	{
		CVector2D result;
		result.mX = this->mX - vector.mX;
		result.mY = this->mY - vector.mY;
		return result;
	}

	CVector2D operator * (float value)
	{
		CVector2D result;
		result.mX = this->mX * value;
		result.mY = this->mY * value;
		return result;
	}

	float operator * (const CVector2D& vector)
	{
		return (this->mX * vector.mX + this->mY * vector.mY);
	}

	static void initUnitVectors();
	static CVector2D& getUnitVector(int angle); // 0 <= angle <= 360
	static CVector2D& getUnitVector(float angle); // 0.0 <= angle <= 360.0

public:
	float mX;
	float mY;

	static CVector2D mUnitVectors[361];
};

// Vector 3D

class CVector3D
{
public:
	CVector3D():
	mX(0),
	mY(0),
	mZ(0)
	{}

	CVector3D(float x, float y, float z):
	mX(x),
	mY(y),
	mZ(z)
	{}

	CVector3D& operator = (const CVector3D& vector)
	{
		this->mX = vector.mX;
		this->mY = vector.mY;
		this->mZ = vector.mZ;
		return *this;
	}

	CVector3D operator + (float value)
	{
		CVector3D result;
		result.mX = this->mX + value;
		result.mY = this->mY + value;
		result.mZ = this->mZ + value;
		return result;
	}

	CVector3D operator + (const CVector3D& vector)
	{
		CVector3D result;
		result.mX = this->mX + vector.mX;
		result.mY = this->mY + vector.mY;
		result.mZ = this->mZ + vector.mZ;
		return result;
	}

	CVector3D operator - (float value)
	{
		CVector3D result;
		result.mX = this->mX - value;
		result.mY = this->mY - value;
		result.mZ = this->mZ - value;
		return result;
	}

	CVector3D operator - (const CVector3D& vector)
	{
		CVector3D result;
		result.mX = this->mX - vector.mX;
		result.mY = this->mY - vector.mY;
		result.mZ = this->mZ - vector.mZ;
		return result;
	}

	CVector3D operator * (float value)
	{
		CVector3D result;
		result.mX = this->mX * value;
		result.mY = this->mY * value;
		result.mZ = this->mZ * value;
		return result;
	}

	float operator * (const CVector3D& vector)
	{
		return (this->mX * vector.mX + this->mY * vector.mY + this->mZ * vector.mZ);
	}

public:
	float mX;
	float mY;
	float mZ;
};

// Fast math functions

inline float xmath_sqrt(float value)
{
	_asm
	{
		FLD value
		FSQRT
		FSTP value
	}
	return value;
}

inline float xmath_abs(float value)
{
	_asm
	{
		FLD value
		FABS
		FSTP value
	}
	return value;
}

inline int xmath_round(float value)
{
	int round;
	_asm
	{
		FLD value
		FISTP round
	}
	return round;
}

float xmath_wrap_angle(float angle);