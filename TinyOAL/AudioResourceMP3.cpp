// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "AudioResourceMP3.h"
#include "WaveFunctions.h"
#include "TinyOAL.h"

using namespace tinyoal;

 bss::BlockAlloc<DatStream> AudioResourceMP3::_datalloc;

 AudioResourceMP3::AudioResourceMP3(void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop) : AudioResource(data, datalength, flags, TINYOAL_FILETYPE_MP3, loop)
{
  mpg123_handle* h = (mpg123_handle*)OpenStream();
  if(!h) return;
  int enc=0;
  long freq=_freq;
  int err=TinyOAL::Instance()->mp3Funcs->fn_mpgGetFormat(h, &freq, (int*)&_channels, &enc);
  if(!err)
    err=TinyOAL::Instance()->mp3Funcs->fn_mpgFormatNone(h); // Lock format so it doesn't change
  if(!err)
    err=TinyOAL::Instance()->mp3Funcs->fn_mpgFormat(h, freq,(int)_channels,enc);
  _freq=freq;
  if(err!=MPG123_OK)
    TINYOAL_LOG(1,"Failed to get format information from MP3");
  else
  {
    _samplebits=(enc==MPG123_ENC_SIGNED_16)?16:32;
    _format=TinyOAL::GetFormat(_channels,_samplebits,false);
    _bufsize = (_freq * _channels * (_samplebits>>3))>>2; // Sets buffer size to 250 ms, which is freq * bytes per sample / 4 (quarter of a second) 
    _bufsize -= (_bufsize % (_channels*(_samplebits>>3)));
    _total = TinyOAL::Instance()->mp3Funcs->fn_mpgLength(h); // OpenStream already called mpgScan so this value will be as accurate as we can get.
  }
  CloseStream(h);
}
AudioResourceMP3::~AudioResourceMP3()
{
  _destruct();
}
void* AudioResourceMP3::OpenStream()
{
  auto fn = TinyOAL::Instance()->mp3Funcs;
  if(!fn) return 0;
  int err;
  mpg123_handle* h = fn->fn_mpgNew(0,&err);
  if(!h || err!=MPG123_OK) {
    TINYOAL_LOG(1,"Failed to create new mpg instance");
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
    TINYOAL_LOG(1,"Failed to open mpg handle");
    fn->fn_mpgClose(h); 
    fn->fn_mpgDelete(h); 
    return 0; 
  }
  fn->fn_mpgScan(h);
  return h;
}
void AudioResourceMP3::CloseStream(void* stream)
{
  auto fn = TinyOAL::Instance()->mp3Funcs;
  mpg123_handle* h = (mpg123_handle*)stream;
  fn->fn_mpgClose(h);
  fn->fn_mpgDelete(h);
}
unsigned long AudioResourceMP3::_read(void* stream, char* buffer, unsigned int len, bool& eof)
{
  size_t done=0;
  unsigned int total=0;
  int err=0;
  while(total<len && !err)
  {
    err = TinyOAL::Instance()->mp3Funcs->fn_mpgRead((mpg123_handle*)stream,(unsigned char*)buffer+total,(size_t)len-total,&done);
    total+=done;
    if(err==MPG123_NEW_FORMAT)
    {
      long a;
      int b,c;
      TinyOAL::Instance()->mp3Funcs->fn_mpgGetFormat((mpg123_handle*)stream,&a,&b,&c); // older versions insist we call this, so we do, just in case.
      err=0;
    }
  }
  eof=(err==MPG123_DONE);
  if(err!=MPG123_DONE && err!=0)
    TINYOAL_LOG(1, "Error while decoding mp3");
  return done;
}
unsigned long AudioResourceMP3::Read(void* stream, char* buffer, unsigned int len, bool& eof)
{
  return _read(stream,buffer,len,eof);
}
bool AudioResourceMP3::Reset(void* stream)
{
  return Skip(stream,0);
}
bool AudioResourceMP3::Skip(void* stream, uint64_t samples)
{
  off_t err = TinyOAL::Instance()->mp3Funcs->fn_mpgSeek((mpg123_handle*)stream,samples,SEEK_SET);
  if(err>=0) return true;
  TINYOAL_LOG(2, "fn_mpgSeek failed with error code %i", (int)err);
  return false;
}
uint64_t AudioResourceMP3::Tell(void* stream)
{
  return TinyOAL::Instance()->mp3Funcs->fn_mpgTell((mpg123_handle*)stream);
}

