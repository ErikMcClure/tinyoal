// Copyright ©2018 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __AUDIO_RESOURCE_OGG_H__TOAL__
#define __AUDIO_RESOURCE_OGG_H__TOAL__

#include "tinyoal/AudioResource.h"
#include "OggFunctions.h"

namespace tinyoal {
  struct OggVorbis_FileEx { // To make things simpler, we append data streaming information to the end of the ogg file.
    OggVorbis_File ogg;
    DatStream stream;
  };

  // This is a resource class for OGG files, and handles all the IO operations from the given buffer 
  class AudioResourceOGG : public AudioResource
  {
  public:
    AudioResourceOGG(void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop);
    ~AudioResourceOGG();
    virtual void* OpenStream(); // This returns a pointer to the internal stream on success, or NULL on failure 
    virtual void CloseStream(void* stream); //This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, unsigned int len, bool& eof); // Reads next chunk of data - buffer must be at least GetBufSize() long 
    virtual bool Reset(void* stream); // This resets a stream to the beginning 
    virtual bool Skip(void* stream, uint64_t samples); // Sets a stream to given sample 
    virtual uint64_t Tell(void* stream); // Gets what sample a stream is currently on

    static size_t Construct(void* p, void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop);
    static bool ScanHeader(const char* fileheader);
    static std::pair<void*, unsigned int> ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags);

  protected:
    static unsigned long _read(void* stream, char* buffer, unsigned int len, bool& eof, char bytes, unsigned int channels); // Reads next chunk of data - buffer must be at least GetBufSize() long 
    bool _openstream(OggVorbis_FileEx* target);
    static void _setcallbacks(ov_callbacks& callbacks, bool isfile);

    ov_callbacks _callbacks;
  };
}

#endif