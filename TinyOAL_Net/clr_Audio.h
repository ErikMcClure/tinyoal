// Copyright ©2008-2009 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __CLR_AUDIO_H__
#define __CLR_AUDIO_H__

namespace TinyOAL {
  class cAudio;
  class cAudioRef;
}

namespace TinyOAL_net {
  ref class clr_TinyOAL;
  
  public ref class clr_AudioRef
  {
  public:
    clr_AudioRef(const clr_AudioRef% copy);
    clr_AudioRef(System::String^ file, unsigned char flags);
    clr_AudioRef(System::String^ data, bool isdata, unsigned char flags);
    explicit clr_AudioRef(TinyOAL::cAudioRef* ref);
    ~clr_AudioRef();
    bool IsValid();
    inline TinyOAL::cAudioRef* __getref() { return _ref; }

    clr_AudioRef% operator=(const clr_AudioRef% copy);

  protected:
    TinyOAL::cAudioRef* _ref;
  };

	/* Managed wrapper for cAudio class. Due to the nature of the audio engine, no other Managed wrappers are actually necessary. */
  public ref class clr_Audio
  {
  public:
		/* Constructor for loading audio from a file */
    clr_Audio(System::String^ file, unsigned char flags);
		/* Constructor for loading audio from either a file or from data. If it is data, isdata must be true, and data must be a string containing the audio information. The data will be loaded into memory regardless of the flags set */
    clr_Audio(System::String^ data, unsigned char flags, bool isdata);
    clr_Audio(clr_AudioRef^ ref, unsigned char addflags);
    explicit clr_Audio(clr_AudioRef^ ref);
		/* Destructor */
    ~clr_Audio();
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
		/* Sets the pitch (which is actually just the sample playback rate). 1.0 means no change in pitch, 2.0 double the pitch, etc. */
    void SetPitch(float range);
		/* This sets the panning of the sound - nearly all the way to the left is -1.0 and nearly all the way to the right is 1.0, and centered is 0.0 */
    void SetPosition(float X);
		/* This sets the position of the sound in a 3D space. This function's parameters are RELATIVE - that means if you set Y and Z to 0, the X value will become meaningless. By default Y and Z are 0.0, so nearly all the way to the left is -1.0 and nearly all the way to the right is 1.0, and centered is 0.0 */
    void SetPosition(float X, float Y, float Z);
    /* Sets loop point in seconds, or samples */
    void SetLoopPoint(double seconds);
    void SetLoopPointSample(unsigned __int64 sample);
    /* Get Flags */
    int GetFlags();
    /* Grab reference to audio resource used by this cAudio instance */
    clr_AudioRef^ GetAudioRef();

		//The following flags are taken from cAudio.h AUDIO_FLAGS enum class
    static const int TINYOAL_LOADINTOMEMORY = 1; //This gaurentees that the audio file in its entirety will be loaded into memory so it can be played multiple times simulatenously
    static const int TINYOAL_LOOP = 2; //This will make it so an audio object will not stop playing until it is stopped, looping the sample.
    static const int TINYOAL_AUTOPLAY = 4; //This will make the sound play as soon as its loaded
    static const int TINYOAL_NODOUBLEPLAY = 8; //This makes it so any attempt to play an already playing audio object will simply do nothing instead of restarting the stream (This applies to individual sounds, not the actual resource)

  private:
    TinyOAL::cAudio* _ref; //pointer to unmanaged object
  };
}

#endif