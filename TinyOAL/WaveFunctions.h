// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __WAVE_FUNCTIONS_H__
#define __WAVE_FUNCTIONS_H__

#include <stdio.h>
#include "tinyoal/AudioResource.h"

#ifdef BSS_PLATFORM_WIN32
  #pragma pack(push)
  #pragma pack(8)
  #define WINVER        0x0601 //_WIN32_WINNT_WIN7
  #define _WIN32_WINNT  0x0601
  #define NTDDI_VERSION 0x06010000 // NTDDI_WIN7
  #define WIN32_LEAN_AND_MEAN
  #ifndef NOMINMAX // Some compilers enable this by default
    #define NOMINMAX
  #endif
  #define NODRAWTEXT
  #define NOBITMAP
  #define NOMCX
  #define NOSERVICE
  #define NOHELP
  #include <windows.h>
  #pragma pack(pop)

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
  unsigned short wFormatTag;
  unsigned short nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  unsigned short nBlockAlign;
  unsigned short wBitsPerSample;
  unsigned short cbSize;
} WAVEFORMATEX;

typedef struct
{
  WAVEFORMATEX Format;
  union
  {
    unsigned short wValidBitsPerSample;
    unsigned short wSamplesPerBlock;
    unsigned short wReserved;
  } Samples;
  uint32_t dwChannelMask;
  uint32_t Data1; // This is an expanded GUID
  unsigned short Data2;
  unsigned short Data3;
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
    unsigned int GetALFormat(WAVEFILEINFO& wave); // cast this to ALenum
    unsigned int WriteHeader(char* buffer, unsigned int length, unsigned short channels, unsigned short bits,
                             uint32_t freq);
  };
}

#endif // _CWAVES_H_
