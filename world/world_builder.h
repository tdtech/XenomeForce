#pragma once

#include <windows.h>

class IWorldBuilderListener
{
public:
	virtual ~IWorldBuilderListener() {}

	virtual void onProgress(int progress) = 0; // 0 <= progress <= 100
};

class CWorldBuilder
{
public:
	CWorldBuilder(const char* fileName, IWorldBuilderListener* listener);
	~CWorldBuilder();

	bool buildWorld(); // builds world in seperate thread, true if building is started successfully
	bool isBuilding();

private:
	void notifyProgress(int progress);
	static DWORD WINAPI buildWorldThreadFunc(void* data);

private:
	char*                  mFileName;
	IWorldBuilderListener* mListener;
	volatile bool          mIsBuilding;
};