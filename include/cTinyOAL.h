/* TinyOAL - An OpenAL-Soft Audio engine
   Copyright ©2013 Erik McClure

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

#ifndef __C_TINY_OAL_H__TOAL__
#define __C_TINY_OAL_H__TOAL__

#include "cAudioResource.h"
#include "bss_util/cSingleton.h"
#include "bss_util/cAVLtree.h"

#define TINYOAL_LOG(level)(TinyOAL::cTinyOAL::Instance()->FormatLog(__FILE__,__LINE__) << (level) << ": ")
#define TINYOAL_LOGM(level,message) (TINYOAL_LOG(level) << (message) << std::endl)

struct OPENALFNTABLE;

namespace TinyOAL {
  static const VersionType TINYOAL_VERSION = { TINYOAL_VERSION_MAJOR,TINYOAL_VERSION_MINOR,TINYOAL_VERSION_REVISION };

  class cOggFunctions;
  class cMp3Functions;
  class cWaveFunctions;

	// This is the main engine class. It loads functions tables and is used to load audio resources. It also updates all currently playing audio 
  class TINYOAL_DLLEXPORT cTinyOAL : protected bss_util::cSingleton<cTinyOAL>
  {
  public:
    // Constructors
    cTinyOAL(unsigned char defnumbuf=4, std::ostream* errout=0);
    cTinyOAL(const char* logfile, unsigned char defnumbuf);
    ~cTinyOAL();
		// This updates any currently playing samples and returns the number that are still playing after the update. The time between calls
    // to this update function can never exceed the length of a buffer, or the sound will cut out.
    unsigned int Update();
		// Gets the error out stream 
    inline std::ostream& ErrOut() { return *_errout; }
    // Creates an instance of a sound either from an existing resource or by creating a new resource
    inline cAudio* PlaySound(cAudioResource* resource, TINYOAL_FLAG flags) { return !resource?0:resource->Play(flags|TINYOAL_ISPLAYING); }
    inline cAudio* PlaySound(const char* file, TINYOAL_FLAG flags) { return PlaySound(cAudioResource::Create(file,flags),flags); }
    inline cAudio* PlaySound(void* data, unsigned int len, TINYOAL_FLAG flags) { return PlaySound(cAudioResource::Create(data,len,flags),flags); }
    inline cAudio* PlaySound(FILE* file, unsigned int len, TINYOAL_FLAG flags) { return PlaySound(cAudioResource::Create(file,len,flags),flags); }
    // Gets the formatted error out stream 
    std::ostream& BSS_FASTCALL FormatLog(const char* FILE, unsigned int LINE);
    // Gets the instance (overriden so we can ensure it comes from the right DLL)
    static cTinyOAL* Instance();
    // Gets the name of the default device
    const char* GetDefaultDevice();
    // Sets current device to the given device
    bool SetDevice(const char* device);
    // Gets a null-seperated list of all available devices, terminated by a double null character.
    const char* GetDevices();

    OPENALFNTABLE* oalFuncs;
    cOggFunctions* oggFuncs;
    cMp3Functions* mp3Funcs;
    cWaveFunctions* waveFuncs;
    const unsigned char defNumBuf;

  protected:
    friend class cAudio;
    friend class cAudioResource;

    void BSS_FASTCALL _construct(std::ostream* errout,const char* logfile);
    void BSS_FASTCALL _addaudio(cAudio* ref, cAudioResource* res);
    void BSS_FASTCALL _removeaudio(cAudio* ref, cAudioResource* res);
    char* BSS_FASTCALL _allocdecoder(unsigned int sz);
    void BSS_FASTCALL _deallocdecoder(char* p, unsigned int sz);

    std::ostream* _errout; //outstream that controls where the errors go
    std::filebuf* _errbuf; //used as backup
    cAudioResource* _activereslist;
    cAudioResource* _reslist;
    bss_util::cFixedAllocVoid _bufalloc;
    bss_util::cAVLtree<unsigned int,std::unique_ptr<bss_util::cFixedAllocVoid>> _treealloc;
  };
}

#endif