// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cTinyOAL.h"
#include <fstream>
#include "openAL\aldlist.h"
#include "openAL\al.h"
#include "openAL\alc.h"
#define WINVER 0x0502
#define _WIN32_WINNT 0x0502    
#define WIN32_LEAN_AND_MEAN
#include "openAL\CWaves.h"
#include "cAudioResource.h"
#include "cAudio.h"
#include "cOggFunctions.h"
#include "cMp3Functions.h"

using namespace TinyOAL;
using namespace bss_util;

// We manually define these to use windows functions because we don't want to import the whole bss_util library just for its fast convert functions.
extern size_t BSS_FASTCALL UTF8toUTF16(const char* input,wchar_t* output, size_t buflen)
{
  return (size_t)MultiByteToWideChar(CP_UTF8, 0, input, -1, output, !output?0:buflen);
}

extern size_t BSS_FASTCALL UTF16toUTF8(const wchar_t* input, char* output, size_t buflen)
{
  return (size_t)WideCharToMultiByte(CP_UTF8, 0, input, -1, output, !output?0:buflen, NULL, NULL);
}

#pragma warning(push)
#pragma warning(disable:4355)
cTinyOAL::cTinyOAL(char defaultbuffers, std::ostream* errout) : cSingleton<cTinyOAL>(this), _audiolist(0), _errbuf(0)
{
  if(!errout) //if errout is zero, produce a default filestream to write to
  {
    _errbuf = new std::filebuf();
    _errout = new std::ostream(_errbuf);
    _errbuf->open("TinyOAL_log.txt", std::ios_base::trunc+std::ios_base::out); //clear and open file for writing
  }
  else
    _errout = errout;

  _oggfuncs = new cOggFunctions(_errout);
#ifdef __INCLUDE_MP3
  _mp3funcs = new cMp3Functions(_errout);
#endif
  _construct(defaultbuffers);
}

cTinyOAL::cTinyOAL(const char* logfile, char defaultbuffers) : cSingleton<cTinyOAL>(this), _audiolist(0), _errbuf(new std::filebuf())
{
  _errout = new std::ostream(_errbuf);
  _errbuf->open(!logfile?L"TinyOAL_log.txt":cStrW(logfile), std::ios_base::trunc+std::ios_base::out); //clear and open file for writing

  _oggfuncs = new cOggFunctions(_errout);
#ifdef __INCLUDE_MP3
  _mp3funcs = new cMp3Functions(_errout);
#endif
  _construct(defaultbuffers);
}
#pragma warning(pop)

cTinyOAL::~cTinyOAL()
{
  /* DEBUG VERIFY */
#if defined(DEBUG) || defined(_DEBUG)
  cKhash_Pointer<char,false> hasher;
  cAudio* dcur = _audiolist;
  khiter_t iter;
  while(dcur) {
    iter = hasher.GetIterator(dcur);
    assert(!hasher.Exists(iter));
    hasher.Insert(dcur,0);
    dcur=dcur->next;
  }
#endif

  /* Destroy managed pointers */
  cAudio* cur = _audiolist;
  while(cur) {
    if(cur->GetFlags()&TINYOAL_MANAGED)
      delete cur;
    cur->prev=0;
    cur=cur->next;
  }

  cAudioResource::DeleteAll();

  if(_errbuf) //if this is not 0, it means we used it as a backup error output and we need to blow it up
  {
    _errbuf->close();
    delete _errbuf;
    delete _errout; //If we created _errbuf, we also created this, so blow it up
    _errbuf = 0; //just in case
    _errout = 0;
  } //otherwise _errout is just a pointer that we don't want to mess with

  if(_functions) //If _functions doesn't exist, we don't need to do (or are capable of doing) this
  {
	  ALCcontext* pContext = _functions->alcGetCurrentContext();
	  ALCdevice* pDevice = _functions->alcGetContextsDevice(pContext);
  	
	  _functions->alcMakeContextCurrent(NULL);
	  _functions->alcDestroyContext(pContext);
	  _functions->alcCloseDevice(pDevice);
  }

  if(_devicelist)
    delete _devicelist;
  if(_waveLoader)
    delete _waveLoader;
  if(_oggfuncs)
    delete _oggfuncs;
}

