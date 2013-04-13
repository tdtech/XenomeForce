#pragma once

template <class T>
class CCyclicQueue
{
public:
	CCyclicQueue(int capacity);
	~CCyclicQueue();

	void addItem(T* item);
	T*   nextItem(); // MUST be called only when queue IS NOT empty

private:
	int mCapacity;
	int mLength;
	int mIndex;

	T** mQueue;
};

#include "cyclic_queue.cpp"