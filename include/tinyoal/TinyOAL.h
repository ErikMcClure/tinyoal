/* TinyOAL - An OpenAL-Soft Audio engine
   Copyright (c)2020 Erik McClure

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

#ifndef TOAL__TINY_OAL_H__
#define TOAL__TINY_OAL_H__

#include "AudioResource.h"
#include <stdarg.h>
#include <memory>

#define TINYOAL_LOG(level, format, ...) tinyoal::TinyOAL::Instance()->Log(__FILE__, __LINE__, level, format, ##__VA_ARGS__)

namespace tinyoal {
  class OggFunctions;
  class Mp3Functions;
  class WaveFunctions;
  class FlacFunctions;
  class Engine;

  // This is the main engine class. It loads functions tables and is used to load audio resources. It also updates all
  // currently playing audio
  class TINYOAL_DLLEXPORT TinyOAL
  {
    typedef int (*FNLOG)(const char*, unsigned int, unsigned char, const char*, va_list);

  public:
    // Constructors
    TinyOAL(unsigned char bufferCount = 4, FNLOG fnLog = 0, const char* forceOAL = 0, const char* forceOGG = 0,
            const char* forceFLAC = 0, const char* forceMP3 = 0);
    ~TinyOAL();
    // This updates any currently playing samples and returns the number that are still playing after the update. The time
    // between calls to this update function can never exceed the length of a buffer, or the sound will cut out.
    unsigned int Update();
    // Creates an instance of a sound either from an existing resource or by creating a new resource
    inline Audio* PlaySound(AudioResource* resource, TINYOAL_FLAG flags)
    {
      return !resource ? 0 : resource->Play(flags | TINYOAL_ISPLAYING);
    }
    inline Audio* PlaySound(const char* file, TINYOAL_FLAG flags)
    {
      return PlaySound(AudioResource::Create(file, flags), flags);
    }
    inline Audio* PlaySound(const void* data, unsigned int len, TINYOAL_FLAG flags)
    {
      return PlaySound(AudioResource::Create(data, len, flags), flags);
    }
    inline Audio* PlaySound(FILE* file, unsigned int len, TINYOAL_FLAG flags)
    {
      return PlaySound(AudioResource::Create(file, len, flags), flags);
    }
    // Writes a line to the log using the logging function
    int Log(const char* file, unsigned int line, unsigned char level, const char* format, ...);
    // Gets the instance (overriden so we can ensure it comes from the right DLL)
    static TinyOAL* Instance();
    // Gets the underlying engine
    Engine* GetEngine();
    // Gets the name of the default device
    const char* GetDefaultDevice();
    // Sets current device to the given device
    bool SetDevice(const char* device);
    // Gets a null-seperated list of all available devices, terminated by a double null character.
    const char* GetDevices();
    // Sets the logging function, returns the previous one.
    FNLOG SetLogging(FNLOG fnLog);
    // Given a file or stream, creates or overwrites the openal config file in the proper magical location (%APPDATA% on
    // windows)
    static void SetSettings(const char* file);
    static void SetSettingsStream(const char* data);
    // Gets functions for a particular codec
    inline OggFunctions* GetOgg() const { return _oggFuncs.get(); }
    inline Mp3Functions* GetMp3() const { return _mp3Funcs.get(); }
    inline FlacFunctions* GetFlac() const { return _flacFuncs.get(); }
    inline WaveFunctions* GetWave() const { return _waveFuncs.get(); }

    typedef size_t (*CODEC_CONSTRUCT)(void* p, void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop);
    typedef bool (*CODEC_SCANHEADER)(const char* fileheader);
    typedef std::pair<void*, unsigned int> (*CODEC_TOWAVE)(void* data, unsigned int datalength, TINYOAL_FLAG flags);

    struct Codec
    {
      CODEC_CONSTRUCT construct;
      CODEC_SCANHEADER scanheader;
      CODEC_TOWAVE towave;
    };

    void RegisterCodec(unsigned char filetype, CODEC_CONSTRUCT construct, CODEC_SCANHEADER scanheader, CODEC_TOWAVE towave);
    Codec* GetCodec(unsigned char filetype);

    static const bssVersionInfo Version;

    template<class T> T* AllocViaPool() { return reinterpret_cast<T*>(_allocDecoder(sizeof(T))); }
    template<class T> void DeallocViaPool(T* p) { _deallocDecoder(reinterpret_cast<char*>(p), sizeof(T)); }

  protected:
    friend class Audio;
    friend class AudioResource;

    TinyOAL(const TinyOAL&)            = delete;
    TinyOAL(TinyOAL&&)                 = delete;
    TinyOAL& operator=(const TinyOAL&) = delete;
    TinyOAL& operator=(TinyOAL&&)      = delete;
    void _construct(const char* forceOGG, const char* forceFLAC, const char* forceMP3);
    void _addAudio(Audio* ref, AudioResource* res);
    void _removeAudio(Audio* ref, AudioResource* res);
    char* _allocDecoder(unsigned int sz);
    void _deallocDecoder(char* p, unsigned int sz);
    unsigned char _getFiletype(const char* fileheader); // fileheader must be at least 4 characters long

    static int DefaultLog(const char* FILE, unsigned int LINE, unsigned char level, const char* format, va_list args);

    static TinyOAL* _instance;

    FNLOG _fnLog;
    std::unique_ptr<Engine> _engine;
    AudioResource* _activereslist;
    AudioResource* _reslist;
    bss::Hash<unsigned int, std::unique_ptr<bss::BlockAlloc>, bss::ARRAY_MOVE> _treealloc;
    bss::HashIns<const char*, AudioResource*> _audiohash;
    bss::BlockPolicy<Audio> _allocaudio;
    bss::Hash<unsigned char, Codec> _codecs;
    std::unique_ptr<OggFunctions> _oggFuncs;
    std::unique_ptr<Mp3Functions> _mp3Funcs;
    std::unique_ptr<WaveFunctions> _waveFuncs;
    std::unique_ptr<FlacFunctions> _flacFuncs;
  };

}

#endif