// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_AUDIO_RESOURCE_OGG_H__TOAL__
#define __C_AUDIO_RESOURCE_OGG_H__TOAL__

#include "cAudioResource.h"
#include "openAL/vorbisfile.h"

struct vorbis_info;

namespace TinyOAL {
	/* This is a resource class for OGG files, and handles all the IO operations from the given buffer */
  class cAudioResourceOGG : public cAudioResource
  {
  public:
    /* This returns an AUDIOSTREAM on success, or NULL on failure */
    virtual AUDIOSTREAM* OpenStream();
    /* This reads the next chunk of OGG specific data. pDecodeBuffer must be at least GetBufSize() long */
    virtual unsigned long ReadNext(AUDIOSTREAM* stream, char* pDecodeBuffer);
    /* This resets the stream to the beginning. */
    virtual bool Reset(AUDIOSTREAM* stream);
    /* This closes a stream and destroys any associated data (not the actual audio source itself) */
    virtual void CloseStream(AUDIOSTREAM* stream);
    /* Skips the stream to the given sample */
    virtual bool Skip(AUDIOSTREAM* stream, unsigned __int64 samples);
    /* Gets sample point of given time */
    virtual unsigned __int64 ToSample(AUDIOSTREAM* stream, double seconds);
    /* Get embedded loopstart */
    virtual unsigned __int64 GetLoopStart(AUDIOSTREAM* stream);

  protected:
    friend class cAudioResourceOGG; //This, combined with protected constructors/destructors, ensures a client program cannot, under any circumstance, delete an audio resource without calling the appropriate function
    friend class cAudioResource;

    cAudioResourceOGG(const cAudioResourceOGG& copy);
    cAudioResourceOGG(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags);
    cAudioResourceOGG(const char* file, T_TINYOAL_FLAGS flags);
    cAudioResourceOGG(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags);
    ~cAudioResourceOGG();
    bool _buildstream(bool file);
    void _altclearstream(AUDIOSTREAM* protostream);
    void Swap(short &s1, short &s2);

	  ov_callbacks	sCallbacks;
    bool _readinfo;
    ogg_int64_t _loopstart;
  };
}

#endif