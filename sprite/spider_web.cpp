#include "spider_web.h"

const int sSpiderWebActionMapping[ESpiderWebActionCount][3] =
{
	{0, 0, 1}, // ESpiderWebActionInlace
	{0, 0, 1}, // ESpiderWebActionFly
	{0, 0, 1}  // ESpiderWebActionDeath
};

CSpiderWeb::CSpiderWeb():
COneShotSprite((CTextureContent*)CContentProvider::instance()->querySecondaryContent(ETextureContentSpiderWeb), 0, 0, 48, 1),
mSpiderWebAction(-1)
{
	setAction(ESpiderWebActionInlace);
}

CSpiderWeb::~CSpiderWeb() {}

void CSpiderWeb::setAction(int action)
{
	if (mSpiderWebAction != action)
	{
		setOneShot(action == ESpiderWebActionDeath);

		setAnimationLine(sSpiderWebActionMapping[action][0]);
		setAnimation(sSpiderWebActionMapping[action][1], sSpiderWebActionMapping[action][2]);
		mSpiderWebAction = action;
	}
}

int CSpiderWeb::getAction()
{
	return mSpiderWebAction;
}

TSpriteDepth CSpiderWeb::getDefaultDepth()
{
	return ESpriteDepthWeapons;
}

void CSpiderWeb::doAnimate()
{
	COneShotSprite::doAnimate();
	if (mIsAnimationCompleted && (mSpiderWebAction == ESpiderWebActionDeath)) setEnabled(false);
}