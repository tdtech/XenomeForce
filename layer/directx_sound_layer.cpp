#include "..\common\common.h"
#include "..\log\logger.h"
#include "directx_sound_layer.h"

#define MUSICBUFFER_SIZE 705600
#define MUSICBUFFER_PART_1 2
#define MUSICBUFFER_PART_2 (MUSICBUFFER_SIZE / 2 + 1)

#define DSVOLUME_TO_DB(volume) ((long)(-30 * (100 - volume)))

CDirectXSoundLayer* CDirectXSoundLayer::mInstance = NULL;
int CDirectXSoundLayer::mRefCount = 0;

CDirectXSoundLayer* CDirectXSoundLayer::obtainInstance(HWND hwnd)
{
	if (mInstance == NULL) mInstance = new CDirectXSoundLayer(hwnd);

	mRefCount++;
	return mInstance;
}

CDirectXSoundLayer::CDirectXSoundLayer(HWND hwnd):
mDSoundInterface(NULL),
mDSoundMusicBuffer(NULL),
mDSoundPrimaryBuffer(NULL),
mHwnd(hwnd),
mMusicBufferOffset(0)
{
	unsigned int i;
	for (i = 0; i < MAX_SOUNDS; i++)
		mDSoundBuffer[i] = NULL;
	for (i = 0; i < SIGNAL_EVENTS_COUNT; i++)
		mSignalEvent[i] = NULL;
}

CDirectXSoundLayer::~CDirectXSoundLayer()
{
	releaseResources();
}

int CDirectXSoundLayer::init()
{
	releaseResources();
	if (FAILED(DirectSoundCreate8(&DSDEVID_DefaultPlayback, &mDSoundInterface, NULL)))
	{
		LOGTEXT("CDirectXSoundLayer: DirectSoundCreate() failed");
		return E_FAILED;
	}
	if (FAILED(mDSoundInterface->SetCooperativeLevel(mHwnd, DSSCL_PRIORITY)))
	{
		LOGTEXT("CDirectXSoundLayer: SetCooperativeLevel() with PRIORITY failed");
		releaseResources();
		return E_FAILED;
	}
	WAVEFORMATEX waveFormat;
	ZeroMemory(&waveFormat, sizeof(waveFormat));
	const INT nBitCount     = 16;
	const INT nChannelCount = 2;
	const int nSampleRate   = 44100;
	waveFormat.cbSize = sizeof(WAVEFORMATEX);
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = nChannelCount;
	waveFormat.nSamplesPerSec = nSampleRate;
	waveFormat.nBlockAlign = nChannelCount * nBitCount / 8;
	waveFormat.nAvgBytesPerSec = nSampleRate * waveFormat.nBlockAlign;
	waveFormat.wBitsPerSample = nBitCount;
	DSBUFFERDESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(DSBUFFERDESC));
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.lpwfxFormat = 0;
	if (FAILED(mDSoundInterface->CreateSoundBuffer(&bufferDesc, &mDSoundPrimaryBuffer, NULL)))
	{
		LOGTEXT("CDirectXSoundLayer: Creation of primary buffer failed");
		releaseResources();
		return E_FAILED;
	}
	HRESULT hr;
	if (FAILED(hr = mDSoundPrimaryBuffer->SetFormat(&waveFormat)))
	{
		LOGTEXT("CDirectXSoundLayer: SetFormat() for primary buffer failed");
		releaseResources();
		return E_FAILED;
	}
	for (int i = 0; i < SIGNAL_EVENTS_COUNT; i++)
	{
		if ((mSignalEvent[i] = CreateEvent(0, TRUE, FALSE, NULL)) == NULL)
		{
			LOGTEXT2("CDirectXSoundLayer: Creation of signal#%d stop event failed", i);
			releaseResources();
			return E_FAILED;
		}
	}
	return E_SUCCESS;
}

