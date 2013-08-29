// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_AUDIO_RESOURCE_WAV_H__TOAL__
#define __C_AUDIO_RESOURCE_WAV_H__TOAL__

#include "cAudioResource.h"

class CWaves;
struct wav_callbacks;
namespace TinyOAL {
	/* This is a resource class for WAV files, and handles all the IO operations from the given buffer */
  class cAudioResourceWAV : public cAudioResource
  {
  public:
    /* This returns an AUDIOSTREAM on success, or NULL on failure */
    virtual AUDIOSTREAM* OpenStream();
    /* This reads the next chunk of WAV data. pDecodeBuffer must be at least GetBufSize() long */
    virtual unsigned long ReadNext(AUDIOSTREAM* stream, char* pDecodeBuffer);
    /* This resets the stream to the beginning. */
    virtual bool Reset(AUDIOSTREAM* stream);
    /* This closes a stream and destroys any associated data (not the actual audio source itself) */
    virtual void CloseStream(AUDIOSTREAM* stream);
    /* Skips the stream to the given sample */
    virtual bool Skip(AUDIOSTREAM* stream, unsigned __int64 samples);
    /* Gets sample point of given time */
    virtual unsigned __int64 ToSample(AUDIOSTREAM* stream, double seconds);

  protected:
    friend class cAudioResourceWAV; //This, combined with protected constructors/destructors, ensures a client program cannot, under any circumstance, delete an audio resource without calling the appropriate function
    friend class cAudioResource;
    
    cAudioResourceWAV(const cAudioResourceWAV& copy);
    cAudioResourceWAV(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags);
    cAudioResourceWAV(const char* file, T_TINYOAL_FLAGS flags);
    cAudioResourceWAV(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags);
    ~cAudioResourceWAV();
    bool _buildstream(bool file);

    CWaves* _waves;
    wav_callbacks* _callbacks;
    bool _readinfo;
  };
}

#endif