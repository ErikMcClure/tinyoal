// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_AUDIO_RESOURCE_MP3_H__TOAL__
#define __C_AUDIO_RESOURCE_MP3_H__TOAL__

#include "cAudioResource.h"
#include "cMp3Functions.h"

namespace TinyOAL {
	// This is a resource class for MP3 files, and handles all the IO operations from the given buffer 
  class cAudioResourceMP3 : public cAudioResource
  {
  public:
    cAudioResourceMP3(const cAudioResourceMP3& copy);
    cAudioResourceMP3(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop);
    ~cAudioResourceMP3();
    virtual void* OpenStream(); // This returns a pointer to the internal stream on success, or NULL on failure 
    virtual void CloseStream(void* stream); //This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, unsigned int len, bool& eof); // Reads next chunk of data - buffer must be at least GetBufSize() long 
    virtual bool Reset(void* stream); // This resets a stream to the beginning 
    virtual bool Skip(void* stream, unsigned __int64 samples); // Sets a stream to given sample 
    virtual unsigned __int64 Tell(void* stream); // Gets what sample a stream is currently on
    
    static std::pair<void*,unsigned int> ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags);

  protected:
    static void cb_cleanup(void* dat);
    static unsigned long _read(void* stream, char* buffer, unsigned int len, bool& eof);
    static ssize_t cb_datread(void* stream,void* dst,size_t n);
    static off_t cb_datseek(void* stream,off_t off,int loc);
    static ssize_t cb_fileread(void* stream,void* dst,size_t n);
    static off_t cb_fileseek(void* stream,off_t off,int loc);
    static off_t cb_fileseekoffset(void* stream,off_t off,int loc);

    static bss_util::cFixedAlloc<DatStream> _datalloc;
  };
}
#endif