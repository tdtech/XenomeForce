#pragma once

#include "..\common\xmath.h"
#include "..\landscape\landscape.h"

class CViewPort
{
public:
	// assume that bound and landscape dimensions are greater than view port dimensions
	CViewPort(int width, int height, TRect& bound, ILandscape* landscape);
	~CViewPort();

	inline int getWidth() { return mWidth; }
	inline int getHeight() { return mHeight; }

	inline ILandscape* getLandscape() { return mLandscape; }

	float getDiagonalLength();

	void setPosition(CVector2D& pos);
	void movePosition(CVector2D& offset);
	inline CVector2D& getPosition() { return mPosition; }
	inline CVector2D& getLeftTop() { return mLeftTop; }

	void applyShake(float magnitude);
	void update();

	bool inView(CVector2D& point, float radius);
	CVector2D& toWorld(CVector2D& local);

	void draw();

private:
	void fixPosition();
	void updateLeftTop();
	void processShake();

private:
	int         mWidth;
	int         mHeight;

	TRect       mBound;

	float       mHalfWidth;
	float       mHalfHeight;

	ILandscape* mLandscape;
	TRect       mLandscapeRect;

	CVector2D   mPosition;
	CVector2D   mNewPosition;
	CVector2D   mLeftTop;

	float       mMagnitude;
	CVector2D   mShake;

	CVector2D   mHelpVector;
};