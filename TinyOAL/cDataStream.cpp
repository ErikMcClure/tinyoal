// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cDataStream.h"
#include <stdio.h>
#include <string.h>

using namespace TinyOAL;

cDataStream::cDataStream(const void* data, size_t datalength)
{
  _streampos = _data = (const char*)data;
  _datalength = datalength;
}

size_t cDataStream::read(void *ptr, size_t size, size_t nmemb)
{
  size_t retval = (_datalength-tell())/size;
  retval = nmemb>retval?retval*size:nmemb*size; //This ensures we never read past the end but still conform to size restrictions
  memcpy(ptr, _streampos, retval);
  _streampos += retval; //increment stream pointer
  return retval;
}

int cDataStream::seek(__int64 offset, int whence)
{
  __int64 pos=0;

  switch(whence)
  {
  default:
  case SEEK_CUR:
    if((pos = tell()+offset) < 0 || pos >= _datalength)
      return -1;
    _streampos += offset;
    return 0;
    break;
  case SEEK_END:
    if((pos = _datalength+offset) < 0 || pos > _datalength)
      return -1; //fail
    _streampos = _data+pos;
    return 0;
    break;
  case SEEK_SET:
    if(offset < 0 || (unsigned int)offset > _datalength)
      return -1;
    _streampos = _data+offset;
    return 0;
    break;
  }
}

int cDataStream::close()
{
  _streampos = _data;
  return 0;
}

long cDataStream::tell()
{
  return _streampos-_data;
}

const void* cDataStream::begin()
{
  return (const void*)_data;
}