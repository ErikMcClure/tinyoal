// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "tinyoal/TinyOAL.h"
#include "OALEngine.h"
#include "WASEngine.h"
#include "AudioResourceWAV.h"
#include "AudioResourceOGG.h"
#include "AudioResourceMP3.h"
#include "AudioResourceFLAC.h"
#include "tinyoal/Audio.h"
#include "OggFunctions.h"
#include "Mp3Functions.h"
#include "WaveFunctions.h"
#include "FlacFunctions.h"
#include <fstream>
#include <memory>
#include <stdio.h>

using namespace tinyoal;
using namespace bss;

#define LOG(level, format, ...) Log(__FILE__, __LINE__, level, format, ##__VA_ARGS__)

#ifdef BSS_PLATFORM_WIN32
  #include "win32_includes.h"
  #include <ShlObj.h>

// We manually define these to use windows functions because we don't want to import the whole bss library just for its fast
// convert functions.
size_t UTF8toUTF16(const char* input, ptrdiff_t srclen, wchar_t* output, size_t buflen)
{
  return (size_t)MultiByteToWideChar(CP_UTF8, 0, input, (int)srclen, output, int(!output ? 0 : buflen));
}

size_t UTF16toUTF8(const wchar_t* input, ptrdiff_t srclen, char* output, size_t buflen)
{
  return (size_t)WideCharToMultiByte(CP_UTF8, 0, input, (int)srclen, output, int(!output ? 0 : buflen), nullptr, nullptr);
}
#else // POSIX

#endif

TinyOAL* TinyOAL::_instance           = nullptr;
const bssVersionInfo TinyOAL::Version = { 0, TINYOAL_VERSION_REVISION, TINYOAL_VERSION_MINOR, TINYOAL_VERSION_MAJOR };

TinyOAL::TinyOAL(enum ENGINE_TYPE type, FNLOG fnLog, unsigned char defnumbuf, const char* forceOAL, const char* forceOGG,
                 const char* forceFLAC,
                 const char* forceMP3) :
  _reslist(nullptr),
  _activereslist(nullptr),
  _fnLog((!fnLog) ? (&DefaultLog) : fnLog),
  _allocaudio(5),
  _codecs(AudioResource::TINYOAL_FILETYPE_CUSTOM - 1),
  _audiohash(4)
{
  _instance = this;
  switch(type)
  {
  case ENGINE_OPENAL: _engine.reset(new OALEngine(defnumbuf, forceOAL)); break;
  case ENGINE_WASAPI_SHARED: _engine.reset(new WASEngine(false)); break;
  case ENGINE_WASAPI_EXCLUSIVE: _engine.reset(new WASEngine(true)); break;
  }
  _engine->Init();
  _construct(forceOGG, forceFLAC, forceMP3);
}

TinyOAL::~TinyOAL()
{
  // Destroy managed pointers
  while(_activereslist)
    delete _activereslist;
  while(_reslist)
    delete _reslist;

  // Ensure all destructors are called before TinyOAL deletes it's instance pointer
  _waveFuncs.reset();
  _oggFuncs.reset();
  _mp3Funcs.reset();
  _flacFuncs.reset();
  _engine.reset();

  if(_instance == this)
    _instance = nullptr;
}
uint32_t TinyOAL::Update()
{
  uint32_t a = 0;
  AudioResource* cur;
  AudioResource* hold = _activereslist; // Theoretically an audioresource CAN get destroyed by an update() indirectly.
  Audio* x;
  Audio* t;
  while((cur = hold) != nullptr)
  {
    hold = cur->next;
    t    = cur->_activelist;
    while((x = t) != nullptr)
    {
      t = x->next;
      a += (char)x->Update();
    }
  }
  return a;
}

int TinyOAL::Log(const char* file, uint32_t line, unsigned char level, const char* format, ...)
{
  va_list vl;
  va_start(vl, format);
  int r = _fnLog(file, line, level, format, vl);
  va_end(vl);
  return r;
}
int TinyOAL::DefaultLog(const char* file, uint32_t line, unsigned char level, const char* format, va_list args)
{
  static std::unique_ptr<std::FILE, decltype(&std::fclose)> f(std::fopen("tinyoal.log", "wb"), &std::fclose);
  const char* r  = strrchr(file, '/');
  const char* r2 = strrchr(file, '\\');
  r              = bssmax(r, r2);
  file           = (!r) ? file : (r + 1);

  const char* lvl = "UNKNOWN";
  switch(level)
  {
  case 0: lvl = "FATAL"; break;
  case 1: lvl = "ERROR"; break;
  case 2: lvl = "WARNING"; break;
  case 3: lvl = "NOTICE"; break;
  case 4: lvl = "INFO"; break;
  case 5: lvl = "DEBUG"; break;
  }

  int ret = 0;
  if(f.get())
  {
    ret += fprintf(f.get(), "(%s:%u) %s: ", file, line, lvl);
    ret += vfprintf(f.get(), format, args);
    ret += fwrite("\n", 1, 1, f.get());
    fflush(f.get());
  }
  return ret;
}

