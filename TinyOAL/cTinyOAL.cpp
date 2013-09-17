// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cTinyOAL.h"
#include "openAL/al.h"
#include "openAL/alc.h"
#include "openAL/loadoal.h"
#include "cAudioResource.h"
#include "cAudio.h"
#include "cOggFunctions.h"
#include "cMp3Functions.h"
#include "cWaveFunctions.h"
#include <fstream>

using namespace TinyOAL;
using namespace bss_util;

#ifdef BSS_PLATFORM_WIN32
#include "bss_util/bss_win32_includes.h"

// We manually define these to use windows functions because we don't want to import the whole bss_util library just for its fast convert functions.
extern size_t BSS_FASTCALL UTF8toUTF16(const char* input,wchar_t* output, size_t buflen)
{
  return (size_t)MultiByteToWideChar(CP_UTF8, 0, input, -1, output, !output?0:buflen);
}

extern size_t BSS_FASTCALL UTF16toUTF8(const wchar_t* input, char* output, size_t buflen)
{
  return (size_t)WideCharToMultiByte(CP_UTF8, 0, input, -1, output, !output?0:buflen, NULL, NULL);
}
#else //POSIX

#endif

cTinyOAL::cTinyOAL(unsigned char defnumbuf, std::ostream* errout) : cSingleton<cTinyOAL>(this), _reslist(0), _activereslist(0), _errbuf(0),
  defNumBuf(defnumbuf), _bufalloc(defnumbuf*sizeof(ALuint),5), oalFuncs(0)
{
  _construct(errout,"TinyOAL_log.txt");
}
cTinyOAL::cTinyOAL(const char* logfile, unsigned char defnumbuf) : cSingleton<cTinyOAL>(this), _reslist(0), _activereslist(0), _errbuf(0),
  defNumBuf(defnumbuf), _bufalloc(defnumbuf*sizeof(ALuint),5), oalFuncs(0)
{
  _construct(0,!logfile?"TinyOAL_log.txt":logfile);
}

