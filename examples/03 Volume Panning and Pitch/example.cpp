/* Example 03 - Volume, Panning and Pitch
 * -------------------------
 * This example demonstrates how you can adjust the volume, panning and pitch of a sound.
 *
 * Copyright ©2013 Erik McClure
 */


#include "TinyOAL.h"

using namespace TinyOAL;

int __cdecl main()
{
  cTinyOAL engine;
  cAudioRef songref("..\\media\\mono.ogg", TINYOAL_AUTOPLAY|TINYOAL_LOOP);
  cAudio song(songref); // We can initialize an audio instance from any resource reference.
  
  song.SetPitch(2.0f);
  song.SetPosition(1.0f); //If you do not specify 1.0f on either the y or z axis, the ratios between everything
  // get screwed up, and you will end up with a sound all the way to the right or all the way to the left. Also, remember
  // that unless your sound is mono channel, this will have absolutely no effect whatsoever.
  
  float curvol = 1.0f; //storage
  float amount = 0.001f; //how much we wobble the volume
  while(engine.Update())
  {
    _sleep(1);
    curvol -= amount; //subtract volume
    song.SetVolume(curvol); //apply the volume

    if((curvol < 0.5f && amount > 0) || (curvol >= 1.0f && amount < 0)) //check if we need to swap it
      amount *= -1; //swap the sign
  }
}