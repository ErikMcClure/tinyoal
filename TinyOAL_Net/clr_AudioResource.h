// Copyright ©2014 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __CLR_AUDIO_RESOURCE_H__
#define __CLR_AUDIO_RESOURCE_H__

#include "clr_Audio.h"

namespace TinyOAL { class cAudioResource; }

namespace TinyOAL_net {
	/* Managed wrapper for cAudio class. Due to the nature of the audio engine, no other Managed wrappers are actually necessary. */
  public ref class clr_AudioResource
  {
  public:
    explicit clr_AudioResource(TinyOAL::cAudioResource* p);
    explicit clr_AudioResource(System::String^ file);
    clr_AudioResource(System::String^ file, CLR_TINYOAL_FLAG flags);
    clr_AudioResource(cli::array<System::Byte>^ data, CLR_TINYOAL_FLAG flags);
    ~clr_AudioResource();
    !clr_AudioResource();
    unsigned __int64 ToSamples(double seconds); // Converts given time to sample point 
    property unsigned __int64 LoopPoint { unsigned __int64 get(); void set(unsigned __int64 looppoint); }
    property CLR_TINYOAL_FLAG Flags { CLR_TINYOAL_FLAG get(); void set(CLR_TINYOAL_FLAG flags); }
    property unsigned int Frequency { unsigned int get(); }
    property unsigned int Channels { unsigned int get(); }
    property unsigned int Format { unsigned int get(); }
    property unsigned int NumActive { unsigned int get(); }
    property unsigned int MaxActive { unsigned int get(); void set(unsigned int max); }
    unsigned int GetBufSize();
    clr_Audio^ Play(CLR_TINYOAL_FLAG flags);
    clr_Audio^ Play();
    inline bool IsValid() { return _ref!=0; }

    inline operator TinyOAL::cAudioResource*() { return _ref; }
    
		static const CLR_TINYOAL_FLAG TINYOAL_FILETYPE_UNKNOWN=0;
		static const CLR_TINYOAL_FLAG TINYOAL_FILETYPE_OGG=32;
		static const CLR_TINYOAL_FLAG TINYOAL_FILETYPE_MP3=64;
		static const CLR_TINYOAL_FLAG TINYOAL_FILETYPE_WAV=96;
    static const CLR_TINYOAL_FLAG TINYOAL_FILETYPE_FLAC=128;

  protected:

    TinyOAL::cAudioResource* _ref; //pointer to unmanaged object
  };
}

#endif