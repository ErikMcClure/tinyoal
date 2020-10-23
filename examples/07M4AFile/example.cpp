/* Example 07 - M4A file
 * -------------------------
 * This example demonstrates a custom windows-specific
 * AAC decoder that is registered to read M4A files.
 *
 * Copyright (c)2020 Erik McClure
 */

#include "tinyoal/TinyOAL.h"
#include "AudioResourceM4A.h"

using namespace tinyoal;

#ifdef BSS_PLATFORM_WIN32
#define SLEEP(n) _sleep(n)
#else
#include <unistd.h>
#define SLEEP(n) usleep(n*1000) //translate milliseconds to microseconds
#endif


int main()
{
  TinyOAL::SetSettingsStream(0); // Done in case testbed failed and left a settings file in.
  TinyOAL engine;

  // The RegisterCodec can be used to register new codecs at runtime. Codecs must implement the AudioResource interface and provide
  // three static functions that construct the object, scan a file header for a signature, and convert to in-memory wave.
  AudioResource::RegisterCodec(AudioResourceM4A::TINYOAL_FILETYPE_M4A, AudioResourceM4A::Construct, AudioResourceM4A::ScanHeader, AudioResourceM4A::ToWave);

  // TinyOAL supports streaming FLAC directly from a file, but FLAC's codec is really bad. It will loop,
  // just not seamlessly. It's highly recommended you use TINYOAL_FORCETOWAVE unless the FLAC is huge.
  Audio song(AudioResource::Create("../../media/cgc_idea2.m4a", (TINYOAL_FLAG)TINYOAL_COPYINTOMEMORY,0),TINYOAL_ISPLAYING);

  while(engine.Update())
    SLEEP(1);
}