off_t __mp3_foffset;
off_t __mp3_flength;
off_t AudioResourceMP3::cb_fileseekoffset(void* stream,off_t off,int loc)
{
  if(loc==SEEK_END) { loc=SEEK_SET; off+=__mp3_flength; }
  if(loc==SEEK_SET) off+=__mp3_foffset;
  if(!fseek((FILE*)stream,off,loc))
    return ftell((FILE*)stream);
  return -1;
}

size_t AudioResourceMP3::Construct(void* p, void* data, unsigned int datalength, TINYOAL_FLAG flags, uint64_t loop)
{
  if(p) new(p) AudioResourceMP3(data, datalength, flags, loop);
  return sizeof(AudioResourceMP3);
}
bool AudioResourceMP3::ScanHeader(const char* fileheader)
{
  return !strncmp(fileheader, "ID3", 3) || 0x3FF == (0x3FF & (*(unsigned short*)fileheader));
}

std::pair<void*,unsigned int> AudioResourceMP3::ToWave(void* data, unsigned int datalength, TINYOAL_FLAG flags)
{
  auto fn = TinyOAL::Instance()->mp3Funcs;
  int err;
  mpg123_handle* h;
  if(!fn || !(h = fn->fn_mpgNew(0,&err)) || err!=MPG123_OK) {
    TINYOAL_LOG(1,"Failed to create new mpg instance");
    return std::pair<void*,unsigned int>((void*)0,0);
  }

  DatStream dat;
  if(flags&TINYOAL_ISFILE)
  {
    //fseek((FILE*)data,0,SEEK_SET); // we don't do this in here because we could have gotten an external file pointer.
    __mp3_foffset=ftell((FILE*)data);
    __mp3_flength=datalength;
    fn->fn_mpgReplaceReader(h,&cb_fileread,&cb_fileseekoffset,0);
    err=fn->fn_mpgOpenHandle(h,data);
  }
  else
  {
    dat.data=dat.streampos=(char*)data;
    dat.datalength=datalength;
    fn->fn_mpgReplaceReader(h,&cb_datread,&cb_datseek,0); // Don't do cleanup because Datstream is on the stack
    err=fn->fn_mpgOpenHandle(h,&dat);
  }
  auto fnabort = [](mpg123_handle* h, const char* error) -> std::pair<void*,unsigned int> {
    TINYOAL_LOG(1,error);
    TinyOAL::Instance()->mp3Funcs->fn_mpgClose(h); 
    TinyOAL::Instance()->mp3Funcs->fn_mpgDelete(h); 
    return std::pair<void*,unsigned int>((void*)0,0); 
  };
  if(err!=MPG123_OK) return fnabort(h,"Failed to open mpg handle");
  
  err=fn->fn_mpgScan(h);
  off_t len = fn->fn_mpgLength(h);
  if(err!=MPG123_OK || len<0) return fnabort(h,"Failed to scan mp3");
  
  long freq;
  int channels,enc;
  err=fn->fn_mpgGetFormat(h, &freq, &channels, &enc);
  if(!err) err=fn->fn_mpgFormatNone(h);
  if(!err) err=fn->fn_mpgFormat(h, freq,channels,enc);
  if(err!=0) return fnabort(h,"Failed to get or set format");

  unsigned char bits=(enc==MPG123_ENC_SIGNED_16)?16:32;
  unsigned int total=len*(bits>>3)*channels;
  unsigned int header = TinyOAL::Instance()->waveFuncs->WriteHeader(0,0,0,0,0);
  char* buffer = (char*)malloc(total+header);
  bool eof;
  total=_read(h,buffer+header,total,eof);
  TinyOAL::Instance()->waveFuncs->WriteHeader(buffer,total+header,channels,bits,freq);

  fn->fn_mpgClose(h);
  fn->fn_mpgDelete(h);
  return std::pair<void*,unsigned int>(buffer,total+header);
}

void AudioResourceMP3::cb_cleanup(void* dat)
{
  _datalloc.dealloc(dat);
}
ssize_t AudioResourceMP3::cb_datread(void* stream,void* dst,size_t n)
{
  return dat_read_func(dst,1,n,stream);
}
off_t AudioResourceMP3::cb_datseek(void* stream,off_t off,int loc)
{
  if(!dat_seek_func(stream,off,loc))
    return dat_tell_func(stream);
  return -1;
}
ssize_t AudioResourceMP3::cb_fileread(void* stream,void* dst,size_t n)
{
  return fread(dst,1,n,(FILE*)stream);
}
off_t AudioResourceMP3::cb_fileseek(void* stream,off_t off,int loc)
{
  if(!fseek((FILE*)stream,off,loc))
    return ftell((FILE*)stream);
  return -1;
}