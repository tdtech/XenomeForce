#pragma once

/* List of content represented by refs which may be added
   to content provider and queried later from it. */

enum TContentRef
{
	 ETextureContentCursor = 0
	,ETextureContentSplashes
	,ETextureContentSpotLights
	,ETextureContentHuman01
	,ETextureContentSpider
	,ETextureContentSpiderWeb
	,ETextureContentDefenderHandgun
	// add more content refs...
	,EContentCount // this item always should be the last
};