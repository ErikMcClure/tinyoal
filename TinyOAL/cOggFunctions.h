// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#pragma once
#include "openAL/vorbisfile.h"
#include <ostream>
#include "TinyOAL_dlldef.h"

struct HINSTANCE__;

namespace TinyOAL {
  typedef int (*LPOVCLEAR)(OggVorbis_File *vf);
  typedef long (*LPOVREAD)(OggVorbis_File *vf,char *buffer,int length,int bigendianp,int word,int sgned,int *bitstream);
  typedef ogg_int64_t (*LPOVPCMTOTAL)(OggVorbis_File *vf,int i);
  typedef vorbis_info * (*LPOVINFO)(OggVorbis_File *vf,int link);
  typedef vorbis_comment * (*LPOVCOMMENT)(OggVorbis_File *vf,int link);
  typedef int (*LPOVOPENCALLBACKS)(void *datasource, OggVorbis_File *vf,char *initial, long ibytes, ov_callbacks callbacks);
  typedef int (*LPOVTIMESEEK)(OggVorbis_File *vf,double pos);
  typedef int (*LPOVTIMESEEKPAGE)(OggVorbis_File *vf,double pos);
  typedef int (*LPOVRAWSEEK)(OggVorbis_File *vf,ogg_int64_t pos);
  typedef int (*LPOVPCMSEEK)(OggVorbis_File *vf,ogg_int64_t pos);
  typedef ogg_int64_t (*LPOVRAWTELL)(OggVorbis_File *vf);
  typedef ogg_int64_t (*LPOVPCMTELL)(OggVorbis_File *vf);

	/* This is a holder class for the OGG DLL specific functions */
  class cOggFunctions
  {
  public:
    cOggFunctions(std::ostream* errout);
    
    LPOVCLEAR			fn_ov_clear;
    LPOVREAD			fn_ov_read;
    LPOVPCMTOTAL		fn_ov_pcm_total;
    LPOVINFO			fn_ov_info;
    LPOVCOMMENT			fn_ov_comment;
    LPOVOPENCALLBACKS	fn_ov_open_callbacks;
    LPOVTIMESEEK fn_ov_time_seek;
    LPOVTIMESEEKPAGE fn_ov_time_seek_page;
    LPOVRAWSEEK fn_ov_raw_seek;
    LPOVPCMSEEK fn_ov_pcm_seek;
    LPOVRAWTELL fn_ov_raw_tell;
    LPOVPCMTELL fn_ov_pcm_tell;

    static ogg_int64_t GetLoopStart(OggVorbis_File *vf);
    static ogg_int64_t GetCommentSection(OggVorbis_File *vf);
    //static TINYOAL_DLLEXPORT bool WriteLoopStartToFile(const wchar_t* file, OggVorbis_File *vf, ogg_int64_t sample);

  protected:
    HINSTANCE__* g_hVorbisFileDLL;
  };
}