TinyOAL* TinyOAL::Instance() { return _instance; }
Engine* TinyOAL::GetEngine() { return _engine.get(); }
const char* TinyOAL::GetDevices() { return 0; }
size_t TinyOAL::GetDefaultDevice(char* out, size_t len) { return _engine->GetDefaultDevice(out, len); }
bool TinyOAL::SetDevice(const char* device) { return _engine->SetDevice(device) == 0; }

void TinyOAL::_construct(const char* forceOGG, const char* forceFLAC, const char* forceMP3)
{
  _waveFuncs.reset(new WaveFunctions());
  _oggFuncs.reset(new OggFunctions(forceOGG));
  _flacFuncs.reset(new FlacFunctions(forceFLAC));
  _mp3Funcs.reset(new Mp3Functions(forceMP3));

  if(_oggFuncs->Failure())
    _oggFuncs.reset();
  if(_flacFuncs->Failure())
    _flacFuncs.reset();
  if(_mp3Funcs->Failure())
    _mp3Funcs.reset();

  RegisterCodec(AudioResource::TINYOAL_FILETYPE_WAV, AudioResourceWAV::Construct, AudioResourceWAV::ScanHeader,
                AudioResourceWAV::ToWave);
  RegisterCodec(AudioResource::TINYOAL_FILETYPE_OGG, AudioResourceOGG::Construct, AudioResourceOGG::ScanHeader,
                AudioResourceOGG::ToWave);
  RegisterCodec(AudioResource::TINYOAL_FILETYPE_MP3, AudioResourceMP3::Construct, AudioResourceMP3::ScanHeader,
                AudioResourceMP3::ToWave);
  RegisterCodec(AudioResource::TINYOAL_FILETYPE_FLAC, AudioResourceFLAC::Construct, AudioResourceFLAC::ScanHeader,
                AudioResourceFLAC::ToWave);
}

void TinyOAL::_addAudio(Audio* ref, AudioResource* res)
{
  if(!res->_activelist) // If true we need to move it
  {
    bss::LLRemove(res, _reslist);
    bss::LLAdd(res, _activereslist);
  }
  bss::LLRemove(ref, res->_inactivelist);
  bss::LLAdd(ref, res->_activelist, res->_activelistend);
  ++res->_numactive;
}
void TinyOAL::_removeAudio(Audio* ref, AudioResource* res)
{
  bss::LLRemove(ref, res->_activelist, res->_activelistend);
  bss::LLAdd(ref, res->_inactivelist);
  --res->_numactive;
  if(!res->_activelist)
  {
    bss::LLRemove(res, _activereslist);
    bss::LLAdd(res, _reslist);
  }
}

char* TinyOAL::_allocDecoder(uint32_t sz)
{
  auto p = _treealloc[sz];
  if(!p)
  {
    LOG(4, "Created allocation pool of size %u", sz);
    _treealloc.Insert(sz, std::unique_ptr<bss::BlockAlloc>(new BlockAlloc(sz, 3)));
    p = _treealloc[sz];
  }
  return !p ? 0 : (char*)p->Alloc();
}
void TinyOAL::_deallocDecoder(char* s, uint32_t sz)
{
  auto p = _treealloc[sz];
  if(p)
    p->Dealloc(s);
  else
    LOG(2, "decoder buffer deallocation failure.");
}

void TinyOAL::SetSettings(const char* file)
{
  FILE* f;
  FOPEN(f, file, "rb");
  if(!f)
  {
    if(_instance)
      TINYOAL_LOG(2, "Failed to open source config file for reading.");
    return;
  }

  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* buf = (char*)malloc(len);
  fread(buf, 1, len, f);
  fclose(f);
  SetSettingsStream(buf);
}
void TinyOAL::SetSettingsStream(const char* data)
{
  const char*
    magic; // stores the location of the openAL config file, usually buried in %APPDATA% or other dark corners of the abyss.
  Str path;
#ifdef BSS_PLATFORM_WIN32
  path.resize(MAX_PATH); // We have to resize here, putting numbers into the constructor just reserves things instead
  if(SHGetSpecialFolderPathA(nullptr, path.UnsafeString(), CSIDL_APPDATA, FALSE) != FALSE)
  {
    path.RecalcSize();
    path += "\\alsoft.ini";
  }
  magic = path;
#else
  if((magic = getenv("HOME")) != nullptr && *magic)
  {
    path = magic;
    path += "/.alsoftrc";
    magic = path;
  }
  else
    magic = "/etc/openal/alsoft.conf";
#endif
  FILE* f;
  FOPEN(f, magic, "wb");
  if(!f)
  {
    if(_instance)
      TINYOAL_LOG(2, "Failed to open destination config file for writing.");
    return;
  }

  if(data)
    fwrite(data, 1, strlen(data), f);
  fclose(f);
}

