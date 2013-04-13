#include "splash.h"

// {U offset, V offset, frame size}
const int sSplashTypeMapping[ESplashTypeCount][3] =
{
	// Earth splashes
	{0, 0, 8},
	{0, 9, 32},
	{0, 42, 128},

	// Spark splashes
	{0, 171, 8},

	// Human blood splashes
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0}
};

CSplash::CSplash(TSplashType type):
COneShotSprite(obtainTextureContent(type), sSplashTypeMapping[type][0], sSplashTypeMapping[type][1],
               sSplashTypeMapping[type][2], 1)
{}

CSplash::~CSplash() {}

TSpriteDepth CSplash::getDefaultDepth()
{
	return ESpriteDepthSplashes;
}

CTextureContent* CSplash::obtainTextureContent(TSplashType type)
{
	// TODO: add 'switch' if there will be more than one splash content
	return ((CTextureContent*)CContentProvider::instance()->querySecondaryContent(ETextureContentSplashes));
}