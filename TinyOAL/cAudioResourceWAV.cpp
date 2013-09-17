// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResourceWAV.h"
#include "cTinyOAL.h"
#include "openAL/loadoal.h"

using namespace TinyOAL;
bss_util::cFixedAlloc<WAVEFILEINFO> cAudioResourceWAV::_allocwav(3);

cAudioResourceWAV::cAudioResourceWAV(const cAudioResourceWAV& copy) : cAudioResource(copy) {}
cAudioResourceWAV::cAudioResourceWAV(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop) : cAudioResource(data, datalength, flags, loop)
{
  wav_callbacks callbacks;
  if(_flags&TINYOAL_ISFILE) {
    callbacks.read_func = file_read_func;
	  callbacks.seek_func = file_seek_func;
	  callbacks.close_func = file_close_func;
	  callbacks.tell_func = file_tell_func;
  } else {
	  callbacks.read_func = dat_read_func;
	  callbacks.seek_func = dat_seek_func;
	  callbacks.close_func = dat_close_func;
	  callbacks.tell_func = dat_tell_func;
    _sentinel.stream.data=_sentinel.stream.streampos=(const char*)_data;
    _sentinel.stream.datalength=_datalength;
  }

  if(cTinyOAL::Instance()->waveFuncs->Open((_flags&TINYOAL_ISFILE)?_data:&_sentinel.stream, &_sentinel, callbacks)!=0)
    TINYOAL_LOGM("ERROR","Could not find file");
	
  cTinyOAL::Instance()->waveFuncs->Seek(_sentinel,0);

  _channels=_sentinel.wfEXT.Format.nChannels;
	_freq=_sentinel.wfEXT.Format.nSamplesPerSec;
  _format=cTinyOAL::Instance()->oalFuncs->alGetEnumValue(cTinyOAL::Instance()->waveFuncs->GetALFormat(_sentinel));

	// Queue 250ms of audio data
  _bufsize = _sentinel.wfEXT.Format.nAvgBytesPerSec >> 2;
	_bufsize -= (_bufsize % _sentinel.wfEXT.Format.nBlockAlign); // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment

	if(!_format)
  {
    TINYOAL_LOGM("ERROR","Failed to find format information, or unsupported format");
    memset(&_sentinel,0,sizeof(WAVEFILEINFO));
  }
}
cAudioResourceWAV::~cAudioResourceWAV()
{
  _destruct();
}
void* cAudioResourceWAV::OpenStream()
{
  if(!_sentinel.source) return 0; // Indicates a failure on file load
  if(_flags&TINYOAL_ISFILE) cTinyOAL::Instance()->waveFuncs->Seek(_sentinel,0);
  WAVEFILEINFO* r = _allocwav.alloc(1);
  memcpy(r,&_sentinel,sizeof(WAVEFILEINFO));
  r->source=(_flags&TINYOAL_ISFILE)?_data:&r->stream;
  return r;
}

void cAudioResourceWAV::CloseStream(void* stream)
{
  WAVEFILEINFO* r = (WAVEFILEINFO*)stream;
  cTinyOAL::Instance()->waveFuncs->Close(*r);
  _allocwav.dealloc(r);
}
unsigned long cAudioResourceWAV::Read(void* stream, char* buffer)
{
  size_t retval;
  WAVEFILEINFO* r = (WAVEFILEINFO*)stream;
  cTinyOAL::Instance()->waveFuncs->Read(*r, buffer, _bufsize, &retval);
  return retval;
}
bool cAudioResourceWAV::Reset(void* stream)
{
  WAVEFILEINFO* r = (WAVEFILEINFO*)stream;
  return !cTinyOAL::Instance()->waveFuncs->Seek(*r, 0);
}

bool cAudioResourceWAV::Skip(void* stream, unsigned __int64 samples)
{
  WAVEFILEINFO* r = (WAVEFILEINFO*)stream;
  unsigned short bits=r->wfEXT.Format.wBitsPerSample;
  return !cTinyOAL::Instance()->waveFuncs->Seek(*r, samples*(bits>>3));
}