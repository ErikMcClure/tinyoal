// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __C_AUDIO_H__TOAL__
#define __C_AUDIO_H__TOAL__

#include "TinyOAL_dlldef.h"
#include "bss_util\LLBase.h"

struct _iobuf;
struct OPENALFNTABLE;

namespace TinyOAL {
  typedef unsigned char T_TINYOAL_FLAGS;

  enum TINYOAL_FLAGS : T_TINYOAL_FLAGS
  {
    TINYOAL_LOADINTOMEMORY=1, //This gaurentees that the audio file in its entirety will be loaded into memory so it can be played multiple times simulatenously
    TINYOAL_LOOP=2, //This will make it so an audio object will not stop playing until it is stopped, looping the sample.
    TINYOAL_AUTOPLAY=4, //This will make the sound play as soon as its loaded
    TINYOAL_NODOUBLEPLAY=8, //This makes it so any attempt to play an already playing audio object will simply do nothing instead of restarting the stream (This applies to individual sounds, not the actual resource)
    TINYOAL_MANAGED=16 //Setting this flag causes the cAudio to be destroyed when it is removed from the playing queue.
  };

  class cAudioResource;
  struct AUDIOSTREAM;
  class cTinyOAL;
  class cAudioRef;
  class cAudio;

  class TINYOAL_DLLEXPORT cAudioRef
  {
  public:
    cAudioRef(cAudioRef&& mov);
    cAudioRef(const cAudioRef& copy);
    cAudioRef(const char* file, unsigned char flags);
    cAudioRef(void* data, unsigned int datalength, unsigned char flags);
    cAudioRef(_iobuf* file, unsigned int datalength, unsigned char flags);
    ~cAudioRef();
    inline bool IsValid() const { return _res!=0; }

    cAudioRef& operator=(cAudioRef&& mov);
    cAudioRef& operator=(const cAudioRef& copy);

  protected:
    explicit cAudioRef(cAudioResource* res);
    cAudioResource* _res;

    friend class cAudio;
  };

  class TINYOAL_DLLEXPORT cAudio : public bss_util::LLBase<cAudio>
  {
  public:
		/* Constructor - takes a pointer to audio data that is in memory. If TINYOAL_LOADINTOMEMORY is not specified here, then the resource will assume that the data pointer is permanent and simply store a reference to it. This is not recommended because if you screw up the pointer, the audioresource cannot detect buffer overflows and will start corrupting memory en masse. If you know what your doing though, this can save valuable time, so it remains an option. */
    cAudio(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags);
		/* Constructor - takes a file path string and opens it */
    cAudio(const char* file, T_TINYOAL_FLAGS flags);
		/* Constructor - takes a file pointer and reads in the memory frmo that pointer to datalength. TINYOAL_LOADINTOMEMORY is assumed here regardless of whether it was specified */
    cAudio(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags);
    /* Constructor - takes an audio reference */
    explicit cAudio(const cAudioRef& ref, T_TINYOAL_FLAGS addflags=0);
    /* Destructor */
    ~cAudio();
		/* Updates the stream buffers. This is normally called by the engine automatically, but it can be called outside of that, its just usually very pointless */
    void Update();
		/* Plays an audio stream */
    bool Play();
		/* Stops an audio stream and resets the pointer to the beginning. Audio streams automatically stop when the end of the stream is reached, UNLESS the loop flag is specified, in which case it will loop until Stop() is called */
    void Stop();
		/* This pauses an audio stream. This is different from stopping an audio stream, since if you call Play after calling Pause, it will resume where it left off */
    void Pause();
		/* This returns whether the sample is (supposed) to be playing. Whether a sample is actually playing can differ due to starved audio sources and other things. */
    bool IsPlaying();
    /* Attempts to skip to the given song time (in seconds or samples) */
    bool Skip(double seconds);
    bool SkipSample(unsigned __int64 sample);
		/* Sets the volume - 1.0 signifies 100% volume, 0.5 is 50%, 1.5 is 150%, etc. */
    void SetVolume(float range);
		/* Sets the pitch (which is actually just the sample playback rate) - 1.0 means no change in pitch, 2.0 double the pitch, etc. */
    void SetPitch(float range);
		/* This sets the position of the sound in a 3D space. This function's parameters are RELATIVE - that means if you set Y and Z to 0, the X value will become meaningless. By default Z is 1.0, so nearly all the way to the left is -1.0 and nearly all the way to the right is 1.0, and centered is 0.0 */
    void SetPosition(float X, float Y=0.0f, float Z=1.0f);
    /* Sets loop point in seconds, or samples */
    void SetLoopPointSeconds(double seconds);
    void SetLoopPoint(unsigned __int64 sample);
    /* Get Flags */
    inline T_TINYOAL_FLAGS GetFlags() const { return _flags; }
    /* Grab reference to audio resource used by this cAudio instance */
    inline cAudioRef GetAudioRef() const { return cAudioRef(_source); }
    /* Replaces the existing audio resource in this cAudio */
    void Replace(void* data, unsigned int datalength, T_TINYOAL_FLAGS flags);
    void Replace(const char* file, T_TINYOAL_FLAGS flags);
    void Replace(_iobuf* file, unsigned int datalength, T_TINYOAL_FLAGS flags);
    void Replace(const cAudioRef& ref, T_TINYOAL_FLAGS addflags=0);

  protected:
    void _construct(cAudioResource* source,bool init=true);
    void _processbuffers();
    bool _streaming();
    void _applyall(); //In case we have to reset our openAL source, this reapplies all volume/pitch/location modifications
    void _fillbuffers();
    void _queuebuffers(); // IF we lose the source we'll need to re-queue all the buffers
    void _getsource();

    cAudioResource* _source;
    AUDIOSTREAM* _stream;
    bool _playing;
    float _pos[3];
    float _vol;
    float _pitch;
    T_TINYOAL_FLAGS _flags;
    unsigned __int64 _looptime;

  private:
    friend class cTinyOAL;

    unsigned int uiSource;
    unsigned int* uiBuffers;
    const char _nbuffers;
    char _bufstart;
    char _queuebuflen;
    char* pDecodeBuffer;
    OPENALFNTABLE* _funcs;
  };
}

#endif