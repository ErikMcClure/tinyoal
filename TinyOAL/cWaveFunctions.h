// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_WAVE_FUNCTIONS_H__
#define __C_WAVE_FUNCTIONS_H__

#include <stdio.h>
#include "cAudioResource.h"

typedef struct { 
  unsigned short  wFormatTag;
  unsigned short  nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  unsigned short  nBlockAlign;
  unsigned short  wBitsPerSample;
  unsigned short  cbSize;
} WAVEFORMATEX;

typedef struct {
  WAVEFORMATEX Format;
  union {
    unsigned short wValidBitsPerSample;
    unsigned short wSamplesPerBlock;
    unsigned short wReserved;
  } Samples;
  uint32_t        dwChannelMask;
  uint32_t  Data1; // This is an expanded GUID
  unsigned short Data2;
  unsigned short Data3;
  unsigned char  Data4[8];
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;

namespace tinyoal {
  struct wav_callbacks {
    size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource);
    int    (*seek_func)  (void *datasource, int64_t offset, int whence);
    int    (*close_func) (void *datasource);
    long   (*tell_func)  (void *datasource);
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

  class cWaveFunctions  
  {
  public:
    enum WAVERESULT
    {
	    WR_OK = 0,
	    WR_BADWAVEFILE = -1,
	    WR_INVALIDPARAM	= -2,
    };

    cWaveFunctions();
	  WAVERESULT Open(void* source, WAVEFILEINFO* wave, wav_callbacks& callbacks);
	  WAVERESULT Read(WAVEFILEINFO& wave, void* data, size_t len, size_t* pBytesWritten);
	  WAVERESULT Seek(WAVEFILEINFO& wave, int64_t offset);
	  uint64_t Tell(WAVEFILEINFO& wave);
	  WAVERESULT Close(WAVEFILEINFO& wave);
    unsigned int GetALFormat(WAVEFILEINFO& wave); // cast this to ALenum
    unsigned int WriteHeader(char* buffer,unsigned int length,unsigned short channels, unsigned short bits, uint32_t freq);
  };
}

#endif // _CWAVES_H_
