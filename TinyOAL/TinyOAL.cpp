// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "tinyoal/TinyOAL.h"
#include "openAL/al.h"
#include "openAL/alc.h"
#include "openAL/alext.h"
#include "openAL/loadoal.h"
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

#ifdef BSS_PLATFORM_WIN32
#include "bss-util/win32_includes.h"
#include <ShlObj.h>

// We manually define these to use windows functions because we don't want to import the whole bss library just for its fast convert functions.
size_t UTF8toUTF16(const char* input, ptrdiff_t srclen, wchar_t* output, size_t buflen)
{
  return (size_t)MultiByteToWideChar(CP_UTF8, 0, input, (int)srclen, output, int(!output?0:buflen));
}

size_t UTF16toUTF8(const wchar_t* input, ptrdiff_t srclen, char* output, size_t buflen)
{
  return (size_t)WideCharToMultiByte(CP_UTF8, 0, input, (int)srclen, output, int(!output?0:buflen), NULL, NULL);
}
#else //POSIX

#endif

template<>
TinyOAL* Singleton<TinyOAL>::_instance = 0;
const bssVersionInfo TinyOAL::Version = { TINYOAL_VERSION_REVISION, TINYOAL_VERSION_MINOR, TINYOAL_VERSION_MAJOR };

TinyOAL::TinyOAL(unsigned char defnumbuf, FNLOG fnLog, const char* forceOAL, const char* forceOGG, const char* forceFLAC, const char* forceMP3) :
  _reslist(0), _activereslist(0), defNumBuf(defnumbuf), _bufalloc(defnumbuf*sizeof(ALuint),5), oalFuncs(0), _fnLog((!fnLog)?(&DefaultLog): fnLog)
{
  _construct("TinyOAL_log.txt",forceOAL,forceOGG,forceFLAC,forceMP3);
}

TinyOAL::~TinyOAL()
{
  // Destroy managed pointers 
  while(_activereslist) delete _activereslist;
  while(_reslist) delete _reslist;

  if(oalFuncs) //If _functions doesn't exist, we don't need to do (or are capable of doing) this
  {
	  ALCcontext* pContext = oalFuncs->alcGetCurrentContext();
	  ALCdevice* pDevice = oalFuncs->alcGetContextsDevice(pContext);
  	
	  oalFuncs->alcMakeContextCurrent(NULL);
	  oalFuncs->alcDestroyContext(pContext);
	  oalFuncs->alcCloseDevice(pDevice);
    UnloadOAL10Library();
    delete oalFuncs;
  }

  if(waveFuncs) delete waveFuncs;
  if(oggFuncs) delete oggFuncs;
  if(mp3Funcs) delete mp3Funcs;
  if(flacFuncs) delete flacFuncs;
}
unsigned int TinyOAL::Update()
{
  unsigned int a=0;
  AudioResource* cur;
  AudioResource* hold = _activereslist; // Theoretically an audioresource CAN get destroyed by an update() indirectly.
  Audio* x;
  Audio* t;
  while((cur=hold)!=0) {
    hold=cur->next;
    t=cur->_activelist;
    while((x=t)!=0) {
      t=x->next;
      a+=(char)x->Update();
    }
  }
  return a;
}

