// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cOggFunctions.h"
#include "bss-util/bss_util.h"
#include "bss-util/cStr.h"
#include "cTinyOAL.h"
#include <ostream>

#ifdef BSS_PLATFORM_WIN32
#include "bss-util/bss_win32_includes.h"

#define OGG_MODULE_ALT "vorbisfile.dll"
#define OGG_MODULE32 "libvorbisfile.dll"

#ifdef BSS_CPU_x86
#define OGG_MODULE OGG_MODULE32
#elif defined(BSS_CPU_x86_64)
#define OGG_MODULE "libvorbisfile64.dll"
#endif

#define LOADDYNLIB(s) LoadLibraryA(s)
#define GETDYNFUNC(p,s) GetProcAddress((HMODULE)p, s)
#define FREEDYNLIB(p) FreeLibrary((HMODULE)p)
#else
#include <dlfcn.h>
#define OGG_MODULE "libvorbisfile.so.3"
#define LOADDYNLIB(s) dlopen(s, RTLD_LAZY)
#define GETDYNFUNC(p,s) dlsym(p,s)
#define FREEDYNLIB(p) dlclose(p)
#endif

using namespace tinyoal;

cOggFunctions::cOggFunctions(const char* force)
{
  if(!force)
    force=OGG_MODULE;
  memset(this,0,sizeof(cOggFunctions));
  _oggDLL = LOADDYNLIB(force);
  if(!_oggDLL) _oggDLL = LOADDYNLIB(OGG_MODULE32);
  if(!_oggDLL) _oggDLL = LOADDYNLIB(OGG_MODULE_ALT);

	if (_oggDLL)
	{
		fn_ov_clear = (LPOVCLEAR)GETDYNFUNC(_oggDLL, "ov_clear");
		fn_ov_read = (LPOVREAD)GETDYNFUNC(_oggDLL, "ov_read");
		fn_ov_info = (LPOVINFO)GETDYNFUNC(_oggDLL, "ov_info");
		fn_ov_open_callbacks = (LPOVOPENCALLBACKS)GETDYNFUNC(_oggDLL, "ov_open_callbacks");
		fn_ov_time_seek = (LPOVTIMESEEK)GETDYNFUNC(_oggDLL, "ov_time_seek");
		fn_ov_pcm_seek = (LPOVPCMSEEK)GETDYNFUNC(_oggDLL, "ov_pcm_seek");
		fn_ov_pcm_tell = (LPOVPCMTELL)GETDYNFUNC(_oggDLL, "ov_pcm_tell");
		fn_ov_pcm_total = (LPOVPCMTOTAL)GETDYNFUNC(_oggDLL, "ov_pcm_total");
		//fn_ov_comment = (LPOVCOMMENT)GETDYNFUNC(_oggDLL, "ov_comment");

		if(!fn_ov_clear) TINYOAL_LOG(1,"Could not load ov_clear");
		if(!fn_ov_read) TINYOAL_LOG(1,"Could not load ov_read");
		if(!fn_ov_info) TINYOAL_LOG(1,"Could not load ov_info");
		if(!fn_ov_open_callbacks) TINYOAL_LOG(1,"Could not load ov_open_callbacks");
		if(!fn_ov_time_seek) TINYOAL_LOG(1,"Could not load ov_time_seek");
		if(!fn_ov_pcm_seek) TINYOAL_LOG(1,"Could not load ov_pcm_seek");
		if(!fn_ov_pcm_tell) TINYOAL_LOG(1,"Could not load ov_pcm_tell");
		if(!fn_ov_pcm_total) TINYOAL_LOG(1,"Could not load ov_pcm_total");
		//if(!fn_ov_comment) TINYOAL_LOG(1,"Could not load ov_comment");
	}
  else
    TINYOAL_LOG(1,"Could not find the OGG Vorbis DLL (or it may be missing one of its dependencies)");
}