cTinyOAL::~cTinyOAL()
{
  // Destroy managed pointers 
  while(_activereslist) delete _activereslist;
  while(_reslist) delete _reslist;

  if(_errbuf) //if this is not 0, it means we used it as a backup error output and we need to blow it up
  {
    _errbuf->close();
    delete _errbuf;
    delete _errout; //If we created _errbuf, we also created this, so blow it up
    _errbuf = 0; //just in case
    _errout = 0;
  } //otherwise _errout is just a pointer that we don't want to mess with

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
}
unsigned int cTinyOAL::Update()
{
  unsigned int a=0;
  cAudioResource* cur;
  cAudioResource* hold = _activereslist; // Theoretically an audioresource CAN get destroyed by an update() indirectly.
  cAudio* x;
  cAudio* t;
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
std::ostream& BSS_FASTCALL cTinyOAL::FormatLog(const char* FILE, unsigned int LINE)
{
	const char* r=strrchr(FILE,'/');
	const char* r2=strrchr(FILE,'\\');
  r=bssmax(r,r2);
  FILE = (!r)?FILE:(r+1);

  (*_errout) << '(' << FILE << ':' << LINE << ") ";
  return *_errout;
}
cTinyOAL* cTinyOAL::Instance() { return _instance; }

typedef struct
{
	std::string			strDeviceName;
	int				iMajorVersion;
	int				iMinorVersion;
	unsigned int	uiSourceCount;
	std::vector<std::string>	pvstrExtensions;
	bool			bSelected;
} ALDEVICEINFO, *LPALDEVICEINFO;

void cTinyOAL::_construct(std::ostream* errout,const char* logfile)
{
  if(!errout) //if errout is zero, produce a default filestream to write to
  {
    _errbuf = new std::filebuf();
    _errout = new std::ostream(_errbuf);
    _errbuf->open(logfile, std::ios_base::trunc|std::ios_base::out); //clear and open file for writing
  }
  else
    _errout = errout;
  
  oalFuncs=0;
  OPENALFNTABLE* functmp = new OPENALFNTABLE();

  //OpenAL initialization code
	const char *actualDeviceName;
  OPENALFNTABLE& ALFunction=*functmp;
  std::vector<ALDEVICEINFO> vDeviceInfo(5);
	size_t defaultDeviceIndex = 0;

	// grab function pointers for 1.0-API functions, and if successful proceed to enumerate all devices
	if (LoadOAL10Library(NULL, &ALFunction)!=0) {
		if (ALFunction.alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT")) {
			char* devices = (char *)ALFunction.alcGetString(NULL, ALC_DEVICE_SPECIFIER);
			const char* defaultDeviceName = (char *)ALFunction.alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
      TINYOAL_LOG("INFO") << "Default device name is: " << defaultDeviceName << std::endl;
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

	if(vDeviceInfo.size())
	{
    ALCdevice* pDevice = functmp->alcOpenDevice(vDeviceInfo[defaultDeviceIndex].strDeviceName.c_str());
		if (pDevice)
		{
			ALCcontext* pContext = functmp->alcCreateContext(pDevice, NULL);
			if (pContext)
			{
        TINYOAL_LOG("INFO") << "Opened Device: " << functmp->alcGetString(pDevice, ALC_DEVICE_SPECIFIER) << std::endl;
				functmp->alcMakeContextCurrent(pContext);
		    oalFuncs = functmp;
        functmp=0;
			}
			else
      {
				functmp->alcCloseDevice(pDevice);
        TINYOAL_LOG("ERROR") << "Failed to create context for " << functmp->alcGetString(pDevice, ALC_DEVICE_SPECIFIER) << std::endl;
      }
		}
    else
      TINYOAL_LOG("ERROR") << "Failed to open device: " << functmp->alcGetString(pDevice, ALC_DEVICE_SPECIFIER) << std::endl;
	}
  else
    TINYOAL_LOG("ERROR") << "No devices in device list!" << std::endl;
  
  if(functmp) delete functmp; // If functmp is nonzero, something blew up, so delete it

  waveFuncs = new cWaveFunctions();
  oggFuncs = new cOggFunctions(_errout);
  mp3Funcs = 0;
#ifdef __INCLUDE_MP3
  mp3funcs = new cMp3Functions(_errout);
#endif
}
void BSS_FASTCALL cTinyOAL::_addaudio(cAudio* ref, cAudioResource* res)
{
  if(!res->_activelist) // If true we need to move it
  {
    bss_util::LLRemove(res,_reslist);
    bss_util::LLAdd(res,_activereslist);
  }
  bss_util::LLRemove(ref,res->_inactivelist);
  bss_util::LLAdd(ref,res->_activelist,res->_activelistend);
  ++res->_numactive;
}
void BSS_FASTCALL cTinyOAL::_removeaudio(cAudio* ref, cAudioResource* res)
{
  bss_util::LLRemove(ref,res->_activelist,res->_activelistend);
  bss_util::LLAdd(ref,res->_inactivelist);
  --res->_numactive;
  if(!res->_activelist)
  {
    bss_util::LLRemove(res,_activereslist);
    bss_util::LLAdd(res,_reslist);
  }
}

char* BSS_FASTCALL cTinyOAL::_allocdecoder(unsigned int sz)
{
  auto p = _treealloc.GetRef(sz);
  if(!p)
  {
    TINYOAL_LOG("INFO") << "Created allocation pool of size " << sz << std::endl;
    _treealloc.Insert(sz,std::unique_ptr<bss_util::cFixedAllocVoid>(new cFixedAllocVoid(sz,3)));
    p = _treealloc.GetRef(sz);
  }
  assert(p);
  return (char*)(*p)->alloc(1);
}
void BSS_FASTCALL cTinyOAL::_deallocdecoder(char* s, unsigned int sz)
{
  auto p = _treealloc.GetRef(sz);
  if(p) (*p)->dealloc(s);
  else TINYOAL_LOG("WARNING") << "decoder buffer deallocation failure." << std::endl;
}