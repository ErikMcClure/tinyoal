// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#include "cOggFunctions.h"
#include "bss_util/bss_win32_includes.h"
#include "openAL\vorbisfile.h"
#include "bss_util\bss_util.h"
#include "bss_util\cStr.h"
#include "bss_util\bss_deprecated.h"
#include "TinyOAL.h"
#include <ostream>

using namespace TinyOAL;

cOggFunctions::cOggFunctions(std::ostream* errout)
{
  g_hVorbisFileDLL = LoadLibraryW(L"vorbisfile.dll");
	if (g_hVorbisFileDLL)
	{
		fn_ov_clear = (LPOVCLEAR)GetProcAddress(g_hVorbisFileDLL, "ov_clear");
		fn_ov_read = (LPOVREAD)GetProcAddress(g_hVorbisFileDLL, "ov_read");
		fn_ov_pcm_total = (LPOVPCMTOTAL)GetProcAddress(g_hVorbisFileDLL, "ov_pcm_total");
		fn_ov_info = (LPOVINFO)GetProcAddress(g_hVorbisFileDLL, "ov_info");
		fn_ov_comment = (LPOVCOMMENT)GetProcAddress(g_hVorbisFileDLL, "ov_comment");
		fn_ov_open_callbacks = (LPOVOPENCALLBACKS)GetProcAddress(g_hVorbisFileDLL, "ov_open_callbacks");
		fn_ov_time_seek = (LPOVTIMESEEK)GetProcAddress(g_hVorbisFileDLL, "ov_time_seek");
		fn_ov_time_seek_page = (LPOVTIMESEEKPAGE)GetProcAddress(g_hVorbisFileDLL, "ov_time_seek_page");
		fn_ov_raw_seek = (LPOVRAWSEEK)GetProcAddress(g_hVorbisFileDLL, "ov_raw_seek");
		fn_ov_pcm_seek = (LPOVPCMSEEK)GetProcAddress(g_hVorbisFileDLL, "ov_pcm_seek");
		fn_ov_raw_tell = (LPOVRAWTELL)GetProcAddress(g_hVorbisFileDLL, "ov_raw_tell");
		fn_ov_pcm_tell = (LPOVPCMTELL)GetProcAddress(g_hVorbisFileDLL, "ov_pcm_tell");

		if(!(fn_ov_clear && fn_ov_read && fn_ov_pcm_total && fn_ov_info && fn_ov_comment && fn_ov_open_callbacks && fn_ov_time_seek
      && fn_ov_time_seek_page && fn_ov_raw_seek && fn_ov_pcm_seek && fn_ov_raw_tell && fn_ov_pcm_tell))
      TINYOAL_LOGM("ERROR","Could not load all nessacary OGG vorbis callbacks.");
	}
  else
  {
		fn_ov_clear = 0;
		fn_ov_read = 0;
		fn_ov_pcm_total = 0;
		fn_ov_info = 0;
		fn_ov_comment = 0;
		fn_ov_open_callbacks = 0;
		fn_ov_time_seek = 0;
		fn_ov_time_seek_page = 0;
		fn_ov_raw_seek = 0;
		fn_ov_pcm_seek = 0;
		fn_ov_raw_tell = 0;
		fn_ov_pcm_tell = 0;
    TINYOAL_LOGM("ERROR","Could not find vorbisfile.dll (or it may be missing one of its dependencies)");
  }
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
  __int32 length;
  vf->callbacks.seek_func(vf->datasource,index,SEEK_SET); //seek to the proper location
  if(vf->callbacks.read_func(&length,4,1,vf->datasource)!=1) return 0;
  return index+=length+4;
}


ogg_int64_t cOggFunctions::GetLoopStart(OggVorbis_File *vf)
{
  if(vf->seekable==0) return 0; //if not seekable, fail
  long orig = vf->callbacks.tell_func(vf->datasource);
  vf->callbacks.seek_func(vf->datasource,0,SEEK_SET);

  ogg_int64_t index=GetCommentSection(vf);
  __int32 length;
  ogg_int64_t retval=0;

  vf->callbacks.seek_func(vf->datasource,index,SEEK_SET); //seek to the end of the vendor info

  __int32 numcomments;
  if(vf->callbacks.read_func(&numcomments,4,1,vf->datasource)==1) //get number of comments
  {
    cStr comment;
    for(__int32 i=0; i < numcomments; ++i)
    {
      if(vf->callbacks.read_func(&length,4,1,vf->datasource)!=1) break;
      comment.resize(length+1);
      if(vf->callbacks.read_func(comment.UnsafeString(),1,length,vf->datasource)!=length) break;
      comment.UnsafeString()[length]='\0';
      comment.RecalcSize();
      const char* pos=(const char*)memchr(comment.c_str(),'=',length);
      if(pos!=0)
      {
        if(!_strnicmp(comment.c_str(),"LOOPSTART",9))
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
//  __int32 index=GetCommentSection(vf);
//  if(!index) return false;
//
//  vf->callbacks.seek_func(vf->datasource,0,SEEK_SET);
//  vf->callbacks.read_func(sf.UnsafeString(),1,index,vf->datasource);
//  
//  __int32 numcomments;
//  if(vf->callbacks.read_func(&numcomments,4,1,vf->datasource)!=1) return false; //get number of comments
//
//  char* cp = sf.UnsafeString()+index;
//  *((__int32*)cp) = ++numcomments;
//  cp+=sizeof(__int32);
//
//  *((__int32*)cp) = sprintf(cp+sizeof(__int32),"%s%il","LOOPSTART=",sample); //Write out incremented numcomments and our new comment
//  cp+=sizeof(__int32)+*((__int32*)cp);
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