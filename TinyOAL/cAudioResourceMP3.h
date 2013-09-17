// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_AUDIO_RESOURCE_MP3_H__TOAL__
#define __C_AUDIO_RESOURCE_MP3_H__TOAL__

#ifdef __INCLUDE_MP3
#include "cAudioResource.h"
#include "mpg123.h"

namespace TinyOAL {
  class cMp3Functions;

	// This is a resource class for MP3 files, and handles all the IO operations from the given buffer 
  class cAudioResourceMP3 : public cAudioResource
  {
  public:
    // This returns an AUDIOSTREAM on success, or NULL on failure 
    virtual AUDIOSTREAM* OpenStream();
    // This reads the next chunk of OGG specific data. pDecodeBuffer must be at least GetBufSize() long and AUDIOSTREAM must be non-null 
    virtual unsigned long Read(AUDIOSTREAM* stream, char* pDecodeBuffer);
    // This resets the stream to the beginning. 
    virtual bool Reset(AUDIOSTREAM* stream);
    // This closes a stream and destroys any associated data (not the actual audio source itself). Stream will be an invalid pointer after this function is called. 
    virtual void CloseStream(AUDIOSTREAM* stream);
    // Sets stream to given sample 
    virtual bool Skip(AUDIOSTREAM* stream, unsigned __int64 samples);

  protected:
    friend class cAudioResourceMP3; //This, combined with protected constructors/destructors, ensures a client program cannot, under any circumstance, delete an audio resource without calling the appropriate function
    friend class cAudioResource;

    cAudioResourceMP3(const cAudioResourceMP3& copy);
    cAudioResourceMP3(void* data, unsigned int datalength, TINYOAL_FLAG flags);
    cAudioResourceMP3(const char* file, TINYOAL_FLAG flags);
    cAudioResourceMP3(_iobuf* file, unsigned int datalength, TINYOAL_FLAG flags);
    ~cAudioResourceMP3();

	  bool _buildstream(bool file);
    static unsigned int _readstream(AUDIOSTREAM* stream, char* buf, int num);

    mp3data_struct _header;
    cMp3Functions* _functions;
    unsigned char* _filebuf;
    bool _padding;
    int _enc_delay;
    int _enc_padding;
    int _framesize; //size of mp3 frame in bytes
  };
}
#endif

#endif