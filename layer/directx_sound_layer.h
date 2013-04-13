#pragma once

#include <dsound.h>
#include <dmusicc.h>

#include "..\ogg\oggdecoder.h"
#include "sound_layer.h"

#define SIGNAL_EVENTS_COUNT 3

class CDirectXSoundLayer: public ISoundLayer
{
public:
	static CDirectXSoundLayer* obtainInstance(HWND hwnd);

// Initialization
	int init();

// Load and clone
	int loadOGG(char* fileName);
	int loadWAV(char* fileName, SOUND_REF& sound);
	int loadMIDI(char* fileName, MIDI_REF& midi);
	int clone(SOUND_REF srcSound, SOUND_REF& destSound);
	
// Play
	int playMusic();
	int play(SOUND_REF sound);
	int playLoop(SOUND_REF sound);
	int playMIDI(MIDI_REF midi);
	
// Stop
	int stopMusic();
	int stop(SOUND_REF sound);
	int stopMIDI(MIDI_REF midi);
	
// Properties
	int setMusicVolume(int volume);
	int setSoundVolume(SOUND_REF sound, int volume);
	int setSoundFrequency(SOUND_REF sound, int frequency);
	int setSoundPan(SOUND_REF sound, int pan);
	
// Release
	int releaseMusic();
	int releaseSound(SOUND_REF sound);
	int releaseMIDI(MIDI_REF midi);

	int release();
// add more methods...

private:
	CDirectXSoundLayer(HWND hwnd);
	~CDirectXSoundLayer();

	void updateMusic();
	void fillMusicBuffer();
	static DWORD WINAPI updateThreadFunc(void* data);
	void releaseResources();
	int findFreeSoundRef(SOUND_REF& ref);

private:
	LPDIRECTSOUND8             mDSoundInterface;

	LPDIRECTSOUNDBUFFER        mDSoundMusicBuffer;
	LPDIRECTSOUNDBUFFER        mDSoundBuffer[MAX_SOUNDS];
	LPDIRECTSOUNDBUFFER        mDSoundPrimaryBuffer;

	HWND                       mHwnd;
	HANDLE                     mThread;

	/* SignalEvent[1] - Cursor reaches MUSICBUFFER_PART_1
	   SignalEvent[2] - Cursor reaches MUSICBUFFER_PART_2
	   SignalEvent[0] - Signal to stop music thread.
	                    This signal always should be the last */
	HANDLE                     mSignalEvent[SIGNAL_EVENTS_COUNT];

	COGGDecoder                mOggDecoder;

	DWORD                      mMusicBufferOffset;

	static CDirectXSoundLayer* mInstance;
	static int                 mRefCount;
};