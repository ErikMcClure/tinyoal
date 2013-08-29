// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __C_TINY_OAL_H__TOAL__
#define __C_TINY_OAL_H__TOAL__

#include "bss_util/cSingleton.h"
#include "TinyOAL_dlldef.h"
#include "bss_util/cKhash.h"
#include "bss_util/bss_alloc_fixed.h"
#include "bss_util\bss_util_c.h"
#include <iosfwd>

#define TINYOAL_LOG(level)(TinyOAL::cTinyOAL::Instance()->FormatLog(__FILE__,__LINE__) << (level) << ": ")
#define TINYOAL_LOGM(level,message) (TINYOAL_LOG(level) << (message) << std::endl)

class CWaves;
class ALDeviceList;
struct OPENALFNTABLE;

namespace TinyOAL {
  static const VersionType TINYOAL_VERSION = { TINYOAL_VERSION_MAJOR,TINYOAL_VERSION_MINOR,TINYOAL_VERSION_REVISION };

  class cAudio;
  class cAudioResource;
  class cOggFunctions;
  class cMp3Functions;
  class cAudioRef;

	/* This is the main engine class. It loads up OGG functions, the wave loader class, and the openAL functions. It also updates all currently playing audio */
#pragma warning(push)
#pragma warning(disable:4251) //We really don't give a crap about that stupid std::vector warning
  class TINYOAL_DLLEXPORT cTinyOAL : public bss_util::cSingleton<cTinyOAL>
  {
#pragma warning(pop)
  public:
    cTinyOAL(char defaultbuffers=4, std::ostream* errout=0);
    cTinyOAL(const char* logfile, char defaultbuffers);
    ~cTinyOAL();
    /* Gets and sets the default buffer number */
    inline char getDefaultBuffer() const { return _defaultbuffers; }
    inline void setDefaultBuffer(char defaultbuffer) { _defaultbuffers = defaultbuffer; }
		/* This updates any currently playing samples and returns the number that are still playing after the update. The time between calls to this update function can never exceed the length of a buffer, or the sound will cut out. */
    unsigned int Update();
		/* Gets the error out stream */
    inline std::ostream& ErrOut() { return *_errout; }
		/* Gets the core openAL function table. Intended for internal use only */
    inline OPENALFNTABLE* getFunctions() const { return _functions; }
		/* Gets OGG functions, intended for internal use only */
    inline cOggFunctions* getOGG() const { return _oggfuncs; }
		/* Gets MP3 functions, intended for internal use only, will fail if MP3 support was not compiled. */
    inline cMp3Functions* getMP3() const { return _mp3funcs; }
		/* Gets the wave loader for handling WAV files. Intended for internal use only */
    inline CWaves* GetWaveLoader() const { return _waveLoader; }
    /* Plays a managed sound that is deleted as soon as it stops playing (for one shot sound effects that stack on top of each other) */
    cAudio* PlaySound(const cAudioRef& ref);
    cAudio* PlaySound(const char* file, unsigned char flags);
    cAudio* PlaySound(void* data, unsigned int datalength, unsigned char flags);
    cAudio* PlaySound(_iobuf* file, unsigned int datalength, unsigned char flags);
    /* Gets the formatted error out stream */
    std::ostream& BSS_FASTCALL FormatLog(const char* FILE, unsigned int LINE);

  protected:
    friend class cAudio;

    static bool BSS_FASTCALL _addaudio(cAudio* ref);
    static bool BSS_FASTCALL _removeaudio(cAudio* ref);
    static const char* BSS_FASTCALL _trimpath(const char* path);

    std::ostream* _errout; //outstream that controls where the errors go
    std::filebuf* _errbuf; //used as backup
    OPENALFNTABLE* _functions;
    ALDeviceList* _devicelist;
    cOggFunctions* _oggfuncs;
    cMp3Functions* _mp3funcs;
    CWaves* _waveLoader;
    bss_util::cKhash<cAudio*,char,false,&bss_util::KH_POINTER_HASHFUNC<cAudio*>, &bss_util::KH_INT_EQUALFUNC<cAudio*>> _audiohash;
    char _defaultbuffers;
    cAudio* _audiolist;

  private:
    void _construct(int defaultbuffers);
  };
}

#endif