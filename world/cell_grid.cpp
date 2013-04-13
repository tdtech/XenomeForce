#include <stdlib.h>
#include <string.h>

#include "cell_grid.h"

/* Prime number, so cell grid will always overlap landscape dimensions with
   some margin. Made to improve performance and reduce memory usage.
   Restrictions: landscape dimensions should not be a multiple of CELL_SIZE value. */
#define CELL_SIZE 257

#define TO_CELL(x) ((int)x) / CELL_SIZE

bool ICellObject::intersects(CVector2D& position, float radius)
{
	CVector2D sub = position - getPosition();
	radius += getRadius();

	return ((sub * sub) < (radius * radius));
}

//-----------------------------------------------

class CCell
{
public:
	void increaseDepth(int newDepth); // assume newDepth >= mDepth
	void release();

public:
	ICellObject** mObjects;
	int           mDepth;
};

void CCell::release()
{
	delete [] mObjects;
}

void CCell::increaseDepth(int newDepth)
{
	ICellObject** newObjects = new ICellObject*[newDepth];
	memcpy(newObjects, mObjects, mDepth * sizeof(ICellObject*));
	delete [] mObjects;
	mObjects = newObjects;
	mDepth = newDepth;
}

//-----------------------------------------------

CCellGrid::CCellGrid(int initialCapacity, int width, int height, int initialCellDepth):
mCellObjects(initialCapacity),
mSearchToken(0)
{
	mGridWidth = (width / CELL_SIZE) + 1;
	mGridHeight = (height / CELL_SIZE) + 1;

	mObjectCountPerCellSize = mGridWidth * mGridHeight * sizeof(int);
	mObjectCountPerCell = (int*)malloc(mObjectCountPerCellSize);
	memset(mObjectCountPerCell, 0, mObjectCountPerCellSize);

	mGrid = (CCell**)malloc(mGridHeight * sizeof(CCell*));
	for (int i = 0; i < mGridHeight; i++)
	{
		mGrid[i] = (CCell*)malloc(mGridWidth * sizeof(CCell));
		for (int j = 0; j < mGridWidth; j++)
		{
			mGrid[i][j].mObjects = 0;
			mGrid[i][j].mDepth = 0;
			mGrid[i][j].increaseDepth(initialCellDepth);
		}
	}
}

CCellGrid::~CCellGrid()
{
	for (int i = 0; i < mGridHeight; i++)
	{
		for (int j = 0; j < mGridWidth; j++)
			mGrid[i][j].release();

		free(mGrid[i]);
	}
	free(mGrid);

	free(mObjectCountPerCell);
}

void CCellGrid::addCellObject(ICellObject* obj)
{
	obj->mId = mCellObjects.add(obj);
	obj->mToken = mSearchToken;
}

void CCellGrid::removeCellObject(ICellObject* obj)
{
	mCellObjects.remove(obj->mId);
}

void CCellGrid::update()
{
	ICellObject* obj;
	float radius;
	int cellX0, cellY0, cellX1, cellY1, cellX;
	int offsetY, id;

	memset(mObjectCountPerCell, 0, mObjectCountPerCellSize);
	mCellObjects.reset();

	while ((obj = mCellObjects.next()) != 0)
	{
		CVector2D& pos = obj->getPosition();
		radius = obj->getRadius();

		cellX0 = TO_CELL(pos.mX - radius);
		cellY0 = TO_CELL(pos.mY - radius);
		cellX1 = TO_CELL(pos.mX + radius);
		cellY1 = TO_CELL(pos.mY + radius);

		for (; cellY0 <= cellY1; cellY0++)
		{
			offsetY = cellY0 * mGridWidth;
			for (cellX = cellX0; cellX <= cellX1; cellX++)
			{
				id = mObjectCountPerCell[offsetY + cellX]++;
				// if max depth is reached then increase it by 8
				if (id >= mGrid[cellX][cellY0].mDepth) mGrid[cellX][cellY0].increaseDepth(mGrid[cellX][cellY0].mDepth + 8);
				mGrid[cellX][cellY0].mObjects[id] = obj;
			}
		}
	}
}

void CCellGrid::search(CVector2D& pos, float radius, TSearchResults& searchResults)
{
	int cellX0, cellY0, cellX1, cellY1, cellX;
	int offsetY, countPerCell, id;
	ICellObject* obj;

	searchResults.mObjectsCount = 0;
	mSearchToken++;

	cellX0 = TO_CELL(pos.mX - radius);
	cellY0 = TO_CELL(pos.mY - radius);
	cellX1 = TO_CELL(pos.mX + radius);
	cellY1 = TO_CELL(pos.mY + radius);

	if (cellX0 < 0) cellX0 = 0;
	if (cellY0 < 0) cellY0 = 0;
	if (cellX1 >= mGridWidth) cellX1 = mGridWidth - 1;
	if (cellY1 >= mGridHeight) cellY1 = mGridHeight - 1;

	for (; cellY0 <= cellY1; cellY0++)
	{
		offsetY = cellY0 * mGridWidth;
		for (cellX = cellX0; cellX <= cellX1; cellX++)
		{
			countPerCell = mObjectCountPerCell[offsetY + cellX];
			for (id = 0; id < countPerCell; id++)
			{
				obj = mGrid[cellX][cellY0].mObjects[id];
				if (obj->mToken != mSearchToken)
				{
					obj->mToken = mSearchToken;
					if (obj->intersects(pos, radius))
					{
						searchResults.mObjects[searchResults.mObjectsCount++] = obj;
					}
				}
			}
		}
	}
}

void CCellGrid::clear()
{
	mCellObjects.clear();
	memset(mObjectCountPerCell, 0, mObjectCountPerCellSize);
	mSearchToken = 0;
}