// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cAudioResource.h"
#include "cTinyOAL.h"

using namespace tinyoal;
bss_util::cHash<const char*, cAudioResource*, true> cAudioResource::_audiohash;
bss_util::cBlockAlloc<cAudio> cAudioResource::_allocaudio(5);
bss_util::cHash<unsigned char, cAudioResource::Codec> cAudioResource::_codecs;

cAudioResource::cAudioResource(void* data, unsigned int len, TINYOAL_FLAG flags, unsigned char filetype, uint64_t loop) : _data(data), _datalength(len),
  _flags(flags), _loop(loop), _freq(0), _channels(0), _format(0), _bufsize(0), _activelist(0), _activelistend(0), _numactive(0), _inactivelist(0), 
  _maxactive(0), _total(0), _filetype(TINYOAL_FILETYPE(filetype))
{
  bss_util::LLAdd<cAudioResource>(this,cTinyOAL::Instance()->_reslist);
}

cAudioResource::~cAudioResource()
{
  assert(!_activelist); // Activelist HAS to be null here, otherwise the program will explode eventually because we've already lost the virtual functions.
  _audiohash.Remove(_hash);
  if(_flags&TINYOAL_ISFILE)
    fclose((FILE*)_data);
  else if(_flags&TINYOAL_COPYINTOMEMORY && _data!=0)
    free(_data);
  bss_util::LLRemove<cAudioResource>(this,cTinyOAL::Instance()->_reslist);
}

