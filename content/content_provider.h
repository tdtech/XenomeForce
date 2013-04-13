#pragma once

#include "content_refs.h"

/* Any nonprimitive content MUST implement IContent interface.
   Method clone() MUST return a copy of the current content which
   has the same references to primitive resources (i.e. Direct3D 
   surfaces and textures). */
class IContent
{
public:
	virtual ~IContent() {}
	virtual IContent* clone() = 0;
};

/* DO NOT RELEASE primary content outside of a ContentProvider!!!
   It will call destructors of it by itself. */
class CContentProvider
{
public:
	static void init();
	static void release();
	static inline CContentProvider* instance() { return mInstance; }

	int setPrimaryContent(IContent* content, TContentRef ref);
	IContent* querySecondaryContent(TContentRef ref);

	void releasePrimaryContent();

private:
	CContentProvider();
	~CContentProvider();

private:
	IContent*                  mPrimaryContent[EContentCount];

	static CContentProvider*   mInstance;
};