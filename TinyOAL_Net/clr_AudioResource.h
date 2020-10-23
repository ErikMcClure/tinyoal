// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __CLR_AUDIO_RESOURCE_H__
#define __CLR_AUDIO_RESOURCE_H__

#include "clr_Audio.h"

namespace tinyoal { class AudioResource; }

namespace TinyOAL_net {
  typedef unsigned char CLR_TINYOAL_FILETYPE;

  /* Managed wrapper for Audio class. Due to the nature of the audio engine, no other Managed wrappers are actually necessary. */
  public ref class clr_AudioResource
  {
  public:
    explicit clr_AudioResource(tinyoal::AudioResource* p);
    explicit clr_AudioResource(System::String^ file);
    clr_AudioResource(System::String^ file, CLR_TINYOAL_FLAG flags);
    clr_AudioResource(System::String^ file, CLR_TINYOAL_FLAG flags, CLR_TINYOAL_FILETYPE filetype);
    clr_AudioResource(cli::array<System::Byte>^ data, CLR_TINYOAL_FLAG flags);
    clr_AudioResource(cli::array<System::Byte>^ data, CLR_TINYOAL_FLAG flags, CLR_TINYOAL_FILETYPE filetype);
    ~clr_AudioResource();
    !clr_AudioResource();
    uint64_t ToSamples(double seconds); // Converts given time to sample point 
    property uint64_t LoopPoint { uint64_t get(); void set(uint64_t looppoint); }
    property CLR_TINYOAL_FLAG Flags { CLR_TINYOAL_FLAG get(); void set(CLR_TINYOAL_FLAG flags); }
    property CLR_TINYOAL_FILETYPE FileType { CLR_TINYOAL_FILETYPE get(); }
    property unsigned int Frequency { unsigned int get(); }
    property unsigned int Channels { unsigned int get(); }
    property unsigned int Format { unsigned int get(); }
    property unsigned int NumActive { unsigned int get(); }
    property unsigned int MaxActive { unsigned int get(); void set(unsigned int max); }
    unsigned int GetBufSize();
    clr_Audio^ Play(CLR_TINYOAL_FLAG flags);
    clr_Audio^ Play();
    inline bool IsValid() { return _ref != 0; }

    inline operator tinyoal::AudioResource*() { return _ref; }

    static const CLR_TINYOAL_FILETYPE TINYOAL_FILETYPE_UNKNOWN = 0;
    static const CLR_TINYOAL_FILETYPE TINYOAL_FILETYPE_WAV = 1;
    static const CLR_TINYOAL_FILETYPE TINYOAL_FILETYPE_OGG = 2;
    static const CLR_TINYOAL_FILETYPE TINYOAL_FILETYPE_MP3 = 3;
    static const CLR_TINYOAL_FILETYPE TINYOAL_FILETYPE_FLAC = 4;

  protected:
    tinyoal::AudioResource* _ref; //pointer to unmanaged object
  };
}

#endif