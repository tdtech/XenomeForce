#include <string.h>

template <class T>
CCollection<T>::CCollection(int initialCapacity):
mCapacity(initialCapacity)
{
	mHash = new CItem[mCapacity];
	mStack = new int[mCapacity];
	mList = new CItem();

	clear();
}

template <class T>
CCollection<T>::~CCollection()
{
	delete mList;
	delete [] mStack;
	delete [] mHash;
}

template <class T>
int CCollection<T>::add(T* item)
{
	return insert(item, mList->mPrev);
}

template <class T>
int CCollection<T>::insert(T* item, int id)
{
	return insert(item, &mHash[id]);
}

template <class T>
T* CCollection<T>::getPrevious(int id)
{
	return (mHash[id].mPrev)->mData;
}

template <class T>
T* CCollection<T>::get(int id)
{
	return mHash[id].mData;
}

template <class T>
T* CCollection<T>::getNext(int id)
{
	return (mHash[id].mNext)->mData;
}

template <class T>
void CCollection<T>::remove(int id)
{
	if (mIterator == &mHash[id]) mIterator = mHash[id].mPrev;
	mHash[id].mPrev->mNext = mHash[id].mNext;
	mHash[id].mNext->mPrev = mHash[id].mPrev;
	mStack[--mSize] = id;
}

template <class T>
void CCollection<T>::reset()
{
	mIterator = mList;
}

template <class T>
void CCollection<T>::setPosition(int id)
{
	mIterator = &mHash[id];
}

template <class T>
T* CCollection<T>::next()
{
	mIterator = mIterator->mNext;
	return mIterator->mData;
}

template <class T>
void CCollection<T>::clear()
{
	mList->mPrev = mList;
	mList->mNext = mList;
	mList->mData = 0;

	for (int id = 0; id < mCapacity; id++) mStack[id] = id;

	mSize = 0;

	reset();
}

template <class T>
int CCollection<T>::size()
{
	return mSize;
}

template <class T>
void CCollection<T>::increaseCapacity()
{
	int newCapacity = mCapacity + (mCapacity >> 2) + 1; // TODO: maybe pass increment as an input parameter of constructor?
	CItem* newHash = new CItem[newCapacity];
	int* newStack = new int[newCapacity];

	memcpy(newHash, mHash, mCapacity * sizeof(CItem));
	memcpy(newStack, mStack, mCapacity * sizeof(int));

	for (int id = mCapacity; id < newCapacity; id++) newStack[id] = id;

	delete [] mHash;
	delete [] mStack;

	mCapacity = newCapacity;
	mHash = newHash;
	mStack = newStack;
}

template <class T>
int CCollection<T>::insert(T* item, CItem* prev)
{
	int id;

	if (mSize == mCapacity)
	    increaseCapacity();

	id = mStack[mSize++];
	CItem* listItem = &mHash[id];
	listItem->mData = item;

	prev->mNext = listItem;
	listItem->mPrev = prev;
	listItem->mNext = mList;
	mList->mPrev = listItem;

	return id;
}