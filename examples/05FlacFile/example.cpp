/* Example 05 - FLAC file
 * -------------------------
 * This example demonstrates FLAC file support.
 *
 * Copyright ©2018 Black Sphere Studios
 */

#include "tinyoal/TinyOAL.h"

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
  // TinyOAL supports streaming FLAC directly from a file, but FLAC's codec is really bad. It will loop,
  // just not seamlessly. It's highly recommended you use TINYOAL_FORCETOWAVE unless the FLAC is huge.
  Audio song(AudioResource::Create("../../media/idea835.flac",0),TINYOAL_ISPLAYING);

  while(engine.Update())
    SLEEP(1);
}