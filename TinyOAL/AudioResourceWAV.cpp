// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "AudioResourceWAV.h"
#include "tinyoal/TinyOAL.h"
#include "openAL/loadoal.h"

using namespace tinyoal;
bss::BlockAlloc<WAVEFILEINFO> AudioResourceWAV::_allocwav(3);

AudioResourceWAV::AudioResourceWAV(void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop) : AudioResource(data, datalength, flags, TINYOAL_FILETYPE_WAV, loop)
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

  if(TinyOAL::Instance()->waveFuncs->Open((_flags&TINYOAL_ISFILE)?_data:&_sentinel.stream, &_sentinel, callbacks)!=0)
    TINYOAL_LOG(1,"Could not find file");
	
  TinyOAL::Instance()->waveFuncs->Seek(_sentinel,0);

  _channels = _sentinel.wfEXT.Format.nChannels;
	_freq = _sentinel.wfEXT.Format.nSamplesPerSec;
  _samplebits = _sentinel.wfEXT.Format.wBitsPerSample;
  _total = 0;

  if(TinyOAL::Instance()->oalFuncs!=0)
    _format=TinyOAL::Instance()->waveFuncs->GetALFormat(_sentinel);

	// Queue 250ms of audio data
  _bufsize = (_samplebits==24)?(_sentinel.wfEXT.Format.nAvgBytesPerSec/3):(_sentinel.wfEXT.Format.nAvgBytesPerSec >> 2);
  unsigned short align=_sentinel.wfEXT.Format.nBlockAlign;
  if(_samplebits==24) align=((align/3)<<2);
  if(_sentinel.wfEXT.Format.nBlockAlign != 0) // Prevent a divide by zero
    _bufsize -= (_bufsize % _sentinel.wfEXT.Format.nBlockAlign); // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment
  if(_samplebits==24) _samplebits=32;
  if(_samplebits != 0 && _channels != 0) // Prevent a divide by zero
    _total = (_sentinel.size<<3)/(_samplebits*_channels); // This is a re-arranged version of size/(bits>>3)*_channels to prevent divide-by-zero errors

	if(!_format)
  {
    TINYOAL_LOG(1,"Failed to find format information, or unsupported format");
    memset(&_sentinel,0,sizeof(WAVEFILEINFO));
  }
}
AudioResourceWAV::~AudioResourceWAV()
{
  _destruct();
}
void* AudioResourceWAV::OpenStream()
{
  if(!_sentinel.source) return 0; // Indicates a failure on file load
  if(_flags&TINYOAL_ISFILE) TinyOAL::Instance()->waveFuncs->Seek(_sentinel,0);
  WAVEFILEINFO* r = _allocwav.alloc(1);
  memcpy(r,&_sentinel,sizeof(WAVEFILEINFO));
  r->source=(_flags&TINYOAL_ISFILE)?_data:&r->stream;
  return r;
}

void AudioResourceWAV::CloseStream(void* stream)
{
  WAVEFILEINFO* r = (WAVEFILEINFO*)stream;
  TinyOAL::Instance()->waveFuncs->Close(*r);
  _allocwav.dealloc(r);
}
unsigned long AudioResourceWAV::Read(void* stream, char* buffer, unsigned int len, bool& eof)
{
  size_t retval;
  WAVEFILEINFO* r = (WAVEFILEINFO*)stream;
  TinyOAL::Instance()->waveFuncs->Read(*r, buffer, len, &retval);
  eof=(retval!=len); // If we didn't read len bytes we must have hit the end of the file
  return retval;
}
bool AudioResourceWAV::Reset(void* stream)
{
  WAVEFILEINFO* r = (WAVEFILEINFO*)stream;
  return !TinyOAL::Instance()->waveFuncs->Seek(*r, 0);
}

bool AudioResourceWAV::Skip(void* stream, uint64_t samples)
{
  WAVEFILEINFO* r = (WAVEFILEINFO*)stream;
  unsigned short bits=r->wfEXT.Format.wBitsPerSample;
  return !TinyOAL::Instance()->waveFuncs->Seek(*r, samples*(bits>>3)*_channels);
}
uint64_t AudioResourceWAV::Tell(void* stream)
{
  WAVEFILEINFO* r = (WAVEFILEINFO*)stream;
  unsigned short bits=r->wfEXT.Format.wBitsPerSample;
  uint64_t pos = (bits>>3)*_channels;
  return !pos?0:TinyOAL::Instance()->waveFuncs->Tell(*r)/pos;
}

size_t AudioResourceWAV::Construct(void* p, void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop)
{
  if(p) new(p) AudioResourceWAV(data, datalength, flags, loop);
  return sizeof(AudioResourceWAV);
}
bool AudioResourceWAV::ScanHeader(const char* fileheader)
{
  return !strncmp(fileheader, "RIFF", 4) || !strncmp(fileheader, "RIFX", 4);
}
std::pair<void*, unsigned int> AudioResourceWAV::ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags)
{
  std::pair<void*, unsigned int> d = { malloc(datalength), datalength };
  if(flags&TINYOAL_ISFILE)
    fread(d.first, 1, datalength, (FILE*)data);
  else
    memcpy(d.first, data, datalength);
  return d;
}