void cTinyOAL::_construct(int defaultbuffers)
{
  _defaultbuffers = defaultbuffers;
  _functions = 0;
  _waveLoader = 0;

  //create the wave loader
  _waveLoader = new CWaves();

  //OpenAL initialization code
  _devicelist = NULL;
	ALCcontext* pContext = NULL;
	ALCdevice* pDevice = NULL;

	_devicelist = new ALDeviceList();
	if ((_devicelist) && (_devicelist->GetNumDevices()))
	{
    pDevice = _devicelist->getFunctions()->alcOpenDevice(_devicelist->GetDeviceName(_devicelist->GetDefaultDevice()));
		if (pDevice)
		{
			pContext = _devicelist->getFunctions()->alcCreateContext(pDevice, NULL);
			if (pContext)
			{
        TINYOAL_LOG("INFO") << "Opened Device: " << _devicelist->getFunctions()->alcGetString(pDevice, ALC_DEVICE_SPECIFIER) << std::endl;
				_devicelist->getFunctions()->alcMakeContextCurrent(pContext);
		    _functions = _devicelist->getFunctions();
			}
			else
      {
				_devicelist->getFunctions()->alcCloseDevice(pDevice);
        TINYOAL_LOG("ERROR") << "Failed to create context for " << _devicelist->getFunctions()->alcGetString(pDevice, ALC_DEVICE_SPECIFIER) << std::endl;
      }
		}
    else
      TINYOAL_LOG("ERROR") << "Failed to open device: " << _devicelist->getFunctions()->alcGetString(pDevice, ALC_DEVICE_SPECIFIER) << std::endl;
	}
  else if(!_devicelist->GetNumDevices())
    TINYOAL_LOG("WARNING") << "No devices in device list!" << std::endl;
  else
    TINYOAL_LOG("ERROR") << "Failed to create device list" << std::endl;

}
bool cTinyOAL::_addaudio(cAudio* ref)
{ // We have to ensure ref is not in the list, _instance exists, and its not in the hash.
  if(_instance!=0 && !ref->prev && ref!=_instance->_audiolist && !(_instance->_audiohash.Exists(_instance->_audiohash.GetIterator(ref))))
  {
    LLAdd(ref,_instance->_audiolist);
    return _instance->_audiohash.Insert(ref,1);
  }
  return false;
}

bool cTinyOAL::_removeaudio(cAudio* ref)
{
  if(!ref->prev && ref!=_instance->_audiolist) return false; // You aren't in the list.
  if(_instance)
  {
    if(_instance->_audiohash.Remove(ref)) {
      LLRemove(ref,_instance->_audiolist);
      ref->prev=0;
      if(ref->_flags&TINYOAL_MANAGED)
        delete ref;
      return true;
    }
  }
  assert(false); // Should never get here or something is broken.
  return false;
}

unsigned int cTinyOAL::Update()
{
  cAudio* cur = _audiolist;
  cAudio* hold;
  while(cur) {
    hold=cur->next;
    cur->Update();
    cur=hold;
  }
  return _audiohash.Length();
}
std::ostream& BSS_FASTCALL cTinyOAL::FormatLog(const char* FILE, unsigned int LINE)
{
  (*_errout) << '(' << _trimpath(FILE) << ':' << LINE << ") ";
  return *_errout;
}
const char* BSS_FASTCALL cTinyOAL::_trimpath(const char* path)
{
	const char* r=strrchr(path,'/');
	const char* r2=strrchr(path,'\\');
  r=bssmax(r,r2);
  return (!r)?path:(r+1);
}

//void cTinyOAL::PermaLoad(const char* file)
//{
//  cAudioResource::CreateAudioReference(file, TINYOAL_LOADINTOMEMORY);
//}
//
//void cTinyOAL::PermaLoad(void* data, unsigned int datalength)
//{
//  cAudioResource::CreateAudioReference(data,datalength, TINYOAL_LOADINTOMEMORY);
//}
//
//void cTinyOAL::PermaLoad(_iobuf* file, unsigned int datalength)
//{
//  cAudioResource::CreateAudioReference(file,datalength, TINYOAL_LOADINTOMEMORY);
//}
//
//void cTinyOAL::UnPermaLoad(const char* file)
//{
//  cAudioResource* retval = cAudioResource::CreateAudioReference(file, TINYOAL_LOADINTOMEMORY);
//  retval->Drop(); //Negates our call to CreateAudioReference, and...
//  retval->Drop(); //actually drops the permanent reference
//}
//
//void cTinyOAL::UnPermaLoad(void* data, unsigned int datalength)
//{
//  cAudioResource* retval = cAudioResource::CreateAudioReference(data, datalength, TINYOAL_LOADINTOMEMORY);
//  retval->Drop(); //Negates our call to CreateAudioReference, and...
//  retval->Drop(); //actually drops the permanent reference
//}
//
//void cTinyOAL::UnPermaLoad(_iobuf* file, unsigned int datalength)
//{
//  cAudioResource* retval = cAudioResource::CreateAudioReference(file,datalength, TINYOAL_LOADINTOMEMORY);
//  retval->Drop(); //Negates our call to CreateAudioReference, and...
//  retval->Drop(); //actually drops the permanent reference
//}
//cAudio* cTinyOAL::ManagedLoad(const char* file, int flags)
//{
//  return new cAudio(file,flags|TINYOAL_MANAGED);
//}
//cAudio* cTinyOAL::ManagedLoad(void* data, unsigned int datalength, int flags)
//{
//  return new cAudio(data,datalength,flags|TINYOAL_MANAGED);
//}
//cAudio* cTinyOAL::ManagedLoad(_iobuf* file, unsigned int datalength, int flags)
//{
//  return new cAudio(file,datalength,flags|TINYOAL_MANAGED);
//}
cAudio* cTinyOAL::PlaySound(const cAudioRef& ref)
{
  return new cAudio(ref,TINYOAL_MANAGED);
}
cAudio* cTinyOAL::PlaySound(const char* file, unsigned char flags)
{
  return new cAudio(file,flags|TINYOAL_MANAGED);
}
cAudio* cTinyOAL::PlaySound(void* data, unsigned int datalength, unsigned char flags)
{
  return new cAudio(data,datalength,flags|TINYOAL_MANAGED);
}
cAudio* cTinyOAL::PlaySound(_iobuf* file, unsigned int datalength, unsigned char flags)
{
  return new cAudio(file,datalength,flags|TINYOAL_MANAGED);
}