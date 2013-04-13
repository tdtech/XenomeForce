#pragma once

#include "content_refs.h"
#include "content_provider.h"

class CContentBuilder
{
public:
	CContentBuilder(int capacity = EContentCount);
	~CContentBuilder();

	int addContent(TContentRef ref);
	void buildContent();

private:
	IContent* buildContent(TContentRef ref);

private:
	TContentRef* mRefs;
	int          mRefsCount;
	int          mCapacity;
};