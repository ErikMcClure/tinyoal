// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_AUDIO_RESOURCE_OGG_H__TOAL__
#define __C_AUDIO_RESOURCE_OGG_H__TOAL__

#include "cAudioResource.h"
#include "openAL/vorbisfile.h"

namespace TinyOAL {
  struct OggVorbis_FileEx { // To make things simpler, we append data streaming information to the end of the ogg file.
    OggVorbis_File ogg;
    DatStream stream;
  };

	// This is a resource class for OGG files, and handles all the IO operations from the given buffer 
  class cAudioResourceOGG : public cAudioResource
  {
  public:
    cAudioResourceOGG(const cAudioResourceOGG& copy);
    cAudioResourceOGG(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop);
    ~cAudioResourceOGG();
    virtual void* OpenStream(); // This returns a pointer to the internal stream on success, or NULL on failure 
    virtual void CloseStream(void* stream); //This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer); // Reads next chunk of data - buffer must be at least GetBufSize() long 
    virtual bool Reset(void* stream); // This resets a stream to the beginning 
    virtual bool Skip(void* stream, unsigned __int64 samples); // Sets a stream to given sample 
    virtual unsigned __int64 ToSample(void* stream, double seconds); // Converts given time to sample point

  protected:
    bool _openstream(OggVorbis_FileEx* target);

    ov_callbacks _callbacks;
  };
}

#endif