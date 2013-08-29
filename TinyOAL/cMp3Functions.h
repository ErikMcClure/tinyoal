// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h
// Notice: This header file does not need to be included in binary distributions of the library

#ifndef __C_MP3_FUNCTIONS_H__TOAL__
#define __C_MP3_FUNCTIONS_H__TOAL__

#ifdef __INCLUDE_MP3
#include <ostream>
#include "mpg123.h"

struct HINSTANCE__;

namespace TinyOAL {
  typedef int (__cdecl *LPLDDECODEINIT)(void);
  typedef int (__cdecl *LPLDDECODE)(unsigned char* mp3buf, int len, short pcm_l[], short pcm_r[]);
  typedef int (__cdecl *LPLDDECODEHEADERS)(unsigned char* mp3buf, int len, short pcm_l[], short pcm_r[], mp3data_struct* mp3data);
  typedef int (__cdecl *LPLDDECODEONE)(unsigned char* mp3buf, int len, short pcm_l[], short pcm_r[]);
  typedef int (__cdecl *LPLDDECODEONEHEADERS)(unsigned char* mp3buf, int len, short pcm_l[], short pcm_r[], mp3data_struct* mp3data);
  typedef int (__cdecl *LPLDDECODEONEHEADERSB)(unsigned char* mp3buf, int len, short pcm_l[], short pcm_r[], mp3data_struct* mp3data, int* enc_delay, int* enc_padding);
  typedef int (__cdecl *LPLDDECODEEXIT)(void);
  typedef unsigned int (__cdecl *LPLDGETID3V2TAG)(lame_global_flags * gfp, unsigned char* buffer, size_t size);
  typedef lame_global_flags* (__cdecl *LPLDLAMEINIT)(void);
  typedef int (__cdecl *LPLDSETDECODEONLY)(lame_global_flags*, int);
  typedef int (__cdecl *LPLDLAMEEXIT)(lame_global_flags*);

//lame_global_flags * CDECL lame_init(void);
//int  CDECL lame_close (lame_global_flags *);

  // This class holds the lame_enc functions.
	class cMp3Functions
	{
	public:
		cMp3Functions(std::ostream* errout);
    ~cMp3Functions();

    LPLDDECODEINIT		fn_lameInitDecoder;
    LPLDDECODE		fn_lameDecode;
    LPLDDECODEHEADERS		fn_lameDecodeHeaders;
    LPLDDECODEONE		fn_lameDecode1;
    LPLDDECODEONEHEADERS			fn_lameDecode1Headers;
    LPLDDECODEONEHEADERSB	fn_lameDecode1HeadersB;
    LPLDDECODEEXIT	fn_lameExitDecoder;
    LPLDGETID3V2TAG fn_lameGetId3v2Tag;

	protected:
    void _invalidate();
		HINSTANCE__* g_ldMp3DLL;
	};
}

#endif

#endif