int CDirectXSoundLayer::loadOGG(char* fileName)
{
	if (mDSoundInterface)
	{
		if (releaseMusic() == E_SUCCESS)
		{
			if (mOggDecoder.openFile(fileName))
			{
				WAVEFORMATEX waveFormat;
				DSBUFFERDESC bufferDesc;
				ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));
				ZeroMemory(&bufferDesc, sizeof(DSBUFFERDESC));
				waveFormat.cbSize          = sizeof(WAVEFORMATEX);
				waveFormat.wFormatTag      = WAVE_FORMAT_PCM;
				waveFormat.nChannels       = mOggDecoder.getChannels();
				waveFormat.nSamplesPerSec  = mOggDecoder.getFrequency();
				waveFormat.wBitsPerSample  = 16;
				waveFormat.nBlockAlign     = mOggDecoder.getChannels() * waveFormat.wBitsPerSample / 8;
				waveFormat.nAvgBytesPerSec = mOggDecoder.getFrequency() * waveFormat.nBlockAlign;
				bufferDesc.dwSize          = sizeof(DSBUFFERDESC);
				bufferDesc.dwFlags         = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY;
				bufferDesc.dwBufferBytes   = MUSICBUFFER_SIZE;
				bufferDesc.lpwfxFormat     = &waveFormat;
				LPDIRECTSOUNDNOTIFY8 lpDsNotify;
				DSBPOSITIONNOTIFY positionNotify[2];
				positionNotify[0].dwOffset = MUSICBUFFER_PART_1;
				positionNotify[0].hEventNotify = mSignalEvent[1];
				positionNotify[1].dwOffset = MUSICBUFFER_PART_2;
				positionNotify[1].hEventNotify = mSignalEvent[2];
				if (FAILED(mDSoundInterface->CreateSoundBuffer(&bufferDesc, &mDSoundMusicBuffer, NULL)))
				{
					LOGTEXT("CDirectXSoundLayer: CreateSoundBuffer() for music buffer failed");
					return E_FAILED;
				}
				if (FAILED(mDSoundMusicBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&lpDsNotify)))
				{
					LOGTEXT("CDirectXSoundLayer: Creation of IDirectSoundNotify for music buffer failed");
					mDSoundMusicBuffer->Release();
					mDSoundMusicBuffer = NULL;
					return E_FAILED;
				}
				if (FAILED(lpDsNotify->SetNotificationPositions(2, positionNotify)))
				{
					LOGTEXT("CDirectXSoundLayer: SetNotificationPositions() for music buffer failed");
					lpDsNotify->Release();
					mDSoundMusicBuffer->Release();
					mDSoundMusicBuffer = NULL;
					return E_FAILED;
				}
				lpDsNotify->Release();
				return E_SUCCESS;
			}
			LOGTEXT("CDirectXSoundLayer: mOggDecoder.openFile() failed");
		}
	}
	LOGTEXT("CDirectXSoundLayer: loadOGG() failed");
	return E_FAILED;
}

