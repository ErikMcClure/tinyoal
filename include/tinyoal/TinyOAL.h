/* TinyOAL - An OpenAL-Soft Audio engine
   Copyright ©2017 Black Sphere Studios

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __TINY_OAL_H__TOAL__
#define __TINY_OAL_H__TOAL__

#include "AudioResource.h"
#include "bss-util/Singleton.h"
#include "bss-util/AVLTree.h"
#include <stdarg.h>

#define TINYOAL_LOG(level, format, ...) tinyoal::TinyOAL::Instance()->Log(__FILE__,__LINE__, level, format, __VA_ARGS__)

struct OPENALFNTABLE;

namespace tinyoal {
  static const VersionType TINYOAL_VERSION = { TINYOAL_VERSION_MAJOR,TINYOAL_VERSION_MINOR,TINYOAL_VERSION_REVISION };

  class OggFunctions;
  class Mp3Functions;
  class WaveFunctions;
  class FlacFunctions;
  struct ALSoftSettings;

	// This is the main engine class. It loads functions tables and is used to load audio resources. It also updates all currently playing audio 
  class TINYOAL_DLLEXPORT TinyOAL : public bss::Singleton<TinyOAL>
  {
    typedef int(*FNLOG)(const char*, unsigned int, unsigned char, const char*, va_list);

  public:
    // Constructors
    TinyOAL(unsigned char defnumbuf=4, FNLOG fnLog = 0, const char* forceOAL=0, const char* forceOGG=0, const char* forceFLAC=0, const char* forceMP3=0);
    ~TinyOAL();
		// This updates any currently playing samples and returns the number that are still playing after the update. The time between calls
    // to this update function can never exceed the length of a buffer, or the sound will cut out.
    unsigned int Update();
    // Creates an instance of a sound either from an existing resource or by creating a new resource
    inline Audio* PlaySound(AudioResource* resource, TINYOAL_FLAG flags) { return !resource?0:resource->Play(flags|TINYOAL_ISPLAYING); }
    inline Audio* PlaySound(const char* file, TINYOAL_FLAG flags) { return PlaySound(AudioResource::Create(file,flags),flags); }
    inline Audio* PlaySound(const void* data, unsigned int len, TINYOAL_FLAG flags) { return PlaySound(AudioResource::Create(data,len,flags),flags); }
    inline Audio* PlaySound(FILE* file, unsigned int len, TINYOAL_FLAG flags) { return PlaySound(AudioResource::Create(file,len,flags),flags); }
    // Writes a line to the log using the logging function
    int Log(const char* file, unsigned int line, unsigned char level, const char* format, ...);
    // Gets the instance (overriden so we can ensure it comes from the right DLL)
    static TinyOAL* Instance();
    // Gets the name of the default device
    const char* GetDefaultDevice();
    // Sets current device to the given device
    bool SetDevice(const char* device);
    // Gets a null-seperated list of all available devices, terminated by a double null character.
    const char* GetDevices();
    // Sets the logging function, returns the previous one.
    FNLOG SetLogging(FNLOG fnLog);
    // Handy function for figuring out formats
    static unsigned int GetFormat(unsigned short channels, unsigned short bits, bool rear);
    // Given a file or stream, creates or overwrites the openal config file in the proper magical location (%APPDATA% on windows)
    static void SetSettings(const char* file);
    static void SetSettingsStream(const char* data);

    OPENALFNTABLE* oalFuncs;
    OggFunctions* oggFuncs;
    Mp3Functions* mp3Funcs;
    WaveFunctions* waveFuncs;
    FlacFunctions* flacFuncs;
    const unsigned char defNumBuf;

  protected:
    friend class Audio;
    friend class AudioResource;

    void _construct(const char* logfile, const char* forceOAL, const char* forceOGG, const char* forceFLAC, const char* forceMP3);
    void _addAudio(Audio* ref, AudioResource* res);
    void _removeAudio(Audio* ref, AudioResource* res);
    char* _allocDecoder(unsigned int sz);
    void _deallocDecoder(char* p, unsigned int sz);

    static int DefaultLog(const char* FILE, unsigned int LINE, unsigned char level, const char* format, va_list args);

    FNLOG _fnLog;
    AudioResource* _activereslist;
    AudioResource* _reslist;
    bss::BlockAllocVoid _bufalloc;
    bss::AVLTree<unsigned int, std::unique_ptr<bss::BlockAllocVoid>> _treealloc;
  };

}

#endif