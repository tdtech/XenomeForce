#pragma once

#include "..\common\collection.h"
#include "..\landscape\landscape.h"

#define MAX_DECALS_PER_FRAME 32

// Decal interface
class IDecal
{
public:
	virtual ~IDecal() {}

	virtual void draw() = 0;
};

// Sprite interface
class ISprite
{
	friend class CSpriteManager;

public:
	virtual ~ISprite() {}

	virtual void animate() = 0;
	virtual void draw() = 0;

private:
	int mId;
	int mDepth;
};

class CSpriteManager
{
public:
	/* [0..(maxDepth - 1)] - depth interval.
	   Try to use small max depth value, 1024 or lower is a good choice. */
	static void init(int maxDepth);
	static void release();
	static inline CSpriteManager* instance() { return mInstance; }

	void addDecal(IDecal* decal);
	void drawDecals(ILandscape* landscape);

	void addSprite(ISprite* sprite, int depth);
	void removeSprite(ISprite* sprite);
	void animateSprites();
	void drawSprites();
	void releaseSprites();

private:
	class CDummySprite: public ISprite
	{
	public:
		void animate() {}
		void draw() {}
	};

	struct TDepthBlock32
	{
		unsigned long mBitMask;
		int           mHighestDepth;
	};

private:
	CSpriteManager(int maxDepth);
	~CSpriteManager();

	int                    mMaxDepth;

	IDecal*                mDecals[MAX_DECALS_PER_FRAME];
	int                    mDecalsCount;

	CCollection<ISprite>   mSprites;

	CDummySprite           mDummySprite;
	TDepthBlock32*         mDepthBlocks;
	int*                   mDepthEndIds;
	int*                   mSpritesCountPerDepth;

	static CSpriteManager* mInstance;
};