#include "..\layer\layer_manager.h"
#include "texture_content.h"

CTextureContent::CTextureContent(char* fileName)
{
	initialize(fileName, CLayerManager::instance()->getRenderLayer()->ARGB(255, 255, 255, 255));
}

CTextureContent::CTextureContent(char* fileName, unsigned long colorKey)
{
	initialize(fileName, colorKey);
}

CTextureContent::CTextureContent(const CTextureContent& content):
mTextureRef(content.mTextureRef)
{
	CLayerManager::instance()->getRenderLayer()->incTextureRef(mTextureRef);
}

CTextureContent::~CTextureContent()
{
	CLayerManager::instance()->getRenderLayer()->releaseTexture(mTextureRef);
}

IContent* CTextureContent::clone()
{
	return new CTextureContent(*this);
}

void CTextureContent::initialize(char* fileName, unsigned long colorKey)
{
	unsigned int textureWidth, textureHeight;
	CLayerManager::instance()->getRenderLayer()->createTextureFromFile(fileName, colorKey, RENDER_FLAG_MANAGEDMEM, textureWidth, textureHeight, mTextureRef);
}