int TinyOAL::Log(const char* file, unsigned int line, unsigned char level, const char* format, ...)
{
  va_list vl;
  va_start(vl, format);
  int r = _fnLog(file, line, level, format, vl);
  va_end(vl);
  return r;
}
int TinyOAL::DefaultLog(const char* file, unsigned int line, unsigned char level, const char* format, va_list args)
{
  static std::unique_ptr<std::FILE, decltype(&std::fclose)> f(std::fopen("tinyoal.log", "wb"), &std::fclose);
  const char* r = strrchr(file, '/');
  const char* r2 = strrchr(file, '\\');
  r = bssmax(r, r2);
  file = (!r) ? file : (r + 1);

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

typedef struct
{
	std::string			strDeviceName;
	int				iMajorVersion;
	int				iMinorVersion;
	unsigned int	uiSourceCount;
	std::vector<std::string>	pvstrExtensions;
	bool			bSelected;
} ALDEVICEINFO, *LPALDEVICEINFO;

const char* TinyOAL::GetDevices()
{
  if(!oalFuncs) return 0;
  return oalFuncs->alcGetString(NULL, ALC_DEVICE_SPECIFIER);
}
const char* TinyOAL::GetDefaultDevice()
{
  if(!oalFuncs) return 0;
  return oalFuncs->alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
}
bool TinyOAL::SetDevice(const char* device)
{
  ALCdevice* pDevice = oalFuncs->alcOpenDevice(device);
	if (!pDevice)
	{
    TINYOAL_LOG(1, "Failed to open device: %s", device);
    return false;
  }
	ALCcontext* pContext = oalFuncs->alcCreateContext(pDevice, NULL);
	if(pContext)
	{
    TINYOAL_LOG(4, "Opened Device: %s", device);
		oalFuncs->alcMakeContextCurrent(pContext);
    return true;
	}
	oalFuncs->alcCloseDevice(pDevice);
  TINYOAL_LOG(1, "Failed to create context for %s", device);
  return false;
}

void TinyOAL::_construct(const char* logfile, const char* forceOAL, const char* forceOGG, const char* forceFLAC, const char* forceMP3)
{
  oalFuncs=0;
  OPENALFNTABLE* functmp = new OPENALFNTABLE();

  //OpenAL initialization code
	const char *actualDeviceName;
  OPENALFNTABLE& ALFunction=*functmp;
  std::vector<ALDEVICEINFO> vDeviceInfo;
	size_t defaultDeviceIndex = 0;

	// grab function pointers for 1.0-API functions, and if successful proceed to enumerate all devices
	if (LoadOAL10Library(forceOAL, &ALFunction)!=0) {
		if (ALFunction.alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT")) {
			char* devices = (char *)ALFunction.alcGetString(NULL, ALC_DEVICE_SPECIFIER);
			const char* defaultDeviceName = (char *)ALFunction.alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
      TINYOAL_LOG(4, "Default device name is: %s", defaultDeviceName);
			size_t index = 0;
			// go through device list (each device terminated with a single NULL, list terminated with double NULL)
			while ((*devices) != NULL) {
				if(!strcmp(defaultDeviceName, devices)) {
					defaultDeviceIndex = index;
				}
				ALCdevice *device = ALFunction.alcOpenDevice(devices);
				if (device) {
					ALCcontext *context = ALFunction.alcCreateContext(device, NULL);
					if (context) {
						ALFunction.alcMakeContextCurrent(context);
						actualDeviceName = ALFunction.alcGetString(device, ALC_DEVICE_SPECIFIER);
						bool bNewName = true;
						for (size_t i = 0; i < vDeviceInfo.size(); i++) {
							if (!strcmp(vDeviceInfo[i].strDeviceName.c_str(), actualDeviceName))
								bNewName = false;
						}
						if ((bNewName) && (actualDeviceName != NULL) && (strlen(actualDeviceName) > 0)) {
              vDeviceInfo.resize(vDeviceInfo.size()+1);
              ALDEVICEINFO& ALDeviceInfo = vDeviceInfo.back();
							ALDeviceInfo.bSelected = true;
							ALDeviceInfo.strDeviceName = actualDeviceName;
							ALFunction.alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &ALDeviceInfo.iMajorVersion);
							ALFunction.alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &ALDeviceInfo.iMinorVersion);

              const char* exts[] = { "ALC_EXT_CAPTURE", "ALC_EXT_EFX", "AL_EXT_OFFSET", "AL_EXT_LINEAR_DISTANCE","AL_EXT_EXPONENT_DISTANCE",
                "EAX2.0","EAX3.0","EAX4.0","EAX5.0","EAX-RAM" };

							// Check for Extensions
              for(int i = 0; i < sizeof(exts)/sizeof(const char*); ++i)
							if (ALFunction.alcIsExtensionPresent(device, exts[i]) == AL_TRUE)
								ALDeviceInfo.pvstrExtensions.push_back(exts[i]);

							ALDeviceInfo.uiSourceCount = 0; // Get Source Count
						}
						ALFunction.alcMakeContextCurrent(NULL);
						ALFunction.alcDestroyContext(context);
					}
					ALFunction.alcCloseDevice(device);
				}
				devices += strlen(devices) + 1;
				index += 1;
			}
		}
	}
  
  oalFuncs = functmp;
	if(!vDeviceInfo.size())
    TINYOAL_LOG(1, "No devices in device list!");
  else if(SetDevice(vDeviceInfo[defaultDeviceIndex].strDeviceName.c_str()))
    functmp=0;
  
  if(functmp) { oalFuncs=0; delete functmp; } // If functmp is nonzero, something blew up, so delete it

  waveFuncs = new WaveFunctions();
  oggFuncs = new OggFunctions(forceOGG);
  flacFuncs = new FlacFunctions(forceFLAC);
  mp3Funcs = new Mp3Functions(forceMP3);
  if(oggFuncs->Failure()) { delete oggFuncs; oggFuncs=0; }
  if(flacFuncs->Failure()) { delete flacFuncs; flacFuncs=0; }
  if(mp3Funcs->Failure()) { delete mp3Funcs; mp3Funcs=0; }

  AudioResource::RegisterCodec(AudioResource::TINYOAL_FILETYPE_WAV, AudioResourceWAV::Construct, AudioResourceWAV::ScanHeader, AudioResourceWAV::ToWave);
  AudioResource::RegisterCodec(AudioResource::TINYOAL_FILETYPE_OGG, AudioResourceOGG::Construct, AudioResourceOGG::ScanHeader, AudioResourceOGG::ToWave);
  AudioResource::RegisterCodec(AudioResource::TINYOAL_FILETYPE_MP3, AudioResourceMP3::Construct, AudioResourceMP3::ScanHeader, AudioResourceMP3::ToWave);
  AudioResource::RegisterCodec(AudioResource::TINYOAL_FILETYPE_FLAC, AudioResourceFLAC::Construct, AudioResourceFLAC::ScanHeader, AudioResourceFLAC::ToWave);
}
unsigned int TinyOAL::GetFormat(unsigned short channels, unsigned short bits, bool rear)
{
  unsigned short hash = (channels<<8)|bits;
  switch(hash)
  {
  case ((1<<8)|8): return AL_FORMAT_MONO8;
  case ((1<<8)|16): return AL_FORMAT_MONO16;
  case ((1<<8)|32): return AL_FORMAT_MONO_FLOAT32;
  case ((1<<8)|64): return AL_FORMAT_MONO_DOUBLE_EXT;
  case ((2<<8)|8): return rear?AL_FORMAT_REAR8:AL_FORMAT_STEREO8;
  case ((2<<8)|16): return rear?AL_FORMAT_REAR16:AL_FORMAT_STEREO16;
  case ((2<<8)|32): return rear?AL_FORMAT_REAR32:AL_FORMAT_STEREO_FLOAT32;
  case ((2<<8)|64): return AL_FORMAT_STEREO_DOUBLE_EXT;
  case ((4<<8)|8): return AL_FORMAT_QUAD8;
  case ((4<<8)|16): return AL_FORMAT_QUAD16;
  case ((4<<8)|32): return AL_FORMAT_QUAD32;
  case ((6<<8)|8): return AL_FORMAT_51CHN8;
  case ((6<<8)|16): return AL_FORMAT_51CHN16;
  case ((6<<8)|32): return AL_FORMAT_51CHN32;
  case ((7<<8)|8): return AL_FORMAT_61CHN8;
  case ((7<<8)|16): return AL_FORMAT_61CHN16;
  case ((7<<8)|32): return AL_FORMAT_61CHN32;
  case ((8<<8)|8): return AL_FORMAT_71CHN8;
  case ((8<<8)|16): return AL_FORMAT_71CHN16;
  case ((8<<8)|32): return AL_FORMAT_71CHN32;
  }
  return 0;
}

