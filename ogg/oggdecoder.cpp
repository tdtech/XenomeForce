#include "oggdecoder.h"

static size_t vorbisRead(void* ptr, size_t size, size_t num, void* dataSource)
{
	FILE* file = static_cast<FILE*>(dataSource);
	return fread(ptr, size, num, file);
}

static int vorbisSeek(void* dataSource, ogg_int64_t offset, int whence)
{
	FILE* file = static_cast<FILE*>(dataSource);
	return fseek(file, (long)offset, whence);
}

static long vorbisTell(void* dataSource)
{
	FILE* file = static_cast<FILE*>(dataSource);
	return ftell(file);
}

static int vorbisClose(void* dataSource)
{
	return 1;
}

COGGDecoder::COGGDecoder():
mFile(0),
mChannels(0),
mFrequency(0),
mLength(0),
mBPS(0)
{
}

COGGDecoder::~COGGDecoder()
{
	if (mFile)
	{
		ov_clear(&mOggFile);
		fclose(mFile);
	}
}

bool COGGDecoder::openFile(const char *fileName)
{
	if (mFile)
	{
		ov_clear(&mOggFile);
		fclose(mFile);
	}
	if (!fopen_s(&mFile, fileName, "rb"))
	{
		ov_callbacks callbacks;
		callbacks.close_func = vorbisClose;
		callbacks.read_func = vorbisRead;
		callbacks.seek_func = vorbisSeek;
		callbacks.tell_func = vorbisTell;
		if (!ov_open_callbacks(mFile, &mOggFile, 0, 0, callbacks))
		{
			vorbis_info* info = ov_info(&mOggFile, -1);
			mFrequency = info->rate;
			mChannels  = info->channels;
			mLength    = (int)ov_pcm_total(&mOggFile, -1);
			mBPS       = 1 == mChannels ? 2 : 4;
			return true;
		}
	}
	return false;
}

int COGGDecoder::decode(void *buffer, int size)
{
	int nDecoded = 0;
	if (buffer)
	{
		int nCurrPos = (int)ov_pcm_tell(&mOggFile);
		int nCurr = (mLength - nCurrPos) * mBPS;
		if (nCurr < size) size = nCurr;
		while (nDecoded < size)
		{
			nDecoded += ov_read(&mOggFile, (char*)buffer + nDecoded, size - nDecoded, 0, 2, 1, &nCurr);
		}
	}
	return nDecoded;
}

void COGGDecoder::reset()
{
	ov_pcm_seek(&mOggFile, 0);
}