// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __AUDIO_RESOURCE_M4A_H__TOAL__
#define __AUDIO_RESOURCE_M4A_H__TOAL__

#include "tinyoal/AudioResource.h"

struct IMFSourceReader;

namespace tinyoal {
  // This is a resource class for MP4/M4A files using windows media foundation
  class AudioResourceM4A : public AudioResource
  {
    struct sMP4
    {
      IMFSourceReader* reader;
      std::vector<char> buf;
      int64_t cur;
    };

  public:
    AudioResourceM4A(void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop);
    ~AudioResourceM4A();
    virtual void* OpenStream(); // This returns a pointer to the internal stream on success, or NULL on failure 
    virtual void CloseStream(void* stream); //This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, unsigned int len, bool& eof); // Reads next chunk of data - buffer must be at least GetBufSize() long 
    virtual bool Reset(void* stream); // This resets a stream to the beginning 
    virtual bool Skip(void* stream, uint64_t samples); // Sets a stream to given sample 
    virtual uint64_t Tell(void* stream); // Gets what sample a stream is currently on

    static size_t Construct(void* p, void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop);
    static bool ScanHeader(const char* fileheader);
    static std::pair<void*, unsigned int> ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags);

    static const int TINYOAL_FILETYPE_M4A = AudioResource::TINYOAL_FILETYPE_CUSTOM;

  protected:
  };
}

#endif