void TinyOAL::_addAudio(Audio* ref, AudioResource* res)
{
  if(!res->_activelist) // If true we need to move it
  {
    bss::LLRemove(res,_reslist);
    bss::LLAdd(res,_activereslist);
  }
  bss::LLRemove(ref,res->_inactivelist);
  bss::LLAdd(ref,res->_activelist,res->_activelistend);
  ++res->_numactive;
}
void TinyOAL::_removeAudio(Audio* ref, AudioResource* res)
{
  bss::LLRemove(ref,res->_activelist,res->_activelistend);
  bss::LLAdd(ref,res->_inactivelist);
  --res->_numactive;
  if(!res->_activelist)
  {
    bss::LLRemove(res,_activereslist);
    bss::LLAdd(res,_reslist);
  }
}

char* TinyOAL::_allocDecoder(unsigned int sz)
{
  auto p = _treealloc.GetRef(sz);
  if(!p)
  {
    TINYOAL_LOG(4, "Created allocation pool of size %u", sz);
    _treealloc.Insert(sz,std::unique_ptr<bss::BlockAllocVoid>(new BlockAllocVoid(sz,3)));
    p = _treealloc.GetRef(sz);
  }
  return !p?0:(char*)(*p)->alloc(1);
}
void TinyOAL::_deallocDecoder(char* s, unsigned int sz)
{
  auto p = _treealloc.GetRef(sz);
  if(p) (*p)->dealloc(s);
  else TINYOAL_LOG(2, "decoder buffer deallocation failure.");
}

