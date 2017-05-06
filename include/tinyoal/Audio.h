// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __AUDIO_H__TOAL__
#define __AUDIO_H__TOAL__

#include "TinyOAL_dlldef.h"
#include "bss-util/LLBase.h"
#include "bss-util/BitField.h"
#include "bss-util/bss_util.h"

namespace tinyoal {
  typedef unsigned char TINYOAL_FLAG;

  enum TINYOAL_FLAGS : TINYOAL_FLAG
  {
    TINYOAL_COPYINTOMEMORY=1, // This will copy whatever you're loading into internal memory
    TINYOAL_ISPLAYING=2, // Indicates the audio is playing. If specified in the constructor, will cause the instance to start playing immediately.
    TINYOAL_MANAGED=4, // Instance will be deleted by the engine when it stops playing
    TINYOAL_ISFILE=8,
    TINYOAL_FORCETOWAVE=16+1, // Forces the resource to be copied into memory as an uncompressed wave for efficient playback. Implies TINYOAL_COPYINTOMEMORY
  };

  class AudioResource;
  
  class TINYOAL_DLLEXPORT Audio : public bss::LLBase<Audio>
  {
  public:
    // Constructors
    Audio(const Audio& copy);
    Audio(Audio&& mov);
    explicit Audio(AudioResource* ref, TINYOAL_FLAG addflags=0, void* userdata=0);
    // Destructor 
    ~Audio();
		// Updates the stream buffers, returns false if, after updating the buffers, the sound is no longer playing.
    bool Update();
		// Plays an audio stream 
    bool Play();
		// Stops an audio stream and resets the pointer to the beginning. If the loop point is set to -1, will stop playing once it reaches the end, otherwise it will loop back to that point indefinitely.
    void Stop();
		// This pauses an audio stream. Calling Play() will resume playing the stream from where it left off.
    void Pause();
		// This returns whether the sample is (supposed) to be playing. Whether a sample is actually playing can differ due to starved audio sources and other things. 
    bool IsPlaying() const;
    // Attempts to skip to the given song time (in seconds or samples) 
    bool SkipSeconds(double seconds);
    bool Skip(uint64_t sample);
    // Gets the current sample location of the stream
    uint64_t IsWhere() const;
		// Sets the volume - 1.0 signifies 100% volume, 0.5 is 50%, 1.5 is 150%, etc. 
    void SetVolume(float range);
    inline float GetVolume() const { return _vol; }
		// Sets the pitch (which is actually just the sample playback rate) - 1.0 means no change in pitch, 2.0 double the pitch, etc. 
    void SetPitch(float range);
    inline float GetPitch() const { return _pitch; }
		// This sets the position of the sound in a 3D space. This function's parameters are RELATIVE - that means if you set Y and Z to 0, the X value will become meaningless. By default Z is 0.5, so nearly all the way to the left is -10.0 and nearly all the way to the right is 10.0, and centered is 0.0 
    void SetPosition(float X, float Y=0.0f, float Z=0.5f);
    inline const float* GetPosition() const { return _pos; }
    // Sets loop point in seconds, or samples 
    void SetLoopPointSeconds(double seconds);
    void SetLoopPoint(uint64_t sample);
    inline uint64_t GetLoopPoint() const { return _looptime; }
    // Get Flags 
    inline TINYOAL_FLAG GetFlags() const { return _flags; }
    // Grab reference to audio resource used by this Audio instance 
    inline AudioResource* GetResource() const { return _source; }
    // Invalidates this instance by setting _stream and _source to NULL
    void Invalidate();

    void* userdata;
    void (*ONDESTROY)(Audio* ref);

  protected:
    void _stop();
    void _processBuffers();
    bool _streaming() const;
    void _applyAll(); //In case we have to reset our openAL source, this reapplies all volume/pitch/location modifications
    void _fillBuffers();
    void _getSource();
    void _queueBuffers();
    unsigned long _readBuffer();

    AudioResource* _source;
    void* _stream;
    float _pos[3];
    float _vol;
    float _pitch;
    bss::BitField<TINYOAL_FLAG> _flags;
    uint64_t _looptime;
    unsigned int uiSource;
    unsigned int* uiBuffers;
    unsigned int _bufsize;
    char _bufstart;
    char _queuebuflen;
    char* pDecodeBuffer;
  };
}

#endif