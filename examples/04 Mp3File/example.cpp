/* Example 04 - MP3 file
 * -------------------------
 * This example demonstrates MP3 file support. You MUST have compiled TinyOAL 
 * with __INCLUDE_MP3 defined, or this won't work!
 *
 * Copyright ©2013 Erik McClure
 */

#include "TinyOAL.h"

using namespace TinyOAL;

int __cdecl main()
{
  //Initialize the engine with default number of buffers
  cTinyOAL* engine = new cTinyOAL();
  cAudio* song = new cAudio("..\\media\\14.mp3", TINYOAL_AUTOPLAY|TINYOAL_LOOP);

  while(engine->Update())
  {
    _sleep(1);
  }

  delete song;
  delete engine;
}