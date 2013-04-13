#pragma once

#include "vorbisfile.h"

class COGGDecoder
{
public:
	COGGDecoder();
	~COGGDecoder();

	bool openFile(const char* fileName);
	
	int decode(void* buffer, int size);
	void reset();

	inline int getChannels() { return mChannels; }
	inline int getFrequency() { return mFrequency; }
	inline int getLength() { return mLength; }

private:
	FILE*          mFile;
	int            mChannels;
	int            mFrequency;
	int            mLength;
	int            mBPS;
	OggVorbis_File mOggFile;
};