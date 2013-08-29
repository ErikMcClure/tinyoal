/* Example 01 - Wav Files
 * -------------------------
 * This example demonstrates how to open up a wav file and play a simple sound.
 *
 * Copyright ©2013 Erik McClure
 */

#include "TinyOAL.h"

using namespace TinyOAL;

int __cdecl main()
{
  //We initialize the engine here with the default number of buffers. You can also specify the error log file.
  cTinyOAL engine(4);

  // Here, we are going to play a "sound" by loading up a resource reference. We tell the resource that
  // any audio created using it should immediately start playing, and should be loaded into memory. Short
  // sounds benefit from being loaded into memory, but longer sounds, such as music, should not (if the
  // LOADINTOMEMORY flag is not specified, it is streamed from the disk).
  cAudioRef songref("..\\media\\wave.wav", TINYOAL_AUTOPLAY|TINYOAL_LOADINTOMEMORY);

  int count=0;
  // Now we tell the engine to create a new instance of the sound and play it. The engine will delete the
  // sound instance once it has finished playing. This is useful for one-shot sounds that need to overlap
  // each other when being played multiple times, which is *usually* what you want.
  engine.PlaySound(songref); 

  // Update returns how many sounds are playing. You should call this every frame, but the absolute minimum
  // is calling it before a single buffer runs out. This is usually somewhere around 200 ms, but it varies.
  while(engine.Update()) 
  {
    _sleep(100);
    //if(++count > 100000)
    //{
    //  count=0;
    //  song->Play();
    //}
  }
  
  // We let the stack delete our objects for us, because the engine cleans up after itself. Just be sure to
  // destroy all audio objects BEFORE the engine object itself gets destroyed!
}