cOggFunctions::~cOggFunctions()
{
  if(_oggDLL) FREEDYNLIB(_oggDLL);
}
ogg_int64_t cOggFunctions::GetCommentSection(OggVorbis_File *vf)
{
  const size_t BUFSIZE=128;
  const size_t READSIZE=BUFSIZE-5;
  char buf[BUFSIZE];
  memset(buf,0,BUFSIZE);
  int numhits=0;
  char* pos;  
  ptrdiff_t index=0;
  ptrdiff_t num;
  while(numhits<2)
  {
    if((num=vf->callbacks.read_func(buf+5,1,READSIZE,vf->datasource))!=READSIZE) return 0; //this means we hit the end of the file and it must not be valid
    if((pos=(char*)bss_util::bytesearch(buf,BUFSIZE,(void*)"vorbis",6))!=0) {
      ++numhits;
      num=pos-buf;
      if(numhits<2 && num>6 && (pos=(char*)bss_util::bytesearch(pos+=6,BUFSIZE,(void*)"vorbis",6))!=0) {
        ++numhits;
      }
    }
    index+=READSIZE;
    memcpy(buf,buf+READSIZE,5); //We have to save the last 5 characters in case the buffer ends in "vorbi" or we'll miss it
  }
  index-=READSIZE;
  index+=(pos-buf) + 6 - 5; //+6 to skip past "vorbis", -5 to remove 5 lead bytes on buffer
  int32_t length;
  vf->callbacks.seek_func(vf->datasource,index,SEEK_SET); //seek to the proper location
  if(vf->callbacks.read_func(&length,4,1,vf->datasource)!=1) return 0;
  return index+=length+4;
}


ogg_int64_t cOggFunctions::GetLoopStart(OggVorbis_File *vf)
{
  if(vf->seekable==0) return -1; //if not seekable, fail
  long orig = vf->callbacks.tell_func(vf->datasource);
  vf->callbacks.seek_func(vf->datasource,0,SEEK_SET);

  ogg_int64_t index=GetCommentSection(vf);
  int32_t length;
  ogg_int64_t retval=-1;

  vf->callbacks.seek_func(vf->datasource,index,SEEK_SET); //seek to the end of the vendor info

  int32_t numcomments;
  if(vf->callbacks.read_func(&numcomments,4,1,vf->datasource)==1) //get number of comments
  {
    cStr comment;
    for(int32_t i=0; i < numcomments; ++i)
    {
      if(vf->callbacks.read_func(&length,4,1,vf->datasource)!=1) break;
      comment.resize(length+1);
      if(vf->callbacks.read_func(comment.UnsafeString(),1,length,vf->datasource)!=length) break;
      comment.UnsafeString()[length]='\0';
      comment.RecalcSize();
      const char* pos=(const char*)memchr(comment.c_str(),'=',length);
      if(pos!=0)
      {
        if(!STRNICMP(comment.c_str(),"LOOPSTART",9))
        {
          retval=atol(++pos);
          break;
        }
      }
    }
  }

  vf->callbacks.seek_func(vf->datasource,orig,SEEK_SET); //seek back to where we were
  return retval;
}
//
//bool cOggFunctions::WriteLoopStartToFile(const wchar_t* file, OggVorbis_File *vf, ogg_int64_t sample)
//{
//  if(vf->seekable==0) return 0; //if not seekable, fail
//  vf->callbacks.seek_func(vf->datasource,0,SEEK_END);
//  long length = vf->callbacks.tell_func(vf->datasource);
//  cStr sf(length+32); //9 for LOOPSTART, 1 for =, 4 for comment length, then some buffer for the samples
//  memset(sf.UnsafeString(),0,length+32);
//
//  vf->callbacks.seek_func(vf->datasource,0,SEEK_SET);
//  int32_t index=GetCommentSection(vf);
//  if(!index) return false;
//
//  vf->callbacks.seek_func(vf->datasource,0,SEEK_SET);
//  vf->callbacks.read_func(sf.UnsafeString(),1,index,vf->datasource);
//  
//  int32_t numcomments;
//  if(vf->callbacks.read_func(&numcomments,4,1,vf->datasource)!=1) return false; //get number of comments
//
//  char* cp = sf.UnsafeString()+index;
//  *((int32_t*)cp) = ++numcomments;
//  cp+=sizeof(int32_t);
//
//  *((int32_t*)cp) = sprintf(cp+sizeof(int32_t),"%s%il","LOOPSTART=",sample); //Write out incremented numcomments and our new comment
//  cp+=sizeof(int32_t)+*((int32_t*)cp);
//  cp+=vf->callbacks.read_func(cp,1,length-index,vf->datasource);
//
//  FILE* f=0;
//  WFOPEN(f,file,L"wb");
//  if(!f) return false;
//  fwrite(sf.String(),1,cp-sf.String(),f);
//  fclose(f);
//
//  return true;
//}