void TinyOAL::RegisterCodec(unsigned char filetype, CODEC_CONSTRUCT construct, CODEC_SCANHEADER scanheader,
                            CODEC_TOWAVE towave)
{
  Codec c = { construct, scanheader, towave };
  _codecs.Insert(filetype, c);
}

TinyOAL::Codec* TinyOAL::GetCodec(unsigned char filetype) { return _codecs.Get(filetype); }

// This function does NOT check to see if fileheader is 8 characters long
unsigned char TinyOAL::_getFiletype(const char* fileheader)
{
  for(auto [k, v] : _codecs)
  {
    if(v.scanheader(fileheader))
      return k;
  }

  return AudioResource::TINYOAL_FILETYPE_UNKNOWN;
}

// I ultimately decided against putting this in the engine because it was easier to just write the config files and move them
/*

  struct ALSoftSettings { // Explanations of all these options can be found in alsoftrc.sample
    enum : unsigned char { DISABLE_CPU_SSE=1, DISABLE_CPU_NEON=2 } disable_cpu_exts; //disable-cpu-exts;
    enum : unsigned char { CHANNELS_MONO, CHANNELS_STEREO, CHANNELS_QUAD, CHANNELS_SURROUND51, CHANNELS_SURROUND61,
  CHANNELS_SURROUND71 } channels; enum : unsigned char {
  SAMPLETYPE_INT8,SAMPLETYPE_UINT8,SAMPLETYPE_INT16,SAMPLETYPE_UINT16,SAMPLETYPE_INT32,SAMPLETYPE_UINT32,SAMPLETYPE_FLOAT32
  } sample_type; //sample-type bool hrtf; Str hrtf_tables; unsigned char cf_level; //0-6 bool wide_stereo; //wide-stereo
    uint32_t frequency;
    enum : unsigned char { RESAMPLER_POINT,RESAMPLER_LINEAR,RESAMPLER_CUBIC } resampler;
    bool rt_prio; //rt-prio
    uint16_t period_size;
    unsigned char periods;
    uint16_t sources;
    Str drivers; //full list: pulse,alsa,core,oss,solaris,sndio,mmdevapi,dsound,winmm,port,opensl,null,wave
    enum : unsigned char { EXCLUDE_EAXREVERB=1,EXCLUDE_REVERB=2,EXCLUDE_ECHO=4,EXCLUDE_MODULATOR=8,EXCLUDE_DEDICATED=16 }
  excludefx; unsigned char slots; unsigned char sends; float layout[8]; //back-left(0), side-left(1), front-left(2),
  front-center(3), front-right(4), side-right(5), back-right(6), and back-center(7) float layout_stereo[2]; // fl, fr float
  layout_quad[4]; // fl, fr, bl, br float layout_surround51[5]; //fl, fr, fc, bl, br float layout_surround61[6]; //fl, fr,
  fc, sl, sr, bc float layout_surround71[7]; //fl, fr, fc, sl, sr, bl, br enum : unsigned char {
  REVERB_NONE,REVERB_GENERIC,REVERB_PADDEDCELL,REVERB_ROOM,REVERB_BATHROOM,REVERB_LIVINGROOM,REVERB_STONEROOM,
      REVERB_AUDITORIUM,REVERB_CONCERTHALL,REVERB_CAVE,REVERB_ARENA,REVERB_HANGER,REVERB_CARPETHALLWAY,REVERB_HALLWAY,REVERB_STONECORRIDOR,
      REVERB_ALLEY,REVERB_FOREST,REVERB_CITY,REVERB_MOUNTAINS,REVERB_QUARRY,REVERB_PLAIN,REVERB_PARKINGLOT,REVERB_SEWERPIPE,REVERB_UNDERWATER,
      REVERB_DRUGGED,REVERB_DIZZY,REVERB_PSYCHOTIC } default_reverb; //default-reverb
    bool trap_alc_error; //trap-alc-error
    bool trap_al_error; //trap-al-error
    struct reverb {
      float boost;
      bool emulate_eax; // emulate-eax
    };
    struct alsa {
      Str device;
      Str device_prefix; //device-prefix
      Str capture;
      Str capture_prefix; //capture-prefix
      bool mmap;
    };
    struct oss {
      Str device;
      Str capture;
    };
    struct solaris {
      Str device;
    };
    struct mmdevapi {};
    struct dsound {};
    struct winmm {};
    struct port {
      bool spawn_server; //spawn-server
      bool allow_moves; //allow-moves
    };
    struct wave {
      Str file;
    };
  };
  */