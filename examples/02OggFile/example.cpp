/* Example 02 - Ogg Files
 * -------------------------
 * This example demonstrates how to open up an OGG file, adjust its flags, and make it loop at a loop point.
 *
 * Copyright ©2016 Black Sphere Studios
 */

#include "cTinyOAL.h"
#include <iostream>
#include <time.h>

#ifdef BSS_PLATFORM_WIN32
#define SLEEP(n) _sleep(n)
#else
#include <unistd.h>
#define SLEEP(n) usleep(n*1000) //translate milliseconds to microseconds
#endif

using namespace tinyoal;
using namespace bss_util;

int main()
{
  cTinyOAL::SetSettingsStream(0); // Done in case testbed failed and left a settings file in.
  cTinyOAL engine(4); // Initialize the engine with default number of buffers

  // Here we load a resource, and then immediately load it into an audio instance. Because this audio instance 
  // isn't managed, it won't be destroyed until we destroy it (in this case, when it goes out of scope). This
  // time, we're loading an OGG file - TinyOAL automatically detects the filetype for both files and datastreams.
  // We specify the TINYOAL_ISPLAYING flag when we create the cAudio instance, which causes it to immediately
  // start playing. We also specify TINYOAL_COPYINTOMEMORY when we load the OGG file, which causes the entire
  // OGG file to be copied into RAM and the file released. You can only have a single instance of a file-based
  // resource playing at any time, so if you need multiple instances, copy it into memory.
  cAudio music(cAudioResource::Create("../media/idea803.ogg",(TINYOAL_FLAG)TINYOAL_COPYINTOMEMORY),TINYOAL_ISPLAYING);

  // Songs can have loop points in the middle of them. These can be set per-resource, per-instance, or
  // embedded in the OGG metadata. LoopUtility is a utility program included in this SDK to help you do that.
  music.SetLoopPoint(587974); // loop points must be precise, so it's recommended you set a sample number
  engine.Update();
  music.Pause(); // Audio instances support all the operations one would expect to find, Play, Pause, and Stop
  music.Play(); // Pausing stops playback - when Play is called again, playback will resume where it left off.
  music.SkipSeconds(2.0); // You can also skip to any point either while playing or while stopped or paused.
  music.Stop(); // Stop stops playback and resets the stream back to the beginning, no matter what.
  music.Pause(); // Pausing while stopped or playing while already playing have no effect
  music.Play(); // Stop is essentially equivalent to Pause() followed by Skip(0)

  time_t seconds;
  time_t start=time(NULL);
  time_t last=time(NULL);
  while(engine.Update())
  {
    SLEEP(1);
    seconds = time(NULL);
    if(seconds!=last)
    {
      std::cout << (seconds-start) << std::endl;
      last=seconds;
    }
  }

  return 0; //This code never really does exit because the sound loops indefinitely
}