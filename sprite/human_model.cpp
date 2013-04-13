#include "human_model.h"

const int sHumanActionMapping[EHumanActionCount][3] =
{
	{0, 0, 1}, // EHumanActionHandgunStay
	{0, 0, 8}, // EHumanActionHandgunWalk
	{1, 0, 6}, // EHumanActionHandgunRightStrafe
	{2, 0, 6}, // EHumanActionHandgunLeftStrafe
	{0, 0, 1}, // EHumanActionRifleStay
	{0, 0, 1}, // EHumanActionRifleWalk
	{0, 0, 1}, // EHumanActionRifleRightStrafe
	{0, 0, 1}, // EHumanActionRifleLeftStrafe
	{0, 0, 1}  // EHumanActionDeath
};

CHumanModel::CHumanModel(CTextureContent* content, int frameSize, int delimiterWidth):
COneShotSprite(content, 0, 0, frameSize, delimiterWidth),
mHumanAction(-1)
{
	setAction(EHumanActionHandgunStay);
}

CHumanModel::~CHumanModel() {}

void CHumanModel::setAction(int action)
{
	if (mHumanAction != action) {
		setOneShot(action == EHumanActionDeath);
		setAnimationLine(sHumanActionMapping[action][0]);
		setAnimation(sHumanActionMapping[action][1], sHumanActionMapping[action][2]);
		mHumanAction = action;
	}
}

int CHumanModel::getAction()
{
	return mHumanAction;
}

TSpriteDepth CHumanModel::getDefaultDepth()
{
	return ESpriteDepthModels;
}