// Copyright (c)2020 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "tinyoal/AudioResource.h"
#include "tinyoal/TinyOAL.h"

using namespace tinyoal;

AudioResource::AudioResource(void* data, unsigned int len, TINYOAL_FLAG flags, unsigned char filetype, uint64_t loop) :
  _data(data),
  _datalength(len),
  _flags(flags),
  _loop(loop),
  _freq(0),
  _channels(0),
  _format(0),
  _bufsize(0),
  _activelist(0),
  _activelistend(0),
  _numactive(0),
  _inactivelist(0),
  _maxactive(0),
  _total(0),
  _filetype(TINYOAL_FILETYPE(filetype))
{
  bss::LLAdd<AudioResource>(this, TinyOAL::Instance()->_reslist);
}

AudioResource::~AudioResource()
{
  assert(!_activelist); // Activelist HAS to be null here, otherwise the program will explode eventually because we've
                        // already lost the virtual functions.
  TinyOAL::Instance()->_audiohash.Remove(_hash);
  if(_flags & TINYOAL_ISFILE)
    fclose((FILE*)_data);
  else if(_flags & TINYOAL_COPYINTOMEMORY && _data != 0)
    free(_data);
  bss::LLRemove<AudioResource>(this, TinyOAL::Instance()->_reslist);
}

void AudioResource::_destruct()
{
  // This MUST be called BEFORE reaching ~AudioResource! Otherwise, the virtual functions will have been destructed and the
  // resulting calls from stopping the audio will try to call a pure function.
  if(_activelist)
  {
    assert(_refs > 0); // Ensure that we did not mess up our reference counting
    Grab(); // We must grab a reference to ourselves or the refcount will drop to zero and we'll delete ourselves twice.
    while(_activelist)
      _activelist->Stop();
    for(Audio* cur = _inactivelist; cur != 0; cur = cur->next)
      cur->Invalidate();
  }
}

void AudioResource::DestroyThis() { delete this; }
Audio* AudioResource::Play(TINYOAL_FLAG flags)
{
  Audio* r = _activelistend;
  if(!_maxactive || _numactive < _maxactive)
  {
    r = TinyOAL::Instance()->_allocaudio.allocate(1);
    flags |= TINYOAL_MANAGED;
  }
  else
  {
    flags = ((flags & (~TINYOAL_MANAGED)) | (r->GetFlags() & TINYOAL_MANAGED));
    r->~Audio();
  }
  new(r) Audio(this, flags);
  return r;
}

AudioResource* AudioResource::Create(const char* file, TINYOAL_FLAG flags, unsigned char filetype, uint64_t loop)
{
  FILE* f;
#ifdef BSS_PLATFORM_WIN32
  _wfopen_s(&f, bss::StrW(file).c_str(), L"rb");
#else
  FOPEN(f, file, "rb");
#endif
  if(!f)
    return 0;
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  AudioResource* r = _fcreate(f, len, flags, filetype, file, loop);
  if(flags & TINYOAL_COPYINTOMEMORY)
    fclose(f);
  return r;
}
AudioResource* AudioResource::Create(FILE* file, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype,
                                     uint64_t loop)
{
  return AudioResource::_fcreate(file, datalength, flags | TINYOAL_COPYINTOMEMORY, filetype, bss::StrF("%p", file), loop);
}

AudioResource* AudioResource::Create(const void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype,
                                     uint64_t loop)
{
  if(!data || datalength < 8) // bad file pointer
  {
    if(!data)
      TINYOAL_LOG(2, "NULL pointer passed in to AudioResource::Create()");
    else
      TINYOAL_LOG(2, "datalength equal to 0 in AudioResource::Create()");
    return 0;
  }

  if(!filetype)
    filetype = TinyOAL::Instance()->_getFiletype((const char*)data);

  if((flags & TINYOAL_FORCETOWAVE) == TINYOAL_FORCETOWAVE)
    return _force(const_cast<void*>(data), datalength, flags, filetype, bss::StrF("%p", data), loop);
  else if(flags & TINYOAL_COPYINTOMEMORY)
  {
    void* ndata = malloc(datalength);
    if(ndata)
      memcpy(ndata, data, datalength);
    data = ndata;
  } // We do a const cast here because data must be stored as void* in case it needs to be deleted. Otherwise, we don't
    // touch it.
  return _create(const_cast<void*>(data), datalength, flags, filetype, bss::StrF("%p", data), loop);
}

