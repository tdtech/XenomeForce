#include "..\common\common.h"
#include "..\layer\layer_manager.h"
#include "sprite_manager.h"

#define SPRITES_INITIAL_CAPACITY 1024

CSpriteManager* CSpriteManager::mInstance = 0;

void CSpriteManager::init(int maxDepth)
{
	if (mInstance == 0) mInstance = new CSpriteManager(maxDepth);
}

void CSpriteManager::release()
{
	delete mInstance;
	mInstance = 0;
}

CSpriteManager::CSpriteManager(int maxDepth):
mMaxDepth(maxDepth),
mDecalsCount(0),
mSprites(SPRITES_INITIAL_CAPACITY + 1) // +1 for dummy sprite
{
	int blocksCount = ((maxDepth - 1) >> 5) + 1;

	mDepthBlocks = new TDepthBlock32[blocksCount];
	mDepthEndIds = new int[maxDepth];
	mSpritesCountPerDepth = new int[maxDepth];

	mDummySprite.mId = mSprites.add(&mDummySprite);
	mDummySprite.mDepth = 0;

	for (int i = 0; i < blocksCount; i++)
	{
		mDepthBlocks[i].mBitMask = 0;
		mDepthBlocks[i].mHighestDepth = 0;
	}

	memset(mSpritesCountPerDepth, 0, maxDepth * sizeof(int));
}

CSpriteManager::~CSpriteManager()
{
	delete [] mSpritesCountPerDepth;
	delete [] mDepthEndIds;
	delete [] mDepthBlocks;
}

void CSpriteManager::addSprite(ISprite* sprite, int depth)
{
	int insertId = mDepthEndIds[depth];

	if (mSpritesCountPerDepth[depth]++ == 0) // is it a first sprite with this depth?
	{
		int blockId = (depth >> 5);
		int depthOffset = (depth - blockId);

		if ((mDepthBlocks[blockId].mBitMask & ((1 << depthOffset) - 1)) == 0) // are there no any depths in this block which are lower than the current depth?
		{
			// need to find the closest preceding block
			int precedingBlockId = blockId - 1;
			while ((precedingBlockId >= 0) && (mDepthBlocks[precedingBlockId].mBitMask == 0)) precedingBlockId--;

			if (precedingBlockId < 0)
			{
				insertId = mDummySprite.mId;
			}
			else
			{
				insertId = mDepthEndIds[mDepthBlocks[precedingBlockId].mHighestDepth];
			}
		}
		else // need to find the closest preceding depth in this block
		{
			unsigned long bitMask = mDepthBlocks[blockId].mBitMask;

			int offset = depthOffset - 1; // -1 to avoid checking bit of current depth
			/* no need to check offset for negative value since we are sure
			   that this block definitely contains depth which is lower than current */
			while ((bitMask & (1 << offset)) == 0) offset--;

			insertId = mDepthEndIds[(blockId << 5) + offset];
		}

		// add new depth to this block
		mDepthBlocks[blockId].mBitMask |= (1 << depthOffset);
		mDepthBlocks[blockId].mHighestDepth = XMATH_MAX(mDepthBlocks[blockId].mHighestDepth, depth);
	}

	sprite->mId = mDepthEndIds[depth] = mSprites.insert(sprite, insertId);
	sprite->mDepth = depth;
}

void CSpriteManager::removeSprite(ISprite* sprite)
{
	int depth = sprite->mDepth;
	if (--mSpritesCountPerDepth[depth] == 0) // is it a last sprite with this depth?
	{
		int blockId = (depth >> 5);

		if (mDepthBlocks[blockId].mHighestDepth == depth)
		{
			// need to find another highest depth for this block
			ISprite* prev = mSprites.getPrevious(sprite->mId);

			if ((prev->mDepth >> 5) == blockId)
			{
				mDepthBlocks[blockId].mHighestDepth = prev->mDepth;
			}
			else
			{
				// this block has no depths left, set it's highest depth to minimal value
				mDepthBlocks[blockId].mHighestDepth = 0;
			}
		}

		mDepthBlocks[blockId].mBitMask &= (~(1 << (depth - (blockId << 5))));
	}
	else if (mDepthEndIds[depth] == sprite->mId)
	{
		// need to set new depth end id
		mDepthEndIds[depth] = (mSprites.getPrevious(sprite->mId))->mId; // getPrevious() method cannot return NULL here because of dummy sprite
	}

	mSprites.remove(sprite->mId);
}

void CSpriteManager::animateSprites()
{
	ISprite* sprite;
	mSprites.setPosition(mDummySprite.mId);
	while ((sprite = mSprites.next()) != 0) sprite->animate();
}

void CSpriteManager::drawSprites()
{
	CLayerManager::instance()->getRenderLayer()->beginSprite();

	ISprite* sprite;
	mSprites.setPosition(mDummySprite.mId);
	while ((sprite = mSprites.next()) != 0) sprite->draw();

	CLayerManager::instance()->getRenderLayer()->endSprite();
}

void CSpriteManager::releaseSprites()
{
	ISprite* sprite;
	mSprites.setPosition(mDummySprite.mId);
	while ((sprite = mSprites.next()) != 0) delete sprite;

	mSprites.clear();
	mDummySprite.mId = mSprites.add(&mDummySprite);
}

void CSpriteManager::addDecal(IDecal* decal)
{
	if (mDecalsCount < MAX_DECALS_PER_FRAME)
	{
		mDecals[mDecalsCount++] = decal;
	}
}

void CSpriteManager::drawDecals(ILandscape* landscape)
{
	if (mDecalsCount > 0)
	{
		landscape->beginUpdate();

		CLayerManager::instance()->getRenderLayer()->beginSprite();
		for (int id = 0; id < mDecalsCount; id++)
			mDecals[id]->draw();
		CLayerManager::instance()->getRenderLayer()->endSprite();

		landscape->endUpdate();

		mDecalsCount = 0;
	}
}