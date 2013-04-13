#pragma once

#include "..\common\xmath.h"
#include "..\common\collection.h"

#define DEFAULT_CELL_DEPTH 16 // TODO: find compromise between performance and memory usage

class ICellObject
{
	friend class CCellGrid;

public:
	virtual ~ICellObject() {}

	virtual CVector2D& getPosition() = 0;
	virtual float getRadius() = 0;

private:
	bool intersects(CVector2D& position, float radius);

private:
	int           mId;
	unsigned long mToken;
};

struct TSearchResults
{
	ICellObject** mObjects;
	int           mObjectsCount;
};

class CCell;

class CCellGrid
{
public:
	CCellGrid(int initialCapacity, int width, int height, int initialCellDepth = DEFAULT_CELL_DEPTH);
	~CCellGrid();

	void addCellObject(ICellObject* obj);
	void removeCellObject(ICellObject* obj);

	void update();
	void search(CVector2D& pos, float radius, TSearchResults& searchResults);
	void clear();

private:
	int                      mGridWidth; // in cells
	int                      mGridHeight; // in cells

	CCollection<ICellObject> mCellObjects;

	/* separated from CCell and made as one dimension array for better performance,
	   since memset() works faster than the set of zero assignments. */
	int*                     mObjectCountPerCell;
	int                      mObjectCountPerCellSize;

	unsigned long            mSearchToken;

	CCell**                  mGrid;
};