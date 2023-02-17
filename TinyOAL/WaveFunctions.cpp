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

#include "WaveFunctions.h"
#include "buntils/sseVec.h"
#include <string.h> //STRNICMP
#include "tinyoal/TinyOAL.h"

using namespace tinyoal;
using namespace bun;

#pragma pack(push, 4)
struct WAVEFILEHEADER
{
  char RIFF[4];
  uint32_t sz;
  char WAVE[4];
};

struct RIFFCHUNK
{
  char name[4];
  uint32_t size;
};
#pragma pack(pop)

WaveFunctions::WaveFunctions() {}

WaveFunctions::WAVERESULT WaveFunctions::Open(void* source, WAVEFILEINFO* wave, wav_callbacks& callbacks)
{
  if(!wave || !source)
    return WR_INVALIDPARAM;
  memset(wave, 0,
         sizeof(WAVEFILEINFO) -
           sizeof(DatStream)); // Don't zero the datstream because technically it's not part of our info struct.
  memcpy(&wave->callbacks, &callbacks, sizeof(wav_callbacks));
  wave->source = source;

  WAVEFILEHEADER header;
  RIFFCHUNK chunk;
  callbacks.read_func(&header, 1, sizeof(WAVEFILEHEADER), source);
  if(STRNICMP(header.RIFF, "RIFF", 4) != 0 || STRNICMP(header.WAVE, "WAVE", 4) != 0)
    return WR_BADWAVEFILE;

  while(callbacks.read_func(&chunk, 1, sizeof(RIFFCHUNK), source) == sizeof(RIFFCHUNK))
  {
    if(!STRNICMP(chunk.name, "fmt ", 4) && chunk.size <= sizeof(WAVEFORMATEXTENSIBLE)) // This is the format chunk
      callbacks.read_func(&wave->wfEXT, 1, chunk.size, source);
    else if(!STRNICMP(chunk.name, "data", 4))
    { // This is the data chunk
      wave->offset = callbacks.tell_func(source);
      wave->size   = chunk.size;
      callbacks.seek_func(source, chunk.size, SEEK_CUR);
    }
    else // Otherwise it's an unknown chunk so just skip it
      callbacks.seek_func(source, chunk.size, SEEK_CUR);

    if(chunk.size & 1)                          // Ensure we are aligned on an even byte boundary
      callbacks.seek_func(source, 1, SEEK_CUR); // If we're on an odd byte, bump the pointer forward one byte.
  }

  if(!wave->offset ||
     !wave->size) // Don't worry about the wave file format. If it's illegal, we'll find out when we try to assign a format.
    return WR_BADWAVEFILE;
  return WR_OK;
}
WaveFunctions::WAVERESULT WaveFunctions::Read(WAVEFILEINFO& wave, void* data, size_t len, size_t* pBytesWritten)
{
  if(!data || !len || !pBytesWritten)
    return WR_INVALIDPARAM;

  unsigned long cur_offset = wave.callbacks.tell_func(wave.source);

  if(wave.wfEXT.Format.wBitsPerSample == 24)
    len = (len >> 2) * 3; // change len to the number of bytes we will actually read.

  if((cur_offset - wave.offset + len) > wave.size)
    len = wave.size - (cur_offset - wave.offset);
  *pBytesWritten = wave.callbacks.read_func(data, 1, len, wave.source);

  if(wave.wfEXT.Format.wBitsPerSample ==
     24) // We tell the user 24 bit streams are 32-bit and convert them behind the scenes. Ninja conversions.
  {
    float* dest = (float*)data;
    char* src   = (char*)data;
    len /= 3;                     // Now len is the number of individual channel samples
    for(size_t i = len; i-- > 0;) // We go backwards so we don't trip over ourselves
    { // This whole thing can be SSE optimized to operate on 4 at a time but it'd be a total bitch to do
      int a = *(int*)(src + (i * 3));
      a     = (a & 0x00FFFFFF) |
          (0xFF000000 * ((a & 0x800000) !=
                         0)); // Cut out top 8 bits and the set them all equal to 1 if it was negative or 0 if it wasn't
      dest[i] =
        (float)a /
        8388607.0f; // max signed integer value. This is exactly FLT_EPS, so this should preserve all 24-bits of precision.
                    // Whether or not the FPU is set to a high enough precision for this to actually happen is up in the air.
    }
    *pBytesWritten = (len << 2); // We actually wrote 4*number of samples, not what we put in here earlier, so fix it
  }

  if(wave.wfEXT.Format.wBitsPerSample == 32 &&
     wave.wfEXT.Format.wFormatTag != 3) // We can't read 32-bit integers, only floats, so convert.
  {                                     // If we're reading 32-bit ints, convert them to floats.
    size_t i    = 4;
    size_t sz   = (*pBytesWritten) / 4;
    int* src    = (int*)data; // Look at all this pointer aliasing!
    float* dest = (float*)data;
    for(; i < sz; i += 4) // SSE optimized conversions. We're losing about 8 bits of precision here, but we don't care
                          // because no one can hear past 20 bits anyway.
      BUN_SSE_STORE_UPS(dest + (i - 4), sseVec(sseVeci(BUN_UNALIGNED<const int>(src + (i - 4)))) / sseVec(2147483648.0f));
    for(i -= 4; i < sz; ++i)
    { // Traditional conversion for the last few
      dest[i] = (float)src[i] / 2147483648.0f;
    }
  }
  return WR_OK;
}
WaveFunctions::WAVERESULT WaveFunctions::Seek(WAVEFILEINFO& wave, int64_t offset)
{
  if(!wave.source)
    return WR_INVALIDPARAM;
  wave.callbacks.seek_func(wave.source, wave.offset + offset, SEEK_SET);
  return WR_OK;
}
WaveFunctions::WAVERESULT WaveFunctions::Close(WAVEFILEINFO& wave)
{
  wave.callbacks.close_func(wave.source);
  return WR_OK;
}
uint64_t WaveFunctions::Tell(WAVEFILEINFO& wave) { return wave.callbacks.tell_func(wave.source) - wave.offset; }