AudioResource* AudioResource::_force(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype,
                                     const char* path, uint64_t loop)
{
  TinyOAL::Codec* c = TinyOAL::Instance()->GetCodec(filetype);
  if(!c)
  {
    TINYOAL_LOG(2, "%p is using an unknown or unrecognized format, or may be corrupt.", data);
    return 0;
  }
  std::pair<void*, unsigned int> d = c->towave(data, datalength, flags);

  if(!d.first)
    return 0;
  return _create(d.first, d.second, flags & (~TINYOAL_ISFILE), TINYOAL_FILETYPE_WAV, path, loop);
}
AudioResource* AudioResource::_fcreate(FILE* file, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype,
                                       const char* path, uint64_t loop)
{
  if(!file || datalength < 8) // bad file pointer
  {
    if(!file)
      TINYOAL_LOG(2, "NULL pointer passed in to AudioResource::Create()");
    else
      TINYOAL_LOG(2, "datalength equal to 0 in AudioResource::Create()");
    return 0;
  }

  if(!filetype)
  {
    char fheader[8] = { 0 };
    fread(fheader, 1, 8, file);
    fseek(file, -8, SEEK_CUR); // reset file pointer (do NOT use set here or we'll lose the relative positioning
    filetype = TinyOAL::Instance()->_getFiletype(fheader);
  }

  if((flags & TINYOAL_FORCETOWAVE) == TINYOAL_FORCETOWAVE)
    return _force(file, datalength, flags | TINYOAL_ISFILE, filetype, path, loop);
  else if(flags & TINYOAL_COPYINTOMEMORY)
  {
    void* data = malloc(datalength);
    if(data)
      fread(data, 1, datalength, file);
    return _create(data, datalength, flags, filetype, path, loop);
  }

  return _create(file, datalength, flags | TINYOAL_ISFILE, filetype, path, loop);
}

AudioResource* AudioResource::_create(void* data, unsigned int datalength, TINYOAL_FLAG flags, unsigned char filetype,
                                      const char* path, uint64_t loop)
{
  const char* hash = (flags & TINYOAL_COPYINTOMEMORY) ? "" : path;
  AudioResource* r = TinyOAL::Instance()->_audiohash[hash];
  if(r != 0)
  {
    r->Grab();
    return r;
  }

  TinyOAL::Codec* c = TinyOAL::Instance()->GetCodec(filetype);
  if(!c)
  {
    TINYOAL_LOG(2, "%p is using an unknown or unrecognized format, or may be corrupt.", data);
    return 0; // Unknown format
  }
  TINYOAL_LOG(4, "Loading %p with codec ID %i", data, (int)filetype);
  size_t len = c->construct(0, 0, 0, 0, 0);
  r          = (AudioResource*)malloc(len);
  c->construct(r, data, datalength, flags, loop);

  if(hash[0])
    TinyOAL::Instance()->_audiohash.Insert((r->_hash = hash).c_str(), r);

  r->Grab(); // gotta grab the thing
  return r;
}

// 8 functions - Four for parsing pure void*, and four for reading files
size_t tinyoal::dat_read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
{
  DatStream* data = (DatStream*)datasource;
  size_t retval   = (data->datalength - dat_tell_func(datasource)) / size;
  retval          = nmemb > retval ? retval : nmemb; // This ensures we never read past the end but still conform to size restrictions
  memcpy(ptr, data->streampos, retval * size);
  data->streampos += retval * size; // increment stream pointer
  return retval;
}

int tinyoal::dat_seek_func(void* datasource, int64_t offset, int whence) // Who the hell names a parameter "whence"?!
{
  DatStream* data = (DatStream*)datasource;
  int64_t pos     = 0;

  switch(whence)
  {
  case SEEK_END:
    if((pos = data->datalength + offset) < 0 || pos > data->datalength)
      return -1; // fail
    data->streampos = data->data + pos;
    return 0;
  case SEEK_SET:
    if(offset < 0 || (unsigned int)offset > data->datalength)
      return -1;
    data->streampos = data->data + offset;
    return 0;
  default:
  case SEEK_CUR:
    if((pos = dat_tell_func(datasource) + offset) < 0 || pos > data->datalength)
      return -1;
    data->streampos += offset;
    return 0;
  }
}

int tinyoal::dat_close_func(void* datasource)
{
  return 0; // We manage file opening and closing.
}

long tinyoal::dat_tell_func(void* datasource)
{
  DatStream* data = (DatStream*)datasource;
  return data->streampos - data->data;
}

size_t tinyoal::file_read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
{
  return fread(ptr, size, nmemb, (FILE*)datasource);
}

int tinyoal::file_seek_func(void* datasource, int64_t offset, int whence)
{
  return fseek((FILE*)datasource, (long)offset, whence);
}

int tinyoal::file_close_func(void* datasource)
{
  return 0; // We manage file opening and closing.
}

long tinyoal::file_tell_func(void* datasource) { return ftell((FILE*)datasource); }