#pragma once

#include "..\layer\render_layer.h"
#include "content_provider.h"

class CTextureContent: public IContent
{
public:
	CTextureContent(char* fileName);
	CTextureContent(char* fileName, unsigned long colorKey);
	~CTextureContent();

	IContent* clone();

	inline TEXTURE_REF getContent() { return mTextureRef; }

private:
	CTextureContent(const CTextureContent& content);

	void initialize(char* fileName, unsigned long colorKey);

private:
	TEXTURE_REF mTextureRef;
};