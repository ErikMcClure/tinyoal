/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "cWaveFunctions.h"
#include "bss_util/bss_deprecated.h"
#include <string.h> //STRNICMP

using namespace TinyOAL;

#pragma pack(push, 4)
struct WAVEFILEHEADER
{
	char RIFF[4];
	unsigned __int32	sz;
	char WAVE[4];
};

struct RIFFCHUNK
{
	char name[4];
	unsigned __int32	size;
};
#pragma pack(pop)

#define WAVE_FORMAT_PCM 1
#define  WAVE_FORMAT_EXTENSIBLE 0xFFFE

cWaveFunctions::cWaveFunctions() {}

cWaveFunctions::WAVERESULT cWaveFunctions::Open(void* source, WAVEFILEINFO* wave, wav_callbacks& callbacks)
{
  if(!wave || !source) return WR_INVALIDPARAM;
  memset(wave,0,sizeof(WAVEFILEINFO)-sizeof(DatStream)); // Don't zero the datstream because technically it's not part of our info struct.
  memcpy(&wave->callbacks,&callbacks,sizeof(wav_callbacks));
  wave->source=source;

  WAVEFILEHEADER header;
  RIFFCHUNK chunk;
  callbacks.read_func(&header,1,sizeof(WAVEFILEHEADER),source);
  if(STRNICMP(header.RIFF, "RIFF", 4)!=0 || STRNICMP(header.WAVE, "WAVE", 4)!=0) return WR_BADWAVEFILE;

	while(callbacks.read_func(&chunk, 1, sizeof(RIFFCHUNK), source) == sizeof(RIFFCHUNK))
  {
    if(!STRNICMP(chunk.name, "fmt ", 4) && chunk.size <= sizeof(WAVEFORMATEXTENSIBLE)) // This is the format chunk
      callbacks.read_func(&wave->wfEXT, 1, chunk.size, source);
    else if(!STRNICMP(chunk.name, "data", 4)) { // This is the data chunk
      wave->offset=callbacks.tell_func(source);
      wave->size=chunk.size;
      callbacks.seek_func(source,chunk.size,SEEK_CUR);
    } else // Otherwise it's an unknown chunk so just skip it
      callbacks.seek_func(source,chunk.size,SEEK_CUR);
	  
	  if(chunk.size&1) // Ensure we are aligned on an even byte boundary
		  callbacks.seek_func(source, 1, SEEK_CUR); // If we're on an odd byte, bump the pointer forward one byte.
  }

  if(!wave->offset || !wave->size || (wave->wfEXT.Format.wFormatTag!=WAVE_FORMAT_PCM && wave->wfEXT.Format.wFormatTag!=WAVE_FORMAT_EXTENSIBLE))
    return WR_BADWAVEFILE;
  return WR_OK;
}
cWaveFunctions::WAVERESULT cWaveFunctions::Read(WAVEFILEINFO& wave, void *data, size_t len, size_t*pBytesWritten)
{
  if(!data || !len || !pBytesWritten) return WR_INVALIDPARAM;
  
  unsigned long cur_offset = wave.callbacks.tell_func(wave.source);

  if ((cur_offset - wave.offset + len) > wave.size)
    len = wave.size - (cur_offset - wave.offset);
  *pBytesWritten = wave.callbacks.read_func(data, 1, len, wave.source);

	return WR_OK;
}
cWaveFunctions::WAVERESULT cWaveFunctions::Seek(WAVEFILEINFO& wave, __int64 offset)
{
  if(!wave.source) return WR_INVALIDPARAM;
  wave.callbacks.seek_func(wave.source, wave.offset + offset, SEEK_SET);
	return WR_OK;
}
cWaveFunctions::WAVERESULT cWaveFunctions::Close(WAVEFILEINFO& wave)
{
  wave.callbacks.close_func(wave.source);
  return WR_OK;
}
unsigned __int64 cWaveFunctions::Tell(WAVEFILEINFO& wave)
{
  return wave.callbacks.tell_func(wave.source) - wave.offset;
}

// WAVE file speaker masks (taken from the ksmedia.h windows file)
#define SPEAKER_FRONT_LEFT              0x1
#define SPEAKER_FRONT_RIGHT             0x2
#define SPEAKER_FRONT_CENTER            0x4
#define SPEAKER_LOW_FREQUENCY           0x8
#define SPEAKER_BACK_LEFT               0x10
#define SPEAKER_BACK_RIGHT              0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER    0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER   0x80
#define SPEAKER_BACK_CENTER             0x100
#define SPEAKER_SIDE_LEFT               0x200
#define SPEAKER_SIDE_RIGHT              0x400
#define SPEAKER_TOP_CENTER              0x800
#define SPEAKER_TOP_FRONT_LEFT          0x1000
#define SPEAKER_TOP_FRONT_CENTER        0x2000
#define SPEAKER_TOP_FRONT_RIGHT         0x4000
#define SPEAKER_TOP_BACK_LEFT           0x8000
#define SPEAKER_TOP_BACK_CENTER         0x10000
#define SPEAKER_TOP_BACK_RIGHT          0x20000

const char* cWaveFunctions::GetALFormat(WAVEFILEINFO& wave)
{
  if(wave.wfEXT.Format.wFormatTag == WAVE_FORMAT_PCM)
  {
	  if (wave.wfEXT.Format.nChannels == 1)
		  return wave.wfEXT.Format.wBitsPerSample == 16 ? "AL_FORMAT_MONO16" : "AL_FORMAT_MONO8";
	  else if (wave.wfEXT.Format.nChannels == 2)
		  return wave.wfEXT.Format.wBitsPerSample == 16 ? "AL_FORMAT_STEREO16" : "AL_FORMAT_STEREO8";
	  else if ((wave.wfEXT.Format.nChannels == 4) && (wave.wfEXT.Format.wBitsPerSample == 16))
		  return "AL_FORMAT_QUAD16";
  }
  else if(wave.wfEXT.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
  {
	  if ((wave.wfEXT.Format.nChannels == 1) && (wave.wfEXT.dwChannelMask == SPEAKER_FRONT_CENTER))
		  return wave.wfEXT.Format.wBitsPerSample == 16 ? "AL_FORMAT_MONO16" : "AL_FORMAT_MONO8";
	  else if ((wave.wfEXT.Format.nChannels == 2) && (wave.wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)))
		  return wave.wfEXT.Format.wBitsPerSample == 16 ? "AL_FORMAT_STEREO16" : "AL_FORMAT_STEREO8";
	  else if ((wave.wfEXT.Format.nChannels == 2) && (wave.wfEXT.Format.wBitsPerSample == 16) && (wave.wfEXT.dwChannelMask == (SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
		  return  "AL_FORMAT_REAR16";
	  else if ((wave.wfEXT.Format.nChannels == 4) && (wave.wfEXT.Format.wBitsPerSample == 16) && (wave.wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
		  return "AL_FORMAT_QUAD16";
	  else if ((wave.wfEXT.Format.nChannels == 6) && (wave.wfEXT.Format.wBitsPerSample == 16) && (wave.wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)))
		  return "AL_FORMAT_51CHN16";
	  else if ((wave.wfEXT.Format.nChannels == 7) && (wave.wfEXT.Format.wBitsPerSample == 16) && (wave.wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_BACK_CENTER)))
		  return "AL_FORMAT_61CHN16";
	  else if ((wave.wfEXT.Format.nChannels == 8) && (wave.wfEXT.Format.wBitsPerSample == 16) && (wave.wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT)))
		  return "AL_FORMAT_71CHN16";
  }
  return 0;
}