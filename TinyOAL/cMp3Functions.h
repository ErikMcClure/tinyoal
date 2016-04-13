// Copyright ©2016 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_MP3_FUNCTIONS_H__TOAL__
#define __C_MP3_FUNCTIONS_H__TOAL__

#include <stddef.h> // for ptrdiff_t
typedef ptrdiff_t ssize_t;
#include "mpg123/mpg123.h"

namespace tinyoal {
  typedef int (*LPMPGINIT)(void);
  typedef void (*LPMPGEXIT)(void);
  typedef mpg123_handle* (*LPMPGNEW)(const char*, int*);
  typedef void (*LPMPGDELETE)(mpg123_handle*);
  typedef const char* (*LPMPGSTRERROR)(mpg123_handle*);
  typedef int (*LPMPGFORMATNONE)(mpg123_handle*);
  typedef int (*LPMPGFORMAT)(mpg123_handle*, long, int, int);
  typedef int (*LPMPGGETFORMAT)(mpg123_handle*, long*, int*, int*);
  typedef int (*LPMPGOPENFD)(mpg123_handle*, int);
  typedef int (*LPMPGOPENHANDLE)(mpg123_handle*, void*);
  typedef int (*LPMPGCLOSE)(mpg123_handle*);
  typedef int (*LPMPGREAD)(mpg123_handle*, unsigned char*, size_t, size_t*);
  typedef off_t (*LPMPGTELL)(mpg123_handle*);
  typedef off_t (*LPMPGSEEK)(mpg123_handle*, off_t, int);
  typedef int (*LPMPGINFO)(mpg123_handle*, struct mpg123_frameinfo*);
  typedef int (*LPMPGSCAN)(mpg123_handle*);
  typedef off_t (*LPMPGLENGTH)(mpg123_handle*);
  typedef int (*LPMPGID3)(mpg123_handle*, mpg123_id3v1 **, mpg123_id3v2 **);
  typedef int (*LPMPGREPLACEREADER)(mpg123_handle*, ssize_t (*r_read) (void *, void *, size_t), off_t (*r_lseek)(void *, off_t, int), void (*cleanup)(void*));

	class cMp3Functions
	{
	public:
		cMp3Functions(const char* force);
    ~cMp3Functions();
    inline bool Failure() { return _mpgDLL==0; }

    LPMPGINIT fn_mpgInit;
    LPMPGEXIT fn_mpgExit;
    LPMPGNEW fn_mpgNew;
    LPMPGDELETE fn_mpgDelete;
    LPMPGSTRERROR fn_mpgStrError;
    LPMPGFORMATNONE fn_mpgFormatNone;
    LPMPGFORMAT fn_mpgFormat;
    LPMPGGETFORMAT fn_mpgGetFormat;
    LPMPGOPENFD fn_mpgOpenFD;
    LPMPGOPENHANDLE fn_mpgOpenHandle;
    LPMPGCLOSE fn_mpgClose;
    LPMPGREAD fn_mpgRead;
    LPMPGTELL fn_mpgTell;
    LPMPGSEEK fn_mpgSeek;
    LPMPGINFO fn_mpgInfo;
    LPMPGSCAN fn_mpgScan;
    LPMPGLENGTH fn_mpgLength;
    LPMPGID3 fn_mpgID3;
    LPMPGREPLACEREADER fn_mpgReplaceReader;

	protected:
		void* _mpgDLL;
	};
}

#endif