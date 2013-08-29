// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_DATA_STREAM_H__TOAL__
#define __C_DATA_STREAM_H__TOAL__

namespace TinyOAL {
  /* Mimics file operations on a stream of data stored in memory. It does NOT create an internal copy of the memory! The memory is treated like a file, so it must continue to exist. */
  class cDataStream
  {
  public:
    cDataStream(const void* data, size_t datalength);
    size_t read(void *ptr, size_t size, size_t nmemb);
    int seek(__int64 offset, int whence);
    int close();
    long tell();
    const void* begin();

  protected:
    const char* _data; //data is stored as the char* type for ease of operations
    size_t _datalength;
    const char* _streampos;
  };
}

#endif