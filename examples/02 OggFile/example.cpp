/* Example 02 - Ogg Files
 * -------------------------
 * This example demonstrates how to open up an OGG file, adjust its flags, and make it loop at a loop point
 *
 * Copyright ©2013 Erik McClure
 */

#include "cTinyOAL.h"
#include <process.h>
#include <time.h>
#include <iostream>

using namespace TinyOAL;
using namespace bss_util;

int __cdecl main()
{
  cTinyOAL engine(4); // Initialize the engine with default number of buffers

  // Here we are going to directly load the music into an audio instance so we have more control over
  // it. This is usually done for music, but is sometimes done for certain sound effects that you want
  // to retrigger instead of stacking on top of each other. We can also directly initialize any audio
  // instance from any audio resource reference, too. We specify the LOOP flag to tell the engine to 
  // simply keep looping the song until we call Stop() on the instance.
  cAudio music(cAudioResource::Create("..\\media\\du.ogg",0,136048),TINYOAL_ISPLAYING);
  // Songs can have loop points in the middle of them. These can be set manually, or embedded in the OGG
  // metadata. LoopUtility is a utility program included in this SDK to help you do that.
  //music.SetLoopPointSeconds(4.36); 
  //music.SetLoopPoint(136048); // loop points must be precise, so it's recommended you set a sample number

  time_t seconds;
  time_t start=time(NULL);
  time_t last=time(NULL);
  while(engine.Update())
  {
    _sleep(1);
    seconds = time(NULL);
    if(seconds!=last)
    {
      std::cout << (seconds-start) << std::endl;
      last=seconds;
    }
  }

  return 0; //This code never really does exit because the sound loops indefinitely
}