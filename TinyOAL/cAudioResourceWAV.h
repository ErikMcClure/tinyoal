// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_AUDIO_RESOURCE_WAV_H__TOAL__
#define __C_AUDIO_RESOURCE_WAV_H__TOAL__

#include "cAudioResource.h"
#include "cWaveFunctions.h"

namespace TinyOAL {
	// This is a resource class for WAV files, and handles all the IO operations from the given buffer 
  class cAudioResourceWAV : public cAudioResource
  {
  public:
    cAudioResourceWAV(const cAudioResourceWAV& copy);
    cAudioResourceWAV(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop);
    ~cAudioResourceWAV();
    virtual void* OpenStream(); // This returns a pointer to the internal stream on success, or NULL on failure 
    virtual void CloseStream(void* stream); //This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, unsigned int len, bool& eof); // Reads next chunk of data - buffer must be at least GetBufSize() long 
    virtual bool Reset(void* stream); // This resets a stream to the beginning 
    virtual bool Skip(void* stream, unsigned __int64 samples); // Sets a stream to given sample 
    virtual unsigned __int64 Tell(void* stream); // Gets what sample a stream is currently on

  protected:
    WAVEFILEINFO _sentinel; // stored wave file information state at the beginning of the file
    static bss_util::cFixedAlloc<WAVEFILEINFO> _allocwav;
  };
}

#endif