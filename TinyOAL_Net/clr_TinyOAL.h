// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __CLR_TINYOAL_H__
#define __CLR_TINYOAL_H__

#include "clr_AudioResource.h"

namespace tinyoal { class cTinyOAL; }

namespace TinyOAL_net {
  // Managed wrapper for cTinyOAL class 
  public ref class clr_TinyOAL
  {
  public:
    clr_TinyOAL();
    clr_TinyOAL(int defaultbuffers);
    clr_TinyOAL(int defaultbuffers, System::String^ logfile);
    clr_TinyOAL(int defaultbuffers, System::String^ logfile, System::String^ forceOAL, System::String^ forceOGG, System::String^ forceFLAC, System::String^ forceMP3);
    ~clr_TinyOAL();
    !clr_TinyOAL();
    // This updates any currently playing samples and returns the number that are still playing after the update. The time between calls
    // to this update function can never exceed the length of a buffer, or the sound will cut out.
    unsigned int Update();
    // Creates an instance of a sound either from an existing resource or by creating a new resource
    inline clr_Audio^ PlaySound(clr_AudioResource^ resource, CLR_TINYOAL_FLAG flags) { return resource->Play(flags | clr_Audio::TINYOAL_ISPLAYING); }
    inline clr_Audio^ PlaySound(System::String^ file, CLR_TINYOAL_FLAG flags) { return PlaySound(gcnew clr_AudioResource(file, flags, 0), flags); }
    inline clr_Audio^ PlaySound(System::String^ file, CLR_TINYOAL_FLAG flags, CLR_TINYOAL_FILETYPE filetype) { return PlaySound(gcnew clr_AudioResource(file, flags, filetype), flags); }
    inline clr_Audio^ PlaySound(cli::array<System::Byte>^ data, CLR_TINYOAL_FLAG flags) { return PlaySound(gcnew clr_AudioResource(data, flags, 0), flags); }
    inline clr_Audio^ PlaySound(cli::array<System::Byte>^ data, CLR_TINYOAL_FLAG flags, CLR_TINYOAL_FILETYPE filetype) { return PlaySound(gcnew clr_AudioResource(data, flags, filetype), flags); }
    // Gets the name of the default device
    System::String^ GetDefaultDevice();
    // Sets current device to the given device
    bool SetDevice(System::String^ device);
    // Gets a null-seperated list of all available devices, terminated by a double null character.
    cli::array<System::String^>^ GetDevices();
    // Handy function for figuring out formats
    static unsigned int GetFormat(unsigned short channels, unsigned short bits, bool rear);
    // Given a file or stream, creates or overwrites the openal config file in the proper magical location (%APPDATA% on windows)
    static void SetSettings(System::String^ file);
    static void SetSettingsStream(System::String^ data);

  private:
    tinyoal::cTinyOAL* _ref; //pointer to unmanaged instance
  };
}

#endif