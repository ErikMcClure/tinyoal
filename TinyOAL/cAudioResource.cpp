// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResourceOGG.h"
#include "cAudioResourceWAV.h"
#include "cAudioResourceMP3.h"
#include "cTinyOAL.h"
#include <stdio.h>

using namespace TinyOAL;
bss_util::cKhash_StringIns<cAudioResource*> cAudioResource::_audiohash;
bss_util::cFixedAlloc<cAudio> cAudioResource::_allocaudio(5);

cAudioResource::cAudioResource(const cAudioResource& copy) { assert(false); }
cAudioResource::cAudioResource(void* data, unsigned int len, TINYOAL_FLAG flags, unsigned __int64 loop) : _data(data), _datalength(len),
  _flags(flags), _loop(loop), _freq(0), _channels(0), _format(0), _bufsize(0), _activelist(0), _activelistend(0), _numactive(0), 
  _inactivelist(0), _maxactive(0)
{
  bss_util::LLAdd<cAudioResource>(this,cTinyOAL::Instance()->_reslist);
}

cAudioResource::~cAudioResource()
{
  assert(!_activelist);
  _audiohash.Remove(_hash);
  if(_flags&TINYOAL_ISFILE)
    fclose((FILE*)_data);
  else if(_flags&TINYOAL_COPYINTOMEMORY && _data!=0)
    free(_data);
  bss_util::LLRemove<cAudioResource>(this,cTinyOAL::Instance()->_reslist);
}

void cAudioResource::DestroyThis() { delete this; }
cAudio* cAudioResource::Play(TINYOAL_FLAG flags)
{ 
  cAudio* r = _activelistend;
  if(!_maxactive || _numactive<_maxactive) {
    r = _allocaudio.alloc(1);
    flags|=TINYOAL_MANAGED;
  } else {
    flags=((flags&(~TINYOAL_MANAGED))|(r->GetFlags()&TINYOAL_MANAGED));
    r->~cAudio();
  }
  new(r) cAudio(this,flags);
  return r;
}

void cAudioResource::_destruct()
{
  while(_activelist) _activelist->Stop();
  for(cAudio* cur=_inactivelist; cur!=0; cur=cur->next) cur->Invalidate();
}
//This function does NOT check to see if fileheader is 4 characters long
unsigned char BSS_FASTCALL cAudioResource::_getfiletype(const char* fileheader)
{ 
	if(!strncmp(fileheader, "OggS", 4)) return TINYOAL_FILETYPE_OGG;
	if(!strncmp(fileheader, "fLaC", 4)) return TINYOAL_FILETYPE_FLAC; // We don't support FLAC yet
  if(!strncmp(fileheader, "RIFF", 4) || !strncmp(fileheader, "RIFX", 4)) return TINYOAL_FILETYPE_WAV;
  if(!strncmp(fileheader,"ID3",3) || 0x7FF==(0x7FF&(*(unsigned short*)fileheader))) return TINYOAL_FILETYPE_MP3;

	return TINYOAL_FILETYPE_UNKNOWN;
}

cAudioResource* cAudioResource::Create(const char* file, TINYOAL_FLAG flags, unsigned __int64 loop)
{
  FILE* f;
  FOPEN(f,file,"rb");
  fseek(f,0,SEEK_END);
  long len=ftell(f);
  fseek(f,0,SEEK_SET);
  cAudioResource* r = _fcreate(f,len,flags,file,loop);
  if(flags&TINYOAL_COPYINTOMEMORY) fclose(f);
  return r;
}
cAudioResource* cAudioResource::Create(_iobuf* file, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop)
{
  return cAudioResource::_fcreate(file,datalength,flags|TINYOAL_COPYINTOMEMORY,cStrF("%p",file),loop);
}

cAudioResource* cAudioResource::Create(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned __int64 loop)
{
  if(!data || datalength < 4) //bad file pointer
  {
    if(!data) TINYOAL_LOGM("WARNING", "NULL pointer passed in to cAudioResource::Create()");
    else TINYOAL_LOGM("WARNING", "datalength equal to 0 in cAudioResource::Create()");
    return 0;
  }

  if(!(flags&TINYOAL_FILETYPEMASK))
    flags|=_getfiletype((const char*)data);
  if(flags&TINYOAL_COPYINTOMEMORY)
  {
    void* ndata = malloc(datalength);
    memcpy(ndata,data,datalength);
    data=ndata;
  }
  return _create(data,datalength,flags,cStrF("%p",data),loop);
}