int CDirectXSoundLayer::loadWAV(char* fileName, SOUND_REF& sound)
{
	if (mDSoundInterface)
	{
		if (findFreeSoundRef(sound))
		{
			HMMIO         hwav;    
			MMCKINFO      parent, child;   
			WAVEFORMATEX  wfmtx;
			WAVEFORMATEX  pcmwf;
			DSBUFFERDESC  dsbd;
			unsigned char *snd_buffer;
			unsigned char *audio_ptr_1 = NULL;
			unsigned char *audio_ptr_2 = NULL; 
			unsigned long audio_length_1 = 0;  
			unsigned long audio_length_2 = 0;

			parent.ckid         = (FOURCC)0;
			parent.cksize       = 0;
			parent.fccType      = (FOURCC)0;
			parent.dwDataOffset = 0;
			parent.dwFlags      = 0;

			child = parent;

			if ((hwav = mmioOpenA(fileName, NULL, MMIO_READ | MMIO_ALLOCBUF)) == NULL)
			{
				LOGTEXT("CDirectXSoundLayer: mmioOpenA() failed");
				return E_FAILED;
			}

			parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

			if (mmioDescend(hwav, &parent, NULL, MMIO_FINDRIFF))
			{
				LOGTEXT("CDirectXSoundLayer: mmioDescend() with MMIO_FINDRIFF failed");
				mmioClose(hwav, 0);
				return E_FAILED;
			}

			child.ckid = mmioFOURCC('f', 'm', 't', ' ');

			if (mmioDescend(hwav, &child, &parent, 0))
			{
				LOGTEXT("CDirectXSoundLayer: mmioDescend() with 0 failed");
				mmioClose(hwav, 0);
				return E_FAILED;
			}

			if (mmioRead(hwav, (char *)&wfmtx, sizeof(wfmtx)) != sizeof(wfmtx))
			{
				LOGTEXT("CDirectXSoundLayer: mmioRead() failed");
				mmioClose(hwav, 0);
				return E_FAILED;
			}

			if (wfmtx.wFormatTag != WAVE_FORMAT_PCM)
			{
				LOGTEXT("CDirectXSoundLayer: loadWAV() failed - invalid WAV format");
				mmioClose(hwav, 0);
				return E_FAILED;
			}

			if (mmioAscend(hwav, &child, 0))
			{
				LOGTEXT("CDirectXSoundLayer: mmioAscend() failed");
				mmioClose(hwav, 0);
				return E_FAILED;
			}

			child.ckid = mmioFOURCC('d', 'a', 't', 'a');

			if (mmioDescend(hwav, &child, &parent, MMIO_FINDCHUNK))
			{
				LOGTEXT("CDirectXSoundLayer: mmioDescend() with MMIO_FINDCHUNK failed");
				mmioClose(hwav, 0);
				return E_FAILED;
			}

			snd_buffer = (unsigned char *)malloc(child.cksize);
			mmioRead(hwav, (char *)snd_buffer, child.cksize);
			mmioClose(hwav, 0);

			memset(&pcmwf, 0, sizeof(WAVEFORMATEX));

			pcmwf.wFormatTag      = WAVE_FORMAT_PCM;
			pcmwf.nChannels       = 1;
			pcmwf.nSamplesPerSec  = 11025;
			pcmwf.nBlockAlign     = 1;                
			pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
			pcmwf.wBitsPerSample  = 8;
			pcmwf.cbSize          = 0;

			dsbd.dwSize          = sizeof(DSBUFFERDESC);
			dsbd.dwFlags         = DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
			dsbd.dwBufferBytes   = child.cksize;
			dsbd.lpwfxFormat     = &wfmtx;//&pcmwf;
			dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;
			dsbd.dwReserved      = NULL;

			if (FAILED(mDSoundInterface->CreateSoundBuffer(&dsbd, &mDSoundBuffer[sound], NULL)))
			{
				LOGTEXT("CDirectXSoundLayer: CreateSoundBuffer() failed");
				free(snd_buffer);
				mDSoundBuffer[sound] = NULL;
				return E_FAILED;
			}

			if (FAILED(mDSoundBuffer[sound]->Lock(0, child.cksize, (void **) &audio_ptr_1, &audio_length_1, (void **)&audio_ptr_2, &audio_length_2, DSBLOCK_FROMWRITECURSOR)))
			{
				LOGTEXT("CDirectXSoundLayer: Unable to lock sound buffer");
				free(snd_buffer);
				mDSoundBuffer[sound]->Release();
				mDSoundBuffer[sound] = NULL;
				return E_FAILED;
			}

			memcpy(audio_ptr_1, snd_buffer, audio_length_1);
			memcpy(audio_ptr_2, (snd_buffer + audio_length_1), audio_length_2);
			free(snd_buffer);

			if (FAILED(mDSoundBuffer[sound]->Unlock(audio_ptr_1, audio_length_1, audio_ptr_2, audio_length_2)))
			{
				LOGTEXT("CDirectXSoundLayer: Unable to unlock sound buffer");
				mDSoundBuffer[sound]->Release();
				mDSoundBuffer[sound] = NULL;
				return E_FAILED;
			}

			return E_SUCCESS;
		}
	}
	return E_FAILED;
}

int CDirectXSoundLayer::loadMIDI(char* fileName, MIDI_REF& midi)
{
	// stub
	return E_FAILED;
}

