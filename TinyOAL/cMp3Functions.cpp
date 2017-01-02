// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cMp3Functions.h"
#include "bss-util/bss_util.h"
#include "cTinyOAL.h"

#ifdef BSS_PLATFORM_WIN32
#include "bss-util/bss_win32_includes.h"

#ifdef BSS_CPU_x86
#define MP3_MODULE "libmpg123.dll"
#elif defined(BSS_CPU_x86_64)
#define MP3_MODULE "libmpg123_64.dll"
#endif

#define LOADDYNLIB(s) LoadLibraryA(s)
#define GETDYNFUNC(p,s) GetProcAddress((HMODULE)p, s)
#define FREEDYNLIB(p) FreeLibrary((HMODULE)p)
#else
#include <dlfcn.h>
#define MP3_MODULE "libmpg123.so.0"
#define LOADDYNLIB(s) dlopen(s, RTLD_LAZY)
#define GETDYNFUNC(p,s) dlsym(p,s)
#define FREEDYNLIB(p) dlclose(p)
#endif

#define DYNFUNC(v,t,n) v = (t)GETDYNFUNC(_mpgDLL, n); \
		if(!v) TINYOAL_LOGM("ERROR","Could not load " n)

using namespace tinyoal;

cMp3Functions::cMp3Functions(const char* force)
{
	if(!force)
    force=MP3_MODULE;

  memset(this,0,sizeof(cMp3Functions));
  _mpgDLL=LOADDYNLIB(force);

	if(_mpgDLL)
	{
    DYNFUNC(fn_mpgInit,LPMPGINIT,"mpg123_init");
    DYNFUNC(fn_mpgExit,LPMPGEXIT,"mpg123_exit");
    DYNFUNC(fn_mpgNew,LPMPGNEW,"mpg123_new");
    DYNFUNC(fn_mpgDelete,LPMPGDELETE,"mpg123_delete");
    DYNFUNC(fn_mpgStrError,LPMPGSTRERROR,"mpg123_strerror");
    DYNFUNC(fn_mpgFormatNone,LPMPGFORMATNONE,"mpg123_format_none");
    DYNFUNC(fn_mpgFormat,LPMPGFORMAT,"mpg123_format");
    DYNFUNC(fn_mpgGetFormat,LPMPGGETFORMAT,"mpg123_getformat");
    DYNFUNC(fn_mpgOpenFD,LPMPGOPENFD,"mpg123_open_fd");
    DYNFUNC(fn_mpgOpenHandle,LPMPGOPENHANDLE,"mpg123_open_handle");
    DYNFUNC(fn_mpgClose,LPMPGCLOSE,"mpg123_close");
    DYNFUNC(fn_mpgRead,LPMPGREAD,"mpg123_read");
    DYNFUNC(fn_mpgTell,LPMPGTELL,"mpg123_tell");
    DYNFUNC(fn_mpgSeek,LPMPGSEEK,"mpg123_seek");
    DYNFUNC(fn_mpgInfo,LPMPGINFO,"mpg123_info");
    DYNFUNC(fn_mpgScan,LPMPGSCAN,"mpg123_scan");
    DYNFUNC(fn_mpgLength,LPMPGLENGTH,"mpg123_length");
    DYNFUNC(fn_mpgID3,LPMPGID3,"mpg123_id3");
    DYNFUNC(fn_mpgReplaceReader,LPMPGREPLACEREADER,"mpg123_replace_reader_handle");
    
    if(!fn_mpgInit || fn_mpgInit()!=MPG123_OK) {
      TINYOAL_LOGM("ERROR","Failed to initialize mpg123");
      if(fn_mpgExit!=0) fn_mpgExit();
      FREEDYNLIB(_mpgDLL);
      memset(this,0,sizeof(cMp3Functions));
    }
  }
  else
    TINYOAL_LOGM("ERROR","Could not find the mpg123 DLL (or it may be missing one of its dependencies)");
}

cMp3Functions::~cMp3Functions()
{
  if(fn_mpgExit!=0) fn_mpgExit();
  if(_mpgDLL) FREEDYNLIB(_mpgDLL);
}