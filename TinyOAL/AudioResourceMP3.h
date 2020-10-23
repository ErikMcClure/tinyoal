// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __AUDIO_RESOURCE_MP3_H__TOAL__
#define __AUDIO_RESOURCE_MP3_H__TOAL__

#include "tinyoal/AudioResource.h"
#include "Mp3Functions.h"

namespace tinyoal {
  // This is a resource class for MP3 files, and handles all the IO operations from the given buffer
  class AudioResourceMP3 : public AudioResource
  {
  public:
    AudioResourceMP3(void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop);
    ~AudioResourceMP3();
    virtual void* OpenStream();             // This returns a pointer to the internal stream on success, or NULL on failure
    virtual void CloseStream(void* stream); // This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, unsigned int len,
                               bool& eof); // Reads next chunk of data - buffer must be at least GetBufSize() long
    virtual bool Reset(void* stream);      // This resets a stream to the beginning
    virtual bool Skip(void* stream, uint64_t samples); // Sets a stream to given sample
    virtual uint64_t Tell(void* stream);               // Gets what sample a stream is currently on

    static size_t Construct(void* p, void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop);
    static bool ScanHeader(const char* fileheader);
    static std::pair<void*, unsigned int> ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags);

  protected:
    static void cb_cleanup(void* dat);
    static unsigned long _read(void* stream, char* buffer, unsigned int len, bool& eof);
    static ssize_t cb_datread(void* stream, void* dst, size_t n);
    static off_t cb_datseek(void* stream, off_t off, int loc);
    static ssize_t cb_fileread(void* stream, void* dst, size_t n);
    static off_t cb_fileseek(void* stream, off_t off, int loc);
    static off_t cb_fileseekoffset(void* stream, off_t off, int loc);
  };
}
#endif