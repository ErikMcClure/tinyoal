/* Example 03 - Volume, Panning and Pitch
 * -------------------------
 * This example demonstrates how you can adjust the volume, panning and pitch of a sound.
 *
 * Copyright ©2013 Erik McClure
 */

#include "cTinyOAL.h"

using namespace TinyOAL;

int main()
{
  cTinyOAL engine;
  cAudio tone(cAudioResource::Create("..\\media\\shape.wav",TINYOAL_COPYINTOMEMORY,0),TINYOAL_ISPLAYING);
  cAudio echo(tone);
  cAudio echo2(echo);
  echo.SetPosition(2);
  echo2.SetPosition(-2);

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
  for(int i = 0; i < 12; ++i) freqs[i]/=523.25; //Gives me all the ratios between frequencies, which we can then horribly abuse

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
  {
    tone.SetVolume(1 - (pos%188)/187.0);
    tone.SetPitch(fn(song[pos/188]));
    echo.SetVolume(0.5 - ((pos+94)%188)/375.0);
    echo.SetPitch(fn(song[bss_util::bssmod(pos-94-188,songsz*188)/188]));
    echo2.SetVolume(0.25 - (pos%188)/751.0);
    echo2.SetPitch(fn(song[bss_util::bssmod(pos-(188*3),songsz*188)/188]));
    pos=(pos+1)%(songsz*188);
    SLEEP(1);
  }
}