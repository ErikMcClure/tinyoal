// Copyright ©2013 Erik McClure
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifdef __INCLUDE_MP3
#include "cMp3Functions.h"
#include "bss_util/bss_win32_includes.h"

using namespace TinyOAL;

cMp3Functions::cMp3Functions(std::ostream* errout)
{
	if(!(g_ldMp3DLL = LoadLibraryW(L"lame_enc.dll")))
    g_ldMp3DLL = LoadLibraryW(L"libmp3lame.dll");

	if(g_ldMp3DLL)
	{
	  fn_lameInitDecoder=(LPLDDECODEINIT) GetProcAddress(g_ldMp3DLL, "lame_decode_init");
	  fn_lameDecode=(LPLDDECODE) GetProcAddress(g_ldMp3DLL, "lame_decode");
	  fn_lameDecodeHeaders=(LPLDDECODEHEADERS) GetProcAddress(g_ldMp3DLL, "lame_decode_headers");
	  fn_lameDecode1=(LPLDDECODEONE) GetProcAddress(g_ldMp3DLL, "lame_decode1");
	  fn_lameDecode1Headers=(LPLDDECODEONEHEADERS) GetProcAddress(g_ldMp3DLL, "lame_decode1_headers");
	  fn_lameDecode1HeadersB=(LPLDDECODEONEHEADERSB) GetProcAddress(g_ldMp3DLL,"lame_decode1_headersB");
	  fn_lameExitDecoder=(LPLDDECODEEXIT) GetProcAddress(g_ldMp3DLL,"lame_decode_exit");
	  fn_lameGetId3v2Tag=(LPLDGETID3V2TAG) GetProcAddress(g_ldMp3DLL,"lame_get_id3v2_tag");

		if(!(fn_lameInitDecoder && fn_lameDecode && fn_lameDecodeHeaders && fn_lameDecode1 && fn_lameDecode1Headers && fn_lameDecode1HeadersB))
    {
      _invalidate();
      (*errout) << "Error: Could not load all nessacary LAME MP3 callbacks.";
    }
    else if(fn_lameInitDecoder()!=0) //Initialize the encoder
    {
      _invalidate();
      (*errout) << "Error: Could not initalize LAME MP3 decoder.";
    }
  }
  else
  {
    _invalidate();
    (*errout) << "Error: Could not find lame_enc.dll (or it may be missing one of its dependencies)";
  }
}

cMp3Functions::~cMp3Functions()
{
	if(g_ldMp3DLL!=0 && fn_lameExitDecoder!=0)
    fn_lameExitDecoder(); //de-initialize the LAME MP3 Decoder
}

void cMp3Functions::_invalidate()
{
  fn_lameInitDecoder=0;
  fn_lameDecode=0;
  fn_lameDecodeHeaders=0;
  fn_lameDecode1=0;
  fn_lameDecode1Headers=0;
  fn_lameDecode1HeadersB=0;
  fn_lameExitDecoder=0;   
}
#endif