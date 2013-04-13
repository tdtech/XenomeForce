#include "defender_handgun_model.h"

const int sDefenderHandgunActionMapping[EDefenderHandgunActionCount][3] =
{
	{0, 0, 1}, // EDefenderHandgunActionStay
	{0, 1, 4}, // EDefenderHandgunActionShoot
	{0, 0, 1}  // EDefenderHandgunActionDeath
};

CDefenderHandgunModel::CDefenderHandgunModel():
COneShotSprite((CTextureContent*)CContentProvider::instance()->querySecondaryContent(ETextureContentDefenderHandgun), 0, 0, 31, 1),
mDefenderAction(-1)
{
	setAction(EDefenderHandgunActionStay);
}

CDefenderHandgunModel::~CDefenderHandgunModel() {}

void CDefenderHandgunModel::setAction(int action)
{
	if (mDefenderAction != action)
	{
		setOneShot(action == EDefenderHandgunActionShoot || action == EDefenderHandgunActionDeath);
		setAnimationLine(sDefenderHandgunActionMapping[action][0]);
		setAnimation(sDefenderHandgunActionMapping[action][1], sDefenderHandgunActionMapping[action][2]);
		mDefenderAction = action;
	}
}

int CDefenderHandgunModel::getAction()
{
	return mDefenderAction;
}

TSpriteDepth CDefenderHandgunModel::getDefaultDepth()
{
	return ESpriteDepthWeapons;
}

void CDefenderHandgunModel::doAnimate()
{
	COneShotSprite::doAnimate();

	if ((mIsAnimationCompleted) && (mDefenderAction == EDefenderHandgunActionShoot))
	{
		setAction(EDefenderHandgunActionStay);
	}
}