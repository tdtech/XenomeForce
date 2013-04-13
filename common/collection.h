#pragma once

#define INVALID_ID -1

/* List collection with O(1) complexity of operations.
   Modifying the collection while iterating through it IS ALLOWED.
   This collection is intended to use with pointers to class objects and other data structures,
   It is not recommended to use it just for primitive types such as int, long and etc.,
   especially because of memory usage.
   Methods of this collection are not safe: assumed that all input parameters are valid. */
template <class T>
class CCollection
{
public:
	CCollection(int initialCapacity);
	~CCollection();

	/* Adds new item to the end of collection and returns it's unique id,
	   which can be used later for getting or removing this item from collection.
	   COMPLEXITY: O(1) */ 
	int add(T* item);

	/* Inserts new item to collection after the item with specified id and
	   returns unique id of inserted item. 
	   COMPLEXITY: O(1) */
	int insert(T* item, int id);

	/* Returns the item preceding the item with specified id.
	   NULL if no preceding item.
	   COMPLEXITY: O(1) */
	T* getPrevious(int id);

	/* Returns the item with specified id.
	   COMPLEXITY: O(1) */
	T* get(int id);

	/* Returns the item following the item with specified id.
	   NULL if no following item.
	   COMPLEXITY: O(1) */
	T* getNext(int id);

	/* Removes the item with specified id.
	   COMPLEXITY: O(1) */
	void remove(int id);

	/* Resets iteration index to the first item.
	   COMPLEXITY: O(1) */
	void reset();

	/* Sets iteration index to the item with specified id.
	   COMPLEXITY: O(1) */
	void setPosition(int id);

	/* Iterates through collection and returns the next item.
	   NULL means the end of collection, calling next() again in case of end will
	   reset collection and return the first item. If collection contains NULL items,
	   then use size() method and your own counter to determine the end of collection.
	   COMPLEXITY: O(1) */
	T* next();

	// Clears collection.
	void clear();

	// Returns size of collection.
	int size();

private:
	struct CItem
	{
		CItem* mPrev;
		CItem* mNext;
		T*     mData;
	};

	void increaseCapacity();
	int  insert(T* item, CItem* prev);

private:
	int    mCapacity;
	int    mSize;

	CItem* mList;
	CItem* mIterator;
	CItem* mHash;

	int*   mStack;
};

#include "collection.cpp"