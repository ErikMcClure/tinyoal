/* Example 03 - Volume, Panning and Pitch
 * -------------------------
 * This example demonstrates how you can adjust the volume, panning and pitch of a sound. It then proceeds to horribly
 * abuse these parameters to play a song (you really shouldn't do this in a real program, ever).
 *
 * Copyright (c)2020 Erik McClure
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
  // While we support the lossless FLAC format, the FLAC codec is terrible, and can't do accurate seeking. Because of this,
  // FLAC streams cannot do seamless looping. However, they can still be decoded into wave files. To do this, we specify
  // TINYOAL_FORCETOWAVE in the resource loading flags. This converts whatever audio we're loading into WAVE format using
  // an internal memory buffer (consequently, TINYOAL_FORCETOWAVE implies TINYOAL_COPYTOMEMORY). Then, we'll get a
  // resource back for the converted wave format instead, which will release its contents when it is no longer referenced.
  Audio tone(AudioResource::Create("../../media/shape.flac", (TINYOAL_FLAG)TINYOAL_FORCETOWAVE, 0, 0), TINYOAL_ISPLAYING);
  Audio echo(tone);  // Audio instances can be copied and moved around. Copied instances retain all aspects of their
  Audio echo2(echo); // parents, including if they were playing, where they were playing, volume/pitch/position/flags/etc.
  echo.SetPosition(2); // You can modify the volume, pitch, and location of any audio instance. This pans to the right.
  echo2.SetPosition(-2); // This pans to the left. These values are in 3D, so they're sensitive to the Z value given.

  double freqs[] = { 
    523.25, // C5	
    554.37, // C#5/Db5	
    587.33, // D5	
    622.25, // D#5/Eb5	
    659.26, // E5	
    698.46, // F5	
    739.99, // F#5/Gb5	
    783.99, // G5	
    830.61, // G#5/Ab5
    880.00, // A5
    932.33, // A#5/Bb5
    987.77, // B5
  };
  for(int i = 0; i < 12; ++i) freqs[i]/=523.25; //Gives us all the ratios between frequencies, which we can then horribly abuse

  // This song was picked because it barely stays within acceptable pitch-bending boundaries. OpenAL has trouble with pitch
  // changes beyond 0.5 (half the frequency) and 2.0 (twice the frequency).
  int song[] = { 0,7,15,0,7,15,0,7,17,0,7,17,0,7,17,19,-4,3,12,-4,3,12,-4,3,15,-4,3,15,-4,3,17,15,-2,5,14,-2,5,14,-2,5,14,-2,5,14,-2,5,15,14,
                 0,7,12,0,7,12,0,7,14,0,7,14,0,7,14,15};
  int songsz=sizeof(song)/sizeof(int);
  for(int i = 0; i < songsz; ++i) song[i]+=4;

  int pos=0;
  auto fn = [&](int i) -> double { 
    double f = freqs[i%12];
    double s = (i/12)+1;
    return f*s*0.5;
  };
  while(engine.Update())
  { // The values used here are 60/160 BPM = 0.375 seconds = 375 ms. 375 ms is 188 ms, so we're working on the half-beats.
    tone.SetVolume(1 - (pos%188)/187.0); // This creates a falloff in volume we use to make a "pluck" sound.
    tone.SetPitch(fn(song[pos/188])); // This sets the pitch for the current "note" we are on.
    echo.SetVolume(0.5 - ((pos+94)%188)/375.0); // Note that 1.0 is full volume, and 0.0 is silent.
    echo.SetPitch(fn(song[bss::bssMod(pos-94-188,songsz*188)/188]));
    echo2.SetVolume(0.25 - (pos%188)/751.0);
    echo2.SetPitch(fn(song[bss::bssMod(pos-(188*3),songsz*188)/188]));
    pos=(pos+1)%(songsz*188);
    SLEEP(1); // This is hilariously inaccurate timing. You'll notice if you do anything else while it's playing.
  }
}