void cAudioResource::_destruct()
{
  // This MUST be called BEFORE reaching ~cAudioResource! Otherwise, the virtual functions will have been destructed and the resulting calls from stopping the audio will try to call a pure function.
  if(_activelist)
  {
    assert(_refs > 0); // Ensure that we did not mess up our reference counting
    Grab(); // We must grab a reference to ourselves or the refcount will drop to zero and we'll delete ourselves twice.
    while(_activelist) _activelist->Stop();
    for(cAudio* cur = _inactivelist; cur != 0; cur = cur->next) cur->Invalidate();
  }
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

//This function does NOT check to see if fileheader is 8 characters long
unsigned char BSS_FASTCALL cAudioResource::_getfiletype(const char* fileheader)
{ 
  auto iter = _codecs.begin();
  while(iter.IsValid())
  {
    if(_codecs.GetValue(*iter)->scanheader(fileheader))
      return _codecs.GetKey(*iter);
    ++iter;
  }

	return TINYOAL_FILETYPE_UNKNOWN;
}

cAudioResource* cAudioResource::Create(const char* file, TINYOAL_FLAG flags, unsigned char filetype, uint64_t loop)
{
  FILE* f;
#ifdef BSS_PLATFORM_WIN32
  _wfopen_s(&f,cStrW(file).c_str(),L"rb");
#else
  FOPEN(f,file,"rb");
#endif
  if(!f) return 0;
  fseek(f,0,SEEK_END);
  long len=ftell(f);
  fseek(f,0,SEEK_SET);
  cAudioResource* r = _fcreate(f, len, flags, filetype, file, loop);
  if(flags&TINYOAL_COPYINTOMEMORY) fclose(f);
  return r;
}
cAudioResource* cAudioResource::Create(FILE* file, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype, uint64_t loop)
{
  return cAudioResource::_fcreate(file, datalength, flags | TINYOAL_COPYINTOMEMORY, filetype, cStrF("%p", file), loop);
}

cAudioResource* cAudioResource::Create(const void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype, uint64_t loop)
{
  if(!data || datalength < 8) //bad file pointer
  {
    if(!data) TINYOAL_LOGM("WARNING", "NULL pointer passed in to cAudioResource::Create()");
    else TINYOAL_LOGM("WARNING", "datalength equal to 0 in cAudioResource::Create()");
    return 0;
  }

  if(!filetype)
    filetype = _getfiletype((const char*)data);
  
  if((flags&TINYOAL_FORCETOWAVE) == TINYOAL_FORCETOWAVE)
    return _force(const_cast<void*>(data), datalength, flags, filetype, cStrF("%p", data), loop);
  else if(flags&TINYOAL_COPYINTOMEMORY)
  {
    void* ndata = malloc(datalength);
    memcpy(ndata,data,datalength);
    data=ndata;
  } // We do a const cast here because data must be stored as void* in case it needs to be deleted. Otherwise, we don't touch it.
  return _create(const_cast<void*>(data), datalength, flags, filetype, cStrF("%p", data), loop);
}

cAudioResource* cAudioResource::_force(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype, const char* path, uint64_t loop)
{
  Codec* c = GetCodec(filetype);
  if(!c)
  {
    TINYOAL_LOG("WARNING") << data << " is using an unknown or unrecognized format, or may be corrupt." << std::endl;
    return 0;
  }
  std::pair<void*, unsigned int> d = c->towave(data, datalength, flags);

  if(!d.first) return 0;
  return _create(d.first, d.second, flags&(~TINYOAL_ISFILE), TINYOAL_FILETYPE_WAV, path, loop);
}
cAudioResource* cAudioResource::_fcreate(FILE* file, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype, const char* path, uint64_t loop)
{
  if(!file || datalength < 8) //bad file pointer
  {
    if(!file) TINYOAL_LOGM("WARNING", "NULL pointer passed in to cAudioResource::Create()");
    else TINYOAL_LOGM("WARNING", "datalength equal to 0 in cAudioResource::Create()");
    return 0;
  }

  if(!filetype)
  {
    char fheader[8]={0};
    fread(fheader, 1, 8,file);
    fseek(file, -8, SEEK_CUR); // reset file pointer (do NOT use set here or we'll lose the relative positioning
    filetype = _getfiletype(fheader);
  }

  if((flags&TINYOAL_FORCETOWAVE) == TINYOAL_FORCETOWAVE)
    return _force(file, datalength, flags | TINYOAL_ISFILE, filetype, path, loop);
  else if(flags&TINYOAL_COPYINTOMEMORY) {
    void* data = malloc(datalength);
    fread(data, 1, datalength, file);
    return _create(data, datalength, flags, filetype, path, loop);
  }

  return _create(file, datalength, flags | TINYOAL_ISFILE, filetype, path, loop);
}

cAudioResource* cAudioResource::_create(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype, const char* path, uint64_t loop)
{
  const char* hash=(flags&TINYOAL_COPYINTOMEMORY)?"":path;
  cAudioResource* r = _audiohash[hash];
  if(r!=0) { r->Grab(); return r; }

  Codec* c = GetCodec(filetype);
  if(!c)
  {
    TINYOAL_LOG("WARNING") << data << " is using an unknown or unrecognized format, or may be corrupt." << std::endl;
    return 0; //Unknown format
  }
  TINYOAL_LOG("INFO") << "Loading " << (void*)data << " with codec ID " << filetype << std::endl;
  size_t len = c->construct(0, 0, 0, 0, 0);
  r = (cAudioResource*)malloc(len);
  c->construct(r, data, datalength, flags, loop);

  if(hash[0]) _audiohash.Insert((r->_hash=hash).c_str(), r);
  r->Grab(); //gotta grab the thing
  return r;
}

void cAudioResource::RegisterCodec(unsigned char filetype, CODEC_CONSTRUCT construct, CODEC_SCANHEADER scanheader, CODEC_TOWAVE towave)
{
  Codec c = { construct, scanheader, towave };
  _codecs.Insert(filetype, c);
}

cAudioResource::Codec* cAudioResource::GetCodec(unsigned char filetype)
{
  return _codecs.Get(filetype);
}


//8 functions - Four for parsing pure void*, and four for reading files
size_t tinyoal::dat_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
  DatStream* data = (DatStream*)datasource;
  size_t retval = (data->datalength-dat_tell_func(datasource))/size;
  retval = nmemb>retval?retval:nmemb; //This ensures we never read past the end but still conform to size restrictions
  memcpy(ptr, data->streampos, retval*size);
  data->streampos += retval*size; //increment stream pointer
  return retval;
}

int tinyoal::dat_seek_func(void *datasource, int64_t offset, int whence) //Who the hell names a parameter "whence"?!
{
  DatStream* data = (DatStream*)datasource;
  int64_t pos=0;

  switch(whence)
  {
  case SEEK_END:
    if((pos = data->datalength+offset) < 0 || pos > data->datalength)
      return -1;//fail
    data->streampos = data->data+pos;
    return 0;
  case SEEK_SET:
    if(offset < 0 || (unsigned int)offset > data->datalength)
      return -1;
    data->streampos = data->data+offset;
    return 0;
  default:
  case SEEK_CUR:
    if((pos = dat_tell_func(datasource)+offset) < 0 || pos > data->datalength)
      return -1;
    data->streampos += offset;
    return 0;
  }
}

int tinyoal::dat_close_func(void *datasource)
{
  return 0; //We manage file opening and closing.
}

long tinyoal::dat_tell_func(void *datasource)
{
  DatStream* data = (DatStream*)datasource;
  return data->streampos-data->data;
}

size_t tinyoal::file_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE*)datasource);
}

int tinyoal::file_seek_func(void *datasource, int64_t offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}

int tinyoal::file_close_func(void *datasource)
{
  return 0; //We manage file opening and closing.
}

long tinyoal::file_tell_func(void *datasource)
{
	return ftell((FILE*)datasource);
}