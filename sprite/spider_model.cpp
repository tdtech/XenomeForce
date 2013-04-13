#include "spider_model.h"

const int sSpiderActionMapping[ESpiderActionCount][3] =
{
	{0, 0, 1}, // ESpiderActionStay
	{0, 0, 1}, // ESpiderActionWalk
	{0, 0, 1}, // ESpiderActionWebAttack
	{0, 0, 1}, // ESpiderActionSoftDeath
	{0, 0, 1}  // ESpiderActionCruelDeath
};

CSpiderModel::CSpiderModel():
COneShotSprite((CTextureContent*)CContentProvider::instance()->querySecondaryContent(ETextureContentSpider), 0, 0, 32, 1),
mSpiderAction(-1)
{
	setAction(ESpiderActionStay);
}

CSpiderModel::~CSpiderModel() {}

void CSpiderModel::setAction(int action)
{
	if (mSpiderAction != action)
	{
		setOneShot(action == ESpiderActionWebAttack ||
		           action == ESpiderActionSoftDeath ||
		           action == ESpiderActionCruelDeath);

		setAnimationLine(sSpiderActionMapping[action][0]);
		setAnimation(sSpiderActionMapping[action][1], sSpiderActionMapping[action][2]);
		mSpiderAction = action;
	}
}

int CSpiderModel::getAction()
{
	return mSpiderAction;
}

TSpriteDepth CSpiderModel::getDefaultDepth()
{
	return ESpriteDepthModels;
}

void CSpiderModel::doAnimate()
{
	COneShotSprite::doAnimate();
	if (mIsAnimationCompleted)
	{
		switch (mSpiderAction)
		{
			case ESpiderActionWebAttack:
				setAction(ESpiderActionStay);
				break;

			case ESpiderActionSoftDeath:
			case ESpiderActionCruelDeath:
				setEnabled(false);
				break;
		}
	}
}