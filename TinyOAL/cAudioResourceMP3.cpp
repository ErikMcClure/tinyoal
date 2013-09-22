// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResourceMP3.h"
#include "cWaveFunctions.h"
#include "cTinyOAL.h"

using namespace TinyOAL;

 bss_util::cFixedAlloc<DatStream> cAudioResourceMP3::_datalloc;

cAudioResourceMP3::cAudioResourceMP3(const cAudioResourceMP3& copy) : cAudioResource(copy) {}
cAudioResourceMP3::cAudioResourceMP3(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop) : cAudioResource(data,datalength,flags,loop)
{
  mpg123_handle* h = (mpg123_handle*)OpenStream();
  if(!h) return;
  int enc=0;
  long freq=_freq;
  int err=cTinyOAL::Instance()->mp3Funcs->fn_mpgGetFormat(h, &freq, (int*)&_channels, &enc);
  if(!err)
    err=cTinyOAL::Instance()->mp3Funcs->fn_mpgFormatNone(h); // Lock format so it doesn't change
  if(!err)
    err=cTinyOAL::Instance()->mp3Funcs->fn_mpgFormat(h, freq,(int)_channels,enc);
  _freq=freq;
  if(err!=MPG123_OK)
    TINYOAL_LOGM("ERROR","Failed to get format information from MP3");
  else
  {
    _samplebits=(enc==MPG123_ENC_SIGNED_16)?16:32;
    _format=cTinyOAL::GetFormat(_channels,_samplebits,false);
    _bufsize = (_freq * _channels * (_samplebits>>3))>>2; // Sets buffer size to 250 ms, which is freq * bytes per sample / 4 (quarter of a second) 
    _bufsize -= (_bufsize % (_channels*(_samplebits>>3)));
  }
  CloseStream(h);
}
cAudioResourceMP3::~cAudioResourceMP3()
{
  _destruct();
}
void* cAudioResourceMP3::OpenStream()
{
  auto fn = cTinyOAL::Instance()->mp3Funcs;
  if(!fn) return 0;
  int err;
  mpg123_handle* h = fn->fn_mpgNew(0,&err);
  if(!h || err!=MPG123_OK) {
    TINYOAL_LOGM("ERROR","Failed to create new mpg instance");
    return 0;
  }

  if(_flags&TINYOAL_ISFILE)
  {
    fseek((FILE*)_data,0,SEEK_SET); // If we're a file, reset the pointer
    fn->fn_mpgReplaceReader(h,&cb_fileread,&cb_fileseek,0);
    err=fn->fn_mpgOpenHandle(h,_data);
  }
  else
  {
    DatStream* dat=_datalloc.alloc(1);
    dat->data=dat->streampos=(char*)_data;
    dat->datalength=_datalength;
    fn->fn_mpgReplaceReader(h,&cb_datread,&cb_datseek,&cb_cleanup);
    err=fn->fn_mpgOpenHandle(h,dat);
  }
  if(err!=MPG123_OK) { 
    TINYOAL_LOGM("ERROR","Failed to open mpg handle");
    fn->fn_mpgClose(h); 
    fn->fn_mpgDelete(h); 
    return 0; 
  }
  fn->fn_mpgScan(h);
  return h;
}
void cAudioResourceMP3::CloseStream(void* stream)
{
  auto fn = cTinyOAL::Instance()->mp3Funcs;
  mpg123_handle* h = (mpg123_handle*)stream;
  fn->fn_mpgClose(h);
  fn->fn_mpgDelete(h);
}
unsigned long cAudioResourceMP3::_read(void* stream, char* buffer, unsigned int len, bool& eof)
{
  size_t done=0;
  unsigned int total=0;
  int err=0;
  while(total<len && !err)
  {
    err = cTinyOAL::Instance()->mp3Funcs->fn_mpgRead((mpg123_handle*)stream,(unsigned char*)buffer+total,(size_t)len-total,&done);
    total+=done;
    if(err==MPG123_NEW_FORMAT)
    {
      long a;
      int b,c;
      cTinyOAL::Instance()->mp3Funcs->fn_mpgGetFormat((mpg123_handle*)stream,&a,&b,&c); // older versions insist we call this, so we do, just in case.
      err=0;
    }
  }
  eof=(err==MPG123_DONE);
  if(err!=MPG123_DONE && err!=0)
    TINYOAL_LOGM("Warning", "Error while decoding mp3");
  return done;
}
unsigned long cAudioResourceMP3::Read(void* stream, char* buffer, unsigned int len, bool& eof)
{
  return _read(stream,buffer,len,eof);
}
bool cAudioResourceMP3::Reset(void* stream)
{
  return Skip(stream,0);
}
bool cAudioResourceMP3::Skip(void* stream, unsigned __int64 samples)
{
  off_t err = cTinyOAL::Instance()->mp3Funcs->fn_mpgSeek((mpg123_handle*)stream,samples,SEEK_SET);
  if(err>=0) return true;
  TINYOAL_LOG("WARNING") << "fn_mpgSeek failed with error code " << err << std::endl;
  return false;
}
unsigned __int64 cAudioResourceMP3::Tell(void* stream)
{
  return cTinyOAL::Instance()->mp3Funcs->fn_mpgTell((mpg123_handle*)stream);
}
std::pair<void*,unsigned int> cAudioResourceMP3::ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags)
{
  auto fn = cTinyOAL::Instance()->mp3Funcs;
  int err;
  mpg123_handle* h;
  if(!fn || !(h = fn->fn_mpgNew(0,&err)) || err!=MPG123_OK) {
    TINYOAL_LOGM("ERROR","Failed to create new mpg instance");
    return std::pair<void*,unsigned int>((void*)0,0);
  }

  DatStream dat;
  if(flags&TINYOAL_ISFILE)
  {
    fseek((FILE*)data,0,SEEK_SET); // If we're a file, reset the pointer
    fn->fn_mpgReplaceReader(h,&cb_fileread,&cb_fileseek,0);
    err=fn->fn_mpgOpenHandle(h,data);
  }
  else
  {
    dat.data=dat.streampos=(char*)data;
    dat.datalength=datalength;
    fn->fn_mpgReplaceReader(h,&cb_datread,&cb_datseek,0); // Don't do cleanup because Datstream is on the stack
    err=fn->fn_mpgOpenHandle(h,&dat);
  }
  if(err!=MPG123_OK) { 
    TINYOAL_LOGM("ERROR","Failed to open mpg handle");
    fn->fn_mpgClose(h); 
    fn->fn_mpgDelete(h); 
    return std::pair<void*,unsigned int>((void*)0,0); 
  }
  
  err=fn->fn_mpgScan(h);
  off_t len = fn->fn_mpgLength(h);
  if(err!=MPG123_OK || len<0) { TINYOAL_LOGM("ERROR","Failed to scan mp3"); fn->fn_mpgClose(h); fn->fn_mpgDelete(h); return std::pair<void*,unsigned int>((void*)0,0); }
  
  long freq;
  int channels,enc;
  err=fn->fn_mpgGetFormat(h, &freq, &channels, &enc);
  if(!err) err=fn->fn_mpgFormatNone(h);
  if(!err) err=fn->fn_mpgFormat(h, freq,channels,enc);
  if(err!=0) { TINYOAL_LOGM("WARNING","Failed to get or set format");  fn->fn_mpgClose(h); fn->fn_mpgDelete(h); return std::pair<void*,unsigned int>((void*)0,0); }

  unsigned char bits=(enc==MPG123_ENC_SIGNED_16)?16:32;
  unsigned int total=len*(bits>>3)*channels;
  unsigned int header = cTinyOAL::Instance()->waveFuncs->WriteHeader(0,0,0,0,0);
  char* buffer = (char*)malloc(total+header);
  bool eof;
  total=_read(h,buffer+header,total,eof);
  cTinyOAL::Instance()->waveFuncs->WriteHeader(buffer,total+header,channels,bits,freq);

  fn->fn_mpgClose(h);
  fn->fn_mpgDelete(h);
  return std::pair<void*,unsigned int>(buffer,total+header);
}

void cAudioResourceMP3::cb_cleanup(void* dat)
{
  _datalloc.dealloc(dat);
}
ssize_t cAudioResourceMP3::cb_datread(void* stream,void* dst,size_t n)
{
  return dat_read_func(dst,1,n,stream);
}
off_t cAudioResourceMP3::cb_datseek(void* stream,off_t off,int loc)
{
  if(!dat_seek_func(stream,off,loc))
    return dat_tell_func(stream);
  dat_seek_func(stream,off,loc);
  return -1;
}
ssize_t cAudioResourceMP3::cb_fileread(void* stream,void* dst,size_t n)
{
  return fread(dst,1,n,(FILE*)stream);
}
off_t cAudioResourceMP3::cb_fileseek(void* stream,off_t off,int loc)
{
  if(!fseek((FILE*)stream,off,loc))
    return ftell((FILE*)stream);
  return -1;
}