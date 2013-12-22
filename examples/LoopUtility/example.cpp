/* Loop Utility
 * -------------------------
 * This example encodes loop information into OGG files
 *
 * Copyright ©2013 Black Sphere Studios
 */

#include "cTinyOAL.h"
#include <iostream>
#include "bss_util\cStr.h"
#include "cOggFunctions.h"
#include "taginclude/fileref.h"
#include "taginclude/vorbisfile.h"
#include "taginclude/xiphcomment.h"
#include <time.h>

using namespace TinyOAL;

#pragma comment(lib, "tag.lib")

int main()
{
  //Initialize the engine with default number of buffers
  cTinyOAL engine;
  std::cout << "TinyOAL Loop Utility" << std::endl << std::endl;

  char file[256]={0};
  FILE* ftest=0;
  while(!ftest)
  {
    std::cout << "OGG file name: ";
    std::cin >> file;
    if(!file[0]) return 0; // quit
    FOPEN(ftest,file,"rb");
    if(!ftest) std::cout << "Invalid file path" << std::endl << std::endl;
  }
  fclose(ftest);

  cAudio song(cAudioResource::Create(file));

  std::cout << "Do you want to play this OGG file? [y/n]: ";
  char c;
  cin >> c;
  if(c == 'y' || c == 'Y')
  {
    song.Play();
    time_t seconds;
    time_t start=time(NULL);
    time_t last=time(NULL);
    while(engine.Update())
    {
      seconds = time(NULL);
      if(seconds!=last)
      {
        std::cout << (seconds-start) << std::endl;
        last=seconds;
      }
    }
    return 0;
  }
  
  double seconds;
  std::cout << "Do you want to specify the time using BPM? [y/n]: ";
  cin >> c;
  if(c == 'y' || c == 'Y')
  {
    int BPM;
    double beats;
    double bar;
    std::cout << "BPM: ";
    std::cin >> BPM;
    std::cout << "Beats per bar (usually 4): ";
    std::cin >> beats;
    std::cout << "Bar to loop on: ";
    std::cin >> bar;

    seconds=((beats*(bar-1.0))/BPM)*60.0;
  }
  else 
  {
    std::cout << "Loop Start Point (seconds): ";
    std::cin >> seconds;
  }

  ogg_int64_t samples = song.GetResource()->ToSample(seconds);

  TagLib::FileRef g(file);
  ((TagLib::Ogg::XiphComment*)g.tag())->addField("LOOPSTART",cStrF("%il",samples).c_str());
  g.save();
  system("PAUSE");
}