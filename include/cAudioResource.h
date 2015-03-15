// Copyright ©2015 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __C_AUDIO_RESOURCE_H__TOAL__
#define __C_AUDIO_RESOURCE_H__TOAL__

#include "bss-util/cHash.h"
#include "bss-util/cRefCounter.h"
#include "bss-util/cStr.h"
#include "bss-util/bss_alloc_block.h"
#include "cAudio.h"
#include <stdio.h>

namespace TinyOAL {
  // Holds information about a given audio resource. An audio resource is different from an actual cAudio instance, in that it holds the raw audio information, which is then ACCESSED by any number of cAudio instances. This prevents memory wasting. 
  class TINYOAL_DLLEXPORT cAudioResource : public bss_util::cRefCounter, public bss_util::LLBase<cAudioResource>
  {
  public:
    virtual void* OpenStream()=0; // This returns a pointer to the internal stream on success, or NULL on failure 
    virtual void CloseStream(void* stream)=0; //This closes an AUDIOSTREAM pointer
    virtual unsigned long Read(void* stream, char* buffer, unsigned int len, bool& eof)=0; // Reads next chunk of data - buffer must be at least bufsize long 
    virtual bool Reset(void* stream)=0; // This resets a stream to the beginning 
    virtual bool Skip(void* stream, unsigned __int64 samples)=0; // Sets a stream to given sample 
    virtual unsigned __int64 Tell(void* stream)=0; // Gets what sample a stream is currently on
    inline unsigned __int64 ToSamples(double seconds) const { return (unsigned __int64)(seconds*_freq); } // Converts given time to sample point 
    inline double ToSeconds(unsigned __int64 samples) const { return samples/(double)_freq; } // converts sample point to time
    inline unsigned __int64 GetLoopPoint() const { return _loop; }
    inline void SetLoopPoint(unsigned __int64 loop) { _loop=loop; }

    inline TINYOAL_FLAG GetFlags() const { return _flags; }
    inline void SetFlags(TINYOAL_FLAG flags) { _flags=flags; }
    inline unsigned int GetFreq() const { return _freq; }
    inline unsigned int GetChannels() const { return _channels; }
    inline unsigned int GetFormat() const { return _format; }
    inline unsigned int GetBufSize() const { return _bufsize; }
    inline unsigned __int64 GetTotalSamples() const { return _total; }
    inline double GetLength() const { return ToSeconds(_total); }
    inline unsigned short GetBitsPerSample() const { return _samplebits; }
    inline cAudio* GetActiveInstances() const { return _activelist; }
    inline cAudio* GetInactiveInstances() const { return _inactivelist; }
    inline unsigned int GetNumActive() const { return _numactive; }
    inline unsigned int GetMaxActive() const { return _maxactive; }
    inline void SetMaxActive(unsigned int max=0) { _maxactive=max; }
    virtual void DestroyThis(); // Make sure we get deleted in the right DLL
    cAudio* Play(TINYOAL_FLAG flags=TINYOAL_ISPLAYING);

    // Creates a cAudioResource based on whether or not its an OGG, wav, or mp3. You can override the filetype in the flags parameter
    static cAudioResource* Create(const char* file, TINYOAL_FLAG flags=0, unsigned __int64 loop=(unsigned __int64)-1);
    static cAudioResource* Create(const void* data, unsigned int datalength, TINYOAL_FLAG flags=0, unsigned __int64 loop=(unsigned __int64)-1);
    // On Windows, file-locks are binary-exclusive, so if you don't explicitely set the sharing properly, this won't work.
    static cAudioResource* Create(FILE* file, unsigned int datalength, TINYOAL_FLAG flags=0, unsigned __int64 loop=(unsigned __int64)-1);
    
    enum TINYOAL_FILETYPE : TINYOAL_FLAG
	  {
		  TINYOAL_FILETYPE_UNKNOWN=0,
		  TINYOAL_FILETYPE_OGG=32,
		  TINYOAL_FILETYPE_MP3=64,
		  TINYOAL_FILETYPE_WAV=96,
      TINYOAL_FILETYPE_FLAC=128,
	  };

  protected:
    friend class cAudio;
    friend class cTinyOAL;

    cAudioResource(const cAudioResource& copy); // These are protected ensure we can ONLY be created inside this DLL
    cAudioResource(void* data, unsigned int len, TINYOAL_FLAG flags, unsigned __int64 loop);
    virtual ~cAudioResource();
    void _destruct();

    static cAudioResource* _fcreate(FILE* file, unsigned int datalength, TINYOAL_FLAG flags, const char* path, unsigned __int64 loop);
    static cAudioResource* _create(void* data, unsigned int datalength, TINYOAL_FLAG flags, const char* path, unsigned __int64 loop);
    static cAudioResource* _force(void* data, unsigned int datalength, TINYOAL_FLAG flags, const char* path, unsigned __int64 loop);
    static unsigned char BSS_FASTCALL _getfiletype(const char* fileheader); // fileheader must be at least 4 characters long
    static bss_util::cHash<const char*, cAudioResource*, true, true> _audiohash;
    static bss_util::cBlockAlloc<cAudio> _allocaudio;

    void* _data;
    size_t _datalength;
    bss_util::cBitField<TINYOAL_FLAG> _flags;
    unsigned int _freq;
	  unsigned int _channels;
    unsigned int _format;
    unsigned int _bufsize;
    unsigned short _samplebits;
    unsigned __int64 _loop;
    unsigned __int64 _total; // total number of samples
    cStr _hash;
    cAudio* _activelist;
    cAudio* _activelistend;
    cAudio* _inactivelist;
    unsigned int _numactive;
    unsigned int _maxactive;
  };

  typedef struct DATSTREAM {
    const char* data;
    size_t datalength;
    const char* streampos;
  } DatStream;

  //8 functions - Four for parsing pure void*, and four for reading files
  extern size_t dat_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
  extern int dat_seek_func(void *datasource, __int64 offset, int whence);
  extern int dat_close_func(void *datasource);
  extern long dat_tell_func(void *datasource);
  extern size_t file_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
  extern int file_seek_func(void *datasource, __int64 offset, int whence);
  extern int file_close_func(void *datasource);
  extern long file_tell_func(void *datasource);
}

#endif