// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __WAVE_FUNCTIONS_H__
#define __WAVE_FUNCTIONS_H__

#include <stdio.h>
#include "tinyoal/AudioResource.h"

#ifdef BUN_PLATFORM_WIN32
#include "win32_includes.h"
  #include <mmreg.h>
#else
  // WAVE file speaker masks (taken from the ksmedia.h windows file)
  #define SPEAKER_FRONT_LEFT            0x1
  #define SPEAKER_FRONT_RIGHT           0x2
  #define SPEAKER_FRONT_CENTER          0x4
  #define SPEAKER_LOW_FREQUENCY         0x8
  #define SPEAKER_BACK_LEFT             0x10
  #define SPEAKER_BACK_RIGHT            0x20
  #define SPEAKER_FRONT_LEFT_OF_CENTER  0x40
  #define SPEAKER_FRONT_RIGHT_OF_CENTER 0x80
  #define SPEAKER_BACK_CENTER           0x100
  #define SPEAKER_SIDE_LEFT             0x200
  #define SPEAKER_SIDE_RIGHT            0x400
  #define SPEAKER_TOP_CENTER            0x800
  #define SPEAKER_TOP_FRONT_LEFT        0x1000
  #define SPEAKER_TOP_FRONT_CENTER      0x2000
  #define SPEAKER_TOP_FRONT_RIGHT       0x4000
  #define SPEAKER_TOP_BACK_LEFT         0x8000
  #define SPEAKER_TOP_BACK_CENTER       0x10000
  #define SPEAKER_TOP_BACK_RIGHT        0x20000

  #define WAVE_FORMAT_PCM        1
  #define WAVE_FORMAT_EXTENSIBLE 0xFFFE
  #define WAVE_FORMAT_IEEE_FLOAT 0x0003 /* IEEE Float */
  #define WAVE_FORMAT_ALAW       0x0006 /* ALAW */
  #define WAVE_FORMAT_MULAW      0x0007 /* MULAW */
  #define WAVE_FORMAT_IMA_ADPCM  0x0011 /* IMA ADPCM */

typedef struct
{
  uint16_t wFormatTag;
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
  uint16_t cbSize;
} WAVEFORMATEX;

typedef struct
{
  WAVEFORMATEX Format;
  union
  {
    uint16_t wValidBitsPerSample;
    uint16_t wSamplesPerBlock;
    uint16_t wReserved;
  } Samples;
  uint32_t dwChannelMask;
  uint32_t Data1; // This is an expanded GUID
  uint16_t Data2;
  uint16_t Data3;
  unsigned char Data4[8];
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;

#endif

namespace tinyoal {
  struct wav_callbacks
  {
    size_t (*read_func)(void* ptr, size_t size, size_t nmemb, void* datasource);
    int (*seek_func)(void* datasource, int64_t offset, int whence);
    int (*close_func)(void* datasource);
    long (*tell_func)(void* datasource);
  };

  typedef struct WaveFileInfo
  {
    WAVEFORMATEXTENSIBLE wfEXT; // This contains WAVEFORMATEX as well
    size_t offset;
    size_t size;
    wav_callbacks callbacks;
    void* source;
    DatStream stream;
  } WAVEFILEINFO;

  class WaveFunctions
  {
  public:
    enum WAVERESULT
    {
      WR_OK           = 0,
      WR_BADWAVEFILE  = -1,
      WR_INVALIDPARAM = -2,
    };

    WaveFunctions();
    WAVERESULT Open(void* source, WAVEFILEINFO* wave, wav_callbacks& callbacks);
    WAVERESULT Read(WAVEFILEINFO& wave, void* data, size_t len, size_t* pBytesWritten);
    WAVERESULT Seek(WAVEFILEINFO& wave, int64_t offset);
    uint64_t Tell(WAVEFILEINFO& wave);
    WAVERESULT Close(WAVEFILEINFO& wave);
    uint32_t GetALFormat(WAVEFILEINFO& wave); // cast this to ALenum
    uint32_t WriteHeader(char* buffer, uint32_t length, uint16_t channels, uint16_t bits,
                             uint32_t freq);
  };
}

#endif // _CWAVES_H_
