#pragma once

#include <windows.h>

#define MAX_SOUNDS 128
#define MAX_MIDI 32

typedef unsigned int SOUND_REF;
typedef unsigned int MIDI_REF;

class ISoundLayer
{
public:

// Initialization
	virtual int init() = 0;

// Load and clone
	virtual int loadOGG(char* fileName) = 0; // This is for music
	virtual int loadWAV(char* fileName, SOUND_REF& sound) = 0;
	virtual int loadMIDI(char* fileName, MIDI_REF& midi) = 0;
	virtual int clone(SOUND_REF srcSound, SOUND_REF& destSound) = 0;

// Play
	virtual int playMusic() = 0;
	virtual int play(SOUND_REF sound) = 0;
	virtual int playLoop(SOUND_REF sound) = 0;
	virtual int playMIDI(MIDI_REF midi) = 0;
	
// Stop
	virtual int stopMusic() = 0;
	virtual int stop(SOUND_REF sound) = 0;
	virtual int stopMIDI(MIDI_REF midi) = 0;
	
// Properties
	virtual int setMusicVolume(int volume) = 0; // [0..100]
	virtual int setSoundVolume(SOUND_REF sound, int volume) = 0; // [0..100]
	virtual int setSoundFrequency(SOUND_REF sound, int frequency) = 0; // in HZ
	virtual int setSoundPan(SOUND_REF sound, int pan) = 0; // [-10000..10000]
	
// Release
	virtual int releaseMusic() = 0;
	virtual int releaseSound(SOUND_REF sound) = 0;
	virtual int releaseMIDI(MIDI_REF midi) = 0;

	virtual int release() = 0;
// add more methods...

protected:
	virtual ~ISoundLayer() {}
};

// ==========================================================

ISoundLayer* obtainDirectXSoundLayer(HWND hwnd);