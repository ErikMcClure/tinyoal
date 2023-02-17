/* Example 01 - Wav Files
 * -------------------------
 * This example demonstrates how to open up a wav file and play a simple sound.
 *
 * Copyright (c)2020 Erik McClure
 */

#include "tinyoal/TinyOAL.h"

#ifdef BUN_PLATFORM_WIN32
#define SLEEP(n) _sleep(n)
#else
#include <unistd.h>
#define SLEEP(n) usleep(n*1000) //translate milliseconds to microseconds
#endif

using namespace tinyoal;

int main()
{
  TinyOAL::SetSettingsStream(0); // Done in case testbed failed and left a settings file in.

  // We initialize the engine here with the default number of buffers. You can also specify the error log file.
  TinyOAL engine(ENGINE_OPENAL, nullptr, 4);
  // The engine will use the default device at first, but you can get a null-seperated list of devices with 
  // GetDevices(), and change the device to one of your choosing with SetDevice().
  std::vector<bun::Str> devices = bun::Str::Explode(0,engine.GetDevices());

  // Here, we load a sound resource. TinyOAL supports a wide range of formats, and this particular wave file is
  // stored using the u-Law format to save space. The high frequency artifacts you might hear are a result of
  // the u-Law algorithm. We didn't specify any flags, so by default this will be streamed from disk.
  AudioResource* songref = AudioResource::Create("../media/idea549.wav",0);
  if(!songref) return 0; // If the file failed to exist or wasn't a recognizable format, Create will return NULL

  // Now we tell the resource to create a new instance of this sound, and start playing it. The sound instance
  // will be deleted for you once it stops playing, so only use the returned handle to make initial adjustments.
  // This is useful for "fire and forget" sounds that you don't want to bother keeping track of.
  songref->Play();

  // Update returns how many sounds are playing. You should call this every frame, but the absolute minimum
  // is calling it before a single buffer runs out. This is usually somewhere around 200 ms, but it depends on
  // the exact configuration openAL is using.
  while(engine.Update()) 
    SLEEP(100);
  
  songref->Play(); // We tell the resource to create another Audio instance right before we exit. This won't
                   // cause any problems, because the engine cleans up after itself. You don't need to worry
                   // about any dangling references, because all sounds will simply stop playing.
  
  return 0;
}
