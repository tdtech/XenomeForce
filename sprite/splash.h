#pragma once

#include "..\content\texture_content.h"
#include "sprite_depth.h"
#include "oneshot_sprite.h"

enum TSplashType
{
	// Earth splashes
	 EEarthSplashSmall
	,EEarthSplashMedium
	,EEarthSplashLarge

	// Spark splashes
	,ESparkSplash

	// Human blood splashes
	,EHumanBloodSplashSmall
	,EHumanBloodSplashMedium
	,EHumanBloodSplashLarge

	,ESplashTypeCount // always should be the last
};

class CSplash: public COneShotSprite
{
public:
	CSplash(TSplashType type);
	~CSplash();

	TSpriteDepth getDefaultDepth();

private:
	static CTextureContent* obtainTextureContent(TSplashType type);
};