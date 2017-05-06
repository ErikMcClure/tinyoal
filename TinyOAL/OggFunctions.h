// Copyright ©2017 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __OGG_FUNCTIONS_H__TOAL__
#define __OGG_FUNCTIONS_H__TOAL__

#include "openAL/vorbisfile.h"

namespace tinyoal {
  typedef int (*LPOVCLEAR)(OggVorbis_File *vf);
  typedef long (*LPOVREAD)(OggVorbis_File *vf,char *buffer,int length,int bigendianp,int word,int sgned,int *bitstream);
  typedef vorbis_info * (*LPOVINFO)(OggVorbis_File *vf,int link);
  typedef int (*LPOVOPENCALLBACKS)(void *datasource, OggVorbis_File *vf,char *initial, long ibytes, ov_callbacks callbacks);
  typedef int (*LPOVTIMESEEK)(OggVorbis_File *vf,double pos);
  typedef int (*LPOVPCMSEEK)(OggVorbis_File *vf,ogg_int64_t pos);
  typedef ogg_int64_t (*LPOVPCMTELL)(OggVorbis_File *vf);
  typedef ogg_int64_t (*LPOVPCMTOTAL)(OggVorbis_File *vf,int i);
  //typedef vorbis_comment * (*LPOVCOMMENT)(OggVorbis_File *vf,int link);

	// This is a holder class for the OGG DLL specific functions 
  class OggFunctions
  {
  public:
    OggFunctions(const char* force);
    ~OggFunctions();
    inline bool Failure() { return _oggDLL==0; }

    LPOVCLEAR			fn_ov_clear;
    LPOVREAD			fn_ov_read;
    LPOVINFO			fn_ov_info;
    LPOVOPENCALLBACKS	fn_ov_open_callbacks;
    LPOVTIMESEEK fn_ov_time_seek;
    LPOVPCMSEEK fn_ov_pcm_seek;
    LPOVPCMTELL fn_ov_pcm_tell;
    LPOVPCMTOTAL fn_ov_pcm_total;
    //LPOVCOMMENT fn_ov_comment;

    static ogg_int64_t GetLoopStart(OggVorbis_File *vf);
    static ogg_int64_t GetCommentSection(OggVorbis_File *vf);
    //static TINYOAL_DLLEXPORT bool WriteLoopStartToFile(const wchar_t* file, OggVorbis_File *vf, ogg_int64_t sample);

  protected:
    void* _oggDLL;
  };
}

#endif