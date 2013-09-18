using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TinyOAL_net;

namespace _05Managed
{
  class Program
  {
    static void Main(string[] args)
    {
      clr_TinyOAL engine = new clr_TinyOAL(4);

      string[] devices = engine.GetDevices();
      clr_AudioResource songref = new clr_AudioResource("../media/wave.wav",clr_Audio.TINYOAL_COPYINTOMEMORY);
      if(!songref.IsValid()) return;
      songref.SetMaxActive(2);
      songref.Play();
      songref.Play().SkipSeconds(1.0);
      songref.Play().SkipSeconds(2.0);
      songref.Play().SkipSeconds(3.0);
      clr_Audio r = songref.Play();
      r.SkipSeconds(4.0);
      clr_Audio r2 = new clr_Audio(r);

      while(engine.Update()!=0) 
        System.Threading.Thread.Sleep(100);
  
      clr_Audio music = new clr_Audio(new clr_AudioResource("..\\media\\du.ogg",0),clr_Audio.TINYOAL_ISPLAYING);
      // Songs can have loop points in the middle of them. These can be set manually, or embedded in the OGG
      // metadata. LoopUtility is a utility program included in this SDK to help you do that.
      //music.SetLoopPointSeconds(4.36); 
      music.SetLoopPoint(136048); // loop points must be precise, so it's recommended you set a sample number
      engine.Update();
      music.Pause();
      music.SetPitch(0.5f);
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
