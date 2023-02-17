// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef TOAL__AUDIO_RESOURCE_WAV_H
#define TOAL__AUDIO_RESOURCE_WAV_H

#include "tinyoal/AudioResource.h"
#include "WaveFunctions.h"

namespace tinyoal {
  // This is a resource class for WAV files, and handles all the IO operations from the given buffer
  class AudioResourceWAV : public AudioResource
  {
  public:
    AudioResourceWAV(void* data, uint32_t datalength, TINYOAL_FLAG flags, uint64_t loop);
    ~AudioResourceWAV();
    virtual void* OpenStream();             // This returns a pointer to the internal stream on success, or NULL on failure
    virtual void CloseStream(void* stream); // This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, uint32_t len,
                               bool& eof); // Reads next chunk of data - buffer must be at least GetBufSize() long
    virtual bool Reset(void* stream);      // This resets a stream to the beginning
    virtual bool Skip(void* stream, uint64_t samples); // Sets a stream to given sample
    virtual uint64_t Tell(void* stream);               // Gets what sample a stream is currently on

    static size_t Construct(void* p, void* data, uint32_t datalength, TINYOAL_FLAG flags, uint64_t loop);
    static bool ScanHeader(const char* fileheader);
    static std::pair<void*, uint32_t> ToWave(void* data, uint32_t datalength, TINYOAL_FLAG flags);

  protected:
    WAVEFILEINFO _sentinel; // stored wave file information state at the beginning of the file
  };
}

#endif