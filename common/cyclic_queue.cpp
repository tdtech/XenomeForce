template <class T>
CCyclicQueue<T>::CCyclicQueue(int capacity):
mCapacity(capacity),
mLength(0),
mIndex(-1)
{
	mQueue = new T*[mCapacity];
}

template <class T>
CCyclicQueue<T>::~CCyclicQueue()
{
	delete [] mQueue;
}

template <class T>
void CCyclicQueue<T>::addItem(T* item)
{
	if (mLength < mCapacity) mQueue[++mLength] = item;
}

template <class T>
T* CCyclicQueue<T>::nextItem()
{
	// assume that mLength is not 0 here - small performance improvement
	mIndex = (mIndex + 1) % mLength;
	return mQueue[mIndex];
}