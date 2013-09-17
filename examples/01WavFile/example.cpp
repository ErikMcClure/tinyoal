/* Example 01 - Wav Files
 * -------------------------
 * This example demonstrates how to open up a wav file and play a simple sound.
 *
 * Copyright ©2013 Erik McClure
 */

#include "cTinyOAL.h"

#ifdef BSS_PLATFORM_WIN32
#define SLEEP(n) _sleep(n)
#else
#include <unistd.h>
#define SLEEP(n) usleep(n*1000) //translate milliseconds to microseconds
#endif

using namespace TinyOAL;

int main()
{
  //We initialize the engine here with the default number of buffers. You can also specify the error log file.
  cTinyOAL engine(4);
  std::vector<cStr> devices = cStr::Explode(0,engine.GetDevices());

  // Here, we are going to play a "sound" by loading up a resource reference. We tell the resource that
  // any audio created using it should immediately start playing, and should be loaded into memory. Short
  // sounds benefit from being loaded into memory, but longer sounds, such as music, should not (if the
  // LOADINTOMEMORY flag is not specified, it is streamed from the disk).
  cAudioResource* songref = cAudioResource::Create("..\\media\\wave.wav",TINYOAL_COPYINTOMEMORY);
  if(!songref) return 0;
  songref->SetMaxActive(2);

  int count=0;
  // Now we tell the engine to create a new instance of the sound and play it. The engine will delete the
  // sound instance once it has finished playing. This is useful for one-shot sounds that need to overlap
  // each other when being played multiple times, which is *usually* what you want.
  songref->Play();
  songref->Play()->SkipSeconds(1.0);
  songref->Play()->SkipSeconds(2.0);
  songref->Play()->SkipSeconds(3.0);
  songref->Play()->SkipSeconds(4.0);

  // Update returns how many sounds are playing. You should call this every frame, but the absolute minimum
  // is calling it before a single buffer runs out. This is usually somewhere around 200 ms, but it varies.
  while(engine.Update()) 
  {
    SLEEP(100);
    //if(++count > 100000)
    //{
    //  count=0;
    //  song->Play();
    //}
  }
  
  songref->Play();
  
  // We let the stack delete our objects for us, because the engine cleans up after itself. Just be sure to
  // destroy all audio objects BEFORE the engine object itself gets destroyed!
}