#include "defender_handgun.h"

CDefenderHandgun::CDefenderHandgun()
{
	mModel = new CDefenderHandgunModel();
	mBullet = new CDefenderBullet();
	CSpriteManager::instance()->addSprite(mModel, mModel->getDefaultDepth());
}

CDefenderHandgun::~CDefenderHandgun()
{
	CSpriteManager::instance()->removeSprite(mModel);
	delete mBullet;
	delete mModel;
}

void CDefenderHandgun::setPosition(CVector2D& position)
{
	mModel->setPosition(position);
}

void CDefenderHandgun::getPosition(CVector2D& position)
{
	position = mModel->getPosition();
}

void CDefenderHandgun::setAngle(float angle)
{
	mModel->setAngle(angle);
}

float CDefenderHandgun::getAngle()
{
	return mModel->getAngle();
}

void CDefenderHandgun::setActive(bool active)
{
	mModel->setEnabled(active);
}

bool CDefenderHandgun::fire()
{
	if (mModel->getAction() != EDefenderHandgunActionShoot)
	{
		mModel->setAction(EDefenderHandgunActionShoot);
		mBullet->activate(mModel->getPosition(), mModel->getAngle());
		return true;
	}

	return false;
}