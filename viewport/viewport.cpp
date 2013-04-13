#include "..\layer\layer_manager.h"
#include "viewport.h"

#define EPSILON_MAGNITUDE 2.0f
#define MAGNITUDE_DECELERATION 0.9f
#define MIN_SHAKE 2.0f

CViewPort::CViewPort(int width, int height, TRect& bound, ILandscape* landscape):
mWidth(width),
mHeight(height),
mHalfWidth(width / 2.0f),
mHalfHeight(height / 2.0f),
mLandscape(landscape),
mMagnitude(0.0f)
{
	// Calculate scale between display and viewport
	TDisplayMode displayMode;
	CLayerManager::instance()->getRenderLayer()->getDeviceDisplayMode(displayMode);
	CVector2D scale((float)displayMode.mWidth / mWidth, (float)displayMode.mHeight / mHeight);
	CLayerManager::instance()->getRenderLayer()->setSpriteScale(scale);

	// Set bound
	mBound.mX0 = bound.mX0;
	mBound.mY0 = bound.mY0;
	mBound.mX1 = bound.mX1;
	mBound.mY1 = bound.mY1;

	// By default view port position is (0, 0), so fix it for landscape
	fixPosition();

	mNewPosition = mPosition;
	updateLeftTop();
}

CViewPort::~CViewPort()
{
	delete mLandscape;
}

float CViewPort::getDiagonalLength()
{
	return xmath_sqrt(mWidth * mWidth + mHeight * mHeight);
}

void CViewPort::setPosition(CVector2D& pos)
{
	mNewPosition = pos;
}

void CViewPort::movePosition(CVector2D& offset)
{
	mNewPosition = mNewPosition + offset;
}

void CViewPort::applyShake(float magnitude)
{
	mMagnitude = XMATH_MAX(mMagnitude, magnitude);
}

void CViewPort::update()
{
	mPosition = mNewPosition;
	fixPosition();

	processShake();

	updateLeftTop();
}

bool CViewPort::inView(CVector2D& point, float radius)
{
	radius += radius;
	mHelpVector = point - mPosition + mShake;
	return ((xmath_abs(mHelpVector.mX) < mHalfWidth + radius) && (xmath_abs(mHelpVector.mY) < mHalfHeight + radius));
}

CVector2D& CViewPort::toWorld(CVector2D& local)
{
	mHelpVector.mX = local.mX + mPosition.mX - mHalfWidth;
	mHelpVector.mY = local.mY + mPosition.mY - mHalfHeight;
	return mHelpVector;
}

void CViewPort::draw()
{
	mLandscapeRect.mX0 = (long)(mPosition.mX + mShake.mX - mHalfWidth);
	mLandscapeRect.mY0 = (long)(mPosition.mY + mShake.mY - mHalfHeight);
	mLandscapeRect.mX1 = (long)(mPosition.mX + mShake.mX + mHalfWidth);
	mLandscapeRect.mY1 = (long)(mPosition.mY + mShake.mY + mHalfHeight);

	mLandscape->draw(mLandscapeRect);
}

void CViewPort::fixPosition()
{
	if (mPosition.mX < (mHalfWidth + mBound.mX0)) mPosition.mX = mHalfWidth + mBound.mX0;
	else if (mPosition.mX > (mBound.mX1 - mHalfWidth)) mPosition.mX = mBound.mX1 - mHalfWidth;

	if (mPosition.mY < (mHalfHeight + mBound.mY0)) mPosition.mY = mHalfHeight + mBound.mY0;
	else if (mPosition.mY > (mBound.mY1 - mHalfHeight)) mPosition.mY = mBound.mY1 - mHalfHeight;
}

void CViewPort::updateLeftTop()
{
	mLeftTop.mX = mPosition.mX + mShake.mX - mHalfWidth;
	mLeftTop.mY = mPosition.mY + mShake.mY - mHalfHeight;
}

void CViewPort::processShake()
{
	if (mMagnitude > EPSILON_MAGNITUDE)
	{
		mShake = CVector2D::getUnitVector(rand() % 360) * mMagnitude;

		float leftOffset = mPosition.mX - mHalfWidth;
		float rightOffset = mLandscape->getWidth() - mPosition.mX - mHalfWidth;
		float topOffset = mPosition.mY - mHalfHeight;
		float bottomOffset = mLandscape->getHeight() - mPosition.mY - mHalfHeight;

		if ((mShake.mX + leftOffset) < 0)
		{
			if (leftOffset < MIN_SHAKE)
			{
				mShake.mX = XMATH_MIN(-mShake.mX, rightOffset);
			}
			else
			{
				mShake.mX = -leftOffset;
			}
		}
		else if ((mShake.mX - rightOffset) > 0)
		{
			if (rightOffset < MIN_SHAKE)
			{
				mShake.mX = XMATH_MAX(-mShake.mX, -leftOffset);
			}
			else
			{
				mShake.mX = rightOffset;
			}
		}

		if ((mShake.mY + topOffset) < 0)
		{
			if (topOffset < MIN_SHAKE)
			{
				mShake.mY = XMATH_MIN(-mShake.mY, bottomOffset);
			}
			else
			{
				mShake.mY = -topOffset;
			}
		}
		else if ((mShake.mY - bottomOffset) > 0)
		{
			if (bottomOffset < MIN_SHAKE)
			{
				mShake.mY = XMATH_MAX(-mShake.mY, -topOffset);
			}
			else
			{
				mShake.mY = bottomOffset;
			}
		}

		mMagnitude *= MAGNITUDE_DECELERATION;
	}
	else
	{
		mShake.mX = mShake.mY = 0.0f;
	}
}