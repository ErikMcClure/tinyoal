/* Example 04 - MP3 file
 * -------------------------
 * This example demonstrates MP3 file support. MP3 is protected by a bunch of patent trolls, so 
 * only use this if you've paid the necessary license fees, or you're a non-profit.
 *
 * Copyright (c)2020 Erik McClure
 */

#include "tinyoal/TinyOAL.h"

using namespace tinyoal;

#ifdef BUN_PLATFORM_WIN32
#define SLEEP(n) _sleep(n)
#else
#include <unistd.h>
#define SLEEP(n) usleep(n*1000) //translate milliseconds to microseconds
#endif

int main()
{
  TinyOAL::SetSettingsStream(0); // Done in case testbed failed and left a settings file in.
  TinyOAL engine(ENGINE_OPENAL);
 
  {
    // TinyOAL attempts to use file headers to detect the filetype, and does not rely on extensions. However, this
    // doesn't always work, espiecally for MP3 files, which do not have well-defined file headers. The MP3 being
    // loaded here has a corrupt first header, and won't be recognized by TinyOAL as an MP3 file. To get around this,
    // we pass in TINYOAL_FILETYPE_MP3 to force TinyOAL to attempt loading the file as an MP3. mpg123 can then skip the
    // initial corrupt header and play the rest of the MP3.
    Audio song(AudioResource::Create("../media/idea894.mp3", 0, AudioResource::TINYOAL_FILETYPE_MP3), TINYOAL_ISPLAYING);

    while(engine.Update())
      ;
  }

  // It is possible to get a nearly seamless looping MP3 file, but it requires some extra effort. This file was 
  // originally a lossless wav which was then looped. The loop point in that lossless version was recorded at sample
  // #1524096. Then, this version was saved out to an MP3. mpTrim.exe was used to chop off the very last frame from
  // the end of the mp3. This method is hit and miss, so use OGG if you want true seamless looping. Unlike last time,
  // this time we're passing the loop sample # into the resource itself, which will then be used by any audio 
  // instance derived from that source. However, if the file in question has embedded metadata that contains a loop
  // point, that value will override whatever you pass into the constructor here. If this is a problem, you can
  // always set the loop point in the audio resource itself, or set it on each individual instance.
  //Audio loop(AudioResource::Create("../media/idea813.mp3", 0, AudioResource::TINYOAL_FILETYPE_MP3, 1524096),
  //           TINYOAL_ISPLAYING);
  
  //while(engine.Update())
  //  ;
}