int CDirectXSoundLayer::clone(SOUND_REF srcSound, SOUND_REF& destSound)
{
	if ((mDSoundInterface) && (srcSound < MAX_SOUNDS) && (destSound < MAX_SOUNDS))
	{
		if (findFreeSoundRef(destSound))
		{
			if (FAILED(mDSoundInterface->DuplicateSoundBuffer(mDSoundBuffer[srcSound], &mDSoundBuffer[destSound])))
			{
				LOGTEXT("CDirectXSoundLayer: DuplicateSoundBuffer() failed");
				mDSoundBuffer[destSound] = NULL;
				return E_FAILED;
			}
			return E_SUCCESS;
		}
	}
	return E_FAILED;
}

int CDirectXSoundLayer::playMusic()
{
	if (mDSoundMusicBuffer)
	{
		if (stopMusic() == E_SUCCESS)
		{
			mOggDecoder.reset();
			mMusicBufferOffset = 0;
			if (SUCCEEDED(mDSoundMusicBuffer->SetCurrentPosition(0)))
			{
				fillMusicBuffer();
				if (SUCCEEDED(mDSoundMusicBuffer->Play(0, 0, DSBPLAY_LOOPING)) &&
					(mThread = (HANDLE)CreateThread(NULL, 65536, updateThreadFunc, this, 0, NULL)) != NULL)
				{
					return E_SUCCESS;
				}
			}
			stopMusic();
		}
	}
	return E_FAILED;
}

int CDirectXSoundLayer::play(SOUND_REF sound)
{
	if ((sound < MAX_SOUNDS) && (mDSoundBuffer[sound]))
	{
		return (SUCCEEDED(mDSoundBuffer[sound]->SetCurrentPosition(0)) && SUCCEEDED(mDSoundBuffer[sound]->Play(0, 0, 0)));
	}
	return E_FAILED;
}

int CDirectXSoundLayer::playLoop(SOUND_REF sound)
{
	if ((sound < MAX_SOUNDS) && (mDSoundBuffer[sound]))
	{
		return (SUCCEEDED(mDSoundBuffer[sound]->SetCurrentPosition(0)) && SUCCEEDED(mDSoundBuffer[sound]->Play(0, 0, DSBPLAY_LOOPING)));
	}
	return E_FAILED;
}

int CDirectXSoundLayer::playMIDI(MIDI_REF midi)
{
	// stub
	return E_FAILED;
}

int CDirectXSoundLayer::stopMusic()
{
	if (mThread)
	{
		SetEvent(mSignalEvent[0]);
		if (WaitForSingleObject(mThread, INFINITE) != WAIT_OBJECT_0) return E_FAILED;
		CloseHandle(mThread);
		mThread = NULL;
	}
	if (mDSoundMusicBuffer)
	{
		int result = SUCCEEDED(mDSoundMusicBuffer->Stop());
		for (int i = 0; i < SIGNAL_EVENTS_COUNT; i++) ResetEvent(mSignalEvent[i]);
		return result;
	}
	return E_SUCCESS;
}

int CDirectXSoundLayer::stop(SOUND_REF sound)
{
	if ((sound < MAX_SOUNDS) && (mDSoundBuffer[sound]))
	{
		return SUCCEEDED(mDSoundBuffer[sound]->Stop());
	}
	return E_FAILED;
}

int CDirectXSoundLayer::stopMIDI(MIDI_REF midi)
{
	// stub
	return E_FAILED;
}

int CDirectXSoundLayer::setMusicVolume(int volume)
{
	if (mDSoundMusicBuffer)
	{
		return SUCCEEDED(mDSoundMusicBuffer->SetVolume(DSVOLUME_TO_DB(volume)));
	}
	return E_FAILED;
}

int CDirectXSoundLayer::setSoundVolume(SOUND_REF sound, int volume)
{
	if ((sound < MAX_SOUNDS) && (mDSoundBuffer[sound]))
	{
		return SUCCEEDED(mDSoundBuffer[sound]->SetVolume(DSVOLUME_TO_DB(volume)));
	}
	return E_FAILED;
}

int CDirectXSoundLayer::setSoundFrequency(SOUND_REF sound, int frequency)
{
	if ((sound < MAX_SOUNDS) && (mDSoundBuffer[sound]))
	{
		return SUCCEEDED(mDSoundBuffer[sound]->SetFrequency(frequency));
	}
	return E_FAILED;
}

