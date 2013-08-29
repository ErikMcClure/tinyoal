// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __C_AUDIO_RESOURCE_H__TOAL__
#define __C_AUDIO_RESOURCE_H__TOAL__

#include "bss_util/cKhash.h"
#include "bss_util/cRefCounter.h"
#include "bss_util/cStr.h"
#include <vector>
#include "cAudio.h"

struct _iobuf; //FILE*
struct OggVorbis_File;

namespace TinyOAL {
  class cDataStream;

  /* This class allows us to move around FILE and DATA pointers. */
  struct AUDIOSTREAM
  {
    union
    {
      FILE* file; //File pointers can only have one cAudio class referencing them at the same time.
      cDataStream* data; //Data pointers point to a place in RAM and thus can have any number of streams on them.
    };
    bool isfile; //This is an important distinction because data is created and must be deleted whereas file does not.
    union
    {
      OggVorbis_File* oggfile;
      int WaveID;
    };

  private:
    friend class cAudioResource;

    AUDIOSTREAM(const AUDIOSTREAM& copy);
    AUDIOSTREAM(FILE* _file);
    AUDIOSTREAM(void* _data=0, size_t _datalength=0);
    ~AUDIOSTREAM();
  };

  /* Holds information about a given audio resource. An audio resource is different from an actual cAudio instance, in that it holds the raw audio information, which is then ACCESSED by any number of cAudio instances. This prevents memory wasting. */
  class cAudioResource : protected bss_util::cRefCounter //Due to how this works, we don't actually have to export this class, which is kind of cool.
  {
  public:
    /* This returns an AUDIOSTREAM on success, or NULL on failure */
    virtual AUDIOSTREAM* OpenStream();
    /* This reads the next chunk of data. pDecodeBuffer must be at least GetBufSize() long */
    virtual unsigned long ReadNext(AUDIOSTREAM* stream, char* pDecodeBuffer)=0;
    /* This resets the stream to the beginning */
    virtual bool Reset(AUDIOSTREAM* stream);
    /* Sets stream to given sample */
    virtual bool Skip(AUDIOSTREAM* stream, unsigned __int64 samples)=0;
    /* Converts given time to sample point */
    virtual unsigned __int64 ToSample(AUDIOSTREAM* stream, double seconds)=0;
    /* This closes a stream and destroys any associated data (not the actual audio source itself) */
    virtual void CloseStream(AUDIOSTREAM* stream); //This closes an AUDIOSTREAM pointer
    /* Drops a reference to this source */
    void Drop(); //this is overloaded to ensure that this class gets deleted inside this DLL as opposed to the client program
    /* Grabs a reference to this source */
    inline void Grab() { cRefCounter::Grab(); }
    /* If the format has an embedded loop point, get it (in samples) */
    virtual unsigned __int64 GetLoopStart(AUDIOSTREAM* stream);
    inline T_TINYOAL_FLAGS GetFlags() const { return _flags; }

    inline bool operator==(const cAudioResource& right) const { return !_f?((!_data || !_datalength)?false:_data==right._data):_f==right._f; } //File takes prevelence here because of the possibility that its a fake pointer used to identify the audio.
    inline bool operator!=(const cAudioResource& right) const { return false; }
    inline bool operator==(const cAudioResource* right) const { return false; }
    inline bool operator!=(const cAudioResource* right) const { return false; }

    /* Creates a cAudioResource based on whether or not its an OGG, wav, or mp3 (mp3 currently not supported). See the comment on this class for a description of what an audio resource is. */
    static cAudioResource* CreateAudioReference(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags); //If LOADINTOMEMORY is not specified here, then the resource will assume that the data pointer is permanent and simply store a reference to it. This is not recommended because if you screw up the pointer, the audioresource cannot detect buffer overflows and will start corrupting memory en masse. If you know what your doing though, this can save valuable time, so it remains an option.
    static cAudioResource* CreateAudioReference(const char* file, T_TINYOAL_FLAGS flags); //This is actually the only case where the internal file pointer is used instead of the void* data pointer.
    static cAudioResource* CreateAudioReference(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags); //LOADINTOMEMORY is implied here because we cannot simply hope that the file pointer will continue to exist
    /* Destroys all audio resources */
    static void DeleteAll();

		/* These functions retrieve information about the audio data */
    inline unsigned long GetFreq() const { return ulFrequency; }
    inline unsigned long GetChannels() const { return ulChannels; }
    inline unsigned long GetFormat() const { return ulFormat; }
    inline unsigned long GetBufSize() const { return ulBufferSize; }

  protected:
    friend class cAudioResource;

    cAudioResource(const cAudioResource& copy); // This is protected to engineer a situation where the class can ONLY be created inside this DLL, thus preventing accidental memory mishandlings, without us having to redefine the refcounter
    cAudioResource(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags=0);
    cAudioResource(const char* file, T_TINYOAL_FLAGS flags=0);
    cAudioResource(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags=0);
    virtual ~cAudioResource();

    static bss_util::cKhash_StringIns<cAudioResource*> _audiolist;
    static const int CHUNKSIZE = 1024;
    
    _iobuf* _f;
    void* _data;
    size_t _datalength;
    T_TINYOAL_FLAGS _flags;
    unsigned long	ulFrequency;
	  unsigned long	ulChannels;
    unsigned long ulFormat;
    unsigned long ulBufferSize;
    unsigned int _listpos;

  private: 
    static cAudioResource* FindExists(void* filedata, const char* path, T_TINYOAL_FLAGS flags); //This function will figure out which params to actually use
    static cAudioResource* __fastcall _addref(cAudioResource* target);
    static void __fastcall _removeref(cAudioResource* target);
		static unsigned char __fastcall _getfiletype(const char* fileheader);
    static void __fastcall _encodeptr(void* ptr, cStr& buf);

    void __fastcall _loadintomemory();

    bool _externdata; //is _data our variable or a pointer
    cStr _path;
  };
}

#endif