cAudioResource* cAudioResource::_fcreate(_iobuf* file, unsigned int datalength, TINYOAL_FLAG flags, const char* path, unsigned __int64 loop)
{
  if(!file || datalength < 4) //bad file pointer
  {
    if(!file) TINYOAL_LOGM("WARNING", "NULL pointer passed in to cAudioResource::Create()");
    else TINYOAL_LOGM("WARNING", "datalength equal to 0 in cAudioResource::Create()");
    return 0;
  }

  if(!(flags&TINYOAL_FILETYPEMASK))
  {
    char fheader[4]={0};
    fread(fheader, 1, 4,file);
    fseek(file, -4, SEEK_CUR); // reset file pointer (do NOT use set here or we'll lose the relative positioning
    flags|=_getfiletype(fheader);
  }
  if(flags&TINYOAL_COPYINTOMEMORY)
  {
    void* data = malloc(datalength);
    fread(data,1,datalength,file);
    return _create(data,datalength,flags,path,loop);
  }
  return _create(file,datalength,flags|TINYOAL_ISFILE,path,loop);
}

cAudioResource* cAudioResource::_create(void* data, unsigned int datalength, TINYOAL_FLAG flags, const char* path, unsigned __int64 loop)
{
  if(!cTinyOAL::oalFuncs) return 0;

  const char* hash=(flags&TINYOAL_COPYINTOMEMORY)?"":path;
  cAudioResource* r = _audiohash[hash];
  if(r!=0) { r->Grab(); return r; }

	switch(flags&TINYOAL_FILETYPEMASK)
	{
	case TINYOAL_FILETYPE_OGG:
    TINYOAL_LOG("INFO") << "Loading " << (void*)data << " as OGG" << std::endl;
    r = new cAudioResourceOGG(data, datalength, flags, loop);
    break;
	case TINYOAL_FILETYPE_WAV:
    TINYOAL_LOG("INFO") << "Loading " << (void*)data << " as WAVE" << std::endl;
    r = new cAudioResourceWAV(data, datalength, flags, loop); //Assume its a wav
    break;
#ifdef __INCLUDE_MP3
	case TINYOAL_FILETYPE_MP3:
    TINYOAL_LOG("INFO") << "Loading " << (void*)data << " as MP3" << std::endl;
    r = new cAudioResourceMP3(data, datalength, flags, loop);
    break;
#endif
  default:
    TINYOAL_LOG("WARNING") << data << " is using an unknown or unrecognized format, or may be corrupt." << std::endl;
    return 0; //Unknown format
	}
  
  if(hash[0]) _audiohash.Insert((r->_hash=hash).c_str(), r);
  r->Grab(); //gotta grab the thing
  return r;
}


//8 functions - Four for parsing pure void*, and four for reading files
size_t TinyOAL::dat_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
  DatStream* data = (DatStream*)datasource;
  size_t retval = (data->datalength-dat_tell_func(datasource))/size;
  retval = nmemb>retval?retval*size:nmemb*size; //This ensures we never read past the end but still conform to size restrictions
  memcpy(ptr, data->streampos, retval);
  data->streampos += retval; //increment stream pointer
  return retval;
}

int TinyOAL::dat_seek_func(void *datasource, __int64 offset, int whence) //Who the hell names a parameter "whence"?!
{
  DatStream* data = (DatStream*)datasource;
  __int64 pos=0;

  switch(whence)
  {
  case SEEK_END:
    if((pos = data->datalength+offset) < 0 || pos > data->datalength)
      return -1; //fail
    data->streampos = data->data+pos;
    return 0;
  case SEEK_SET:
    if(offset < 0 || (unsigned int)offset > data->datalength)
      return -1;
    data->streampos = data->data+offset;
    return 0;
  default:
  case SEEK_CUR:
    if(((pos = dat_tell_func(datasource))+offset) < 0 || pos >= data->datalength)
      return -1;
    data->streampos += offset;
    return 0;
  }
}

int TinyOAL::dat_close_func(void *datasource)
{
  return 0; //We manage file opening and closing.
}

long TinyOAL::dat_tell_func(void *datasource)
{
  DatStream* data = (DatStream*)datasource;
  return data->streampos-data->data;
}

size_t TinyOAL::file_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE*)datasource);
}

int TinyOAL::file_seek_func(void *datasource, __int64 offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}

int TinyOAL::file_close_func(void *datasource)
{
  return 0; //We manage file opening and closing.
}

long TinyOAL::file_tell_func(void *datasource)
{
	return ftell((FILE*)datasource);
}