void TinyOAL::SetSettings(const char* file)
{
  FILE* f;
  FOPEN(f,file,"rb");
  if(!f) { TINYOAL_LOG(2, "Failed to open source config file for reading."); return; }
  fseek(f,0,SEEK_END);
  long len=ftell(f);
  fseek(f,0,SEEK_SET);
  char* buf=(char*)malloc(len);
  fread(buf,1,len,f);
  fclose(f);
  SetSettingsStream(buf);
}
void TinyOAL::SetSettingsStream(const char* data)
{
  const char* magic; //stores the location of the openAL config file, usually buried in %APPDATA% or other dark corners of the abyss.
  Str path;
#ifdef BSS_PLATFORM_WIN32
  path.resize(MAX_PATH); // We have to resize here, putting numbers into the constructor just reserves things instead
  if(SHGetSpecialFolderPathA(NULL, path.UnsafeString(), CSIDL_APPDATA, FALSE) != FALSE)
  {
    path.RecalcSize();
    path+="\\alsoft.ini";
  }
  magic=path;
#else
  if((magic=getenv("HOME")) != NULL && *magic)
  {
    path=magic;
    path+="/.alsoftrc";
    magic=path;
  }
  else
    magic="/etc/openal/alsoft.conf";
#endif
  FILE* f;
  FOPEN(f,magic,"wb");
  if(!f) { TINYOAL_LOG(2, "Failed to open destination config file for writing."); return; }
  if(data) fwrite(data,1,strlen(data),f);
  fclose(f);
}
   
// I ultimately decided against putting this in the engine because it was easier to just write the config files and move them
/*

  struct ALSoftSettings { // Explanations of all these options can be found in alsoftrc.sample
    enum : unsigned char { DISABLE_CPU_SSE=1, DISABLE_CPU_NEON=2 } disable_cpu_exts; //disable-cpu-exts;
    enum : unsigned char { CHANNELS_MONO, CHANNELS_STEREO, CHANNELS_QUAD, CHANNELS_SURROUND51, CHANNELS_SURROUND61, CHANNELS_SURROUND71 } channels;
    enum : unsigned char { SAMPLETYPE_INT8,SAMPLETYPE_UINT8,SAMPLETYPE_INT16,SAMPLETYPE_UINT16,SAMPLETYPE_INT32,SAMPLETYPE_UINT32,SAMPLETYPE_FLOAT32 } sample_type; //sample-type
    bool hrtf;
    Str hrtf_tables;
    unsigned char cf_level; //0-6
    bool wide_stereo; //wide-stereo
    unsigned int frequency;
    enum : unsigned char { RESAMPLER_POINT,RESAMPLER_LINEAR,RESAMPLER_CUBIC } resampler;
    bool rt_prio; //rt-prio
    unsigned short period_size;
    unsigned char periods;
    unsigned short sources;
    Str drivers; //full list: pulse,alsa,core,oss,solaris,sndio,mmdevapi,dsound,winmm,port,opensl,null,wave
    enum : unsigned char { EXCLUDE_EAXREVERB=1,EXCLUDE_REVERB=2,EXCLUDE_ECHO=4,EXCLUDE_MODULATOR=8,EXCLUDE_DEDICATED=16 } excludefx;
    unsigned char slots;
    unsigned char sends;
    float layout[8]; //back-left(0), side-left(1), front-left(2), front-center(3), front-right(4), side-right(5), back-right(6), and back-center(7)
    float layout_stereo[2]; // fl, fr
    float layout_quad[4]; // fl, fr, bl, br
    float layout_surround51[5]; //fl, fr, fc, bl, br
    float layout_surround61[6]; //fl, fr, fc, sl, sr, bc
    float layout_surround71[7]; //fl, fr, fc, sl, sr, bl, br
    enum : unsigned char { REVERB_NONE,REVERB_GENERIC,REVERB_PADDEDCELL,REVERB_ROOM,REVERB_BATHROOM,REVERB_LIVINGROOM,REVERB_STONEROOM,
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