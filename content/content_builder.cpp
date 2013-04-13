#include "..\common\common.h"
#include "content_builder.h"

#include "..\layer\layer_manager.h"
#include "..\sprite\human_model.h"
#include "..\sprite\defender_handgun_model.h"

CContentBuilder::CContentBuilder(int capacity):
mRefsCount(0),
mCapacity(capacity)
{
	mRefs = new TContentRef[mCapacity];
}

CContentBuilder::~CContentBuilder()
{
	delete [] mRefs;
}

int CContentBuilder::addContent(TContentRef ref)
{
	if (mRefsCount < mCapacity)
	{
		mRefs[mRefsCount++] = ref;
		return E_SUCCESS;
	}
	return E_FAILED;
}

void CContentBuilder::buildContent()
{
	for (int i = 0; i < mRefsCount; i++)
	{
		IContent* content = buildContent(mRefs[i]);
		if (CContentProvider::instance()->setPrimaryContent(content, mRefs[i]) == E_FAILED) delete content;
	}
}

IContent* CContentBuilder::buildContent(TContentRef ref)
{
	// this switch statement MUST process every ref declared in TContentRef enumeration
	switch (ref)
	{
		case ETextureContentCursor: return new CTextureContent("sprites\\cursor.png");
		case ETextureContentSplashes: return new CTextureContent("sprites\\splashes.png");
		case ETextureContentSpotLights: return new CTextureContent("lights\\spotlights.png", CLayerManager::instance()->getRenderLayer()->ARGB(255, 0, 0, 0));
		case ETextureContentHuman01: return new CTextureContent("models\\humans\\human01.png");
		case ETextureContentSpider: return new CTextureContent("models\\spiders\\spider.png");
		case ETextureContentSpiderWeb: return new CTextureContent("sprites\\web.png");
		case ETextureContentDefenderHandgun: return new CTextureContent("models\\weapons\\defender_handgun.png");
	}

	return 0;
}