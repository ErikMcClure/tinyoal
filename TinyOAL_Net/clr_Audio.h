// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __CLR_AUDIO_H__
#define __CLR_AUDIO_H__

#include <vcclr.h>
#include <cstdint>

namespace tinyoal { class Audio; }

namespace TinyOAL_net {
  ref class clr_AudioResource;
  typedef unsigned char CLR_TINYOAL_FLAG;

	/* Managed wrapper for Audio class. Due to the nature of the audio engine, no other Managed wrappers are actually necessary. */
  public ref class clr_Audio
  {
  public:
    explicit clr_Audio(tinyoal::Audio* p);
    clr_Audio(clr_AudioResource^ ref, unsigned char addflags);
    clr_Audio(clr_Audio^ copy);
    explicit clr_Audio(clr_AudioResource^ ref);
    ~clr_Audio();
    !clr_Audio();
		// Updates the stream buffers, returns false if, after updating the buffers, the sound is no longer playing.
    bool Update();
		// Plays an audio stream 
    bool Play();
		// Stops an audio stream and resets the pointer to the beginning. If the loop point is set to -1, will stop playing once it reaches the end, otherwise it will loop back to that point indefinitely.
    void Stop();
		// This pauses an audio stream. Calling Play() will resume playing the stream from where it left off.
    void Pause();
		// This returns whether the sample is (supposed) to be playing. Whether a sample is actually playing can differ due to starved audio sources and other things. 
    property bool Playing { bool get(); }
    // Attempts to skip to the given song time (in seconds or samples) 
    property uint64_t Time { uint64_t get(); void set(uint64_t sample); }
    bool SkipSeconds(double seconds);
		// Sets the volume - 1.0 signifies 100% volume, 0.5 is 50%, 1.5 is 150%, etc. 
    property float Volume { float get(); void set(float volume); }
		// Sets the pitch (which is actually just the sample playback rate) - 1.0 means no change in pitch, 2.0 double the pitch, etc. 
    property float Pitch { float get(); void set(float pitch); }
		// This sets the position of the sound in a 3D space. This function's parameters are RELATIVE - that means if you set Y and Z to 0, the X value will become meaningless. By default Z is 0.5, so nearly all the way to the left is -10.0 and nearly all the way to the right is 10.0, and centered is 0.0 
    property cli::array<float>^ Position { cli::array<float>^ get(); void set(cli::array<float>^ pos); }
    // Sets loop point in seconds, or samples 
    property uint64_t LoopPoint { uint64_t get(); void set(uint64_t looppoint); }
    void SetLoopPointSeconds(double seconds);
    // Get Flags 
    property CLR_TINYOAL_FLAG Flags { CLR_TINYOAL_FLAG get(); }
    // Grab reference to audio resource used by this Audio instance 
    clr_AudioResource^ GetResource();

		//The following flags are taken from Audio.h AUDIO_FLAGS enum class
    static const CLR_TINYOAL_FLAG TINYOAL_COPYINTOMEMORY=1; // This will copy whatever you're loading into internal memory
    static const CLR_TINYOAL_FLAG TINYOAL_ISPLAYING=2; // Indicates the audio is playing. If specified in the constructor, will cause the instance to start playing immediately.
    static const CLR_TINYOAL_FLAG TINYOAL_MANAGED=4; // Instance will be deleted by the engine when it stops playing
    static const CLR_TINYOAL_FLAG TINYOAL_ISFILE=8;

  protected:
    tinyoal::Audio* _ref; //pointer to unmanaged object
    bool _managed;
  };
}

#endif