uint32_t WaveFunctions::WriteHeader(char* buffer, uint32_t length, uint16_t channels, uint16_t bits,
                                        uint32_t freq)
{
  static const int FULL_HEADER_SIZE =
    sizeof(WAVEFILEHEADER) + sizeof(RIFFCHUNK) + sizeof(WAVEFORMATEX) - sizeof(uint16_t) + sizeof(RIFFCHUNK);
  if(!buffer)
    return FULL_HEADER_SIZE;
  if(length < FULL_HEADER_SIZE)
    return 0;

  WAVEFILEHEADER& header = *(WAVEFILEHEADER*)buffer;
  memcpy(header.RIFF, "RIFF", 4);
  header.sz = length - 8; // don't include RIFF or sz itself in this count
  memcpy(header.WAVE, "WAVE", 4);

  buffer += sizeof(WAVEFILEHEADER);
  RIFFCHUNK& fmt = *(RIFFCHUNK*)buffer;
  memcpy(fmt.name, "fmt ", 4);
  fmt.size = sizeof(WAVEFORMATEX) - sizeof(uint16_t); // we aren't going to include cbsize

  buffer += sizeof(RIFFCHUNK);
  WAVEFORMATEX& format   = *(WAVEFORMATEX*)buffer;
  format.wFormatTag      = (bits == 32) ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
  format.nChannels       = channels;
  format.nSamplesPerSec  = freq;
  format.nAvgBytesPerSec = (freq * bits * channels) >> 3;
  format.nBlockAlign     = (bits * channels) >> 3;
  format.wBitsPerSample  = bits;

  buffer += fmt.size;
  RIFFCHUNK& data = *(RIFFCHUNK*)buffer;
  memcpy(data.name, "data", 4);
  data.size = length - FULL_HEADER_SIZE;
  return data.size;
}
