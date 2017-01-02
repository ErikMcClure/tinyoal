/* Example 06 - Managed Code
 * -------------------------
 * This example demonstrates TinyOAL's .net wrapper, which exposes the entire engine to the .net runtime.
 *
 * Copyright ©2017 Black Sphere Studios
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TinyOAL_net;

namespace _06Managed
{
  class Program
  {
    static void Main(string[] args)
    {
      clr_TinyOAL engine = new clr_TinyOAL(4);

      string[] devices = engine.GetDevices();
      clr_AudioResource songref = new clr_AudioResource("../../media/idea549.wav", clr_Audio.TINYOAL_COPYINTOMEMORY);
      songref.MaxActive=1;
      songref.Play();
      songref.Play().Time = songref.ToSamples(2.0);
      clr_Audio r = songref.Play();
      //r.SkipSeconds(4.0);
      r.Time = 0;

      while(engine.Update()!=0) 
        System.Threading.Thread.Sleep(100);
  
      clr_Audio music = new clr_Audio(new clr_AudioResource("../../media/idea803.ogg",0),clr_Audio.TINYOAL_ISPLAYING);
      music.LoopPoint = 587974;
      engine.Update();
      music.Pause();
      engine.Update();
      music.Play();
      music.Stop();
      music.Pause();
      music.Play();

      while(engine.Update()!=0)
        System.Threading.Thread.Sleep(1);
    }
  }
}
