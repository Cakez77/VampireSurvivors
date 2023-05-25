#pragma once
#include "config.h"
#include "platform.h"

// WAV File Data
struct WaveFileHeader
{
  char chunkId[4];
  uint32_t chunkSize;
  char format[4];
  char subChunkId[4];
  uint32_t subChunkSize;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t bytesPerSecond;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
};

struct WaveDataChunk
{
  char dataChunkId[4];
  uint32_t dataSize;
};


global_variable char* SoundFiles[] =
{
  "assets/sounds/Copper Green Intent.wav", // SOUND_BACKGROUND,
  "assets/sounds/schlitz.wav",             // SOUND_SCHLITZ,
};

static_assert(ArraySize(SoundFiles) == SOUND_COUNT);

struct Sound
{
  SoundID ID;
  int sizeInBytes;
  char* data;
};

struct SoundState
{
  int bytesUsed;
  char* buffer;
  int allocatedSoundsCount;
  Sound allocatedSounds[MAX_ALLOCATED_SOUNDS];
};

global_variable SoundState soundState;
