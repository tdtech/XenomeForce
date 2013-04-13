#include <string.h>

#include "..\common\common.h"
#include "content_provider.h"

CContentProvider* CContentProvider::mInstance = 0;

CContentProvider::CContentProvider()
{
	memset(mPrimaryContent, 0, sizeof(mPrimaryContent));
}

CContentProvider::~CContentProvider()
{
	releasePrimaryContent();
}

void CContentProvider::init()
{
	if (mInstance == 0) mInstance = new CContentProvider();
}

void CContentProvider::release()
{
	delete mInstance;
	mInstance = 0;
}

int CContentProvider::setPrimaryContent(IContent* content, TContentRef ref)
{
	if (content)
	{
		mPrimaryContent[ref] = content;
		return E_SUCCESS;
	}
	return E_FAILED;
}

IContent* CContentProvider::querySecondaryContent(TContentRef ref)
{
	IContent* content = 0;
	if (mPrimaryContent[ref])
	{
		content = mPrimaryContent[ref]->clone();
	}
	return content;
}

void CContentProvider::releasePrimaryContent()
{
	for (int i = 0; i < EContentCount; i++)
		delete mPrimaryContent[i];
	memset(mPrimaryContent, 0, sizeof(mPrimaryContent));
}