int CDirectXSoundLayer::setSoundPan(SOUND_REF sound, int pan)
{
	if ((sound < MAX_SOUNDS) && (mDSoundBuffer[sound]))
	{
		return SUCCEEDED(mDSoundBuffer[sound]->SetPan(pan));
	}
	return E_FAILED;
}

int CDirectXSoundLayer::releaseMusic()
{
	if (stopMusic() == E_SUCCESS)
	{
		if (mDSoundMusicBuffer)
		{
			mDSoundMusicBuffer->Release();
			mDSoundMusicBuffer = NULL;
		}
	}
	else
	{
		return E_FAILED;
	}
	return E_SUCCESS;
}

int CDirectXSoundLayer::releaseSound(SOUND_REF sound)
{
	if ((sound < MAX_SOUNDS) && (mDSoundBuffer[sound]))
	{
		mDSoundBuffer[sound]->Stop();
		mDSoundBuffer[sound]->Release();
		mDSoundBuffer[sound] = NULL;
		return E_SUCCESS;
	}
	return E_FAILED;
}

int CDirectXSoundLayer::releaseMIDI(MIDI_REF midi)
{
	// stub
	return E_FAILED;
}

int CDirectXSoundLayer::release()
{
	mRefCount--;

	if (mRefCount == 0)
	{
		delete mInstance;
		mInstance = 0;
	}

	return E_SUCCESS;
}

void CDirectXSoundLayer::updateMusic()
{
	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(SIGNAL_EVENTS_COUNT, mSignalEvent, FALSE, INFINITE);
		if (waitResult == WAIT_OBJECT_0) break;
		if ((waitResult == WAIT_OBJECT_0 + 1) || (waitResult == WAIT_OBJECT_0 + 2))
		{
			ResetEvent(mSignalEvent[waitResult - WAIT_OBJECT_0]);
			fillMusicBuffer();
		}
	}
}

void CDirectXSoundLayer::fillMusicBuffer()
{
	void *pData1, *pData2;
	DWORD nDecoded = 0;
	DWORD dwBytes;
	if (SUCCEEDED(mDSoundMusicBuffer->Lock(mMusicBufferOffset, MUSICBUFFER_SIZE / 2, &pData1, &dwBytes, &pData2, NULL, 0)))
	{
		while (true)
		{
			nDecoded += mOggDecoder.decode((char*)pData1 + nDecoded, dwBytes - nDecoded);
			if (nDecoded < dwBytes) mOggDecoder.reset(); else break;
		}
		mDSoundMusicBuffer->Unlock(pData1, dwBytes, pData2, 0);
	}
	mMusicBufferOffset = (mMusicBufferOffset + (MUSICBUFFER_SIZE / 2)) % MUSICBUFFER_SIZE;
}

unsigned long CDirectXSoundLayer::updateThreadFunc(void* data)
{
	CDirectXSoundLayer* soundEngine = (CDirectXSoundLayer*)data;
	soundEngine->updateMusic();
	ExitThread(1);
}

void CDirectXSoundLayer::releaseResources()
{
	unsigned int i;
	releaseMusic();
	for (i = 0; i < MAX_SOUNDS; i++)
		releaseSound(i);
	for (i = 0; i < MAX_MIDI; i++)
		releaseMIDI(i);
	for (i = 0; i < SIGNAL_EVENTS_COUNT; i++)
	{
		if (mSignalEvent[i])
		{
			CloseHandle(mSignalEvent[i]);
			mSignalEvent[i] = NULL;
		}
	}
	if (mDSoundPrimaryBuffer)
	{
		mDSoundPrimaryBuffer->Release();
		mDSoundPrimaryBuffer = NULL;
	}
	if (mDSoundInterface)
	{
		mDSoundInterface->Release();
		mDSoundInterface = NULL;
	}
}

int CDirectXSoundLayer::findFreeSoundRef(SOUND_REF& ref)
{
	for (ref = 0; ref < MAX_SOUNDS; ref++)
		if (mDSoundBuffer[ref] == NULL) return E_SUCCESS;
	return E_FAILED;
}

// ==========================================================

ISoundLayer* obtainDirectXSoundLayer(HWND hwnd)
{
	return CDirectXSoundLayer::obtainInstance(hwnd);
}