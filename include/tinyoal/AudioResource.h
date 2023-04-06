// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef TOAL__AUDIO_RESOURCE_H
#define TOAL__AUDIO_RESOURCE_H

#include "bss-util/Hash.h"
#include "bss-util/RefCounter.h"
#include "bss-util/Str.h"
#include "bss-util/BlockAlloc.h"
#include "Audio.h"
#include <stdio.h>

namespace tinyoal {
  // Holds information about a given audio resource. An audio resource is different from an actual Audio instance, in that
  // it holds the raw audio information, which is then ACCESSED by any number of Audio instances. This prevents memory
  // wasting.
  class TINYOAL_DLLEXPORT AudioResource : public bss::RefCounter, public bss::LLBase<AudioResource>
  {
  public:
    virtual void* OpenStream() = 0; // This returns a pointer to the internal stream on success, or NULL on failure
    virtual void CloseStream(void* stream) = 0; // This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, unsigned int len,
                               bool& eof)  = 0; // Reads next chunk of data - buffer must be at least bufsize long
    virtual bool Reset(void* stream)       = 0; // This resets a stream to the beginning
    virtual bool Skip(void* stream, uint64_t samples) = 0; // Sets a stream to given sample
    virtual uint64_t Tell(void* stream)               = 0; // Gets what sample a stream is currently on
    inline uint64_t ToSamples(double seconds) const
    {
      return (uint64_t)(seconds * _freq);
    } // Converts given time to sample point
    inline double ToSeconds(uint64_t samples) const { return samples / (double)_freq; } // converts sample point to time
    inline uint64_t GetLoopPoint() const { return _loop; }
    inline void SetLoopPoint(uint64_t loop) { _loop = loop; }

    enum TINYOAL_FILETYPE : unsigned char
    {
      TINYOAL_FILETYPE_UNKNOWN = 0,
      TINYOAL_FILETYPE_WAV,
      TINYOAL_FILETYPE_OGG,
      TINYOAL_FILETYPE_MP3,
      TINYOAL_FILETYPE_FLAC,
      TINYOAL_FILETYPE_CUSTOM, // Add custom filetypes here
    };

    inline TINYOAL_FLAG GetFlags() const { return _flags; }
    inline void SetFlags(TINYOAL_FLAG flags) { _flags = flags; }
    inline TINYOAL_FILETYPE GetFileType() const { return _filetype; }
    inline unsigned int GetFreq() const { return _freq; }
    inline unsigned int GetChannels() const { return _channels; }
    inline unsigned int GetFormat() const { return _format; }
    inline unsigned int GetBufSize() const { return _bufsize; }
    inline uint64_t GetTotalSamples() const { return _total; }
    inline double GetLength() const { return ToSeconds(_total); }
    inline unsigned short GetBitsPerSample() const { return _samplebits; }
    inline Audio* GetActiveInstances() const { return _activelist; }
    inline Audio* GetInactiveInstances() const { return _inactivelist; }
    inline unsigned int GetNumActive() const { return _numactive; }
    inline unsigned int GetMaxActive() const { return _maxactive; }
    inline void SetMaxActive(unsigned int max = 0) { _maxactive = max; }
    virtual void DestroyThis(); // Make sure we get deleted in the right DLL
    Audio* Play(TINYOAL_FLAG flags = TINYOAL_ISPLAYING);

    // Creates a AudioResource based on whether or not its an OGG, wav, or mp3. You can override the filetype in the flags
    // parameter
    static AudioResource* Create(const char* file, TINYOAL_FLAG flags = 0,
                                 unsigned char filetype = TINYOAL_FILETYPE_UNKNOWN, uint64_t loop = (uint64_t)-1);
    static AudioResource* Create(const void* data, unsigned int datalength, TINYOAL_FLAG flags = 0,
                                 unsigned char filetype = TINYOAL_FILETYPE_UNKNOWN, uint64_t loop = (uint64_t)-1);
    // On Windows, file-locks are binary-exclusive, so if you don't explicitely set the sharing properly, this won't work.
    static AudioResource* Create(FILE* file, unsigned int datalength, TINYOAL_FLAG flags = 0,
                                 unsigned char filetype = TINYOAL_FILETYPE_UNKNOWN, uint64_t loop = (uint64_t)-1);

  protected:
    friend class Audio;
    friend class TinyOAL;

    AudioResource(const AudioResource&) = delete;
    AudioResource(AudioResource&&)      = delete;
    AudioResource& operator=(const AudioResource&) = delete;
    AudioResource& operator=(AudioResource&&) = delete;
    AudioResource(void* data, unsigned int len, TINYOAL_FLAG flags, unsigned char filetype, uint64_t loop);
    virtual ~AudioResource();
    void _destruct();

    static AudioResource* _fcreate(FILE* file, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype,
                                   const char* path, uint64_t loop);
    static AudioResource* _create(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype,
                                  const char* path, uint64_t loop);
    static AudioResource* _force(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype,
                                 const char* path, uint64_t loop);

    void* _data;
    size_t _datalength;
    bss::BitField<TINYOAL_FLAG> _flags;
    const TINYOAL_FILETYPE _filetype;
    unsigned int _freq;
    unsigned int _channels;
    unsigned int _format;
    unsigned int _bufsize;
    unsigned short _samplebits;
    uint64_t _loop;
    uint64_t _total; // total number of samples
    bss::Str _hash;
    Audio* _activelist;
    Audio* _activelistend;
    Audio* _inactivelist;
    unsigned int _numactive;
    unsigned int _maxactive;
  };

  typedef struct DATSTREAM
  {
    const char* data;
    size_t datalength;
    const char* streampos;
  } DatStream;

  // 8 functions - Four for parsing pure void*, and four for reading files
  extern size_t dat_read_func(void* ptr, size_t size, size_t nmemb, void* datasource);
  extern int dat_seek_func(void* datasource, int64_t offset, int whence);
  extern int dat_close_func(void* datasource);
  extern long dat_tell_func(void* datasource);
  extern size_t file_read_func(void* ptr, size_t size, size_t nmemb, void* datasource);
  extern int file_seek_func(void* datasource, int64_t offset, int whence);
  extern int file_close_func(void* datasource);
  extern long file_tell_func(void* datasource);
}

#endif